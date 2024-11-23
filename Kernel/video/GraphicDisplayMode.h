#pragma once

#include <video/Screen.h>
#include <utils/types.h>
#include <video/Fonts/font.h>

namespace kernel::video {
    class GraphicDisplayMode final : public Screen
    {
    public:
        void putcar(uint8_t c) override;

        void scrollup(uint8_t n) override;

        static Color_32 get32BitsColor(const Color color)
        {
            switch(color)
            {
                case Black:
                    return Black32;
                case Blue:
                    return Blue32;
                case Green:
                    return Green32;
                case Pink:
                    return Pink32;
                case Red:
                    return Red32;
                case SoftBlue:
                    return SoftBlue32;
                case White:
                    return White32;
                case Yellow:
                    return Yellow32;
                default:
                    return Orange32;
            }
        }

        void clean() override;

    private:
        friend class Screen;
        friend int main(mb_partial_info*);

        GraphicDisplayMode(const VbeModeInfo *info, char *framebuffer);
        GraphicDisplayMode(char *framebuffer, uint32_t width, uint32_t height, uint8_t bitsPerPixel, uint32_t bytePerScanline);

        uint8_t * const _buffer;
        uint8_t * const _frameBuffer;
        uint8_t* _pixel;
        uint32_t _bytePerLine;
        uint8_t  _bitsPerPixel;
        uint8_t  _pixelWidth;
        fonts::Font* _font;
    };
}
