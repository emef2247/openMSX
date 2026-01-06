// Temporary dummy implementations of the Verilator VDP API,
// so that openMSX can link without having the Verilated core
// linked in yet.
//
// Once the real tangnano20k_vdp_cartridge-verilator build
// is integrated, this file should be removed.

#include <cstdint>
#include <cstdio>

extern "C" {

void vdp_cartridge_init(void)
{
    std::fprintf(stderr,
                 "[TangNanoVDP] vdp_cartridge_init() dummy called\n");
}

void vdp_cartridge_release(void)
{
    std::fprintf(stderr,
                 "[TangNanoVDP] vdp_cartridge_release() dummy called\n");
}

void vdp_cartridge_reset(void)
{
    std::fprintf(stderr,
                 "[TangNanoVDP] vdp_cartridge_reset() dummy called\n");
}

void vdp_cartridge_step_clk_1cycle(void)
{
    // no-op for now
}

void vdp_cartridge_write_io(uint16_t /*addr*/, uint8_t /*data*/)
{
    // no-op for now
}

void vdp_cartridge_set_button(uint8_t /*v*/) {}
void vdp_cartridge_set_dipsw(uint8_t /*v*/) {}
void vdp_cartridge_set_debug(int /*enable*/) {}
void vdp_cartridge_set_end_align(int /*enable*/) {}
void vdp_cartridge_set_write_on_posedge(int /*enable*/) {}

uint64_t vdp_cartridge_get_sim_time(void)
{
    return 0;
}

struct VdpVideoMode {
    int width;
    int height;
};

void vdp_get_video_mode(VdpVideoMode* out)
{
    if (!out) return;
    out->width  = 256;
    out->height = 212;
}

void vdp_render_frame_rgb(uint8_t* /*dst*/, int /*pitch*/)
{
    // no-op: leaves buffer as-is (black frame)
}

} // extern "C"

