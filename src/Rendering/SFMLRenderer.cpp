#include "../../include/Rendering/SFMLRenderer.h"

#include <iostream>
#include <math.h>

Rendering::SFMLRenderer::SFMLRenderer(std::string windowTitle, int windowWidth, int windowHeight) : Renderer(windowTitle, windowWidth, windowHeight)
{
}

Rendering::SFMLRenderer::~SFMLRenderer()
{
    destroy();
}

int Rendering::SFMLRenderer::init()
{
    window = new sf::RenderWindow(sf::VideoMode(windowWidth, windowHeight), windowTitle);

    return 0;
}

void Rendering::SFMLRenderer::destroy()
{
    window->close();
    delete window;
}

bool Rendering::SFMLRenderer::pollEvents()
{
    sf::Event event;
    while (window->pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            return true;
        }
    }

    return false;
}

void Rendering::SFMLRenderer::clear()
{
    window->clear();
}

void Rendering::SFMLRenderer::present()
{
    window->display();
}

void Rendering::SFMLRenderer::line(glm::vec2 start, glm::vec2 end, const Color &color)
{
    sf::Color c(color.r, color.g, color.b, color.a);

    sf::Vertex line[] = {
        sf::Vertex(sf::Vector2f(start.x, start.y), c),
        sf::Vertex(sf::Vector2f(end.x, end.y), c),
    };

    window->draw(line, 2, sf::Lines);
}

void Rendering::SFMLRenderer::circle(const Circle &circle, const Color &color, RenderType renderType = RenderType::FILL)
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

void Rendering::SFMLRenderer::rect(const Rect &rect, const Color &color, RenderType renderType = RenderType::FILL)
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

void Rendering::SFMLRenderer::polygon(const std::vector<glm::vec2> &vertices, const Color &color, RenderType renderType = RenderType::FILL)
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