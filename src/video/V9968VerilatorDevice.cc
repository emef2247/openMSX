#include "V9968VerilatorDevice.hh"

#include "Display.hh"
#include "EmuDuration.hh"
#include "MSXMotherBoard.hh"
#include "PostProcessor.hh"
#include "RawFrame.hh"
#include "Reactor.hh"
#include "serialize.hh"

namespace openmsx {

V9968VerilatorDevice::V9968VerilatorDevice(const DeviceConfig& config)
	: MSXDevice(config)
	, syncVSync(getMotherBoard().getScheduler())
	, display_(getReactor().getDisplay())
	, workFrame_(std::make_unique<RawFrame>(FRAME_WIDTH, FRAME_HEIGHT))
{
	workFrame_->init(FrameSource::FieldType::NONINTERLACED);
	display_.attach(*this);
	syncVSync.setSyncPoint(getCurrentTime() + EmuDuration::hz(50));
}

V9968VerilatorDevice::~V9968VerilatorDevice()
{
	display_.detach(*this);
}

void V9968VerilatorDevice::reset(EmuTime /*time*/)
{
	frameCount_ = 0;
}

uint8_t V9968VerilatorDevice::readIO(uint16_t /*port*/, EmuTime /*time*/)
{
	return 0xFF;
}

void V9968VerilatorDevice::writeIO(uint16_t /*port*/, uint8_t /*value*/, EmuTime /*time*/)
{
	// no-op
}

void V9968VerilatorDevice::preVideoSystemChange() noexcept
{
	postProcessor_.reset();
}

void V9968VerilatorDevice::postVideoSystemChange() noexcept
{
	OutputSurface* screen = display_.getOutputSurface();
	if (!screen) return;
	postProcessor_ = std::make_unique<PostProcessor>(
		getMotherBoard(), display_, *screen,
		"V9968Verilator", FRAME_WIDTH, FRAME_HEIGHT, false);
}

void V9968VerilatorDevice::renderStubFrame(RawFrame& frame)
{
	for (unsigned y = 0; y < FRAME_HEIGHT; ++y) {
		auto line = frame.getLineDirect(y);
		for (unsigned x = 0; x < FRAME_WIDTH; ++x) {
			uint8_t r = static_cast<uint8_t>((x * 255) / (FRAME_WIDTH  - 1));
			uint8_t g = static_cast<uint8_t>((y * 255) / (FRAME_HEIGHT - 1));
			uint8_t b = static_cast<uint8_t>(frameCount_ & 0xFF);
			line[x] = (uint32_t(0xFF) << 24) | (uint32_t(r) << 16)
			        | (uint32_t(g) << 8) | uint32_t(b);
		}
		frame.setLineWidth(y, FRAME_WIDTH);
	}
}

void V9968VerilatorDevice::execVSync(EmuTime time)
{
	if (postProcessor_ && workFrame_) {
		renderStubFrame(*workFrame_);
		auto oldFrame = postProcessor_->rotateFrames(std::move(workFrame_), time);
		oldFrame->init(FrameSource::FieldType::NONINTERLACED);
		workFrame_ = std::move(oldFrame);
		++frameCount_;
	}
	syncVSync.setSyncPoint(time + EmuDuration::hz(50));
}

template<typename Archive>
void V9968VerilatorDevice::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<MSXDevice>(*this);
}
INSTANTIATE_SERIALIZE_METHODS(V9968VerilatorDevice);
REGISTER_MSXDEVICE(V9968VerilatorDevice, "V9968VerilatorDevice");

} // namespace openmsx
