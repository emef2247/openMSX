#pragma once

#include "MSXDevice.hh"
#include "FrameSource.hh"
#include "EmuTime.hh"

#include <span>

namespace openmsx {

class TangNanoVdpDevice final
    : public MSXDevice
    , public FrameSource
{
public:
    explicit TangNanoVdpDevice(const DeviceConfig& config);

    void powerUp(EmuTime time) override;
    void reset(EmuTime time) override;

    void writeIO(uint16_t port, byte value, EmuTime time) override;
    byte readIO(uint16_t port, EmuTime time) override;

    // FrameSource
    [[nodiscard]] unsigned getLineWidth(unsigned line) const override;
    [[nodiscard]] std::span<const unsigned int>
    getUnscaledLine(unsigned line,
                    std::span<unsigned int> workBuffer) const override;

private:
    EmuTime lastTime = EmuTime::zero();
};

} // namespace openmsx
