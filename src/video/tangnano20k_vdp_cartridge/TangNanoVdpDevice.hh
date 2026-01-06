#pragma once

#include "MSXDevice.hh"
#include "EmuTime.hh"
#include "TangNanoVdpCartridge.hh"

namespace openmsx {

/**
 * TangNanoVdpDevice
 *
 * MSXDevice wrapper around TangNanoVdpCartridge, which in turn talks
 * to the (eventually) Verilated tangnano20k_vdp_cartridge core.
 *
 * For now this only forwards Z80 I/O writes to the cartridge and
 * advances the internal VDP clock in a very rough way. Video output
 * is still handled separately (no Layer/Renderer integration yet).
 */
class TangNanoVdpDevice final : public MSXDevice
{
public:
	explicit TangNanoVdpDevice(const DeviceConfig& config);
	~TangNanoVdpDevice() override;

	// MSXDevice interface ---------------------------------------------------

	void reset(EmuTime time) override;
	void powerUp(EmuTime time) override;
	void powerDown(EmuTime time) override;

	void writeIO(uint16_t port, byte value, EmuTime time) override;
	[[nodiscard]] byte readIO(uint16_t port, EmuTime time) override;

private:
	/** openMSX の EmuTime から、ざっくり何 VDP クロック進めるかを
	  * 計算して TangNanoVdpCartridge::stepCycles() に渡す。
	  *
	  * ここではまだ単純に「time が進んだら 1 サイクルだけ進める」
	  * 程度の雛形に留め、後で VDP.cc を参考にして埋め直す予定。
	  */
	void syncTo(EmuTime time);

private:
	TangNanoVdpCartridge vdp;
	EmuTime lastSyncTime = EmuTime::zero();
};

} // namespace openmsx

