#include "V9968VerilatorDevice.hh"

#include "VDP.hh"
#include "PostProcessor.hh"
#include "RawFrame.hh"

#include <cstdarg>
#include <cstdio>
#include <unistd.h>

namespace openmsx {

// ---------------------------------------------------------------------------
// Debug helper (reuses the same stderr approach as VDP.cc)
// ---------------------------------------------------------------------------
#ifdef ENABLE_VDP_EVENT_DEBUG
static void verlDebug(const char* fmt, ...)
{
	char buf[512];
	va_list ap;
	va_start(ap, fmt);
	int n = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (n > 0) {
		if (n > (int)sizeof(buf)) n = (int)sizeof(buf);
		write(STDERR_FILENO, buf, (size_t)n);
		write(STDERR_FILENO, "\n", 1);
	}
}
#else
static inline void verlDebug(const char*, ...) {}
#endif

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

V9968VerilatorDevice::V9968VerilatorDevice(VDP& vdp)
	: vdp_(vdp)
{
	verlDebug("[VERL] V9968VerilatorDevice: constructed");

	// Allocate a working frame (256 wide x 212 high, matching SCREEN5).
	workFrame_ = std::make_unique<RawFrame>(256, 212);
	workFrame_->init(RawFrame::FieldType::NONINTERLACED);
}

V9968VerilatorDevice::~V9968VerilatorDevice()
{
	verlDebug("[VERL] V9968VerilatorDevice: destroyed");
}

// ---------------------------------------------------------------------------
// IO hooks
// ---------------------------------------------------------------------------

void V9968VerilatorDevice::onWriteIO(uint16_t port, uint8_t value, EmuTime /*time*/)
{
	verlDebug("[VERL] writeIO port=0x%02x value=0x%02x frame=%u",
	          (unsigned)port, (unsigned)value, frameCount_);
}

uint8_t V9968VerilatorDevice::onReadIO(uint16_t port, EmuTime /*time*/)
{
	verlDebug("[VERL] readIO  port=0x%02x frame=%u",
	          (unsigned)port, frameCount_);
	return 0xFF;
}

// ---------------------------------------------------------------------------
// VSYNC hook – render stub frame and hand it to PostProcessor
// ---------------------------------------------------------------------------

void V9968VerilatorDevice::onVSync(EmuTime time)
{
	verlDebug("[VERL] onVSync frame=%u", frameCount_);

	if (!workFrame_) return;

	PostProcessor* pp = vdp_.getPostProcessor();
	if (!pp) return;

	// Fill the working frame with a gradient test pattern.
	renderStubFrame(*workFrame_);

	// Hand the completed frame to the PostProcessor (same pattern as
	// V9990SDLRasterizer::frameEnd).
	workFrame_ = pp->rotateFrames(std::move(workFrame_), time);
	workFrame_->init(RawFrame::FieldType::NONINTERLACED);

	++frameCount_;
}

// ---------------------------------------------------------------------------
// Stub frame renderer: simple RGB gradient
// ---------------------------------------------------------------------------

void V9968VerilatorDevice::renderStubFrame(RawFrame& frame)
{
	const unsigned height = frame.getHeight();
	const unsigned width  = 256;

	for (unsigned y = 0; y < height; ++y) {
		auto line = frame.getLineDirect(y);
		for (unsigned x = 0; x < width; ++x) {
			uint8_t r = static_cast<uint8_t>((x * 255) / (width  - 1));
			uint8_t g = static_cast<uint8_t>((y * 255) / (height - 1));
			uint8_t b = static_cast<uint8_t>(frameCount_ & 0xFF);
			// ARGB format (alpha=0xFF)
			line[x] = (0xFFu << 24) | (uint32_t(r) << 16) |
			           (uint32_t(g) << 8) | uint32_t(b);
		}
		frame.setLineWidth(y, width);
	}
}

} // namespace openmsx
