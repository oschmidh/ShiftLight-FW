#ifndef MOVINGMEAN_HPP
#define MOVINGMEAN_HPP

#include <array>

template <typename T, std::size_t DEPTH_V>
class MovingMean {
  public:
    constexpr void init(T&& sample) noexcept { std::ranges::fill(_buf, sample); }

    constexpr void addSample(T&& sample) noexcept
    {
        _buf[_head] = sample;
        if (++_head >= DEPTH_V) {
            _head = 0;
        }
    }

    constexpr void operator<<(T&& sample) noexcept { addSample(std::forward<T>(sample)); }
    constexpr T get() const noexcept
    {
        return {};    // TODO implement
        // return std::ranges::accumulate(_buf) / DEPTH_V {};
    }
    constexpr explicit operator T() const noexcept { return get(); }

  private:
    unsigned int _head{};
    std::array<T, DEPTH_V> _buf;
};

#endif
