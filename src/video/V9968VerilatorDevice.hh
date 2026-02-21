#pragma once
#include "MSXDevice.hh"
#include "EmuTime.hh"
#include <cstdint>
#include <memory>

namespace openmsx {

class PostProcessor;
class RawFrame;

/**
 * V9968 Verilator integration device.
 *
 * Standalone MSXDevice registered via DeviceFactory and loaded via
 * share/extensions/V9968Verilator.xml.
 *
 * Phase 1: stub implementation that renders a gradient pattern into
 * openMSX's RawFrame / PostProcessor pipeline.  The real verilated model
 * will replace the stub in a later phase.
 *
 * Debug output is gated on ENABLE_VDP_EVENT_DEBUG (same macro as VDP.cc).
 */
class V9968VerilatorDevice final : public MSXDevice {
public:
	explicit V9968VerilatorDevice(const DeviceConfig& config);
	~V9968VerilatorDevice() override;

	void reset(EmuTime time) override;

	[[nodiscard]] uint8_t readIO(uint16_t port, EmuTime time) override;
	void writeIO(uint16_t port, uint8_t value, EmuTime time) override;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	// Frame dimensions (matching MSX2 SCREEN5)
	static constexpr unsigned FRAME_WIDTH  = 256;
	static constexpr unsigned FRAME_HEIGHT = 212;

	void renderStubFrame(RawFrame& frame);

	std::unique_ptr<RawFrame> workFrame_;
	uint32_t frameCount_ = 0;
};

} // namespace openmsx
