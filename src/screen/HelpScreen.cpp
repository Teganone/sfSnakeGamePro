#include <SFML/Graphics.hpp>

#include <memory>
#include <iostream>

#include "screen/GameScreen.h"
#include "screen/HelpScreen.h"
#include "screen/OptionScreen.h"
#include "Game.h"

using namespace sfSnake;

HelpScreen::HelpScreen()
{ 
    if(!Game::ifShowedHelp) Game::ifShowedHelp = true;

    text_.setFont(Game::GlobalFont);
    text_.setString( 
        sf::String(L"游戏中，按[w/a/s/d]或者[上下左右]键切换方向\n") +
        sf::String(L"长按空格加速\n") +
        sf::String(L"或者点击、长按鼠标（左键或右键），指引蛇的移动\n") +
        sf::String(L"屏幕上有5种不同颜色的水果(圆形)，它们分别对应于得分：\n") +
        sf::String(L"\t黑色、棕色（不得分），红色（3分），蓝色（2分），绿色（1分）\n") +
        sf::String(L"屏幕上有5种不同颜色的屏障(矩形)，撞到它们都会死亡，并且根据游戏难度模式会间隔时间自动生成\n") +
        sf::String(L"\t吃一个黑色、棕色水果可以等效消除一个屏障\n") +
        sf::String(L"游戏共分为简单、中等、困难三种难度:\n") +
        sf::String(L"\t简单：蛇移动速度最慢，屏障每隔3s自动生成一个，贪吃蛇在30s内没有吃到水果会死亡\n") +
        sf::String(L"\t中等：蛇移动速度中等，屏障每隔2s自动生成一个，贪吃蛇在10s内没有吃到水果会死亡\n") +
        sf::String(L"\t困难：蛇移动速度最快，屏障每隔1s自动生成一个，贪吃蛇在5s内没有吃到水果会死亡\n") +
        sf::String(L"点击设置按钮，设置网格开关、网格颜色、背景颜色、难度模式"));
    text_.setCharacterSize(Game::GlobalVideoMode.width / 50.0f);
    text_.setFillColor(Game::Color::Green);
    setOriginMiddle(text_);
    text_.setPosition(Game::GlobalVideoMode.width / 2.0, Game::GlobalVideoMode.height / 5.0 * 3.0);

    returnButton_.update("assets/image/returnUI.png", 1 / 16.0f);
    returnButton_.setPosition(Game::GlobalVideoMode.width / 15.0f, Game::GlobalVideoMode.width / 15.0f);
}

void HelpScreen::handleInput(sf::RenderWindow &window)
{
    auto mousePosition = sf::Mouse::getPosition(window);

    returnButton_.focused(false);
    if (returnButton_.contain(mousePosition))
    {
        returnButton_.focused(true);
        if (!Game::mouseButtonLocked && sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            Game::mouseButtonCDtime = sf::Time::Zero;
            Game::mouseButtonLocked = true;
            Game::MainScreen = Game::TmpScreen;
            Game::TmpScreen = nullptr;
            return;
        }
    }
}

void HelpScreen::update(sf::Time delta)
{
    Game::GlobalTitle.update(delta);
}

void HelpScreen::render(sf::RenderWindow &window)
{
    Game::GlobalTitle.render(window);
    returnButton_.render(window);
    window.draw(text_);
}