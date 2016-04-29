#include "types.h"
#include "io.h"
#include "lib.h"
#include "font.h"
#include "Screen.h"
#include "kmalloc.h"
#include "mm.h"

#include "GraphicDisplayMode.h"
#include "TextDisplayMode.h"

#define RAMSCREEN 0xB8000
#define SIZESCREEN 0xFA0
#define SCREENLIM 0xB8FA0

Screen* Screen::_inst = 0;

Screen& Screen::getScreen()
{	
    return *Screen::_inst;
}

void Screen::initScreen(VbeModeInfo *info)
{
    if(info->ModeAttributes & 0x10)
    {
        char *end = (char*)0x1100000 + info->YResolution * info->BytesPerScanLine;

//        char* v_addr = (char*)info->PhysBasePtr;
//        char* p_addr = (char*)info->PhysBasePtr;

        for(char *v_addr = (char*)0x1100000, *p_addr = (char*)info->PhysBasePtr; v_addr < end; v_addr += PAGESIZE, p_addr += PAGESIZE)
        {
            set_page_frame_used(PAGE(p_addr));

            u32* pde = (u32*) (0xFFFFF000 | (((u32) v_addr & 0xFFC00000) >> 20));

            if((*pde & PG_PRESENT) == 0)
            {
                Screen::getScreen().printError("pd0_add_page() : kernel page table not found for vaddr. STOP");
            }

            //Modification de l'entree dans la table de page
            u32* pte = (u32*) (0xFFC00000 | (((u32) v_addr & 0xFFFFF000) >> 10));
            *pte = ((u32) p_addr) | (PG_PRESENT | PG_WRITE | 0);

            //if(get_p_addr(v_addr) == p_addr)
              //  asm("hlt");

            asm("invlpg %0"::"m"(v_addr));
        }

        Screen::_inst = new GraphicDisplayMode(info);
    }
    else
        Screen::_inst = new TextDisplayMode(info);
}

//void Screen::putcar(uchar c)
//{
//	if(c == 10) //saut de ligne (CR-NL)
//	{
//        _frameBuffer -= _posX * _bitsPerPixel;
//        _frameBuffer += _bytePerLine;
//		_posX = 0;
//		_posY++;
//	}
//	else if(c == 9) //tab
//		_posX = _posX + 8 - (_posX % 8);
//	else if(c == 13) //CR
//		_posX = 0;
//	else
//	{
//        u8 pixelWidth = _bitsPerPixel / 8;
//        uchar *pixel = _frameBuffer + _posX * pixelWidth + _posY * _bytePerLine;
//        uchar *letter = font8x8_basic[c];
//        //FIXME: Gerer le saut a la ligne si on arrive fin de ligne

//        for (int x = 0; x < 8; x++)
//        {
//            for (int y = 0; y < 8; y++)
//            {
//                if(letter[x] & 1 << y)
//                    *pixel = Color::White;
//                else
//                    *pixel = _colors;
//                pixel += pixelWidth;
//            }
//            *pixel = _colors;
//            pixel += _bytePerLine - 8;
//        }

//        _frameBuffer += _bitsPerPixel;
//        _posX++;

//        /*uchar *video = (uchar *) (RAMSCREEN + 2 * _posX + 160 * _posY);
//		*video = c;
//		*(video + 1) = _colors;
//		_posX++;
//		if(_posX > 79)
//		{
//			_posX = 0;
//			_posY++;
//        }*/
//	}
//    if(_posY > _maxY)
//        scrollup(_posY - _maxY);
	
//	if(_showCursor)
//		show_cursor();
//	else
//		hide_cursor();
//}

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
    printBlock("OK", 120 - 8 * strlen("OK"), Green);
	_posY++;
}

void Screen::failMsg()
{
    _posY -= 8;
    printBlock("FAIL", 120 - 8 * strlen("FAIL"), Red);
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
