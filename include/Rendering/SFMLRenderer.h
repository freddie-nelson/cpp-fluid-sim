#pragma once

#include "./Renderer.h"

#include <SFML/Graphics.hpp>

namespace Rendering
{

    class SFMLRenderer : public Renderer
    {
    public:
        SFMLRenderer(std::string windowTitle, int windowWidth, int windowHeight);
        ~SFMLRenderer();

        int init();
        void destroy();

        bool pollEvents();

        void clear();
        void present();

        void line(glm::vec2 start, glm::vec2 end, const Color &color);
        void circle(const Circle &circle, const Color &color, RenderType renderType = RenderType::FILL);
        void rect(const Rect &rect, const Color &color, RenderType renderType = RenderType::FILL);
        void polygon(const std::vector<glm::vec2> &vertices, const Color &color, RenderType renderType = RenderType::FILL);

    private:
        sf::RenderWindow *window;
    };
}