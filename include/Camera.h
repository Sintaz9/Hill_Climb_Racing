#pragma once
#include <SFML/Graphics.hpp>

class Camera {
public:
    Camera(sf::RenderWindow& window, float baseZoom = 1.5f, float zoomRange = 0.5f);

    void update(const sf::Vector2f& targetPos, float speedRatio, float dt);
    void applyToWindow();
    void setWorldBounds(const sf::FloatRect& bounds);
    void reset();
    void setZoom(float zoom); // Добавлен отсутствующий метод

    const sf::View& getView() const;
    float getZoom() const;

private:
    sf::RenderWindow& window;
    sf::View view;
    sf::FloatRect worldBounds;

    float baseZoom;
    float zoomRange;
    float currentZoom;
    float targetZoom;

    sf::Vector2f currentPosition;
    sf::Vector2f targetPosition;

    const float xOffset = 200.f; // Смещение машины от центра
    const float smoothness = 5.f;
};