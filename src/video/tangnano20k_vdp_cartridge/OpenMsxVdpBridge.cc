#include "OpenMsxVdpBridge.hh"
#include <cstring>
#include <stdexcept>

OpenMsxVdpBridge::OpenMsxVdpBridge()
{
    // nothing yet; explicit powerOn() will do the heavy lifting
}

OpenMsxVdpBridge::~OpenMsxVdpBridge()
{
    powerOff();
}

void OpenMsxVdpBridge::powerOn()
{
    if (initialized) return;

    vdp_cartridge_init();

    // Basic configuration: debug off by default (openMSX側で切り替え可)
    vdp_cartridge_set_debug(0);
    vdp_cartridge_set_write_on_posedge(1);
    vdp_cartridge_set_end_align(0);
    vdp_cartridge_set_button(0);
    vdp_cartridge_set_dipsw(0);

    // Query video mode and allocate frame buffer
    vdp_get_video_mode(&videoMode);
    if (videoMode.width <= 0 || videoMode.height <= 0) {
        // Fallback to SCREEN5 defaults
        videoMode.width  = 256;
        videoMode.height = 212;
    }
    framePitch = videoMode.width * 3;
    frameBuffer.resize(static_cast<size_t>(framePitch) * videoMode.height);

    // Bring the core out of reset in roughly the same way as the standalone
    // verilator main.cpp does.
    for (int i = 0; i < 10; ++i) {
        vdp_cartridge_step_clk_1cycle();
    }
    vdp_cartridge_reset();
    for (int i = 0; i < 10; ++i) {
        vdp_cartridge_step_clk_1cycle();
    }

    initialized = true;
}

void OpenMsxVdpBridge::powerOff()
{
    if (!initialized) return;
    vdp_cartridge_release();
    initialized = false;
}

void OpenMsxVdpBridge::reset()
{
    if (!initialized) return;
    vdp_cartridge_reset();
}

void OpenMsxVdpBridge::writeIO(uint16_t port, uint8_t value)
{
    if (!initialized) return;
    // NOTE:
    // In the tangnano20k design, the VDP is mapped as an MSX slot device and
    // only looks at the low 8 bits of the address on the slot bus, so we can
    // forward the port number as-is.
    vdp_cartridge_write_io(port, value);
}

void OpenMsxVdpBridge::stepCycles(int cycles)
{
    if (!initialized) return;
    for (int i = 0; i < cycles; ++i) {
        vdp_cartridge_step_clk_1cycle();
    }
}

void OpenMsxVdpBridge::renderFrame()
{
    if (!initialized) return;
    if (frameBuffer.empty()) return;

    vdp_render_frame_rgb(frameBuffer.data(), framePitch);
}

