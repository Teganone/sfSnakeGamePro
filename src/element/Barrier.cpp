#include <SFML/Graphics.hpp>

#include "Barrier.h"
#include "Game.h"

using namespace sfSnake;

Barrier::Barrier(sf::Vector2f position, sf::Color color, int size)
    : size_(size)
{
    shape_.setSize(sf::Vector2f((size+5)*(Game::GlobalVideoMode.width / 256.0f),(size+5)*(Game::GlobalVideoMode.width / 256.0f)));
    // shape_.setRadius((size+1)*(Game::GlobalVideoMode.width / 256.0f));
    setOriginMiddle(shape_);
    shape_.setPosition(position);
    shape_.setFillColor(color);
}

void Barrier::render(sf::RenderWindow &window)
{
    window.draw(shape_);
}
