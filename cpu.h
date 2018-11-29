#ifndef CPU_H_
#define CPU_H_

#include <cstdint>
#include <vector>
#include <memory>
#include "memory.h"
#include "register.h"

struct InsnData {
	struct ModRM {
		bool is_reg;
		Memory::addr_t addr;
		union {
			uint8_t raw;
			struct {
				uint8_t rm  : 3;
				uint8_t reg : 3;
				uint8_t mod : 2;
			};
		};
	};
	struct SIB {
		union {
			uint8_t raw;
			struct {
				uint8_t base  : 3;
				uint8_t index : 3;
				uint8_t scale : 2;
			};
		};
	};

	uint32_t size = 0;

	uint8_t prefix, prefix2;
	uint8_t opcode;

	ModRM modrm;
	SIB sib;

	union {
		uint8_t  disp8;
		uint16_t disp16;
		uint32_t disp32 =0x00;
	};

	union {
		uint8_t  imm8;
		uint16_t imm16;
		uint32_t imm32 = 0x00;
	};
};

struct CPU {
	Register	reg_pc; // program counter
	Register	reg[8];
	Eflags		eflags;
	SRegister	sreg[6]; // segment register

	InsnData idata;

	std::shared_ptr<Memory> memory;

	bool halt_flag = false;

	void fetch_prefix(const int n);
	void fetch_decode();
	void exec();

	void parse_modrm();
	void parse_modrm32();
	const Memory::addr_t calc_sib_addr() const;

	void dump_registers() const;

	// get,set rm8,16,32
	inline uint8_t get_rm8() const {
		const auto &modrm = idata.modrm;
		if(modrm.is_reg){
			if(modrm.rm < 4)
				return reg[modrm.rm].r8;
			else
				return reg[modrm.rm-4].h8;
		}
		return memory->get8(modrm.addr);
	}
	inline uint16_t get_rm16() const {
		const auto &modrm = idata.modrm;
		if(modrm.is_reg) return reg[modrm.rm].r16;
		return memory->get16(modrm.addr);
	}
	inline uint32_t get_rm32() const {
		const auto &modrm = idata.modrm;
		if(modrm.is_reg) return reg[modrm.rm].r32;
		return memory->get32(modrm.addr);
	}

	inline void set_rm8(const uint8_t &val){
		const auto &modrm = idata.modrm;
		if(modrm.is_reg){
			if(modrm.rm < 4) reg[modrm.rm].r8 = val;
			else reg[modrm.rm-4].h8 = val;
			return;
		}
		memory->set8(modrm.addr, val);
	}
	void set_rm16(const uint16_t &val){
		const auto &modrm = idata.modrm;
		if(modrm.is_reg) reg[modrm.rm].r16 = val;
		else memory->set16(modrm.addr, val);
	}
	void set_rm32(const uint32_t &val){
		const auto &modrm = idata.modrm;
		if(modrm.is_reg) reg[modrm.rm].r32 = val;
		else memory->set32(modrm.addr, val);
	}
};

#ifndef REG
	#define REG cpu.reg
#endif
#ifndef SREG
	#define SREG cpu.sreg
#endif

// join macro
#define JOIN(a, b) JOIN_INTERNAL(a, b)
#define JOIN_INTERNAL(a, b) a ## b

// 32bit register access
#define EIP JOIN(REG, _pc.r32)

#define EAX	REG[0].r32
#define ECX REG[1].r32
#define EDX REG[2].r32
#define EBX REG[3].r32
#define ESP REG[4].r32
#define EBP REG[5].r32
#define ESI REG[6].r32
#define EDI REG[7].r32

// 16bit register access
#define IP J(REG, _pc.r16)
#define AX REG[0].r16
#define CX REG[1].r16
#define DX REG[2].r16
#define BX REG[3].r16
#define SP REG[4].r16
#define BP REG[5].r16
#define SI REG[6].r16
#define DI REG[7].r16

// 8bit register access
#define AL REG[0].r8
#define CL REG[1].r8
#define DL REG[2].r8
#define BL REG[3].r8
#define AH REG[0].h8
#define CH REG[1].h8
#define DH REG[2].h8
#define BH REG[3].h8
#define GET_R8(num) (num<4 ? REG[num].r8 : REG[num-4].h8)
#define SET_R8(num,val) if(num<4){REG[num].r8=val;}else{REG[num-4].h8=val;}

// segment register access
#define ES SREG[0]
#define CS SREG[1]
#define SS SREG[2]
#define DS SREG[3]
#define FS SREG[4]
#define GS SREG[5]

// insn data access
#define IDATA	cpu.idata
#define _MODRM	IDATA._modrm
#define MODRM	IDATA.modrm
#define _SIB	IDATA._sib
#define SIB		IDATA.sib
#define DISP8	IDATA.disp8
#define DISP16	IDATA.disp16
#define DISP32	IDATA.disp32
#define IMM8	IDATA.imm8
#define IMM16	IDATA.imm16
#define IMM32	IDATA.imm32

#endif
