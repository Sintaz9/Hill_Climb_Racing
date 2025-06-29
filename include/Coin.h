#ifndef COIN_H
#define COIN_H

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

class Coin {
public:
    Coin(b2World& world, const sf::Texture& texture, const b2Vec2& position);
    ~Coin();

    void update(float dt);
    void draw(sf::RenderWindow& window) const;

    b2Vec2 getPosition() const;
    bool isCollected() const;
    void collect();
    int getValue() const { return 1; } // Количество очков за сбор


private:
    b2Body* body;       // Физическое тело Box2D
    sf::Sprite sprite;  // Графическое представление
    bool collected;     // Флаг сбора монеты
    float animTime = 0.f; // Таймер анимации
    static constexpr float SCALE = 100.0f; // Пикселей в метре
    static constexpr float RADIUS = 0.3f;  // Физический радиус

};

#endif // COIN_H