#pragma once

#include <SFML/Graphics.hpp>

namespace sfSnake
{
    struct Barrier
    {
        sf::RectangleShape shape_;
        int size_;

        Barrier(sf::Vector2f position, sf::Color color, int size);

        void render(sf::RenderWindow &window);
    };
}