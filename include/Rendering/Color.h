#pragma once

namespace Rendering
{

    struct Color
    {
        int r;
        int g;
        int b;
        int a;
    };

    Color blend(const Color &bg, const Color &fg);
}