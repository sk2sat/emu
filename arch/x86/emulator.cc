#include <iostream>
#include <sstream>
#include "emulator.h"
#include "register.h"
#include "insndata.h"
#include "instruction.h"
#include "instruction16.h"
#include "instruction32.h"
#include "../../device/pic.h"
#include "../../device/keyboard.h"

using namespace std;

namespace x86{

void Emulator::InitInstructions(){
	// 起動時は16bitリアルモード
	idata = new InsnData(this);
	insn16 = new Instruction16(this, idata);
	insn32 = new Instruction32(this, idata);
	insn16->Init();
	insn32->Init();
	insn = insn16;
	mode = 16;
}

// vol3_i 9.1
void Emulator::InitRegisters(){
	pc.SetName("EIP");
	eflags.SetName("EFLAGS");

	reg = std::vector<x86::Register32>(REG_COUNT);
	const char* reg_name[REG_COUNT] = {
		"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI",
	};
	for(size_t i=0;i<REG_COUNT;i++){
		reg[i].SetName(reg_name[i]);
	}

	sreg = std::vector<x86::SRegister>(SREG_COUNT);
	const char* sreg_name[SREG_COUNT] = {
		"ES", "CS", "SS", "DS", "FS", "GS",
	};
	for(size_t i=0;i<SREG_COUNT;i++)
		sreg[i].SetName(sreg_name[i]);

	idata->sreg = &sreg[3];

	GDTR.SetName("GDTR");
	IDTR.SetName("IDTR");
	TR.SetName("  TR");
	LDTR.SetName("LDTR");
	GDTR.SetSize(sizeof(uint64_t)*3/4);
	IDTR.SetSize(sizeof(uint64_t)*3/4);

	LDTR.selector = 0x00;
	GDTR.base  = IDTR.base = TR.base = LDTR.base	= 0x00;
	GDTR.limit = IDTR.limit = TR.limit = LDTR.limit	= 0xffff;

	CR0.SetName("CR0");
	CR0.reg32 = 0x60000010;

// all_regへの登録
	all_reg.push_back(&pc);
	all_reg.push_back(&eflags);
	for(size_t i=0;i<REG_COUNT;i++)
		all_reg.push_back(&reg[i]);
	for(size_t i=0;i<SREG_COUNT;i++)
		all_reg.push_back(&sreg[i]);
	all_reg.push_back(&GDTR);
	all_reg.push_back(&IDTR);
	all_reg.push_back(&TR);
	all_reg.push_back(&LDTR);
	all_reg.push_back(&CR0);
}

void Emulator::InitMemory(){
	memory->endian = ENDIAN::LITTLE;
}

void Emulator::InitIO(){
	io = new IO();
	auto pic = new Device::PIC();
	io->port[0x20] = io->port[0x21] = io->port[0xa0] = io->port[0xa1] = pic;

	auto keyboard = new Device::Keyboard();
	io->port[0x60] = io->port[0x64] = keyboard;
}

bool Emulator::IsMode16(){ return (insn == insn16); }

void Emulator::RunStep(){
	bool is_mode32	= IsMode32();
	bool is_real	= IsReal();

	bool is_32;

	if(is_real){ // リアルモード
		is_32 = is_mode32;
	}else{
		is_32 = is_mode32; // TODO: 本当はコードセグメントディスクリプタ内のDフラグで判断する
	}

	insn->Fetch();

	idata->is_op32		= is_32 ^ idata->chsiz_op;
	idata->is_addr32	= is_32 ^ idata->chsiz_addr;

	// オペランドサイズ変更
	if(idata->chsiz_op){
		insn_cache = insn;
		insn = ((insn == insn32) ? insn16 : insn32);
	}

	insn->Decode();
	insn->Exec();

	if(idata->chsiz_op)
		insn = insn_cache;

	DOUT(std::endl);
}

}
