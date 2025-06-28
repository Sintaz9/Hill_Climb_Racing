#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <vector>

class Terrain {
public:
    Terrain(b2World& world, const sf::Texture& groundTexture);

    void draw(sf::RenderWindow& window) const;
    const std::vector<b2Vec2>& getPoints() const;

    // Удаляем копирование
    Terrain(const Terrain&) = delete;
    Terrain& operator=(const Terrain&) = delete;

private:
    void generateTerrain();
    void createMesh();
    void createPhysics(b2World& world);

    std::vector<b2Vec2> terrainPoints;
    sf::VertexArray groundMesh;
    sf::RenderStates groundState;
    const sf::Texture& groundTexture;
};