#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include <vector>

class Obstacle {
public:
    Obstacle(b2World& world, const sf::Texture& texture, const b2Vec2& position, float density);
    void update();
    void draw(sf::RenderWindow& window) const;
    bool shouldBreak(b2Body* carBody) const { return false; } // Просто возвращаем false

    b2Vec2 getPosition() const {
        return mainBody->GetPosition();
    }

private:
    b2Body* mainBody;
    sf::Sprite sprite;
};

#endif // OBSTACLE_H