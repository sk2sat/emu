#ifndef X86_INSTRUCTION_H_
#define X86_INSTRUCTION_H_

#include <stdio.h>
#include "../../insn_base.h"
#include "emulator.h"
#include "insndata.h"

namespace x86 {

class Instruction : public InstructionBase {
public:
	Instruction(x86::Emulator *e, x86::InsnData *i) : emu(e), idata(i) { insn_name.resize(256); }
	~Instruction(){}
	virtual void Init();
	void Fetch();
	void Decode();
	void Exec();
protected:
	x86::Emulator *emu;
	x86::InsnData *idata;
	struct Flag {
		static const uint8_t None	= 0b000000;
		static const uint8_t ModRM	= 0b000001;
		static const uint8_t Imm8	= 0b000010;
		static const uint8_t Imm16	= 0b000100;
		static const uint8_t Imm32	= 0b001000;
		static const uint8_t Moffs	= 0b010000;
		static const uint8_t Ptr16	= 0b100000;
	};
	uint8_t insn_flgs[256];

	void not_impl_insn();

	void code_0f(){
		DOUT(std::endl<<"subop=0x"<<std::hex<<static_cast<uint32_t>(idata->subopcode));
		switch(idata->subopcode){
		case 0x01: code_0f01(); break;
		case 0x20: mov_r32_crn(); break;
		case 0x22: mov_crn_r32(); break;

		// Jcc
		#define SET_JCC(subop, opname) \
		case subop: (idata->is_op32 ? opname ## _rel32() : opname ## _rel16()); break;

		SET_JCC(0x80, jo);
		SET_JCC(0x81, jno);
		SET_JCC(0x82, jb);	// = jc,jnae
		SET_JCC(0x83, jae);	// = jnb,jnc
		SET_JCC(0x84, je);	// = jz
		SET_JCC(0x85, jne);	// = jnz
		SET_JCC(0x86, jbe);	// = jna
		SET_JCC(0x87, ja);	// = jnbe
		SET_JCC(0x88, js);
		SET_JCC(0x89, jns);
		SET_JCC(0x8a, jp);	// = jpe
		SET_JCC(0x8b, jpo);
		SET_JCC(0x8c, jl);	// = jnge
		SET_JCC(0x8d, jge);	// = jnl
		SET_JCC(0x8e, jle);	// = jng
		SET_JCC(0x8f, jg);	// = jnle

		case 0xaf: (idata->is_op32 ? imul_r32_rm32() : imul_r16_rm16()); break;
		case 0xb6: (idata->is_op32 ? movzx_r32_rm8() : movzx_r16_rm8()); break;
		case 0xbe: (idata->is_op32 ? movsx_r32_rm8() : movsx_r16_rm8()); break;
		case 0xbf: movsx_r32_rm16(); break;
		default:
			throw "not implemented: 0x0f subop";
		}
	}
		void code_0f01(){
			DOUT(" /"<<static_cast<uint32_t>(idata->modrm.reg));
			switch(idata->modrm.reg){
			case 2: lgdt(); break;
			default:
				std::stringstream ss;
				ss << "not implemented: 0x0f01 /"
					<< static_cast<uint32_t>(idata->modrm.reg);
				throw ss.str();
				break;
			}
		}
			void lgdt(){
				uint32_t addr = idata->CalcMemAddr();
				// GDTR.limit = SRC[0:15];
				uint16_t limit;
				uint32_t base;
				if(!idata->is_op32){
					uint16_t addr16 = static_cast<uint16_t>(addr);
					addr = addr16;
					limit = emu->memory->GetData16(addr16);
					// GDTR.base  = SRC[16:47] & 0xffffff;
					base  = emu->memory->GetData32(addr16+2) & 0xffffff;
				}else{
					limit = emu->memory->GetData16(addr);
					// GDTR.base  = SRC[16:47];
					base  = emu->memory->GetData32(addr+2);
				}
				emu->GDTR.limit = limit;
				emu->GDTR.base  = base;

				DOUT(std::endl<<"LGDT: GDTR <- [0x"<<std::hex<<addr<<"](limit=0x"<<std::hex<<limit<<", base=0x"<<std::hex<<base<<")");
			}
		void mov_r32_crn(){
			auto& r32 = emu->reg[idata->RM];
			auto n   = idata->modrm.reg;
			uint32_t crn = GET_CRN(n);
			r32.reg32 = crn;
			DOUT(std::endl<<__func__<<": "<<r32.GetName()<<" <- CR"<<static_cast<uint32_t>(n)<<"(0x"<<std::hex<<crn<<")");
		}
		void mov_crn_r32(){
			auto& r32 = emu->reg[idata->RM];
			auto n = idata->modrm.reg;
			SET_CRN(n, r32.reg32);
			DOUT(std::endl<<__func__<<": CR"<<static_cast<uint32_t>(n)<<" <- "<<r32.GetName()<<"(0x"<<std::hex<<r32.reg32<<")");
			if(emu->CR0.PE)
				DOUT(std::endl<<"Protect Enable");
			if(emu->CR0.PG)
				throw "not implemented: paging";
		}

		// Jcc: Jump if Condition Is Met
		// j*_rel32
		// j*_rel16

		void imul_r32_rm32(){
			auto& reg = emu->reg[idata->modrm.reg];
			int32_t s_r32 = reg.reg32;
			int32_t s_rm32 = idata->GetRM32();
			int64_t temp = s_r32 * s_rm32;
			reg.reg32 = temp;
			DOUT(__func__ << ": "
				<< reg.GetName() << " <- "
				<< reg.GetName() << "(0x" << std::hex << s_r32 << ")"
				<< " * 0x" << s_rm32
				<< " = 0x" << temp
				<< std::endl);
			if(temp != reg.reg32){
				if(EFLAGS.CF) EFLAGS.OF = 1;
				else EFLAGS.OF = 0;
			}
			// throw __func__;
		}
		void imul_r16_rm16(){
			throw __func__;
		}
		void movzx_r32_rm8(){
			auto& reg = emu->reg[idata->modrm.reg];
			auto rm8 = idata->GetRM8();
			DOUT(std::endl<<__func__<<": "<<reg.GetName()<<" <- ZeroExtended(0x"<<static_cast<uint32_t>(rm8)<<")"<<std::endl);
			reg.reg32 = rm8;
		}
		void movzx_r16_rm8(){
			throw __func__;
		}
		void movsx_r32_rm8(){
			auto& reg = emu->reg[idata->modrm.reg];
			int8_t s_rm8 = idata->GetRM8();
			int32_t set = static_cast<int32_t>(s_rm8);
			DOUT(__func__<<": "<<reg.GetName()<<" <- SignExtended(0x"<<std::hex<<static_cast<int32_t>(s_rm8)<<") = 0x"<<set<<std::endl);
			reg.reg32 = set;
		}
		void movsx_r16_rm8(){
			throw __func__;
		}
		void movsx_r32_rm16(){
			auto& reg = emu->reg[idata->modrm.reg];
			int16_t s_rm16 = idata->GetRM16();
			int32_t set = static_cast<int32_t>(s_rm16);
			DOUT(__func__<<": "<<reg.GetName()<<" <- SignExtended(0x"<<std::hex<<s_rm16<<") = 0x"<<set<<std::endl);
			reg.reg32 = set;
		}

	void and_al_imm8(){
		uint8_t al = AL;
		AL = al & idata->imm8;
		EFLAGS.UpdateAnd(al, idata->imm8, al&idata->imm8);
	}

	void cmp_al_imm8(){
		uint8_t al = AL;
		EFLAGS.Cmp(al, idata->imm8);
	}

// Jump if Condition Is Met
#define DEF_JCC_REL8(flag, is_flag) \
void j ## flag ## _rel8(){ \
	if(is_flag) EIP += idata->imm8; \
}
#define DEF_JCC_REL16(flag, is_flag) \
void j ## flag ## _rel16(){ \
	if(is_flag) EIP += idata->imm16; \
}
#define DEF_JCC_REL32(flag, is_flag) \
void j ## flag ## _rel32(){ \
	if(is_flag) EIP += idata->imm32; \
}

