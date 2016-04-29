#ifndef GRAPHICDISPLAYMODE_H
#define GRAPHICDISPLAYMODE_H

#include "Screen.h"
#include "types.h"
#include "font.h"
#include "lib.h"

class GraphicDisplayMode : public Screen
{
public:
    virtual void putcar(uchar c) override
    {
        if(c == 10) //saut de ligne (CR-NL)
        {
            _pixel -= _posX * _bitsPerPixel;
            _pixel = _frameBuffer + _posY * _bytePerLine * 8;
            _posX = 0;
            _posY++;
        }
        else if(c == 9) //tab
            _pixel += 8 * _bitsPerPixel;
        else if(c == 13) //CR
            _pixel -= _posX * _bitsPerPixel;
        else
        {
            uchar *letter = font8x8_basic[c];
            uchar *tmp = _frameBuffer + _posY * _bytePerLine * 8 + _posX * _bitsPerPixel;
            //FIXME: Gerer le saut a la ligne si on arrive fin de ligne

            if(_posX == _maxX)
                putcar('\n');

            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    /*if(letter[x] & 1 << y)
                        *tmp = getColor() & 0x0F;
                    else
                        *tmp = getColor() >> 4;*/

                    if(letter[x] & 1 << y)
                    {
                        for(u8 i = 0; i < 3; ++i)
                            tmp[i] = (get32BitsColor((Color)(getColor() & 0xF)) >> (8 * i)) & 0xFF;
                    }
                    else
                        for(u8 i = 0; i < 3; ++i)
                            tmp[i] = (get32BitsColor((Color)(getColor() >> 4)) >> (8 * i)) & 0xFF;

                    tmp += _pixelWidth;
                }

                tmp += _bytePerLine - 8 * _pixelWidth;
            }

            _pixel += 8 * _pixelWidth;
            _posX++;
        }

        if(_posY > _maxY)
            scrollup(_posY - _maxY);
    }

    virtual void scrollup(u8 n) override
    {
        uchar * const videoEnd = _frameBuffer + ((_maxY + 1) * _bytePerLine * 8);
        uchar *offset = _frameBuffer + n * _bytePerLine * 8;


        memcpy((char*)_frameBuffer, (char*)offset, (videoEnd - offset));
        memset((char*)(videoEnd - (n * _bytePerLine * 8)), 0, _bytePerLine * 8 * n);


        _posY -= n;
    }

    Color_32 get32BitsColor(Color color)
    {
        switch(color)
        {
        case Color::Black:
            return Color_32::Black32;
        case Color::Blue:
            return Color_32::Blue32;
        case Color::Green:
            return Color_32::Green32;
        case Color::Pink:
            return Color_32::Pink32;
        case Color::Red:
            return Color_32::Red32;
        case Color::SoftBlue:
            return Color_32::SoftBlue32;
        case Color::White:
            return Color_32::White32;
        case Color::Yellow:
            return Color_32::Yellow32;
        default:
            return Color_32::Orange32;
        }
    }

private:
    friend class Screen;
    friend int main(struct mb_partial_info*);

    GraphicDisplayMode(VbeModeInfo *info) : Screen(), _frameBuffer((uchar*)0x1100000), _pixel(_frameBuffer), _bytePerLine(info->BytesPerScanLine),
        _bitsPerPixel(info->BitsPerPixel), _pixelWidth(_bitsPerPixel / 8)
    {
        _maxX = info->XResolution / 8 - 1;
        _maxY = info->YResolution / 8 - 1;
    }

    uchar * const _frameBuffer;
    uchar* _pixel;
    u16 _bytePerLine;
    u8  _bitsPerPixel;
    u8  _pixelWidth;
};

#endif // GRAPHICDISPLAYMODE_H
