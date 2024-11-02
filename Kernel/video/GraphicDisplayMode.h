#pragma once

#include <video/Screen.h>
#include <utils/types.h>
#include <video/font.h>
#include <utils/lib.h>
#include <core/io.h>

class GraphicDisplayMode final : public Screen
{
public:
    void putcar(const uchar c) override
    {
        if(c == 10) //saut de ligne (CR-NL)
        {
            _pixel -= _posX * _bitsPerPixel;
            _pixel = _frameBuffer + _posY * _bytePerLine * 8;
            _posX = 0;
            _posY++;
        }
        else if(c == 9) //tab
        {
            if(_posX + 8 > _maxX)
            {
                _posY++;
                _posX = (_posX + 8) % _maxX;
            }
            else
                _posX += 8;
        }
        else if(c == 13) //CR
            _pixel -= _posX * _bitsPerPixel;
        else
        {
            const uchar *letter = font8x8_basic[c];
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
                            tmp_buffer[i] = (get32BitsColor(static_cast<Color>(getColor() & 0xF)) >> (8 * i)) & 0xFF;
                            tmp[i] = (get32BitsColor(static_cast<Color>(getColor() & 0xF)) >> (8 * i)) & 0xFF;
                        }
                    }
                    else
                    {
                        for(u8 i = 0; i < 3; ++i)
                        {
                            tmp_buffer[i] = (get32BitsColor(static_cast<Color>(getColor() >> 4)) >> (8 * i)) & 0xFF;
                            tmp[i] = (get32BitsColor(static_cast<Color>(getColor() >> 4)) >> (8 * i)) & 0xFF;
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

    void scrollup(u8 n) override
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
        memcpy(reinterpret_cast<char *>(_buffer), reinterpret_cast<char *>(offset), (_buffer + videoEnd - offset));
        memset(reinterpret_cast<char *>(_buffer + videoEnd - (n * _bytePerLine * 8)), 0, _bytePerLine * 8 * n);

        _posY -= n;
    }

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

    void clean() override { memset(reinterpret_cast<char *>(_frameBuffer), 0, (_maxY + 1) * _bytePerLine * 8); memset(reinterpret_cast<char *>(_buffer), 0, (_maxY + 1) * _bytePerLine * 8); }

private:
    friend class Screen;
    friend int main(mb_partial_info*);

    GraphicDisplayMode(const VbeModeInfo *info, char *framebuffer) : _buffer(new uchar[info->YResolution * info->BytesPerScanLine]), _frameBuffer(reinterpret_cast<uchar *>(framebuffer)), _pixel(_frameBuffer),
                                                                     _bytePerLine(info->BytesPerScanLine), _bitsPerPixel(info->BitsPerPixel), _pixelWidth(_bitsPerPixel / 8)
    {
        _maxX = info->XResolution / 8 - 1;
        _maxY = info->YResolution / 8 - 1;

        checkBounds(_buffer, info->YResolution * _bytePerLine);
        memset(reinterpret_cast<char *>(_buffer), 0, info->YResolution * _bytePerLine);

        printDebug("[%s] Framebuffer : %p", __FUNCTION__, framebuffer);
    }

    GraphicDisplayMode(char *framebuffer, const u32 width, const u32 height, const u8 bitsPerPixel, const u32 bytePerScanline) : _buffer(new uchar[height * bytePerScanline]), _frameBuffer(reinterpret_cast<uchar *>(framebuffer)),
                                                                                                                                 _pixel(_frameBuffer), _bytePerLine(bytePerScanline), _bitsPerPixel(bitsPerPixel), _pixelWidth(_bitsPerPixel / 8)
    {
        _maxX = width / 8 - 1;
        _maxY = height / 8 - 1;

        checkBounds(_buffer, height * _bytePerLine);
        memset(reinterpret_cast<char *>(_buffer), 0, height* _bytePerLine);

        printDebug("[%s] Framebuffer : %p", __FUNCTION__, framebuffer);
    }

    uchar * const _buffer;
    uchar * const _frameBuffer;
    uchar* _pixel;
    u32 _bytePerLine;
    u8  _bitsPerPixel;
    u8  _pixelWidth;
};
