#pragma once

#include <utils/types.h>
#include <cstdarg>

#include <core/kernel.h>

#define sScreen kernel::video::Screen::getScreen()

class String;

namespace kernel::video {
    enum Color {
        Black = 0x0,
        Blue = 0x1,
        Green = 0x2,
        SoftBlue = 0x3,
        Red = 0x4,
        Pink = 0x5,
        Yellow = 0x6,
        White = 0x7
    };

    enum Color_32 {
        Black32 = 0x000000,
        Blue32 = 0x0000FF,
        Green32 = 0x00FF00,
        SoftBlue32 = 0x0094FF,
        Red32 = 0xFF0000,
        Pink32 = 0xFF008E,
        Yellow32 = 0xFDFF00,
        White32 = 0xFFFFFF,
        Orange32 = 0xFF9F00
    };

    class Screen {
    public :
        static Screen &getScreen();

        virtual ~Screen() = default;

        virtual void putcar(uint8_t) = 0;

        virtual void clean() {
        }

        void print(const char *, ...);

        void println(const char *, ...);

        void printInfo(const char *, ...);

        void printDebug(const char *, ...);

        void printError(const char *, ...);

        void printk(const char *s, ...);

        void printk(const String *s, ...);

        void setColor(Color fgColor, Color bgColor);

        void setPos(uint8_t posX, uint8_t posY);

        [[nodiscard]] const uint8_t &getColor() const { return _colors; }

        [[nodiscard]] bool isLoading() const { return _isLoading; }

        void okMsg();

        void failMsg();

        void dump(const uint8_t *, int);

        void switchCursor();

        void show_cursor();

        void hide_cursor();

        void showLoadScreen();

        void showTic();

    private :
        friend int kernel::main(mb_partial_info *);

        explicit Screen(VbeModeInfo *) : _colors(SoftBlue), _showCursor(false), _isLoading(false), _ticNbr(0), _posX(0),
                                         _posY(0),
                                         _maxX(0), _maxY(0) { _inst = this; }

        static void initScreen(const mb_partial_info *);

        static Screen *_inst;

        uint8_t _colors;

        bool _showCursor;
        bool _isLoading;
        uint8_t _ticNbr;

        void print_core(const char *, va_list);

        void println_core(const char *, va_list);

        void printk_core(const char *s, va_list ap);

        void printBlock(const char *msg, uint8_t posX, uint8_t colors = 0x0E);

    protected:
        Screen() : _colors(SoftBlue), _showCursor(false), _isLoading(false), _ticNbr(0), _posX(0), _posY(0), _maxX(0),
                   _maxY(0) { _inst = this; }

        virtual void scrollup(uint8_t);

        uint8_t _posX, _posY, _maxX, _maxY;

        static void move_cursor(uint8_t, uint8_t);
    };
}