#define DEF_JCC(flag, is_flag) \
	DEF_JCC_REL8(flag, is_flag); \
	DEF_JCC_REL16(flag, is_flag); \
	DEF_JCC_REL32(flag, is_flag);

	DEF_JCC(o,  EFLAGS.OF);
	DEF_JCC(no, !EFLAGS.OF);
	DEF_JCC(b,  EFLAGS.CF);					// jb =jc =jnae
	DEF_JCC(ae, !EFLAGS.CF);					// jae=jnb=jnc
	DEF_JCC(e,  EFLAGS.ZF);					// je =jz
	DEF_JCC(ne, !EFLAGS.ZF);					// jne=jnz
	DEF_JCC(be, EFLAGS.CF || EFLAGS.ZF);			// jbe=jna
	DEF_JCC(a,  !EFLAGS.CF && !EFLAGS.ZF);			// ja =jnbe
	DEF_JCC(s,  EFLAGS.SF);
	DEF_JCC(ns, !EFLAGS.SF);
	DEF_JCC(p,  EFLAGS.PF);					// jp =jpe
	DEF_JCC(po, !EFLAGS.PF);
	DEF_JCC(l,  EFLAGS.SF != EFLAGS.OF);			// jl =jnge
	DEF_JCC(ge, EFLAGS.SF == EFLAGS.OF);			// jge=jnl
	DEF_JCC(le, EFLAGS.ZF || (EFLAGS.SF != EFLAGS.OF));	// jle=jng
	DEF_JCC(g,  !EFLAGS.ZF && (EFLAGS.SF == EFLAGS.OF));	// jg = jnle

	void code_80(){
		DOUT(" REG="<<static_cast<uint32_t>(idata->modrm.reg)<<" ");
		switch(idata->modrm.reg){
		case 0:	add_rm8_imm8();	break;
		case 4: and_rm8_imm8(); break;
		case 7: cmp_rm8_imm8();	break;
		default:
		{
			std::stringstream ss;
			ss << "not implemented: 0x80 /"<<static_cast<uint32_t>(idata->modrm.reg);
			throw ss.str();
		}
		}
	}
		void add_rm8_imm8(){
			auto rm8 = idata->GetRM8();
			uint16_t tmp = rm8 + idata->imm8;
			idata->SetRM8(static_cast<uint8_t>(tmp));
			EFLAGS.UpdateAdd(rm8, idata->imm8, tmp);
		}
		void and_rm8_imm8(){
			auto rm8 = idata->GetRM8();
			uint16_t tmp = rm8 & idata->imm8;
			idata->SetRM8(static_cast<uint8_t>(tmp));
			EFLAGS.UpdateAnd(rm8, idata->imm8, tmp);
		}
		void cmp_rm8_imm8(){
			EFLAGS.Cmp(idata->GetRM8(), idata->imm8);
		}

	void test_rm8_r8(){
		uint8_t rm8 = idata->GetRM8();
		uint8_t r8 = GET_REG8(idata->modrm.reg);
		EFLAGS.UpdateTest(rm8, r8);
	}

	void mov_rm8_r8(){
		uint8_t reg8 = GET_REG8(idata->modrm.reg);
		DOUT(__func__<<": "<<GET_REG8_NAME(idata->modrm.reg)<<"=0x"<<std::hex<<reg8);
		idata->SetRM8(reg8);
	}

	void mov_r8_rm8(){
		auto rm8 = idata->GetRM8();
		SET_REG8(idata->modrm.reg, rm8);
		DOUT(std::endl<<"\t"<<GET_REG8_NAME(idata->modrm.reg)<<"=0x"<<std::hex<<static_cast<uint32_t>(rm8));
	}

	void mov_sreg_rm16(){
		auto rm16 = idata->GetRM16();
		auto& sreg = emu->sreg[idata->modrm.reg];
		sreg = rm16;
		DOUT(std::endl<<"\t"<<sreg.GetName()<<"=0x"<<std::hex<<rm16);
	}

	void nop(){}

	void mov_moffs8_al(){
		emu->memory->SetData8(idata->moffs, AL);
		DOUT("[0x"<<std::hex<<idata->moffs<<"] = AL("<<static_cast<uint32_t>(AL)<<")"<<std::endl);
	}

