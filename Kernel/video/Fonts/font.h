#pragma once

#include <cstdint>
#include <bitset>

namespace kernel::video::fonts {
    struct Glyph {
    explicit Glyph(const uint8_t height, const std::bitset<8> data[]) : height(height), data(data) {}

    const uint8_t height;
    const std::bitset<8> *data;
};

class Font {
public:
    Font() = default;

    virtual ~Font() = default;

    [[nodiscard]] uint8_t getBytesPerGlyph() const { return _bytesPerGlyph; }
    [[nodiscard]] uint8_t getHeight() const { return _height; }
    [[nodiscard]] uint8_t getWidth() const { return _width; };

    [[nodiscard]] virtual Glyph getGlyph(uint8_t unicode) = 0;

protected:
    Font(const uint8_t bytesPerGlyph, const uint8_t height, const uint8_t width) : _bytesPerGlyph(bytesPerGlyph),
        _height(height), _width(width) {
    };

    uint8_t _bytesPerGlyph{};
    uint8_t _height{};
    uint8_t _width{};
};
}