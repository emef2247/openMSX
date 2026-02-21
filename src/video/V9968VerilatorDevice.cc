#include "V9968VerilatorDevice.hh"

#include "VDP.hh"
#include "Display.hh"
#include "MSXMotherBoard.hh"
#include "OutputSurface.hh"
#include "PostProcessor.hh"
#include "RawFrame.hh"
#include "VideoLayer.hh"

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
	verlDebug("[VERL] V9968VerilatorDevice: constructing (FRAME_WIDTH=%u FRAME_HEIGHT=%u)",
	          FRAME_WIDTH, FRAME_HEIGHT);

	workFrame_ = std::make_unique<RawFrame>(FRAME_WIDTH, FRAME_HEIGHT);
	if (!workFrame_) {
		verlDebug("[VERL] V9968VerilatorDevice: FATAL: workFrame_ allocation failed");
		return;
	}
	workFrame_->init(RawFrame::FieldType::NONINTERLACED);

	verlDebug("[VERL] V9968VerilatorDevice: constructed OK, workFrame_ height=%u",
	          workFrame_->getHeight());
}

V9968VerilatorDevice::~V9968VerilatorDevice()
{
	verlDebug("[VERL] V9968VerilatorDevice: destroyed");
}

// ---------------------------------------------------------------------------
// PostProcessor lifecycle
// ---------------------------------------------------------------------------

void V9968VerilatorDevice::initPostProcessor()
{
	Display& display = vdp_.getDisplay();
	OutputSurface* screen = display.getOutputSurface();
	if (!screen) {
		verlDebug("[VERL] initPostProcessor: no OutputSurface (DummyRenderer?), skipping");
		postProcessor_.reset();
		return;
	}
	MSXMotherBoard& mb = vdp_.getMotherBoard();
	postProcessor_ = std::make_unique<PostProcessor>(
		mb, display, *screen,
		"V9968Verilator", FRAME_WIDTH, FRAME_HEIGHT, false);
	verlDebug("[VERL] initPostProcessor: PostProcessor created for 'V9968Verilator'");
}

void V9968VerilatorDevice::resetPostProcessor()
{
	verlDebug("[VERL] resetPostProcessor: releasing PostProcessor");
	postProcessor_.reset();
}

// ---------------------------------------------------------------------------
// IO hooks
// ---------------------------------------------------------------------------

void V9968VerilatorDevice::onWriteIO(uint16_t port, uint8_t value, EmuTime /*time*/)
{
	verlDebug("[VERL] writeIO port=0x%02x value=0x%02x frame=%u",
	          (unsigned)port, (unsigned)value, frameCount_);
}

void V9968VerilatorDevice::onReadIO(uint16_t port, EmuTime /*time*/)
{
	verlDebug("[VERL] readIO  port=0x%02x frame=%u",
	          (unsigned)port, frameCount_);
}

// ---------------------------------------------------------------------------
// VSYNC hook – render stub frame and hand it to dedicated PostProcessor
// ---------------------------------------------------------------------------

void V9968VerilatorDevice::onVSync(EmuTime time)
{
	verlDebug("[VERL] onVSync frame=%u", frameCount_);

	PostProcessor* pp = postProcessor_.get();
	if (!pp) {
		verlDebug("[VERL] onVSync: PostProcessor is null, skipping");
		return;
	}

	if (!workFrame_) {
		verlDebug("[VERL] onVSync: workFrame_ is null, reallocating");
		workFrame_ = std::make_unique<RawFrame>(FRAME_WIDTH, FRAME_HEIGHT);
		workFrame_->init(RawFrame::FieldType::NONINTERLACED);
	}

	// Validate frame dimensions before rendering
	if (workFrame_->getHeight() != FRAME_HEIGHT) {
		verlDebug("[VERL] onVSync: frame height mismatch (got %u, expected %u), reallocating",
		          workFrame_->getHeight(), FRAME_HEIGHT);
		workFrame_ = std::make_unique<RawFrame>(FRAME_WIDTH, FRAME_HEIGHT);
		workFrame_->init(RawFrame::FieldType::NONINTERLACED);
	}

	renderStubFrame(*workFrame_);

	verlDebug("[VERL] onVSync: calling rotateFrames frame=%u", frameCount_);
	workFrame_ = pp->rotateFrames(std::move(workFrame_), time);

	// rotateFrames may return a frame of a different size; re-validate
	if (!workFrame_) {
		verlDebug("[VERL] onVSync: rotateFrames returned null, reallocating");
		workFrame_ = std::make_unique<RawFrame>(FRAME_WIDTH, FRAME_HEIGHT);
	}
	if (workFrame_->getHeight() != FRAME_HEIGHT) {
		verlDebug("[VERL] onVSync: post-rotate height mismatch (%u), reallocating",
		          workFrame_->getHeight());
		workFrame_ = std::make_unique<RawFrame>(FRAME_WIDTH, FRAME_HEIGHT);
	}
	workFrame_->init(RawFrame::FieldType::NONINTERLACED);

	++frameCount_;
}

// ---------------------------------------------------------------------------
// Primary source query
// ---------------------------------------------------------------------------

bool V9968VerilatorDevice::isPrimary() const
{
	if (!postProcessor_) return false;
	return postProcessor_->getVideoSource() ==
	       postProcessor_->getVideoSourceSetting();
}

// ---------------------------------------------------------------------------
// Stub frame renderer: simple RGB gradient
// ---------------------------------------------------------------------------

void V9968VerilatorDevice::renderStubFrame(RawFrame& frame)
{
	const unsigned height = frame.getHeight();
	const unsigned width  = FRAME_WIDTH;

	verlDebug("[VERL] renderStubFrame: width=%u height=%u frame=%u", width, height, frameCount_);

	if (height == 0) {
		verlDebug("[VERL] renderStubFrame: height==0, skipping");
		return;
	}

	for (unsigned y = 0; y < height; ++y) {
		auto line = frame.getLineDirect(y);
		if (line.size() < width) {
			verlDebug("[VERL] renderStubFrame: line %u too short (%zu < %u), skipping",
			          y, line.size(), width);
			continue;
		}
		for (unsigned x = 0; x < width; ++x) {
			uint8_t r = static_cast<uint8_t>((x * 255) / (width  - 1));
			uint8_t g = static_cast<uint8_t>((y * 255) / (height - 1));
			uint8_t b = static_cast<uint8_t>(frameCount_ & 0xFF);
			line[x] = (0xFFu << 24) | (uint32_t(r) << 16) |
			           (uint32_t(g) << 8) | uint32_t(b);
		}
		frame.setLineWidth(y, width);
	}
}

} // namespace openmsx
