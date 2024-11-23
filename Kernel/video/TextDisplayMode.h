#pragma once

#include <video/Screen.h>
#include <utils/types.h>

namespace kernel::video {
    class TextDisplayMode final : public Screen
    {
    public:
        void putcar(uint8_t c) override;

    private:
        static constexpr auto COLOR_VGA_BASE = 0xB8000;

        friend class Screen;
        friend int main(mb_partial_info*);

        explicit TextDisplayMode(const VbeModeInfo *info) : _frameBuffer(reinterpret_cast<uint8_t *>(COLOR_VGA_BASE))
        {
            _maxX = info->XResolution;
            _maxY = info->YResolution;
        }

        uint8_t * const _frameBuffer;
    };
}
