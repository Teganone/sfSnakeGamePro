#include <SFML/Graphics.hpp>

#include <memory>
#include <iostream>

#include "element/Snake.h"
#include "Game.h"
#include "element/Fruit.h"

#include "screen/GameOverScreen.h"

using namespace sfSnake;

const int Snake::InitialSize = 5;
static const float thresholdAngle = 15.f;
static const float thresholdCosine = std::cos((180.f - thresholdAngle) * 3.14159265f / 180.0f);

Snake::Snake()
    : hitSelf_(false),
      speedup_(false),
      hitBarrier_(false),
      die_(false),
      direction_(Direction(0, -1)),
      nodeRadius_(Game::GlobalVideoMode.width / 100.0f),
      tailOverlap_(0u),
      nodeShape(nodeRadius_),
      nodeMiddle(sf::Vector2f(nodeRadius_ * std::sqrt(3), nodeRadius_)),
      score_(InitialSize),
      rmBarriersNum_(0)
{
    initNodes();

    nodeShape.setFillColor(sf::Color(0xf1c40fff));

    nodeMiddle.setFillColor(sf::Color(0x1c2833ff));

    setOriginMiddle(nodeShape);
    setOriginMiddle(nodeMiddle);

    headTexture.loadFromFile("assets/image/snakeHeadImage.png");
    headTexture.setSmooth(true);
    sf::Vector2u TextureSize = headTexture.getSize();
    float headScale = nodeRadius_ / TextureSize.y * 2.6;
    headSprite.setTexture(headTexture);
    headSprite.setScale(headScale, headScale);

    setOriginMiddle(headSprite);

    pickupBuffer_.loadFromFile("assets/sounds/pickup.wav");
    pickupSound_.setBuffer(pickupBuffer_);
    pickupSound_.setVolume(30);

    dieBuffer_.loadFromFile("assets/sounds/die.wav");
    dieSound_.setBuffer(dieBuffer_);
    dieSound_.setVolume(50);
    GameScreen::LastEatTimer.restart();
}

void Snake::initNodes()
{
    path_.push_back(SnakePathNode(
        Game::GlobalVideoMode.width / 2.0f,
        Game::GlobalVideoMode.height / 2.0f));
    for (int i = 1; i <= 10 * InitialSize; i++)
    {
        path_.push_back(SnakePathNode(
            Game::GlobalVideoMode.width / 2.0f -
                direction_.x * i * nodeRadius_ / 5.0,
            Game::GlobalVideoMode.height / 2.0f -
                direction_.y * i * nodeRadius_ / 5.0));
    }
}

void Snake::handleInput(sf::RenderWindow &window)
{
    if ( (direction_!=Direction(0,1)) &&
        (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::W)))
        direction_ = Direction(0, -1);
    else if ( (direction_!=Direction(0,-1)) &&
        (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::S)))
        direction_ = Direction(0, 1);
    else if ( (direction_!=Direction(1,0)) &&
        (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::A)))
        direction_ = Direction(-1, 0);
    else if ( (direction_!=Direction(-1,0)) &&
        (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::D)))
        direction_ = Direction(1, 0);

    static double directionSize;

    if (!Game::mouseButtonLocked)
    {
        if (
            sf::Mouse::isButtonPressed(sf::Mouse::Left) ||
            sf::Mouse::isButtonPressed(sf::Mouse::Right))
        {
            static sf::Vector2i MousePosition;
            MousePosition = sf::Mouse::getPosition(window);
            if (
                dis(MousePosition,
                    sf::Vector2f(
                        Game::GlobalVideoMode.width / 15.0f * 14.0f,
                        Game::GlobalVideoMode.width / 15.0f)) >
                Game::GlobalVideoMode.width / 16.0f)
            {
                // direction_ =
                //     static_cast<sf::Vector2f>(MousePosition) -
                //     toWindow(path_.front());
                Direction TargetDirection =
                    static_cast<sf::Vector2f>(MousePosition) -
                    toWindow(path_.front());
                float cos = angleCosine(direction_,TargetDirection);
                if(cos>thresholdCosine){
                    directionSize = length(TargetDirection);
                    direction_ = TargetDirection;
                    direction_.x /= directionSize;
                    direction_.y /= directionSize;
                }          
            }
        }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        speedup_ = true;
    else
        speedup_ = false;
}

