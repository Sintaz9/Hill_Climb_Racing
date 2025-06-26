#pragma once
#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <vector>

class Terrain {
public:
    Terrain();

    void generate(int pointCount, float baseY);
    void createPhysics(b2World& world);
    sf::VertexArray createMesh(const sf::Texture& texture);

    const std::vector<b2Vec2>& getPoints() const;

private:
    std::vector<b2Vec2> points;
    b2Body* groundBody;
};