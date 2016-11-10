#include <utils/types.h>
#include <core/io.h>
#include <utils/lib.h>
#include <video/font.h>
#include <video/Screen.h>
#include <memory/kmalloc.h>
#include <memory/mm.h>

#include "GraphicDisplayMode.h"
#include "TextDisplayMode.h"

#define RAMSCREEN           0xB8000
#define SIZESCREEN          0xFA0
#define SCREENLIM           0xB8FA0

#define VBE_INFO            0x00000800
#define FRAMEBUFFER_INFO    0x00001000

#define GRAPHIC_MODE_ATTR   0x10

Screen* Screen::_inst = 0;

Screen& Screen::getScreen()
{	
    return *Screen::_inst;
}

void Screen::initScreen(mb_partial_info *info)
{

    if(info->flags & VBE_INFO)
    {
        VbeModeInfo *vbeInfo = (struct VbeModeInfo*)info->vbe_mode_info;

        if(vbeInfo->ModeAttributes & GRAPHIC_MODE_ATTR)
        {
            char *end = (char*)vbeInfo->PhysBasePtr + vbeInfo->YResolution * vbeInfo->BytesPerScanLine;

            //init_graphicMode_video_memory((char*)info->PhysBasePtr, end);

            struct page *framebuffer = get_page_from_heap((char*)vbeInfo->PhysBasePtr, end);

            Screen::_inst = new GraphicDisplayMode(vbeInfo, framebuffer->v_addr);

            //_inst->printError("Graphic virtual memory starts : %p - ends %p, page : %d", framebuffer->v_addr, framebuffer->v_addr + info->YResolution * info->BytesPerScanLine, (info->YResolution * info->BytesPerScanLine) / PAGESIZE);

            delete framebuffer;
        }
        else
            Screen::_inst = new TextDisplayMode(vbeInfo);
    }
    else if(info->flags & FRAMEBUFFER_INFO)
    {
        if(!(info->framebuffer_addr >> 32))
        {
            char *begin = (char*)(info->framebuffer_addr & 0xFFFFFFFF);
            char *end = (char*)(begin + info->framebuffer_height * info->framebuffer_pitch);

            struct page *framebuffer = get_page_from_heap(begin, end);

            Screen::_inst = new GraphicDisplayMode(framebuffer->v_addr, info->framebuffer_width, info->framebuffer_height, info->framebuffer_bpp, info->framebuffer_pitch);

            delete framebuffer;
        }
    }
}

void Screen::print(const char *string, ...)
{
	va_list ap;
	va_start(ap, string);
	
	printk_core(string, ap);
	
	va_end(ap);
}

void Screen::println(const char *string, ...)
{
	va_list ap;
	va_start(ap, string);
	
	print_core(string, ap);
	putcar('\n');
	
	va_end(ap);
}

void Screen::printInfo(const char *str, ...)
{	
	va_list ap;
	va_start(ap, str);
	
	printBlock("INFO", 0);
	putcar(' ');
	println_core(str, ap);
		
	va_end(ap);
}

void Screen::printDebug(const char *str, ...)
{	
	va_list ap;
	va_start(ap, str);
	
	printBlock("DEBUG", 0, (Yellow << 4) | Black);
	putcar(' ');
	println_core(str, ap);
	
	va_end(ap);
}

void Screen::printError(const char *str, ...)
{	
	va_list ap;
	va_start(ap, str);
	
	printBlock("ERROR", 0, (Red << 4) | White);
	putcar(' ');
	println_core(str, ap);
		
	va_end(ap);
}

void Screen::printk(const char *s, ...)
{
	va_list ap;

	va_start(ap, s);
	
	printk_core(s, ap);
	
	va_end(ap);
}

void Screen::setColor(enum Color fgColor, enum Color bgColor)
{
	_colors = (bgColor << 4) | fgColor;
}

void Screen::setPos(u8 posX, u8 posY)
{
	_posX = posX;
	_posY = posY;
}

void Screen::okMsg()
{
	_posY--;
    printBlock("OK", _maxX - strlen("OK"), Green);
	_posY++;
}

void Screen::failMsg()
{
    _posY -= 8;
    printBlock("FAIL", _maxX - strlen("FAIL"), Red);
	_posY++;
}

void Screen::dump(const uchar* addr, int n)
{
	char c1, c2;
	const char *tab = "0123456789ABCDEF";
	
	while(n--)
	{
		c1 = tab[(*addr & 0xF0) >> 4];
		c2 = tab[*addr & 0x0F];
		addr++;
		putcar(c1);
		putcar(c2);
	}
    println("");
}

void Screen::switchCursor()
{
	_showCursor = !_showCursor;
}

void Screen::show_cursor()
{
	move_cursor(_posX, _posY);
}

void Screen::hide_cursor()
{
	move_cursor(-1, -1);
}

