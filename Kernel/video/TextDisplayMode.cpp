//
// Created by kevin on 23/11/24.
//
#include "TextDisplayMode.h"

namespace kernel::video {
    void TextDisplayMode::putcar(const uint8_t c) {
        if(c == 10) //saut de ligne (CR-NL)
        {
            _posX = 0;
            _posY++;
        }
        else if(c == 9) //tab
            _posX = _posX + 8 - (_posX % 8);
        else if(c == 13) //CR
            _posX = 0;
        else
        {
            auto *video = _frameBuffer + 2 * _posX + 160 * _posY;
            *video = c;
            *(video + 1) = getColor();
            _posX++;

            if(_posX > _maxX)
            {
                _posX = 0;
                _posY++;
            }
        }
        if(_posY > _maxY)
            scrollup(_posY - _maxY);
    }
}