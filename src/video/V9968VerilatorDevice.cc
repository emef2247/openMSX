#include "V9968VerilatorDevice.hh"

#include "DeviceConfig.hh"
#include "RawFrame.hh"
#include "serialize.hh"

#include <cstdarg>
#include <cstdio>
#include <unistd.h>

namespace openmsx {

// ---------------------------------------------------------------------------
// Debug helper (same stderr approach as VDP.cc)
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

V9968VerilatorDevice::V9968VerilatorDevice(const DeviceConfig& config)
	: MSXDevice(config)
{
	verlDebug("[VERL] V9968VerilatorDevice: constructed (MSXDevice)");

	// Allocate a working frame (256 wide x 212 high, matching SCREEN5).
	workFrame_ = std::make_unique<RawFrame>(FRAME_WIDTH, FRAME_HEIGHT);
	workFrame_->init(RawFrame::FieldType::NONINTERLACED);
}

V9968VerilatorDevice::~V9968VerilatorDevice()
{
	verlDebug("[VERL] V9968VerilatorDevice: destroyed");
}

void V9968VerilatorDevice::reset(EmuTime /*time*/)
{
	verlDebug("[VERL] V9968VerilatorDevice: reset");
	frameCount_ = 0;
}

// ---------------------------------------------------------------------------
// IO hooks
// ---------------------------------------------------------------------------

void V9968VerilatorDevice::writeIO(uint16_t port, uint8_t value, EmuTime /*time*/)
{
	verlDebug("[VERL] writeIO port=0x%02x value=0x%02x frame=%u",
	          (unsigned)port, (unsigned)value, frameCount_);
}

uint8_t V9968VerilatorDevice::readIO(uint16_t port, EmuTime /*time*/)
{
	verlDebug("[VERL] readIO  port=0x%02x frame=%u",
	          (unsigned)port, frameCount_);
	return 0xFF;
}

// ---------------------------------------------------------------------------
// Stub frame renderer: simple RGB gradient (reserved for future VSYNC hook)
// ---------------------------------------------------------------------------

void V9968VerilatorDevice::renderStubFrame(RawFrame& frame)
{
	const unsigned height = frame.getHeight();
	const unsigned width  = FRAME_WIDTH;

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

// ---------------------------------------------------------------------------
// Serialization (stub)
// ---------------------------------------------------------------------------

template<typename Archive>
void V9968VerilatorDevice::serialize(Archive& /*ar*/, unsigned /*version*/)
{
}
INSTANTIATE_SERIALIZE_METHODS(V9968VerilatorDevice);

} // namespace openmsx
