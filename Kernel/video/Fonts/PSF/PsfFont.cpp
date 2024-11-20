//
// Created by kevin on 09/11/24.
//

#include "PsfFont.h"

#include <cstdint>

extern char _binary_zap_light16_psf_start;
extern char _binary_zap_light16_psf_end;

namespace kernel::video::fonts {
    constexpr uint16_t PSF1_FONT_MAGIC = 0x0436;
    constexpr uint32_t PSF_FONT_MAGIC = 0x864ab572;

    struct PSF1_Header {
        uint16_t magic;
        uint8_t fontMode;
        uint8_t charSize;
    } __attribute__((__packed__));

    struct PSF_Font {
        uint32_t magic;
        uint32_t version;
        uint32_t headerSize;
        uint32_t flags;
        uint32_t numglyph;
        uint32_t bytesPerGlyph;
        uint32_t height;
        uint32_t width;
    } __attribute__((packed));


    void PsfFont::init() {
        _height = reinterpret_cast<PSF1_Header *>(&_binary_zap_light16_psf_start)->charSize;
        _width = 8;
        _bytesPerGlyph = 1;
        _addr_start = &_binary_zap_light16_psf_start;
        _headerSize = sizeof(PSF1_Header);

        for (auto & _glyph : _glyphs) {
            _glyph = nullptr;
        }
    }

    Glyph PsfFont::getGlyph(uint8_t unicode) {
        unicode = (unicode < 256) ? unicode : 0;
        if (_glyphs[unicode] == nullptr) {
            auto *data = new std::bitset<8>[_height];

            const auto *ptr = _addr_start + _headerSize + unicode * _height;

            for (uint8_t i = 0; i < _height; ++i) {
                data[i] = std::bitset<8>(*(ptr + i));
            }

            _glyphs[unicode] = new Glyph(_height, data);
        }
        return *_glyphs[unicode];
    }

    PsfFont::~PsfFont() {
        for (const auto &_glyph: _glyphs) {
            delete _glyph;
        }
    }
} // psf
// fonts
// video
// kernel
