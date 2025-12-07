#ifndef APP_INCLUDE_LEDBUFFER_HPP
#define APP_INCLUDE_LEDBUFFER_HPP

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

template <unsigned int NUM_LEDS_V, auto BRIGHTNESS_LUT_V>    // TODO specify auto
struct LedBufferCfg { };

template <typename T, std::size_t N>
struct LedBufferCfgParams {
    unsigned int numLeds;
    std::array<T, N> brightnessLut;
};

template <LedBufferCfgParams CFG_V>
static constexpr auto makeLedBufferConfig() noexcept -> LedBufferCfg<CFG_V.numLeds, CFG_V.brightnessLut>
{
    return {};
}

// template <typename T, std::size_t N>
// struct BrightnessTable {
// };

template <typename DRIVER_T, typename CFG_T>
class LedBuffer;

template <typename DRIVER_T, unsigned int NUM_LEDS_V,
          std::array<typename DRIVER_T::BrightnessType, NUM_LEDS_V> BRIGHTNESS_LUT_V>
class LedBuffer<DRIVER_T, LedBufferCfg<NUM_LEDS_V, BRIGHTNESS_LUT_V>> {
  public:
    static constexpr unsigned int numLeds = NUM_LEDS_V;

    constexpr LedBuffer(DRIVER_T& ledDriver, const LedBufferCfg<NUM_LEDS_V, BRIGHTNESS_LUT_V>&) noexcept
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
    std::array<std::optional<typename DRIVER_T::BrightnessType>, numLeds> _ledBuf{};
    const DRIVER_T& _driver;
};

template <typename DRIVER_T, unsigned int NUM_LEDS_V,
          std::array<typename DRIVER_T::BrightnessType, NUM_LEDS_V> BRIGHTNESS_LUT_V>
LedBuffer(DRIVER_T&, const LedBufferCfg<NUM_LEDS_V, BRIGHTNESS_LUT_V>&)
    -> LedBuffer<DRIVER_T, LedBufferCfg<NUM_LEDS_V, BRIGHTNESS_LUT_V>>;

template <typename DRIVER_T, unsigned int NUM_LEDS_V,
          std::array<typename DRIVER_T::BrightnessType, NUM_LEDS_V> BRIGHTNESS_LUT_V>
LedBuffer(DRIVER_T&, const LedBufferCfg<NUM_LEDS_V, BRIGHTNESS_LUT_V>&)
    -> LedBuffer<DRIVER_T, , detail::fillArray<typename DRIVER_T::BrightnessType, NUM_LEDS_V>()>;

#endif    // APP_INCLUDE_LEDBUFFER_HPP