void Snake::update(sf::Time delta)
{
    //超过时间
    // std::cout<<"update:"<< std::to_string(GameScreen::LastEatTimer.getElapsedTime().asSeconds()) << std::endl;
    // std::cout<<"max:"<<std::to_string(GameScreen::MaxLastEatInterval_.asSeconds())<<std::endl;
    if(GameScreen::LastEatTimer.getElapsedTime()>=GameScreen::MaxLastEatInterval_){
        dieSound_.play();
        sf::sleep(sf::seconds(dieBuffer_.getDuration().asSeconds()));
        die_ = true; 
    
    }
    move();
    static int count = 0;
    if (++count >= 40)
    {
        checkOutOfWindow();
        count -= 40;
    }
    checkSelfCollisions();
}

void Snake::checkFruitCollisions(std::deque<Fruit> &fruits)
{
    auto toRemove = fruits.end();
    SnakePathNode headnode = path_.front();

    for (auto i = fruits.begin(); i != fruits.end(); ++i)
    {
        if (dis(
                i->shape_.getPosition(), toWindow(headnode)) <
            nodeRadius_ + i->shape_.getRadius())
            toRemove = i;
    }

    if (toRemove != fruits.end())
    {
        pickupSound_.play();
        grow(toRemove->score_);
        if(toRemove->score_==0){
            rmBarriersNum_ += 1;
        }
        fruits.erase(toRemove);
    }
}


void Snake::checkBarrierCollisions(std::deque<Barrier> &barriers)
{
    for(auto i=0;i<rmBarriersNum_ && i<barriers.size();i++){
        barriers.pop_front();
        // std::cout<<"pop barrier"<<std::endl;
    }
    rmBarriersNum_ = 0;
    auto toRemove = barriers.end();
    SnakePathNode headnode = path_.front();

    for (auto i = barriers.begin(); i != barriers.end(); ++i)
    {   
        // if (dis(
        //         i->shape_.getPosition(), toWindow(headnode)) <
        //     nodeRadius_ + std::max(i->shape_.getGlobalBounds().width, i->shape_.getGlobalBounds().height))
        //     toRemove = i;
        sf::FloatRect bounds = i->shape_.getGlobalBounds();
        if (bounds.left < headnode.x + nodeRadius_ &&
            bounds.left + bounds.width > headnode.x - nodeRadius_ &&
            bounds.top < headnode.y + nodeRadius_ &&
            bounds.top + bounds.height > headnode.y - nodeRadius_)
        {
            toRemove = i;
        }
    }

    if (toRemove != barriers.end())
    {
        // pickupSound_.play();
        dieSound_.play();
        sf::sleep(sf::seconds(dieBuffer_.getDuration().asSeconds()));
        hitBarrier_ = true;
        // grow(toRemove->_);
        // barriers.erase(toRemove);
    }
}

bool Snake::isDead() const{
    return hitSelf_ || hitBarrier_ || die_;
}
void Snake::grow(int score)
{
    GameScreen::LastEatTimer.restart();
    tailOverlap_ += score * 10;
    score_ += score;
}

unsigned Snake::getScore() const
{
    return score_;
}

unsigned Snake::getRmBarriersNum() const
{
    return rmBarriersNum_;
}

bool Snake::hitSelf() const
{
    return hitSelf_;
}

bool Snake::hitBarrier() const
{
    return hitBarrier_;
}

void Snake::move()
{
    SnakePathNode &headNode = path_.front();
    int times = speedup_ ? 2 : 1;
    for (int i = 1; i <= times*Game::speedMode[Game::DifficultyMode]; i++)
    {
        path_.push_front(SnakePathNode(
            headNode.x + direction_.x * i * nodeRadius_ / 5.0,
            headNode.y + direction_.y * i * nodeRadius_ / 5.0));
        if (tailOverlap_)
            tailOverlap_--;
        else
            path_.pop_back();
    }
}

void Snake::checkSelfCollisions()
{
    SnakePathNode head = toWindow(path_.front());
    int count = 0;

    for (auto i = path_.begin(); i != path_.end(); i++, count++)
        // if (count >= 30 && dis(head, toWindow(*i)) < 2.0f * nodeRadius_)
        if (count >= 30 && dis(head, toWindow(*i)) < 1.0f * nodeRadius_)
        {
            dieSound_.play();
            sf::sleep(sf::seconds(dieBuffer_.getDuration().asSeconds()));
            hitSelf_ = true;
            break;
        }
}

