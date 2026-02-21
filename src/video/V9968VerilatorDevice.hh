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
 * Phase 2: runs alongside V9958 (broadcast mode).  IO writes/reads are
 * forwarded from VDP after V9958 has processed them.  The device owns its
 * own PostProcessor registered under the "V9968Verilator" video source,
 * allowing the user to switch between the standard V9958 output and the
 * Verilator output at runtime.
 */
class V9968VerilatorDevice final : public MSXDevice {
public:
	explicit V9968VerilatorDevice(const DeviceConfig& config);
	~V9968VerilatorDevice() override;

	/** (Re-)create the dedicated PostProcessor.
	 *  Called from VDP::createRenderer() and VDP::postVideoSystemChange().
	 *  Safe to call with a null screen (DummyRenderer case). */
	void initPostProcessor();

	/** Release the PostProcessor before the video system is torn down.
	 *  Called from VDP::preVideoSystemChange(). */
	void resetPostProcessor();

	// Called from VDP::writeIO() – notification only, no early return
	void onWriteIO(uint16_t port, uint8_t value, EmuTime time);

	// Called from VDP::readIO() – notification only, return value unused
	void onReadIO(uint16_t port, EmuTime time);

	// Called from VDP::execVSync()
	// Pushes a finished frame into the dedicated PostProcessor.
	void onVSync(EmuTime time);

	/** Returns true when this device's output is the active video source. */
	[[nodiscard]] bool isPrimary() const;

private:
	// Frame dimensions (matching MSX2 SCREEN5)
	static constexpr unsigned FRAME_WIDTH  = 256;
	static constexpr unsigned FRAME_HEIGHT = 212;

	void renderStubFrame(RawFrame& frame);

	VDP& vdp_;
	std::unique_ptr<PostProcessor> postProcessor_;
	std::unique_ptr<RawFrame> workFrame_;
	uint32_t frameCount_ = 0;
};

} // namespace openmsx
