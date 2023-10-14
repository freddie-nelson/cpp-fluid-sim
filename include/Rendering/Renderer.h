#pragma once

#include "./Shapes/Circle.h"
#include "./Shapes/Rect.h"
#include "./Color.h"
#include "../Utility/EventEmitter.h"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace Rendering
{
    enum RenderType
    {
        STROKE,
        FILL
    };

    enum RendererEventType
    {
        WINDOW_CLOSE,
        WINDOW_RESIZE,
        MOUSE_MOVE,
        MOUSE_DOWN,
        MOUSE_UP,
        KEY_DOWN,
        KEY_UP,
    };

    struct RendererEvent
    {
        RendererEventType type;
        void *data;
    };

    /**
     * The renderer.
     *
     * This class is responsible for rendering shapes to the screen and handling window/input events.
     *
     * Events:
     * - WINDOW_CLOSE, data: nullptr
     * - WINDOW_RESIZE, data: glm::vec2
     * - MOUSE_MOVE, data: glm::vec2
     * - MOUSE_DOWN, data: Utility::MouseButton
     * - MOUSE_UP, data: Utility::MouseButton
     * - KEY_DOWN, data: Utility::KeyCode
     * - KEY_UP, data: Utility::KeyCode
     */
    class Renderer : public EventEmitter<RendererEventType, RendererEvent>
    {
    public:
        Renderer(std::string windowTitle, int windowWidth, int windowHeight);
        ~Renderer();

        int init();
        void destroy();

        /**
         * Polls events from the renderer.
         *
         * @return True if the application should exit.
         */
        bool pollEvents();

        void clear();
        void present();

        void line(glm::vec2 start, glm::vec2 end, const Color &color);
        void circle(const Circle &circle, const Color &color, RenderType renderType = RenderType::FILL);
        void rect(const Rect &rect, const Color &color, RenderType renderType = RenderType::FILL);

        /**
         * Draws a polygon with the given vertices.
         *
         * @param vertices The vertices of the polygon. (must be in clockwise order)
         * @param color The color of the polygon.
         */
        void polygon(const std::vector<glm::vec2> &vertices, const Color &color, RenderType renderType = RenderType::FILL);

    protected:
        std::string windowTitle;
        int windowWidth;
        int windowHeight;

        sf::RenderWindow *window;
    };

}