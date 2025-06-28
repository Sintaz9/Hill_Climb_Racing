#pragma once
#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

class Obstacle {
public:
    Obstacle(b2World& world, const sf::Texture& texture, const b2Vec2& position, float density);
    void update();
    void draw(sf::RenderWindow& window) const;
    b2Vec2 getPosition() const;
    b2Body* getBody() const;

private:
    b2Body* body;
    sf::Sprite sprite;
};