#ifndef GRAPHICDISPLAYMODE_H
#define GRAPHICDISPLAYMODE_H

#include "Screen.h"
#include "types.h"
#include "font.h"

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

            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    if(letter[x] & 1 << y)
                        *tmp = getColor() & 0x0F;
                    else
                        *tmp = getColor() >> 4;
                    tmp += _pixelWidth;
                }

                tmp += _bytePerLine - 8 * _pixelWidth;
            }

            _pixel += 8 * _pixelWidth;
            _posX++;

            /*uchar *video = (uchar *) (RAMSCREEN + 2 * _posX + 160 * _posY);
            *video = c;
            *(video + 1) = _colors;
            _posX++;
            if(_posX > 79)
            {
                _posX = 0;
                _posY++;
            }*/
        }
       /* if(_posY > _maxY)
            scrollup(_posY - _maxY);

        if(_showCursor)
            show_cursor();
        else
            hide_cursor();*/
    }

private:
    friend class Screen;
    friend int main(struct mb_partial_info*);

    GraphicDisplayMode(VbeModeInfo *info) : Screen(), _frameBuffer((uchar*)info->PhysBasePtr), _pixel(_frameBuffer), _bytePerLine(info->BytesPerScanLine),
        _bitsPerPixel(info->BitsPerPixel), _pixelWidth(_bitsPerPixel / 8) {}

    uchar * const _frameBuffer;
    uchar* _pixel;
    u16 _bytePerLine;
    u8  _bitsPerPixel;
    u8  _pixelWidth;
};

#endif // GRAPHICDISPLAYMODE_H
