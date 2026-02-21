#ifndef V9968VERILATORDEVICE_HH
#define V9968VERILATORDEVICE_HH

#include "MSXDevice.hh"
#include "VideoSystemChangeListener.hh"

#include "EmuTime.hh"
#include "outer.hh"
#include "Schedulable.hh"

#include <cstdint>
#include <memory>

namespace openmsx {

class Display;
class PostProcessor;
class RawFrame;

class V9968VerilatorDevice final : public MSXDevice
                                 , private VideoSystemChangeListener
{
public:
	explicit V9968VerilatorDevice(const DeviceConfig& config);
	~V9968VerilatorDevice() override;

	// MSXDevice
	void reset(EmuTime time) override;
	[[nodiscard]] uint8_t readIO(uint16_t port, EmuTime time) override;
	void writeIO(uint16_t port, uint8_t value, EmuTime time) override;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	// VideoSystemChangeListener
	void preVideoSystemChange() noexcept override;
	void postVideoSystemChange() noexcept override;

	void renderStubFrame(RawFrame& frame);

	// Scheduler stuff
	struct SyncVSync final : Schedulable {
		explicit SyncVSync(Scheduler& scheduler) : Schedulable(scheduler) {}
		using Schedulable::setSyncPoint;
		void executeUntil(EmuTime time) override {
			auto& dev = OUTER(V9968VerilatorDevice, syncVSync);
			dev.execVSync(time);
		}
	} syncVSync;

	void execVSync(EmuTime time);

private:
	Display& display_;
	std::unique_ptr<PostProcessor> postProcessor_;
	std::unique_ptr<RawFrame> workFrame_;
	uint32_t frameCount_ = 0;

	static constexpr unsigned FRAME_WIDTH  = 256;
	static constexpr unsigned FRAME_HEIGHT = 212;
};

} // namespace openmsx

#endif
