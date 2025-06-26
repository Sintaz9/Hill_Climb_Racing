#include "../include/Terrain.h"
#include <random>
#include <cmath>

constexpr float SCALE = 100.0f;

Terrain::Terrain() : groundBody(nullptr) {
    // Инициализация по умолчанию
}
void Terrain::generate(int pointCount, float baseY) {
    points.clear();
    std::mt19937 rng(std::random_device{}());
    
    const float stepX = 2.0f;
    std::uniform_real_distribution<float> baseStep(-0.6f, 0.6f);
    std::uniform_real_distribution<float> extraStep(-6.0f, 6.0f);
    std::uniform_real_distribution<float> spikeChance(0.f, 1.f);

    float y = baseY;
    points.reserve(pointCount);

    // Начальная платформа (5 точек)
    for (int i = 0; i < 5; ++i) {
        points.emplace_back(i * stepX, baseY);
    }

    for (int i = 5; i < pointCount; ++i) {
        float x = i * stepX;
        float t = static_cast<float>(i) / pointCount;
        float difficulty = std::pow(t, 2.0f);

        float spike = (spikeChance(rng) < 0.05f + 0.1f * difficulty) 
            ? extraStep(rng) * (0.5f + 2.0f * difficulty) : 0.f;
        
        float deltaY = baseStep(rng) + spike;
        y += deltaY;
        y = std::clamp(y, baseY - 30.f, baseY + 35.f);
        
        points.emplace_back(x, y);
    }
}
void Terrain::createPhysics(b2World& world) {
    if (points.empty()) return;

    b2BodyDef groundDef;
    groundBody = world.CreateBody(&groundDef);

    b2FixtureDef groundFixture;
    groundFixture.friction = 0.9f;

    for (size_t i = 0; i < points.size() - 1; ++i) {
        b2EdgeShape edge;
        edge.SetTwoSided(points[i], points[i + 1]);
        groundFixture.shape = &edge;
        groundBody->CreateFixture(&groundFixture);
    }
}

sf::VertexArray Terrain::createMesh(const sf::Texture& texture) {
    sf::VertexArray mesh(sf::TriangleStrip);
    float textureRepeat = 10.0f; // Частота повторения текстуры

    for (size_t i = 0; i < points.size(); ++i) {
        float x = points[i].x * SCALE;
        float yTop = points[i].y * SCALE;

        // Вершина сверху (поверхность)
        mesh.append(sf::Vertex(
            sf::Vector2f(x, yTop),
            sf::Vector2f(x / textureRepeat, 0)
        ));

        // Вершина снизу (дно)
        mesh.append(sf::Vertex(
            sf::Vector2f(x, 10000.0f), // Достаточно большое значение для "дна"
            sf::Vector2f(x / textureRepeat, texture.getSize().y)
        ));
    }

    return mesh;
}

const std::vector<b2Vec2>& Terrain::getPoints() const {
    return points;
}