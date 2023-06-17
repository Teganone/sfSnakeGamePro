#pragma once

#include <SFML/Graphics.hpp>
#include <deque>

#include "screen/Screen.h"
#include "element/Snake.h"
#include "element/Fruit.h"
#include "element/Grid.h"
#include "element/Button.h"
#include "element/Barrier.h"

namespace sfSnake
{
    class GameScreen : public Screen
    {
    public:
        GameScreen();

        void handleInput(sf::RenderWindow &window) override;
        void update(sf::Time delta) override;
        void render(sf::RenderWindow &window) override;

        void generateFruit();
        void generateBarrier();
        
        void pause();
        void resume();
        bool isPaused();

        static sf::Clock barrierTimer;
        static sf::Time barrierSpawnInterval_;   // 障碍物生成时间间隔
        static sf::Clock LastEatTimer;
        static sf::Time MaxLastEatInterval_;  
    private:
        Snake snake_;
        std::deque<Fruit> fruit_;
        std::deque<Barrier> barrier_;
        Grid grid_;

        Button pauseButton_;

        sf::Text score_;
        sf::Text difficultyMode_;
        sf::Text timer_;
        
        // static sf::Clock GameTimer;
        // static sf::Time GameTimerPausedTime;
        // static sf::Clock barrierTimer;
        // static sf::Time barrierSpawnInterval_;   // 障碍物生成时间间隔
 
        bool paused_;
    };
}