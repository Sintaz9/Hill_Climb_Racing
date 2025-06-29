// Coin.h
#ifndef COIN_H
#define COIN_H

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

class Coin {
public:
    enum class Type { NORMAL, GOLD, SPECIAL };

    Coin(b2World& world, const sf::Texture& texture, const b2Vec2& position, Type type = Type::NORMAL);
    ~Coin();

    void update();
    void draw(sf::RenderWindow& window) const;

    b2Vec2 getPosition() const;
    bool isCollected() const;
    void collect();
    int getValue() const;

private:
    b2Body* body;
    sf::Sprite sprite;
    bool collected;
    Type type;
    float animTime = 0.f;
    static constexpr float SCALE = 100.0f;
};

#endif // COIN_H