void Snake::checkOutOfWindow()
{
    auto inWindow = [](SnakePathNode &node) -> bool
    {
        return node.x >= 0 &&
               node.x <= Game::GlobalVideoMode.width &&
               node.y >= 0 &&
               node.y <= Game::GlobalVideoMode.height;
    };
    bool OutOfWindow = true;
    for (auto i : path_)
    {
        if (inWindow(i))
            OutOfWindow = false;
    }
    if (OutOfWindow)
    {
        SnakePathNode &tail = path_.back();
        if (tail.x < 0)
            for (auto &i : path_)
                i.x = i.x + Game::GlobalVideoMode.width;
        else if (tail.x > Game::GlobalVideoMode.width)
            for (auto &i : path_)
                i.x = i.x - Game::GlobalVideoMode.width;
        else if (tail.y < 0)
            for (auto &i : path_)
                i.y = i.y + Game::GlobalVideoMode.height;
        else if (tail.y > Game::GlobalVideoMode.height)
            for (auto &i : path_)
                i.y = i.y - Game::GlobalVideoMode.height;
    }
}

SnakePathNode Snake::toWindow(SnakePathNode node)
{
    while (node.x < 0)
        node.x = node.x + Game::GlobalVideoMode.width;
    while (node.x > Game::GlobalVideoMode.width)
        node.x = node.x - Game::GlobalVideoMode.width;
    while (node.y < 0)
        node.y = node.y + Game::GlobalVideoMode.height;
    while (node.y > Game::GlobalVideoMode.height)
        node.y = node.y - Game::GlobalVideoMode.height;
    return node;
}

void Snake::render(sf::RenderWindow &window)
{
    static int count;
    static SnakePathNode lastSnakeNode, lastMiddleNode, nowSnakeNode;
    static float angle;
    static sf::Vector2f recDirection;
    static SnakePathNode wNowHeadNode;

    lastSnakeNode = *path_.begin();
    wNowHeadNode = toWindow(lastSnakeNode);
    headSprite.setPosition(wNowHeadNode);
    recDirection = direction_;
    angle =
        std::acos(recDirection.y / length(recDirection)) /
        3.14159265358979323846 * 180.0;
    if (direction_.x > 0)
        angle = -angle;
    headSprite.setRotation(angle);

    renderNode(wNowHeadNode, headSprite, window, 3);

    count = 1;
    for (auto i = path_.begin() + 1; i != path_.end(); i++, count++)
    {
        if (count % 5 == 0)
        {
            if (count % 2)
                lastMiddleNode = *i;
            else
            {
                nowSnakeNode = *i;

                recDirection = nowSnakeNode - lastSnakeNode;
                angle =
                    std::acos(recDirection.y / length(recDirection)) /
                    3.14159265358979323846 * 180.0;
                if (recDirection.x > 0)
                    angle = -angle;
                nodeMiddle.setRotation(angle);

                static SnakePathNode wNowSnakeNode;
                static SnakePathNode wMiddleNode;
                wNowSnakeNode = toWindow(nowSnakeNode);
                wMiddleNode = toWindow(lastMiddleNode);
                renderNode(wNowSnakeNode, nodeShape, window, 0);
                renderNode(wMiddleNode, nodeMiddle, window, 0);

                lastSnakeNode = nowSnakeNode;
            }
        }
    }
}

template <typename T>
void Snake::renderNode(sf::Vector2f &nowPosition, T &shape, sf::RenderWindow &window, int offset)
{
    shape.setPosition(nowPosition);
    window.draw(shape);

    if (nowPosition.x <= nodeRadius_ + offset)
    {
        shape.setPosition(nowPosition + sf::Vector2f(Game::GlobalVideoMode.width, 0));
        window.draw(shape);
    }
    else if (nowPosition.x >= Game::GlobalVideoMode.width - nodeRadius_ - offset)
    {
        shape.setPosition(nowPosition - sf::Vector2f(Game::GlobalVideoMode.width, 0));
        window.draw(shape);
    }

    if (nowPosition.y <= nodeRadius_ + offset)
    {
        shape.setPosition(nowPosition + sf::Vector2f(0, Game::GlobalVideoMode.height));
        window.draw(shape);
    }
    else if (nowPosition.y >= Game::GlobalVideoMode.height - nodeRadius_ - offset)
    {
        shape.setPosition(nowPosition - sf::Vector2f(0, Game::GlobalVideoMode.height));
        window.draw(shape);
    }

    if (nowPosition.x <= nodeRadius_ + offset && nowPosition.y <= nodeRadius_ + offset)
    {
        shape.setPosition(nowPosition + sf::Vector2f(Game::GlobalVideoMode.width, Game::GlobalVideoMode.height));
        window.draw(shape);
    }
    else if (nowPosition.x >= Game::GlobalVideoMode.width - nodeRadius_ - offset && nowPosition.y >= Game::GlobalVideoMode.height - nodeRadius_ - offset)
    {
        shape.setPosition(nowPosition - sf::Vector2f(Game::GlobalVideoMode.width, Game::GlobalVideoMode.height));
        window.draw(shape);
    }
}