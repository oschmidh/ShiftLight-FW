#ifndef APP_INCLUDE_LEDBUFFER_HPP
#define APP_INCLUDE_LEDBUFFER_HPP

#include <optional>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <array>

namespace detail {

template <typename T, std::size_t N>
constexpr std::array<T, N> fillArray(T value) noexcept
{
    std::array<T, N> arr{};
    std::ranges::fill(arr, value);
    return arr;
};

}    // namespace detail

template <typename DRIVER_T, unsigned int NUM_LEDS_V,
          std::array<typename DRIVER_T::BrightnessType, NUM_LEDS_V> BRIGHTNESS_LUT_V =
              detail::fillArray<typename DRIVER_T::BrightnessType, NUM_LEDS_V>(
                  std::numeric_limits<typename DRIVER_T::BrightnessType>::max())>
class LedBuffer {
  public:
    static constexpr unsigned int numLeds = NUM_LEDS_V;

    constexpr LedBuffer(DRIVER_T& ledDriver) noexcept
     : _driver(ledDriver)
    { }

    constexpr void setLed(unsigned int idx, bool on) noexcept
    {
        setLed(idx, on ? BRIGHTNESS_LUT_V[idx] : typename DRIVER_T::BrightnessType{});
    }

    constexpr void show() noexcept
    {
        for (unsigned int i = 0; i < _ledBuf.size(); ++i) {
            if (!_isDirty[i]) {
                continue;
            }

            _driver.setBrightness({.channel = i, .duty = _ledBuf[i]});
            _isDirty[i] = false;
        }
    }

  private:
    constexpr void setLed(unsigned int idx, typename DRIVER_T::BrightnessType value) noexcept
    {
        if (idx >= _ledBuf.size()) {
            return;
        }

        if (value == _ledBuf[idx]) {
            return;
        }

        _ledBuf[idx] = value;
        _isDirty[idx] = true;
    }

    std::array<typename DRIVER_T::BrightnessType, numLeds> _ledBuf{};
    std::array<bool, numLeds> _isDirty = detail::fillArray<bool, numLeds>(true);
    const DRIVER_T& _driver;
};

#endif // APP_INCLUDE_LEDBUFFER_HPP
