#include <SFML/Graphics.hpp>

#include <random>
#include <memory>
#include <iostream>

#include "screen/GameScreen.h"
#include "screen/GameOverScreen.h"
#include "screen/PauseScreen.h"
#include "screen/HelpScreen.h"

using namespace sfSnake;

// sf::Clock GameScreen::GameTimer = sf::Clock();
// sf::Time  GameScreen::GameTimerPausedTime = sf::Time::Zero;
sf::Clock GameScreen::barrierTimer = sf::Clock();
sf::Time  GameScreen::barrierSpawnInterval_ = sf::seconds(Game::BarrierGeneratedTimesMode[Game::DifficultyMode]);
sf::Clock GameScreen::LastEatTimer = sf::Clock();
sf::Time  GameScreen::MaxLastEatInterval_ = sf::seconds(Game::MaxLastEatIntervalMode[Game::DifficultyMode]);


GameScreen::GameScreen()
    : snake_(), grid_(), pauseButton_(), paused_(false)
{
    barrierSpawnInterval_ = sf::seconds(Game::BarrierGeneratedTimesMode[Game::DifficultyMode]);
    MaxLastEatInterval_ = sf::seconds(Game::MaxLastEatIntervalMode[Game::DifficultyMode]);
    pauseButton_.update("assets/image/pauseUI.png", 1 / 16.0f);
    pauseButton_.setPosition(
        Game::GlobalVideoMode.width / 15.0 * 14.0,
        Game::GlobalVideoMode.width / 15.0);

    score_.setFont(Game::GlobalFont);
    score_.setString(sf::String(L"分数:") + std::to_string(snake_.getScore()));
    score_.setCharacterSize(Game::GlobalVideoMode.width / 25.0f);
    score_.setFillColor(Game::Color::Yellow);
    setOriginMiddle(score_);
    score_.setPosition(
        Game::GlobalVideoMode.width / 3.0f,
        Game::GlobalVideoMode.width * 0.05f);
    
    difficultyMode_.setFont(Game::GlobalFont);
    difficultyMode_.setString(sf::String(L"模式:\t") + (Game::DifficultyModeStr[Game::DifficultyMode]));
    difficultyMode_.setCharacterSize(Game::GlobalVideoMode.width / 25.0f);
    difficultyMode_.setFillColor(Game::Color::Yellow);
    setOriginMiddle(difficultyMode_);
    difficultyMode_.setPosition(
        Game::GlobalVideoMode.width / 3.f * 2.0f,
        Game::GlobalVideoMode.width * 0.05f);

    timer_.setFont(Game::GlobalFont);
    timer_.setString(sf::String(L"计时(secs):\t") + std::to_string(0.0f));
    timer_.setCharacterSize(Game::GlobalVideoMode.width / 40.0f);
    timer_.setFillColor(Game::Color::Yellow);
    setOriginMiddle(timer_);
    timer_.setPosition(
        Game::GlobalVideoMode.width / 2.0f,
        Game::GlobalVideoMode.width * 0.1f);
    // Game::GameScreenTimer.restart();
    Game::GameScreenTimerPausedTime = sf::Time::Zero;
}

void GameScreen::handleInput(sf::RenderWindow &window)
{
    snake_.handleInput(window);

    auto mousePosition = sf::Mouse::getPosition(window);

    pauseButton_.focused(false);

    if (pauseButton_.contain(mousePosition))
    {
        pauseButton_.focused(true);
        if (
            !Game::mouseButtonLocked &&
            sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            pause();
        
            Game::mouseButtonCDtime = sf::Time::Zero;
            Game::mouseButtonLocked = true;
            Game::TmpGameScreen = Game::MainScreen;
            Game::MainScreen = std::make_shared<PauseScreen>();
            return;
        }
    }
}

void GameScreen::update(sf::Time delta)
{
    if(!paused_){
        Game::GameScreenTimerPausedTime += delta;
    }
    while (fruit_.size() < 5){
        generateFruit();
    }
    if(
        // (barrier_.size() < Game::BarrierNumByMode[Game::DifficultyMode]) &&
    (barrierTimer.getElapsedTime()>=barrierSpawnInterval_)){
        barrierTimer.restart();
        generateBarrier();
    }
    
    snake_.update(delta);
    snake_.checkFruitCollisions(fruit_);
    snake_.checkBarrierCollisions(barrier_);
    // if (snake_.hitSelf() || snake_.hitBarrier())
    if(snake_.isDead())
    {
        Game::MainScreen = std::make_shared<GameOverScreen>(snake_.getScore());
    }

    score_.setString(sf::String(L"分数:\t") + std::to_string(snake_.getScore()));
    timer_.setString(sf::String(L"计时(secs):\t") + std::to_string(Game::GameScreenTimerPausedTime.asSeconds()));
    resume();
}

