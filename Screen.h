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
	
    virtual ~Screen() {}

    virtual void putcar(uchar) = 0;

	void print(const char*, ...);
	void println(const char*, ...);
	void printInfo(const char*, ...);
	void printDebug(const char*, ...);
	void printError(const char*, ...);
	
	void printk(const char *s, ...);
	
	void setColor(enum Color fgColor, enum Color bgColor);
	void setPos(u8 posX, u8 posY);

    const u8& getColor() const { return _colors; }
	
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
    friend int main(struct mb_partial_info *);

    Screen(VbeModeInfo *info) : _posX(0), _posY(0), _colors(0x0E), _showCursor(false), _isLoading(false), _ticNbr(0) { _inst = this; }

    static void initScreen(VbeModeInfo*);

    static Screen *_inst;
				
    u8 _colors;

	
    bool _showCursor;
    bool _isLoading;
    u8 _ticNbr;

    void print_core(const char*, va_list);
    void println_core(const char*, va_list);
    void printk_core(const char *s, va_list ap);
    void printBlock(const char *msg, u8 posX, u8 colors = 0x0E);
	
protected:
    Screen() : _posX(0), _posY(0), _colors(0x0E), _showCursor(false), _isLoading(false), _ticNbr(0) { _inst = this; }

	void scrollup(u8);
    u8 _posX, _posY;
	void move_cursor(u8, u8);
};

#endif
