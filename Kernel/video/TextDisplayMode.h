#ifndef TEXTDISPLAYMODE_H
#define TEXTDISPLAYMODE_H

#include <video/Screen.h>
#include <utils/types.h>

class TextDisplayMode : public Screen
{
public:
    virtual void putcar(uchar c) override
    {
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
            uchar *video = (uchar *) (_frameBuffer + 2 * _posX + 160 * _posY);
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

        /*if(_showCursor)
            show_cursor();
        else
            hide_cursor();*/
    }

private:
    friend class Screen;
    friend int main(struct mb_partial_info*);

    TextDisplayMode(VbeModeInfo *info) : Screen(), _frameBuffer((uchar*)0xB8000)
    {
        _maxX = info->XResolution;
        _maxY = info->YResolution;
    }

    uchar * const _frameBuffer;
};

#endif // GRAPHICDISPLAYMODE_H
