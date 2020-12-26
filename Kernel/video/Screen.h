#ifndef __SCREEN__
#define __SCREEN__

#include <cstdarg>
#include <utils/types.h>

#define sScreen Screen::getScreen()

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

enum Color_32
{
    Black32		= 0x000000,
    Blue32		= 0x0000FF,
    Green32		= 0x00FF00,
    SoftBlue32	= 0x0094FF,
    Red32       = 0xFF0000,
    Pink32		= 0xFF008E,
    Yellow32    = 0xFDFF00,
    White32		= 0xFFFFFF,
    Orange32    = 0xFF9F00
};

class Screen
{
public :
	static Screen& getScreen();
	
    virtual ~Screen() = default;

    virtual void putcar(uchar) = 0;
    virtual void clean() {}

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

    explicit Screen(VbeModeInfo *info) : _posX(0), _posY(0), _maxX(0), _maxY(0), _colors(0x0E), _showCursor(false),
								_isLoading(false), _ticNbr(0) { _inst = this; }

    static void initScreen(mb_partial_info*);
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
    Screen() : _posX(0), _posY(0), _maxX(0), _maxY(0), _colors(0x0E), _showCursor(false), _isLoading(false),
               _ticNbr(0) { _inst = this; }

    virtual void scrollup(u8);
    u8 _posX, _posY, _maxX, _maxY;
	void move_cursor(u8, u8);
};

#endif
