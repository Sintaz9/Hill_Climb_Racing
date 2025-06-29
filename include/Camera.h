#ifndef CAMERA_H
#define CAMERA_H

#include <SFML/Graphics.hpp>

class Camera {
public:
    Camera(sf::RenderWindow& window, float baseZoom, float zoomRange);

    void update(const sf::Vector2f& targetPos, float speedRatio, float dt);
    void applyToWindow();
    const sf::View& getView() const;

    void setFollowParameters(float distance, float height, float smoothness);
    void setZoomParameters(float baseZoom, float zoomRange);

private:
    sf::RenderWindow& m_window;
    sf::View m_view;

    struct {
        float distance = 300.f;    // Дистанция слежения по X
        float height = -150.f;     // Базовое смещение по Y
        float smoothness = 0.15f;  // Плавность движения
    } m_followParams;

    struct {
        float base = 1.0f;        // Базовый зум
        float range = 0.6f;       // Диапазон изменения зума
    } m_zoomParams;

    sf::Vector2f m_position;
    float m_currentZoom;

    sf::Vector2f calculateTargetPosition(const sf::Vector2f& targetPos, float speedRatio) const;
    float calculateTargetZoom(float speedRatio) const;
};

#endif // CAMERA_H