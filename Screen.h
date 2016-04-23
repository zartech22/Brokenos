#ifndef __SCREEN__
#define __SCREEN__

#include <cstdarg>
#include "types.h"

enum Color
{
	Black		= 0x0,
	Blue		= 0x1,
	Green		= 0x2,
	SoftBlue	= 0x3,
	Red			= 0x4,
	Pink		= 0x5,
	Yellow		= 0x6,
	White		= 0x7
};

class Screen
{
public :
	static Screen& getScreen();
	
	Screen() {}
	
	~Screen() {}

	void putcar(uchar);
	void print(const char*, ...);
	void println(const char*, ...);
	void printInfo(const char*, ...);
	void printDebug(const char*, ...);
	void printError(const char*, ...);
	
	void printk(const char *s, ...);
	
	void setColor(enum Color fgColor, enum Color bgColor);
	void setPos(u8 posX, u8 posY);
	
	bool isLoading() const { return _isLoading; }
	
	void okMsg();
	void failMsg();
	
	void dump(const uchar*, int);
	
	void switchCursor();
	void show_cursor();
	void hide_cursor();
	
	void showLoadScreen();
	void showTic();
	
private :				
	static Screen _inst;
				
	static u8 _posX, _posY;
	static u8 _colors;
	
	static bool _showCursor;
	static bool _isLoading;
	static u8 _ticNbr;
	
	void scrollup(u8);
	
	void print_core(const char*, va_list);
	void println_core(const char*, va_list);
	void printk_core(const char *s, va_list ap);
	void printBlock(const char *msg, u8 posX, u8 colors = 0x0E);
	
	void move_cursor(u8, u8);
};

#endif
