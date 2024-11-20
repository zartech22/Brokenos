//
// Created by kevin on 09/11/24.
//

#pragma once

#include <cstdint>

#include "video/Fonts/font.h"


namespace kernel::video::fonts {

class PsfFont final : public Font {
public:
    PsfFont() : Font(1, 16, 8), _glyphs(), _addr_start(nullptr), _headerSize(0) { init(); }
    ~PsfFont() override;
    [[nodiscard]] Glyph getGlyph(uint8_t unicode) override;

private:
    void init();
    Glyph* _glyphs[255];
    char* _addr_start;
    uint32_t _headerSize;
};

} // psf
// fonts
// video
// kernel

