#ifndef I2C_HPP
#define I2C_HPP

#include <span>
#include <cstdint>

class I2c {    // TODO call i2cController?
  public:
    void init() noexcept { }

    void write(std::uint8_t addr, std::span<const std::uint8_t> writebuf) const noexcept
    {
        // TODO implement
    }

    // Write transaction, followed by a read transaction with restart in between
    void transfer(std::uint8_t addr, std::span<const std::uint8_t> writebuf,
                  std::span<std::uint8_t> readbuf) const noexcept
    {
        // TODO implement
    }

  private:
};

#endif
