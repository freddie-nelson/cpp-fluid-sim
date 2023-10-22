#include "../../include/Rendering/Color.h"

#include <iostream>
#include <glm/glm.hpp>

Rendering::Color Rendering::blend(const Color &bg, const Color &fg)
{
    Color blend;

    if (fg.a >= 255)
    {
        blend = fg;
    }
    else if (fg.a <= 1.0e-6)
    {
        blend = bg;
    }
    else
    {
        // convert to 0 to 1 range
        float bgAlpha = bg.a / 255.0f;
        float fgAlpha = fg.a / 255.0f;
        float blendAlpha = 1 - (1 - fgAlpha) * (1 - bgAlpha);

        blend.a = static_cast<int>(blendAlpha * 255.0f);
        blend.r = static_cast<int>((fg.r * fgAlpha / blendAlpha) + (bg.r * bgAlpha * (1 - fgAlpha) / blendAlpha));
        blend.g = static_cast<int>((fg.g * fgAlpha / blendAlpha) + (bg.g * bgAlpha * (1 - fgAlpha) / blendAlpha));
        blend.b = static_cast<int>((fg.b * fgAlpha / blendAlpha) + (bg.b * bgAlpha * (1 - fgAlpha) / blendAlpha));
    }

    return blend;
}