#pragma once

#include <cstdint>
#include <vector>

// Forward decl from the existing Verilator wrapper
extern "C" {
    void     vdp_cartridge_init(void);
    void     vdp_cartridge_release(void);
    void     vdp_cartridge_reset(void);
    void     vdp_cartridge_step_clk_1cycle(void);
    void     vdp_cartridge_write_io(uint16_t addr, uint8_t data);
    void     vdp_cartridge_set_button(uint8_t v);
    void     vdp_cartridge_set_dipsw(uint8_t v);
    void     vdp_cartridge_set_debug(int enable);
    void     vdp_cartridge_set_end_align(int enable);
    void     vdp_cartridge_set_write_on_posedge(int enable);
    uint64_t vdp_cartridge_get_sim_time(void);

    struct VdpVideoMode {
        int width;
        int height;
    };
    void     vdp_get_video_mode(VdpVideoMode* out);
    void     vdp_render_frame_rgb(uint8_t* dst, int pitch);
}

class OpenMsxVdpBridge
{
public:
    OpenMsxVdpBridge();
    ~OpenMsxVdpBridge();

    // Initialization / reset -------------------------------------------------
    void powerOn();
    void powerOff();
    void reset();

    // Z80 I/O interface (as seen by openMSX) --------------------------------
    // These correspond to writes to ports 0x88/0x89/... in the MSX slot.
    void writeIO(uint16_t port, uint8_t value);

    // Timing / scheduling ----------------------------------------------------
    // Advance the Verilated VDP by N master clock cycles (VDP clk).
    void stepCycles(int cycles);

    // Video output -----------------------------------------------------------
    // Render a full RGB frame from the VDP into an internal buffer and
    // return a pointer + pitch. In openMSX this would be hooked into the
    // VideoSource / DisplayRenderer pipeline.
    void renderFrame();
    const uint8_t* getFrameBuffer() const { return frameBuffer.data(); }
    int  getFrameWidth()  const { return videoMode.width; }
    int  getFrameHeight() const { return videoMode.height; }
    int  getFramePitch()  const { return framePitch; }

    // Debug / helpers --------------------------------------------------------
    uint64_t getSimTimePs() const { return vdp_cartridge_get_sim_time(); }

private:
    bool initialized = false;

    VdpVideoMode videoMode {256, 212};
    int  framePitch = 0;
    std::vector<uint8_t> frameBuffer;
};
