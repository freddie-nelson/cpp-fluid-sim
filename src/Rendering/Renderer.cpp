#include "../../include/Rendering/Renderer.h"
#include "../../include/Utility/InputCodes.h"

#include <iostream>
#include <math.h>

Rendering::Renderer::Renderer(std::string windowTitle, int windowWidth, int windowHeight) : windowTitle(windowTitle), windowWidth(windowWidth), windowHeight(windowHeight)
{
}

Rendering::Renderer::~Renderer()
{
    destroy();
}

int Rendering::Renderer::init()
{
    window = new sf::RenderWindow(sf::VideoMode(windowWidth, windowHeight), windowTitle);

    return 0;
}

void Rendering::Renderer::destroy()
{
    window->close();
    delete window;
}

bool Rendering::Renderer::pollEvents()
{
    sf::Event event;
    while (window->pollEvent(event))
    {
        void *data = nullptr;
        RendererEventType type;

        switch (event.type)
        {
        case sf::Event::Closed:
            emit(RendererEventType::WINDOW_CLOSE, RendererEvent{RendererEventType::WINDOW_CLOSE, nullptr});
            return true;

        case sf::Event::Resized:
            type = RendererEventType::WINDOW_RESIZE;
            data = new glm::vec2(event.size.width, event.size.height);
            break;

        case sf::Event::MouseMoved:
            type = RendererEventType::MOUSE_MOVE;
            data = new glm::vec2(event.mouseMove.x, event.mouseMove.y);
            break;

        case sf::Event::MouseButtonPressed:
        case sf::Event::MouseButtonReleased:
            type = event.type == sf::Event::MouseButtonPressed ? RendererEventType::MOUSE_DOWN : RendererEventType::MOUSE_UP;

            if (event.mouseButton.button == sf::Mouse::Left)
            {
                data = new Utility::MouseButton(Utility::MouseButton::MOUSE_LEFT);
            }
            else if (event.mouseButton.button == sf::Mouse::Right)
            {
                data = new Utility::MouseButton(Utility::MouseButton::MOUSE_RIGHT);
            }
            else if (event.mouseButton.button == sf::Mouse::Middle)
            {
                data = new Utility::MouseButton(Utility::MouseButton::MOUSE_MIDDLE);
            }
            else
            {
                data = new Utility::MouseButton(Utility::MouseButton::MOUSE_UNKNOWN);
            }

            break;

        case sf::Event::KeyPressed:
        case sf::Event::KeyReleased:
            type = event.type == sf::Event::KeyPressed ? RendererEventType::KEY_DOWN : RendererEventType::KEY_UP;
            data = new Utility::KeyCode(static_cast<Utility::KeyCode>(event.key.code));
            break;
        }

        emit(type, RendererEvent{type, data});
    }

    return false;
}

void Rendering::Renderer::clear()
{
    window->clear();
}

void Rendering::Renderer::present()
{
    window->display();
}

void Rendering::Renderer::line(glm::vec2 start, glm::vec2 end, const Color &color)
{
    sf::Color c(color.r, color.g, color.b, color.a);

    sf::Vertex line[] = {
        sf::Vertex(sf::Vector2f(start.x, start.y), c),
        sf::Vertex(sf::Vector2f(end.x, end.y), c),
    };

    window->draw(line, 2, sf::Lines);
}

void Rendering::Renderer::circle(const Circle &circle, const Color &color, RenderType renderType)
{
    sf::CircleShape shape(circle.radius);
    shape.setPointCount(50);
    shape.setPosition(circle.centre.x - circle.radius, circle.centre.y - circle.radius);

    sf::Color c(color.r, color.g, color.b, color.a);
    if (renderType == RenderType::FILL)
    {
        shape.setFillColor(c);
    }
    else if (renderType == RenderType::STROKE)
    {
        shape.setOutlineColor(sf::Color(color.r, color.g, color.b, color.a));
        shape.setOutlineThickness(1);
        shape.setFillColor(sf::Color::Transparent);
    }

    window->draw(shape);
};

void Rendering::Renderer::rect(const Rect &rect, const Color &color, RenderType renderType)
{
    sf::RectangleShape shape(sf::Vector2f(rect.w, rect.h));
    shape.setPosition(rect.topLeft.x, rect.topLeft.y);

    sf::Color c(color.r, color.g, color.b, color.a);
    if (renderType == RenderType::FILL)
    {
        shape.setFillColor(c);
    }
    else if (renderType == RenderType::STROKE)
    {
        shape.setOutlineColor(sf::Color(color.r, color.g, color.b, color.a));
        shape.setOutlineThickness(1);
        shape.setFillColor(sf::Color::Transparent);
    }

    window->draw(shape);
};

void Rendering::Renderer::polygon(const std::vector<glm::vec2> &vertices, const Color &color, RenderType renderType)
{
    sf::ConvexShape shape(vertices.size());

    sf::Color c(color.r, color.g, color.b, color.a);
    if (renderType == RenderType::FILL)
    {
        shape.setFillColor(c);
    }
    else if (renderType == RenderType::STROKE)
    {
        shape.setOutlineColor(sf::Color(color.r, color.g, color.b, color.a));
        shape.setOutlineThickness(1);
        shape.setFillColor(sf::Color::Transparent);
    }

    for (int i = 0; i < vertices.size(); i++)
    {
        shape.setPoint(i, sf::Vector2f(vertices[i].x, vertices[i].y));
    }

    window->draw(shape);
};