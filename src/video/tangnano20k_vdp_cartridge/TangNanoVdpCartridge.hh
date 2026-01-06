#pragma once

#include "OpenMsxVdpBridge.hh"
#include <cstdint>

namespace openmsx {

/**
 * Thin wrapper that represents the TangNano20K VDP cartridge
 * as a video device inside openMSX.
 *
 * This class is intended to be plugged into the MSX machine
 * as an external VDP-like device (similar in spirit to Video9000 / V9990),
 * but backed by a Verilated tangnano20k_vdp_cartridge core.
 *
 * For now this is a stand‑alone helper; later it can be integrated
 * with MSXDevice / VideoSource once the interface is stable.
 */
class TangNanoVdpCartridge
{
public:
    TangNanoVdpCartridge();
    ~TangNanoVdpCartridge();

    // Life‑cycle -------------------------------------------------------------
    void powerOn();
    void powerOff();
    void reset();

    // Z80 I/O interface: write to VDP I/O ports (e.g. 0x88/0x89/...)
    void writeIO(uint16_t port, uint8_t value);

    // Timing: advance internal VDP clock by N master cycles
    void stepCycles(int cycles);

    // Video output: render a complete frame into an internal RGB buffer
    void renderFrame();

    const uint8_t* getFrameBuffer() const { return bridge.getFrameBuffer(); }
    int  getFrameWidth()  const { return bridge.getFrameWidth(); }
    int  getFrameHeight() const { return bridge.getFrameHeight(); }
    int  getFramePitch()  const { return bridge.getFramePitch(); }

    uint64_t getSimTimePs() const { return bridge.getSimTimePs(); }

private:
    OpenMsxVdpBridge bridge;
    bool powered = false;
};

} // namespace openmsx

