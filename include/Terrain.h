// Terrain.h
#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <vector>

class Terrain {
public:
    Terrain(b2World& world, const sf::Texture& groundTexture);

    // Удаляем копирование и присваивание
    Terrain(const Terrain&) = delete;
    Terrain& operator=(const Terrain&) = delete;

    // Разрешаем перемещение
    Terrain(Terrain&&) = default;
    Terrain& operator=(Terrain&&) = default;

    void draw(sf::RenderWindow& window) const;
    const std::vector<b2Vec2>& getPoints() const;
    float getFinishLineX() const { return finishLineX; }

private:
    void generateTerrain();
    void createMesh();
    void createPhysics(b2World& world);
    b2Vec2 calculateSmoothPoint(size_t i, float t);

    std::vector<b2Vec2> terrainPoints;
    sf::VertexArray groundMesh;
    sf::RenderStates groundState;
    const sf::Texture& groundTexture;
    float finishLineX;
};