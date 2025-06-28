#ifndef CAMERA_H
#define CAMERA_H

#include <SFML/Graphics.hpp>

class Camera {
public:
    Camera(sf::RenderWindow& window, float baseZoom, float zoomRange);

    void setWorldBounds(const sf::FloatRect& bounds);
    void update(const sf::Vector2f& targetPos, float speedRatio, float dt);
    void applyToWindow();
    const sf::View& getView() const;

private:
    sf::RenderWindow& m_window;
    sf::View m_view;
    sf::FloatRect m_worldBounds;

    // Camera parameters
    float m_baseZoom;
    float m_zoomRange;
    float m_currentZoom;
    float m_targetZoom;

    // Positioning
    sf::Vector2f m_currentPosition;
    sf::Vector2f m_targetPosition;
    sf::Vector2f m_velocity;

    // Follow parameters
    float m_followDistance = 300.f;
    float m_followHeight = -150.f;
    float m_smoothTime = 0.2f;
    float m_xOffset = 150.f; // Смещение камеры влево
};

#endif // CAMERA_H