/* TODO: this is 32bit op
	void near_jump(){
		int32_t diff = emu->GetCode32(1);
		EIP += (diff + 4);
	}
*/
	void mov_r8_imm8(){
		uint8_t reg8 = idata->opcode - 0xb0;
		SET_REG8(reg8, idata->imm8);
	}

	void code_c0(){
		switch(idata->modrm.reg){
		case 5: shr_rm8_imm8(); break;
		default:
			std::stringstream ss;
			ss << "not implemented: 0xc0 /"
				<< std::dec << static_cast<uint32_t>(idata->modrm.reg);
			throw ss.str();
		}
	}
		void shr_rm8_imm8(){
			uint8_t rm8 = idata->GetRM8();
			uint64_t result = rm8 >> idata->imm8;
			idata->SetRM8(result);
			EFLAGS.UpdateShr(rm8, idata->imm8, result);
		}

	void mov_rm8_imm8(){
		idata->SetRM8(idata->imm8);
	}

	void int_imm8(){
		DOUT(std::endl<<"\tint "<<static_cast<uint32_t>(idata->imm8));
		emu->bios->Function(static_cast<int>(idata->imm8));
	}

	void in_al_imm8(){
		uint8_t port = idata->imm8;
		AL = emu->io->in8(port);
	}

	void out_imm8_al(){
		uint8_t port = idata->imm8;
		emu->io->out8(port, AL);
	}

	void short_jump(){
		IP += static_cast<uint16_t>(idata->imm8);
	}

	void out_dx_al(){
		DOUT(__func__<<": port[0x"<<std::hex<<DX
			<< "] <- 0x"<<static_cast<uint32_t>(AL)<<std::endl);
		emu->io->out8(DX, AL);
	}

	void hlt(){
		std::cout<<"hlt"<<std::endl;
		emu->halt_flg = true;
	}

	void cli(){
		EFLAGS.IF = 0;
	}
};

};

#endif
