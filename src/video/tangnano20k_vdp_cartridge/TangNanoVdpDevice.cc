#include "TangNanoVdpDevice.hh"

#include "DeviceConfig.hh"
#include "narrow.hh"

namespace openmsx {

TangNanoVdpDevice::TangNanoVdpDevice(const DeviceConfig& config)
	: MSXDevice(config)
{
	// 実際の初期化は powerUp() / reset() で行う
}

TangNanoVdpDevice::~TangNanoVdpDevice()
{
	vdp.powerOff();
}

void TangNanoVdpDevice::powerUp(EmuTime time)
{
	lastSyncTime = time;
	vdp.powerOn();
	vdp.reset();
}

void TangNanoVdpDevice::powerDown(EmuTime /*time*/)
{
	vdp.powerOff();
}

void TangNanoVdpDevice::reset(EmuTime time)
{
	lastSyncTime = time;
	vdp.reset();
}

void TangNanoVdpDevice::syncTo(EmuTime time)
{
	// とりあえずの雛形:
	// time が進んでいれば 1 サイクルだけ進める。
	// 後で VDP.cc のタイミング実装を参考に、CPU クロックと
	// VDP クロックの比からステップ数を決める予定。

	if (time <= lastSyncTime) return;

	vdp.stepCycles(1);
	lastSyncTime = time;
}

void TangNanoVdpDevice::writeIO(uint16_t port, byte value, EmuTime time)
{
	syncTo(time);

	// TangNano 側はスロットバスの 8bit アドレスを見ているので、
	// ここではポート番号の下位 8bit をそのまま渡す。
	const auto addr8 = narrow_cast<uint16_t>(port & 0xFF);
	vdp.writeIO(addr8, value);
}

byte TangNanoVdpDevice::readIO(uint16_t /*port*/, EmuTime /*time*/)
{
	// 現時点では read をサポートしていないので、未接続バスとして 0xFF
	return 0xFF;
}

REGISTER_MSXDEVICE(TangNanoVdpDevice, "TangNanoVdpDevice");
} // namespace openmsx

