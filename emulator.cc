#include "emulator.h"

Emulator::Emulator(){
	
}

void Emulator::Init(int memory_size, uint32_t ip, uint32_t sp){
	bit_mode = DEFAULT_BIT_MODE;
	memory.Init(memory_size);
	IP = ip;
	SP = sp;
}

int Emulator::GetBitMode(){
	return bit_mode;
}

bool Emulator::IsHalt(){
	return halt_flg;
}

bool Emulator::IsReal(){
	if((CR0 & 0x80000000) == 0) return true;
	return false;
}

uint8_t Emulator::GetCode8(int index){
	return memory[pc.reg64 + index];	// 一応64bitにしておく
}


