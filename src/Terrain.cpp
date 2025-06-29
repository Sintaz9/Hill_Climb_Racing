#include "../include/Terrain.h"
#include <random>
#include <algorithm>
#include <cmath>

constexpr float SCALE = 100.0f;
constexpr float TERRAIN_STEP = 2.0f;
constexpr int TERRAIN_POINTS = 750;
constexpr float BASE_Y = 12.0f;

Terrain::Terrain(b2World& world, const sf::Texture& groundTexture)
    : groundTexture(groundTexture) {
    generateTerrain();
    createMesh();
    createPhysics(world);
}

b2Vec2 Terrain::calculateSmoothPoint(size_t i, float t) {
    // Используем кубическую интерполяцию для плавных кривых
    if (i == 0 || i >= terrainPoints.size() - 2) {
        return terrainPoints[i];
    }

    const b2Vec2& p0 = terrainPoints[i - 1];
    const b2Vec2& p1 = terrainPoints[i];
    const b2Vec2& p2 = terrainPoints[i + 1];
    const b2Vec2& p3 = terrainPoints[i + 2];

    // Кубическая интерполяция
    float t2 = t * t;
    float t3 = t2 * t;
    float x = 0.5f * ((2.0f * p1.x) +
        (-p0.x + p2.x) * t +
        (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
        (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3);

    float y = 0.5f * ((2.0f * p1.y) +
        (-p0.y + p2.y) * t +
        (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 +
        (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3);

    return b2Vec2(x, y);
}

void Terrain::generateTerrain() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> baseStep(-0.4f, 0.4f);
    std::uniform_real_distribution<float> extraStep(-3.0f, 3.0f);
    std::uniform_real_distribution<float> spikeChance(0.f, 1.f);

    terrainPoints.clear();
    terrainPoints.reserve(TERRAIN_POINTS);

    // Начальная платформа
    for (int i = 0; i < 20; ++i) {
        terrainPoints.emplace_back(i * TERRAIN_STEP, BASE_Y);
    }

    float y = BASE_Y;
    for (int i = 20; i < TERRAIN_POINTS; ++i) {
        float x = i * TERRAIN_STEP;
        float t = static_cast<float>(i) / TERRAIN_POINTS;
        float difficulty = std::pow(t, 2.0f);

        float spike = (spikeChance(rng) < 0.05f + 0.1f * difficulty)
            ? extraStep(rng) * (0.5f + 2.0f * difficulty) : 0.f;

        y += std::clamp(baseStep(rng) + spike, -2.5f, 2.5f);
        y = std::clamp(y, BASE_Y - 20.f, BASE_Y + 25.f);

        terrainPoints.emplace_back(x, y);
    }

    finishLineX = terrainPoints.back().x - 10.0f;
}

void Terrain::createMesh() {
    groundMesh.setPrimitiveType(sf::TriangleStrip);
    const float bottomY = 5000.0f; // Дно мира

    // Добавляем дополнительные точки для сглаживания
    const int SMOOTH_STEPS = 5;
    for (size_t i = 1; i + 2 < terrainPoints.size(); ++i) {
        for (int j = 0; j < SMOOTH_STEPS; ++j) {
            float t = static_cast<float>(j) / SMOOTH_STEPS;
            b2Vec2 pt = calculateSmoothPoint(i, t);

            float x = pt.x * SCALE;
            float y = pt.y * SCALE;

            // Верхняя точка
            groundMesh.append(sf::Vertex(
                sf::Vector2f(x, y),
                sf::Vector2f(x / 100.f, 0)
            ));

            // Нижняя точка
            groundMesh.append(sf::Vertex(
                sf::Vector2f(x, bottomY),
                sf::Vector2f(x / 100.f, (bottomY - y) / 100.f)
            ));
        }
    }

    groundState.texture = &groundTexture;
}

void Terrain::createPhysics(b2World& world) {
    b2BodyDef groundDef;
    b2Body* groundBody = world.CreateBody(&groundDef);

    b2FixtureDef fixtureDef;
    fixtureDef.friction = 0.9f;

    for (size_t i = 0; i + 1 < terrainPoints.size(); ++i) {
        b2EdgeShape edge;
        edge.SetTwoSided(terrainPoints[i], terrainPoints[i + 1]);
        fixtureDef.shape = &edge;
        groundBody->CreateFixture(&fixtureDef);
    }
}

void Terrain::draw(sf::RenderWindow& window) const {
    window.draw(groundMesh, groundState);
}

const std::vector<b2Vec2>& Terrain::getPoints() const {
    return terrainPoints;
}