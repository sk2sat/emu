#ifndef EMULATOR_H_
#define EMULATOR_H_

#include "common.h"
#include "register.h"
#include "memory.h"
#include "interrupt.h"
#include "instruction16.h"
#include "instruction32.h"

#define DEFAULT_BIT_MODE	16
#define DEFAULT_MEMORY_SIZE	1 * MB

#define REGISTERS_COUNT	8

//16bitレジスタに簡単にアクセスするためのdefine
#define  AX     reg[0].reg16
#define  CX     reg[1].reg16
#define  DX     reg[2].reg16
#define  BX     reg[3].reg16
#define  SP     reg[4].reg16
#define  BP     reg[5].reg16
#define  SI     reg[6].reg16
#define  DI     reg[7].reg16

//32bitレジスタに簡単にアクセスするためのdefine
#define EAX     reg[0].reg32
#define ECX     reg[1].reg32
#define EDX     reg[2].reg32
#define EBX     reg[3].reg32
#define ESP     reg[4].reg32
#define EBP     reg[5].reg32
#define ESI     reg[6].reg32
#define EDI     reg[7].reg32

//特殊なレジスタに簡単にアクセスするためのdefine
#define FLAGS   flags.reg16
#define EFLAGS  flags.reg32
#define IP      pc.reg16
#define EIP     pc.reg32
#define CR0	cr[0].reg32
#define CR1	cr[1].reg32
#define CR2	cr[2].reg32
#define CR3	cr[3].reg32
#define CR4	cr[4].reg32

//segment registers
#define CS      SR[0].reg16
#define DS      SR[1].reg16
#define SS      SR[2].reg16
#define ES      SR[3].reg16
#define FS      SR[4].reg16
#define GS      SR[5].reg16

class Emulator {
public:
	Emulator();
	void Init(int memory_size, uint32_t ip, uint32_t sp);

	Register pc;	// program counter
	Register cr[5]; // control register

	Register reg[REGISTERS_COUNT];
	Flags flags;
	Memory memory;

	Instruction16 insn16;
	Instruction32 insn32;

	int GetBitMode();
	bool IsHalt();

	void LoadBinary(const char *fname, uint32_t addr, unsigned int size){ memory.LoadBinary(fname,addr,size); }

	uint8_t GetCode8(int);
private:
	bool halt_flg;
	int bit_mode;
};

#endif
