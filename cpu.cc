#include <stdexcept>
#define REG this->reg
#include "cpu.h"
#include "util.h"

void CPU::fetch(std::vector<uint8_t> &memory){
	uint8_t opcode = memory[EIP];

	switch(opcode){
		case 0x0f:
			throw std::runtime_error("two byte insn");
		default:
			throw std::runtime_error("unknown opcode: "+hex2str(opcode, 1));
	}
}
