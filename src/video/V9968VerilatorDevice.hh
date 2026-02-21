#pragma once
#include "EmuTime.hh"
#include <cstdint>
#include <memory>

namespace openmsx {

class VDP;
class PostProcessor;
class RawFrame;

/**
 * V9968 Verilator integration device.
 *
 * Phase 1: stub implementation that renders a gradient pattern into
 * openMSX's RawFrame / PostProcessor pipeline.  The real verilated model
 * will replace the stub in a later phase.
 */
class V9968VerilatorDevice {
public:
	explicit V9968VerilatorDevice(VDP& vdp);
	~V9968VerilatorDevice();

	// Called from VDP::writeIO() when version==IODisabled
	void onWriteIO(uint16_t port, uint8_t value, EmuTime time);

	// Called from VDP::readIO()  when version==IODisabled
	uint8_t onReadIO(uint16_t port, EmuTime time);

	// Called from VDP::execVSync() when version==IODisabled
	// Pushes a finished frame into the PostProcessor.
	void onVSync(EmuTime time);

private:
	// Frame dimensions (matching MSX2 SCREEN5)
	static constexpr unsigned FRAME_WIDTH  = 256;
	static constexpr unsigned FRAME_HEIGHT = 212;

	void renderStubFrame(RawFrame& frame);

	VDP& vdp_;
	std::unique_ptr<RawFrame> workFrame_;
	uint32_t frameCount_ = 0;
};

} // namespace openmsx
