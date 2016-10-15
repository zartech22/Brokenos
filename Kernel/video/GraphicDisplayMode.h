#ifndef GRAPHICDISPLAYMODE_H
#define GRAPHICDISPLAYMODE_H

#include <video/Screen.h>
#include <utils/types.h>
#include <video/font.h>
#include <utils/lib.h>
#include <memory/mm.h>
#include <core/io.h>

#include <utils/lib.h>

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
            _posX += 8;
        else if(c == 13) //CR
            _pixel -= _posX * _bitsPerPixel;
        else
        {
            uchar *letter = font8x8_basic[c];
            uchar *tmp = _frameBuffer + _posY * _bytePerLine * 8 + _posX * _bitsPerPixel;
            uchar *tmp_buffer = _buffer + _posY * _bytePerLine * 8 + _posX * _bitsPerPixel;

            if(_posX == _maxX)
                putcar('\n');

            for (unsigned int x = 0; x < 8; x++)
            {
                for (unsigned int y = 0; y < 8; y++)
                {
                    if(letter[x] & 1 << y)
                    {
                        for(u8 i = 0; i < 3; ++i)
                        {
                            tmp_buffer[i] = (get32BitsColor((Color)(getColor() & 0xF)) >> (8 * i)) & 0xFF;
                            tmp[i] = (get32BitsColor((Color)(getColor() & 0xF)) >> (8 * i)) & 0xFF;
                        }
                    }
                    else
                    {
                        for(u8 i = 0; i < 3; ++i)
                        {
                            tmp_buffer[i] = (get32BitsColor((Color)(getColor() >> 4)) >> (8 * i)) & 0xFF;
                            tmp[i] = (get32BitsColor((Color)(getColor() >> 4)) >> (8 * i)) & 0xFF;
                        }
                    }

                    tmp += _pixelWidth;
                    tmp_buffer += _pixelWidth;
                }

                tmp += _bytePerLine - 8 * _pixelWidth;
                tmp_buffer += _bytePerLine - 8 * _pixelWidth;
            }

            _pixel += 8 * _pixelWidth;
            _posX++;
        }

        if(_posY > _maxY)
            scrollup(_posY - _maxY);
    }

    virtual void scrollup(u8 n) override
    {
        unsigned int const videoEnd = ((_maxY + 1) * _bytePerLine * 8);
        uchar *offset = _buffer + n * _bytePerLine * 8;


        //memcpy((char*)_frameBuffer, (char*)offset, (videoEnd - (_frameBuffer + n * _bytePerLine * 8)));

        unsigned int pos;
        u32 val1, val2;

        while(inb(0x3DA) & 0x08);
        while(!(inb(0x3DA) & 0x08));


        for(unsigned int y = _bytePerLine * (n - 1) * 8; y < _maxY * _bytePerLine * 8; y += _bytePerLine)
        {
            for(unsigned int x = 0; x < (_maxX + 1) * 8 * _pixelWidth; x += _pixelWidth)
            {
                pos = x + y;

                val1 = _buffer[pos + 3] << 24 | _buffer[pos + 2] << 16 | _buffer[pos + 1] << 8 | _buffer[pos];
                val2 = _buffer[pos + _bytePerLine * 8 + 3] << 24 | _buffer[pos + 2 + _bytePerLine * 8] << 16 | _buffer[pos + 1 + _bytePerLine * 8] << 8 | _buffer[pos + _bytePerLine * 8];

                if(val1 != val2)
                {
                    _frameBuffer[pos] = _buffer[pos + _bytePerLine * 8];
                    _frameBuffer[pos + 1] = _buffer[pos + _bytePerLine * 8 + 1];
                    _frameBuffer[pos + 2] = _buffer[pos + _bytePerLine * 8 + 2];
                    _frameBuffer[pos + 3] = _buffer[pos + _bytePerLine * 8 + 3];
                }
            }
        }

        for(unsigned int y = videoEnd - _bytePerLine * 8; y < videoEnd; y += _bytePerLine)
        {
            for(unsigned int x = 0; x < (_maxX + 1) * 8 * _pixelWidth; x += _pixelWidth)
            {
                pos = x + y;

                val1 = _buffer[pos + 3] << 24 | _buffer[pos + 2] << 16 | _buffer[pos + 1] << 8 | _buffer[pos];

                if(val1 != 0)
                    _frameBuffer[pos] = _frameBuffer[pos + 1] = _frameBuffer[pos + 2] = _frameBuffer[pos + 3] = 0;
            }
        }

        checkBounds(_buffer, (_buffer + videoEnd - offset));
        memcpy((char*)_buffer, (char*)offset, (_buffer + videoEnd - offset));
        memset((char*)(_buffer + videoEnd - (n * _bytePerLine * 8)), 0, _bytePerLine * 8 * n);

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

    inline virtual void clean() override { memset((char*)_frameBuffer, 0, (_maxY + 1) * _bytePerLine * 8); }

private:
    friend class Screen;
    friend int main(struct mb_partial_info*);

    GraphicDisplayMode(VbeModeInfo *info) : Screen(), _frameBuffer((uchar*)GRAPHIC_MODE_VIDEO), _pixel(_frameBuffer), _bytePerLine(info->BytesPerScanLine),
        _bitsPerPixel(info->BitsPerPixel), _pixelWidth(_bitsPerPixel / 8), _buffer(new uchar[info->YResolution * info->BytesPerScanLine])
    {
        _maxX = info->XResolution / 8 - 1;
        _maxY = info->YResolution / 8 - 1;

        checkBounds(_buffer, info->YResolution * _bytePerLine);
        memset((char*)_buffer, 0, info->YResolution * _bytePerLine);
    }

    uchar * const _buffer;
    uchar * const _frameBuffer;
    uchar* _pixel;
    u16 _bytePerLine;
    u8  _bitsPerPixel;
    u8  _pixelWidth;
};

#endif // GRAPHICDISPLAYMODE_H
