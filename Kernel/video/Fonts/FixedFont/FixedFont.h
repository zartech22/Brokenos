//
// Created by kevin on 09/11/24.
//

#pragma once

#include "video/Fonts/font.h"

namespace kernel::video::fonts {

class FixedFont final : public Font {
    public:
    FixedFont();
    Glyph getGlyph(uint8_t unicode) override;

};

} // fixed
// fonts
// video
// kernel
