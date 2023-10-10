#pragma once

#include "./Renderer.h"

#include <SDL2/SDL.h>

namespace Rendering
{

    /**
     * SDL implementation of the Renderer.
     *
     * The SDL Renderer will not draw filled shapes.
     */
    class SDLRenderer : public Renderer
    {
    public:
        SDLRenderer(std::string windowTitle, int windowWidth, int windowHeight);
        ~SDLRenderer();

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
        SDL_Renderer *renderer;
        SDL_Window *window;
    };
}