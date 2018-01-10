#ifndef X86_INSTRUCTION_H_
#define X86_INSTRUCTION_H_

#include <stdio.h>
#include "../../insn_base.h"
#include "emulator.h"
#include "insndata.h"

namespace x86 {

class Instruction : public InstructionBase {
public:
	Instruction(x86::Emulator *e);
	~Instruction(){ delete idata; }
	virtual void Init();
	void Parse();
	void ExecStep();
protected:
	x86::Emulator *emu;
	x86::InsnData *idata;
	struct Flag {
		static const uint8_t None	= 0b0000;
		static const uint8_t ModRM	= 0b0001;
		static const uint8_t Imm8	= 0b0010;
		static const uint8_t Imm16	= 0b0100;
		static const uint8_t Imm32	= 0b1000;
	};
	uint8_t insn_flgs[256];

	void not_impl_insn();

#define DEFINE_JX(flag, is_flag) \
void j ## flag(){ \
	EIP += (emu->eflags.is_flag() ? emu->GetSignCode8(1) : 0) + 1; \
} \
void jn ## flag(){ \
	EIP += (emu->eflags.is_flag() ? 0 : emu->GetSignCode8(1)) + 1; \
}

	DEFINE_JX(o, IsOverflow);
	DEFINE_JX(c, IsCarry);
	DEFINE_JX(z, IsZero);
	DEFINE_JX(s, IsSign);

	void nop(){ DOUT("nop"); }

	void near_jump(){
		int32_t diff = emu->GetCode32(1);
		EIP += (diff + 4);
	}

	void short_jump(){
		uint8_t diff = (*emu->memory)[EIP ];
		EIP += (static_cast<int8_t>(diff) + 1);
	}
};

};

#endif
