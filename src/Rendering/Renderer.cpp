#include "../../include/Rendering/Renderer.h"
#include "../../include/Utility/InputCodes.h"

#include <iostream>
#include <math.h>
#include <stdexcept>

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

    // init circle shader
    circlesShader = new sf::Shader();
    if (!circlesShader->loadFromFile("./Shaders/circles.vert", "./Shaders/circles.frag"))
    {
        std::cout << "Failed to load circle shader." << std::endl;
        return 1;
    }

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

    // setup pixelImage
    delete pixelImage;
    pixelImage = new sf::Image();
    pixelImage->create(windowWidth, windowHeight, sf::Color::Transparent);
}

void Rendering::Renderer::present()
{
    window->display();
}

void Rendering::Renderer::presentDrawnPixels()
{
    sf::Texture texture;
    texture.loadFromImage(*pixelImage);

    sf::Sprite pixelSprite;
    pixelSprite.setTexture(texture, true);

    window->draw(pixelSprite);
}

void Rendering::Renderer::pixel(glm::vec2 position, const Color &color)
{
    if (position.x < 0 || position.x >= windowWidth || position.y < 0 || position.y >= windowHeight)
    {
        throw std::invalid_argument("Pixel position out of bounds of window.");
    }

    pixelImage->setPixel(position.x, position.y, sf::Color(color.r, color.g, color.b, color.a));
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

void Rendering::Renderer::shaderCircles(Circle circles[], Color color[], int numCircles)
{
    int maxCircles = 500;
    int rendered = 0;

    while (rendered < numCircles)
    {
        int toRender = std::min(maxCircles, numCircles - rendered);

        sf::Glsl::Vec2 positions[toRender];
        sf::Glsl::Vec4 colors[toRender];

        for (int i = 0; i < toRender; i++)
        {
            int ci = rendered + i;

            positions[i] = sf::Glsl::Vec2(circles[ci].centre.x, circles[ci].centre.y);
            colors[i] = sf::Glsl::Vec4(color[ci].r / 255.0f, color[ci].g / 255.0f, color[ci].b / 255.0f, color[ci].a / 255.0f);
        }

        circlesShader->setUniform("u_Radius", circles[0].radius);
        circlesShader->setUniformArray("u_Circles", positions, toRender);
        circlesShader->setUniformArray("u_Colors", colors, toRender);
        circlesShader->setUniform("u_NumCircles", toRender);
        circlesShader->setUniform("u_Resolution", sf::Glsl::Vec2(windowWidth, windowHeight));

        sf::RectangleShape shape(sf::Vector2f(0, 0));
        window->draw(shape, circlesShader);

        rendered += toRender;
    }
};

void Rendering::Renderer::createButton(std::string text, glm::vec2 position, glm::vec2 size, std::function<void()> onClickCallback)
{
}

void Rendering::Renderer::createLabel(std::string text, glm::vec2 position, glm::vec2 size)
{
}

void Rendering::Renderer::createSlider(glm::vec2 position, glm::vec2 size, float min, float max, float value, std::function<void(float)> onChangeCallback)
{
}