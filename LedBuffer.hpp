#ifndef LEDBUFFER_HPP
#define LEDBUFFER_HPP

#include <optional>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <array>

namespace detail {

template <typename T, std::size_t N, T value>
constexpr std::array<T, N> fillArray() noexcept
{
    std::array<T, N> arr{};
    std::ranges::fill(arr);
    return arr;
};

}    // namespace detail

template <typename DRIVER_T, unsigned int NUM_LEDS_V,
          std::array<std::uint8_t, NUM_LEDS_V> BRIGHTNESS_LUT_V =
              detail::fillArray<std::uint8_t, NUM_LEDS_V>(std::numeric_limits<std::uint8_t>::max())>
class LedBuffer {
  public:
    using BrightnessType = std::uint8_t;

    static constexpr unsigned int numLeds = NUM_LEDS_V;

    constexpr LedBuffer(DRIVER_T& ledDriver) noexcept
     : _driver(ledDriver)
    { }

    constexpr void setLed(unsigned int idx, bool on) noexcept
    {
        if (idx >= _ledBuf.size()) {
            return;
        }

        _ledBuf[idx] = on ? BRIGHTNESS_LUT_V[idx] : 0;
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
    std::array<std::optional<BrightnessType>, numLeds> _ledBuf{};
    const DRIVER_T& _driver;
};

#endif
