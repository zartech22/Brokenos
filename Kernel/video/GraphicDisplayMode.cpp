//
// Created by kevin on 23/11/24.
//
#include "GraphicDisplayMode.h"

#include <utils/lib.h>
#include <core/io.h>

#include "Fonts/PSF/PsfFont.h"

namespace kernel::video {
    GraphicDisplayMode::GraphicDisplayMode(const VbeModeInfo *info, char *framebuffer) : _buffer(new uint8_t[info->YResolution * info->BytesPerScanLine]),
    _frameBuffer(reinterpret_cast<uint8_t *>(framebuffer)),
    _pixel(_frameBuffer),
    _bytePerLine(info->BytesPerScanLine),
    _bitsPerPixel(info->BitsPerPixel),
    _pixelWidth(_bitsPerPixel / 8), _font(new kernel::video::fonts::PsfFont)
    {
        _maxX = info->XResolution / 8 - 1;
        _maxY = info->YResolution / _font->getHeight() - 1;

        checkBounds(_buffer, info->YResolution * _bytePerLine);
        memset(_buffer, 0, info->YResolution * _bytePerLine);
    }

GraphicDisplayMode::GraphicDisplayMode(char *framebuffer, const uint32_t width, const uint32_t height, const uint8_t bitsPerPixel, const uint32_t bytePerScanline) : _buffer(new uint8_t[height * bytePerScanline]),
    _frameBuffer(reinterpret_cast<uint8_t *>(framebuffer)),
    _pixel(_frameBuffer), _bytePerLine(bytePerScanline), _bitsPerPixel(bitsPerPixel),
    _pixelWidth(_bitsPerPixel / 8), _font(new kernel::video::fonts::PsfFont)
    {

        _maxX = width / 8 - 1;
        _maxY = height / _font->getHeight() - 1;

        checkBounds(_buffer, height * _bytePerLine);
        memset(_buffer, 0, height * _bytePerLine);
    }



    void GraphicDisplayMode::putcar(uint8_t c) {
        if (c == 10) //saut de ligne (CR-NL)
        {
            _pixel -= _posX * _bitsPerPixel;
            _pixel = _frameBuffer + _posY * _bytePerLine * _font->getHeight();
            _posX = 0;
            _posY++;
        } else if (c == 9) //tab
        {
            if (_posX + 8 > _maxX) {
                _posY++;
                _posX = (_posX + 8) % _maxX;
            } else
                _posX += 8;
        } else if (c == 13) //CR
            _pixel -= _posX * _bitsPerPixel;
        else {
            const kernel::video::fonts::Glyph &letter = _font->getGlyph(c);

            uint8_t *tmp = _frameBuffer + _posY * _bytePerLine * letter.height + _posX * _bitsPerPixel;
            uint8_t *tmp_buffer = _buffer + _posY * _bytePerLine * letter.height + _posX * _bitsPerPixel;

            if (_posX == _maxX)
                putcar('\n');

            for (unsigned int x = 0; x < letter.height; x++) {
                for (int8_t y = 7; y >= 0; y--) {
                    if (letter.data[x][y]) {
                        for (uint8_t i = 0; i < 3; ++i) {
                            tmp_buffer[i] = (get32BitsColor(static_cast<Color>(getColor() & 0xF)) >> (8 * i)) & 0xFF;
                            tmp[i] = (get32BitsColor(static_cast<Color>(getColor() & 0xF)) >> (8 * i)) & 0xFF;
                        }
                    } else {
                        for (uint8_t i = 0; i < 3; ++i) {
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

        if (_posY > _maxY)
            scrollup(_posY - _maxY);
    }

    void GraphicDisplayMode::scrollup(uint8_t n) {
        unsigned int const videoEnd = (_maxY + 1) * _bytePerLine * _font->getHeight();
            uint8_t *offset = _buffer + n * _bytePerLine * _font->getHeight();


            //memcpy((char*)_frameBuffer, (char*)offset, (videoEnd - (_frameBuffer + n * _bytePerLine * 8)));

            unsigned int pos;
            uint32_t val1, val2;

            while(inb(0x3DA) & 0x08);
            while(!(inb(0x3DA) & 0x08));


            for(auto y = _bytePerLine * (n - 1) * _font->getHeight(); y < _maxY * _bytePerLine * _font->getHeight(); y += _bytePerLine)
            {
                for(auto x = 0; x < (_maxX + 1) * 8 * _pixelWidth; x += _pixelWidth)
                {
                    pos = x + y;

                    const auto base = _bytePerLine * _font->getHeight();

                    val1 = _buffer[pos + 3] << 24 | _buffer[pos + 2] << 16 | _buffer[pos + 1] << 8 | _buffer[pos];
                    val2 = _buffer[pos + base + 3] << 24 | _buffer[pos + 2 + base] << 16 | _buffer[pos + 1 + base] << 8 | _buffer[pos + base];

                    if(val1 != val2)
                    {
                        _frameBuffer[pos] = _buffer[pos + base];
                        _frameBuffer[pos + 1] = _buffer[pos + base + 1];
                        _frameBuffer[pos + 2] = _buffer[pos + base + 2];
                        _frameBuffer[pos + 3] = _buffer[pos + base + 3];
                    }
                }
            }

            for(auto y = videoEnd - _bytePerLine * _font->getHeight(); y < videoEnd; y += _bytePerLine)
            {
                for(auto x = 0; x < (_maxX + 1) * 8 * _pixelWidth; x += _pixelWidth)
                {
                    pos = x + y;

                    val1 = _buffer[pos + 3] << 24 | _buffer[pos + 2] << 16 | _buffer[pos + 1] << 8 | _buffer[pos];

                    if(val1 != 0)
                        _frameBuffer[pos] = _frameBuffer[pos + 1] = _frameBuffer[pos + 2] = _frameBuffer[pos + 3] = 0;
                }
            }

            checkBounds(_buffer, (_buffer + videoEnd - offset));
            memcpy(reinterpret_cast<char *>(_buffer), reinterpret_cast<char *>(offset), (_buffer + videoEnd - offset));
            memset(reinterpret_cast<char *>(_buffer + videoEnd - (n * _bytePerLine * _font->getHeight())), 0, _bytePerLine * _font->getHeight() * n);

            _posY -= n;
    }

    void GraphicDisplayMode::clean() {
        memset(_frameBuffer, 0, (_maxY + 1) * _bytePerLine * 8);
        memset(_buffer, 0, (_maxY + 1) * _bytePerLine * 8);
    }
}