void Screen::scrollup(u8 n)
{
	uchar *tmp;
	
	for(uchar *video = (uchar *) RAMSCREEN; video < (uchar *) SCREENLIM; video += 2)
	{
		tmp = (uchar *) (video + n * 160);
		
		if(tmp < (uchar *) SCREENLIM)
		{
			*video = *tmp;
			*(video + 1) = *(tmp + 1);
		}
		else
		{
			*video = 0;
			*(video + 1) = 0x07;
		}
	}
	
	_posY -= n;
	if(_posY < 0)
		_posY = 0;
}

void Screen::showLoadScreen()
{	
	for(uchar *video = (uchar *) RAMSCREEN; video < (uchar *) SCREENLIM; video += 2)
	{
		*video = ' ';
		*(video + 1) = 0x55;
	}
	
	uchar *video = (uchar *) (RAMSCREEN + 70 + 160 * 13);
	
	const char color = 0x57;
	
	*video = 'K';
	*(video + 1) = color;
	
	*(video + 2) = 'e';
	*(video + 3) = color;
	
	*(video + 4) = 'r';
	*(video + 5) = color;
	
	*(video + 6) = 'n';
	*(video + 7) = color;
	
	*(video + 8) = 'e';
	*(video + 9) = color;
	
	*(video + 10) = 'l';
	*(video + 11) = color;
	
	_isLoading = true;
}

void Screen::showTic()
{
	uchar *video = (uchar *) (RAMSCREEN + 70 + 160 * 14);
	video += _ticNbr * 2;
	
	*video = '.';
	*(video + 1) = 0x57;
	
	_ticNbr++;
			
	if(_ticNbr > 6)
	{
		_ticNbr = 0;
		_isLoading = false;
		_posX = 0;
		scrollup(25);
	}
}

void Screen::print_core(const char *string, va_list ap)
{
	printk_core(string, ap);
}

void Screen::println_core(const char *string, va_list ap)
{
	printk_core(string, ap);
	putcar('\n');
}

void Screen::printk_core(const char *s, va_list ap)
{
	char buf[16];
	int i, j, size, buflen, neg;

	unsigned char c;
	int ival;
	unsigned int uival;
	
	while ((c = *s++)) {
		size = 0;
		neg = 0;

		if (c == 0)
			break;
		else if (c == '%') {
			c = *s++;
			if (c >= '0' && c <= '9') {
				size = c - '0';
				c = *s++;
			}

			if (c == 'd') {
				ival = va_arg(ap, int);
				if (ival < 0) {
					uival = 0 - ival;
					neg++;
				} else
					uival = ival;
				itoa(buf, uival, 10);

				buflen = strlen(buf);
				if (buflen < size)
					for (i = size, j = buflen; i >= 0;
					     i--, j--)
						buf[i] =
						    (j >=
						     0) ? buf[j] : '0';

				if (neg)
					printk("-%s", buf);
				else
					printk(buf);
			} else if (c == 'u') {
				uival = va_arg(ap, int);
				itoa(buf, uival, 10);

				buflen = strlen(buf);
				if (buflen < size)
					for (i = size, j = buflen; i >= 0;
					     i--, j--)
						buf[i] =
						    (j >=
						     0) ? buf[j] : '0';

				printk(buf);
			} else if (c == 'x' || c == 'X') {
				uival = va_arg(ap, int);
				itoa(buf, uival, 16);

				buflen = strlen(buf);
				if (buflen < size)
					for (i = size, j = buflen; i >= 0;
					     i--, j--)
						buf[i] =
						    (j >=
						     0) ? buf[j] : '0';

				printk("0x%s", buf);
			} else if(c == 'b') {
				uival = va_arg(ap, int);
				itoa(buf, uival, 2);
				
				buflen = strlen(buf);
				if(buflen < size)
					for(i = size, j = buflen; i >= 0; i--, j--)
						buf[i] = (j >= 0) ? buf[j] : '0';
				
				printk("b%s", buf);
			} else if (c == 'p') {
				uival = va_arg(ap, int);
				itoa(buf, uival, 16);
				size = 8;

				buflen = strlen(buf);
				if (buflen < size)
					for (i = size, j = buflen; i >= 0;
					     i--, j--)
						buf[i] =
						    (j >=
						     0) ? buf[j] : '0';

				printk("0x%s", buf);
			} else if (c == 's') {
				printk((char *) va_arg(ap, int));
			} else if(c == 'c') {
				uival = va_arg(ap, int);
				putcar((uchar) uival);
			}
        } else
            putcar(c);
	}
}

void Screen::printBlock(const char *msg, u8 posX, u8 colors)
{
	if(posX != 0)
        posX -=  2;
	
	u8 tmp = _posX;
	u8 tmpc = _colors;
	
	_posX = posX;
	_colors = colors;
	
	print("[%s]", msg);
		
	
	_posX = tmp;
	_colors = tmpc;
	
	if(posX == 0)
        _posX += strlen(msg) + 2;
}

void Screen::move_cursor(u8 x, u8 y)
{
	u16 c_pos;
	
	c_pos = y * 80 + x;
	
	outb(0x3d4, 0x0f);
	outb(0x3d5, (u8) c_pos);
	outb(0x3d4, 0x0e);
	outb(0x3d5, (u8)(c_pos >> 8));
}
