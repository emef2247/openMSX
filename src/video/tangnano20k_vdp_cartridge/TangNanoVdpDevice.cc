#include "TangNanoVdpDevice.hh"

#include <algorithm>

namespace openmsx {

static constexpr unsigned WIDTH  = 256;
static constexpr unsigned HEIGHT = 212;

TangNanoVdpDevice::TangNanoVdpDevice(const DeviceConfig& config)
    : MSXDevice(config)
    , FrameSource()
{
    FrameSource::init(FieldType::NONINTERLACED);
}

void TangNanoVdpDevice::powerUp(EmuTime time)
{
    lastTime = time;
}

void TangNanoVdpDevice::reset(EmuTime time)
{
    lastTime = time;
}

void TangNanoVdpDevice::writeIO(uint16_t, byte, EmuTime time)
{
    lastTime = time;
}

byte TangNanoVdpDevice::readIO(uint16_t, EmuTime time)
{
    lastTime = time;
    return 0xFF;
}

/* ===== FrameSource ===== */

unsigned TangNanoVdpDevice::getLineWidth(unsigned) const
{
    return WIDTH;
}

std::span<const unsigned int>
TangNanoVdpDevice::getUnscaledLine(
    unsigned /*line*/,
    std::span<unsigned int> workBuffer) const
{
    std::fill(
        workBuffer.begin(),
        workBuffer.begin() + WIDTH,
        0x00000000u); // 黒

    return workBuffer.subspan(0, WIDTH);
}

REGISTER_MSXDEVICE(TangNanoVdpDevice, "TangNanoVdpDevice");

} // namespace openmsx
