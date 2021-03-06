#include <iomanip>
#include "emulator_base.h"

EmulatorBase::EmulatorBase(){
	memory = new Memory();
}

EmulatorBase::~EmulatorBase(){
	delete insn;
	delete memory;
	delete io;
	delete bios;
}

void EmulatorBase::Init(){
	finish_flg = false;
	InitInstructions();
	InitRegisters();
	InitMemory();
	InitIO();
}

void EmulatorBase::SetBios(BIOS::Base *bios){
	if(bios == nullptr) throw "BIOS";
	this->bios = bios;
}

void EmulatorBase::ConnectDevice(Device::Base &dev){
	this->dev.push_back(&dev);
}

void EmulatorBase::Dump(){
	DumpRegisters();
	DumpMemory();
}

void EmulatorBase::DumpRegisters(){
	std::cout
		<< std::endl
		<< "---- dump registers ----"
		<< std::endl;
	for(size_t i=0;i<all_reg.size();i++){
		if(all_reg[i] == nullptr)
			throw "error";
		std::cout
			<< all_reg[i]->GetName()
			<< "\t: "
//			<< "0x"
		//	<< std::showbase	// <- if 0, dosen't work
//			<< std::hex
//			<< std::setw(4)
//			<< std::setfill('0')
//			<< all_reg[i].reg32
			<< all_reg[i]->GetDataByString()
			<< std::endl;
	}
	std::cout
		<< "------------------------"
		<< std::endl;
}

void EmulatorBase::DumpMemory(){
	memory->Dump("memdump.bin", 0, memory->size);
}

