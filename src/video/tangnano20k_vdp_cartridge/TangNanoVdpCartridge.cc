#include "TangNanoVdpCartridge.hh"

namespace openmsx {

TangNanoVdpCartridge::TangNanoVdpCartridge()
{
    // lazy powerOn: defer heavy init until explicitly requested
}

TangNanoVdpCartridge::~TangNanoVdpCartridge()
{
    powerOff();
}

void TangNanoVdpCartridge::powerOn()
{
    if (powered) return;
    bridge.powerOn();
    powered = true;
}

void TangNanoVdpCartridge::powerOff()
{
    if (!powered) return;
    bridge.powerOff();
    powered = false;
}

void TangNanoVdpCartridge::reset()
{
    if (!powered) return;
    bridge.reset();
}

void TangNanoVdpCartridge::writeIO(uint16_t port, uint8_t value)
{
    if (!powered) return;
    bridge.writeIO(port, value);
}

void TangNanoVdpCartridge::stepCycles(int cycles)
{
    if (!powered) return;
    bridge.stepCycles(cycles);
}

void TangNanoVdpCartridge::renderFrame()
{
    if (!powered) return;
    bridge.renderFrame();
}

} // namespace openmsx

