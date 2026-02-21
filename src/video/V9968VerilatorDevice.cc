#include "V9968VerilatorDevice.hh"

#include "VDP.hh"
#include "Display.hh"
#include "MSXMotherBoard.hh"
#include "OutputSurface.hh"
#include "PostProcessor.hh"
#include "RawFrame.hh"

namespace openmsx {

V9968VerilatorDevice::V9968VerilatorDevice(VDP& vdp)
	: vdp_(vdp)
{
	workFrame_ = std::make_unique<RawFrame>(FRAME_WIDTH, FRAME_HEIGHT);
	workFrame_->init(RawFrame::FieldType::NONINTERLACED);
}

V9968VerilatorDevice::~V9968VerilatorDevice() = default;

void V9968VerilatorDevice::initPostProcessor()
{
	Display& display = vdp_.getDisplay();
	OutputSurface* screen = display.getOutputSurface();
	if (!screen) {
		postProcessor_.reset();
		return;
	}
	MSXMotherBoard& mb = vdp_.getMotherBoard();
	postProcessor_ = std::make_unique<PostProcessor>(
		mb, display, *screen,
		"V9968Verilator", FRAME_WIDTH, FRAME_HEIGHT, false);
}

void V9968VerilatorDevice::resetPostProcessor()
{
	postProcessor_.reset();
}

void V9968VerilatorDevice::onWriteIO(uint16_t /*port*/, uint8_t /*value*/, EmuTime /*time*/)
{
	// notification only -- no action required in Phase 2 stub
}

void V9968VerilatorDevice::onReadIO(uint16_t /*port*/, EmuTime /*time*/)
{
	// notification only -- no action required in Phase 2 stub
}

void V9968VerilatorDevice::onVSync(EmuTime time)
{
	PostProcessor* pp = postProcessor_.get();
	if (!pp) return;

	renderStubFrame(*workFrame_);
	workFrame_ = pp->rotateFrames(std::move(workFrame_), time);
	if (!workFrame_) {
		workFrame_ = std::make_unique<RawFrame>(FRAME_WIDTH, FRAME_HEIGHT);
	}
	workFrame_->init(RawFrame::FieldType::NONINTERLACED);
	++frameCount_;
}

bool V9968VerilatorDevice::isPrimary() const
{
	if (!postProcessor_) return false;
	return postProcessor_->getVideoSource() ==
	       postProcessor_->getVideoSourceSetting();
}

void V9968VerilatorDevice::renderStubFrame(RawFrame& frame)
{
	const unsigned height = frame.getHeight();
	const unsigned width  = FRAME_WIDTH;

	for (unsigned y = 0; y < height; ++y) {
		auto line = frame.getLineDirect(y);
		for (unsigned x = 0; x < width; ++x) {
			auto r = static_cast<uint8_t>((x * 255) / (width  - 1));
			auto g = static_cast<uint8_t>((y * 255) / (height - 1));
			auto b = static_cast<uint8_t>(frameCount_ & 0xFF);
			line[x] = 0xFF000000u | (uint32_t(r) << 16) |
			          (uint32_t(g) << 8) | uint32_t(b);
		}
		frame.setLineWidth(y, width);
	}
}

} // namespace openmsx