void GameScreen::render(sf::RenderWindow &window)
{
    if (Game::GridVisibility)
        grid_.render(window);
    snake_.render(window);
    for (auto barrier : barrier_)
        barrier.render(window);
    for (auto fruit : fruit_)
        fruit.render(window);
    pauseButton_.render(window);
    window.draw(score_);
    window.draw(difficultyMode_);
    window.draw(timer_);
}

void GameScreen::pause() 
{ 
    // Game::GameScreenTimerPausedTime += Game::GameScreenTimer.getElapsedTime();
    // Game::GameScreenTimer.restart();
    paused_=true; 
} 
void GameScreen::resume() 
{ 
    paused_=false;
    // Game::GameScreenTimerPausedTime += Game::GameScreenTimer.getElapsedTime();
    // sf::Time elapsed = Game::GameScreenTimer.getElapsedTime() - Game::GameScreenTimerPausedTime;
    // Game::GameScreenTimer.restart();
}
bool GameScreen::isPaused() { return paused_; }
void GameScreen::generateFruit()
{
    static std::default_random_engine engine(time(NULL));
    static std::default_random_engine colorEngine(time(NULL));

    static std::uniform_int_distribution<int> xPos(
        Game::GlobalVideoMode.width / 15.0f,
        Game::GlobalVideoMode.width -
            Game::GlobalVideoMode.width / 10.0f);

    static std::uniform_int_distribution<int> yPos(
        Game::GlobalVideoMode.width / 10.0f,
        Game::GlobalVideoMode.height -
            Game::GlobalVideoMode.width / 15.0f);

    static std::uniform_int_distribution<int> fruitColor(0, 7);

    switch (fruitColor(colorEngine))
    {
    case 0: // black
        fruit_.push_back(Fruit(
            sf::Vector2f(xPos(engine), yPos(engine)),
            Game::Color::Fruit[0],
            0));
        break;
    case 1: // brown
        fruit_.push_back(Fruit(
            sf::Vector2f(xPos(engine), yPos(engine)),
            Game::Color::Fruit[1],
            0));
        break;
    case 2:
    case 3: // red
        fruit_.push_back(Fruit(
            sf::Vector2f(xPos(engine), yPos(engine)),
            Game::Color::Fruit[2],
            3));
        break;
    case 4:
    case 5: // blue
        fruit_.push_back(Fruit(
            sf::Vector2f(xPos(engine), yPos(engine)),
            Game::Color::Fruit[3],
            2));
        break;
    case 6:
    case 7: // green
        fruit_.push_back(Fruit(
            sf::Vector2f(xPos(engine), yPos(engine)),
            Game::Color::Fruit[4],
            1));
        break;
    default:
        break;
    }
}


void GameScreen::generateBarrier()
{
    static std::default_random_engine engine(time(NULL));
    static std::default_random_engine colorEngine(time(NULL));

    static std::uniform_int_distribution<int> xPos(
        Game::GlobalVideoMode.width / 40.0f,
        Game::GlobalVideoMode.width -
            Game::GlobalVideoMode.width / 20.0f);

    static std::uniform_int_distribution<int> yPos(
        Game::GlobalVideoMode.width / 30.0f,
        Game::GlobalVideoMode.height -
            Game::GlobalVideoMode.width / 10.0f);

    static std::uniform_int_distribution<int> barrierColor(0, 7);

    switch (barrierColor(colorEngine))
    {
    case 0: // black
        barrier_.push_back(Barrier(
            sf::Vector2f(xPos(engine), yPos(engine)),
            Game::Color::Fruit[0],
            0));
        break;
    case 1: // brown
        barrier_.push_back(Barrier(
            sf::Vector2f(xPos(engine), yPos(engine)),
            Game::Color::Fruit[1],
            0));
        break;
    case 2:
    case 3: // red
        barrier_.push_back(Barrier(
            sf::Vector2f(xPos(engine), yPos(engine)),
            Game::Color::Fruit[2],
            3));
        break;
    case 4:
    case 5: // blue
        barrier_.push_back(Barrier(
            sf::Vector2f(xPos(engine), yPos(engine)),
            Game::Color::Fruit[3],
            2));
        break;
    case 6:
    case 7: // green
        barrier_.push_back(Barrier(
            sf::Vector2f(xPos(engine), yPos(engine)),
            Game::Color::Fruit[4],
            1));
        break;
    default:
        break;
    }
}