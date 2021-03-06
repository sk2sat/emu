#ifndef BIOS_H_
#define BIOS_H_

#include "../emulator_base.h"
#include "../device/display.h"

namespace BIOS {

class Base {
public:
	Base(EmulatorBase *e) : emu_base(e) { if(e==nullptr) throw "ぬるぽ"; }
	virtual void Boot() = 0;
	virtual void Function(size_t num){} // for junk BIOS
	virtual void SetDisplay(Device::Display&){}
protected:
	EmulatorBase *emu_base;
};

}

#endif
