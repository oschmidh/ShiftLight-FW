#ifndef LEDBUFFER_HPP
#define LEDBUFFER_HPP

#include <optional>
#include <cstdint>
#include <array>

template <typename DRIVER_T, unsigned int NUM_LEDS_V>
class LedBuffer {
  public:
    static constexpr unsigned int numLeds = NUM_LEDS_V;

    constexpr LedBuffer(DRIVER_T& ledDriver) noexcept
     : _driver(ledDriver)
    { }

    constexpr void setLed(unsigned int idx, std::uint8_t brightness) noexcept
    {
        if (idx >= _ledBuf.size()) {
            return;
        }

        _ledBuf[idx] = brightness;
    }

    constexpr void show() noexcept
    {
        for (unsigned int i = 0; i < _ledBuf.size(); ++i) {
            if (!_ledBuf[i].has_value()) {
                continue;
            }

            _driver.setBrightness({.channel = i, .duty = _ledBuf[i].value()});
            _ledBuf[i].reset();
        }
    }

  private:
    std::array<std::optional<std::uint8_t>, numLeds> _ledBuf{};
    const DRIVER_T& _driver;
};

#endif
