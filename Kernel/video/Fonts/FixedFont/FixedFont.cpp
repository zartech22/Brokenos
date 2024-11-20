//
// Created by kevin on 09/11/24.
//

#include "FixedFont.h"
#include "FixedFontData.h"

#include <bit>
#include <bitset>

namespace kernel::video::fonts {
    FixedFont::FixedFont() : Font (1, 8, 8) {
    }

    Glyph FixedFont::getGlyph(const uint8_t unicode) {
        const Glyph glyph(8, fontData[unicode]);

        return glyph;
    }
}
// fixed
// fonts
// video
// kernel