#include "../include/Camera.h"
#include <algorithm>
#include <cmath>

Camera::Camera(sf::RenderWindow& window, float baseZoom, float zoomRange)
    : m_window(window),
    m_baseZoom(baseZoom),
    m_zoomRange(zoomRange),
    m_currentZoom(baseZoom),
    m_targetZoom(baseZoom) {
    m_view = window.getDefaultView();
    m_worldBounds = { 0, -500, 10000, 5000 };
    m_velocity = { 0, 0 };
}

void Camera::setWorldBounds(const sf::FloatRect& bounds) {
    m_worldBounds = bounds;
}

void Camera::update(const sf::Vector2f& targetPos, float speedRatio, float dt) {
    // 1. Set target parameters
    m_targetPosition = targetPos + sf::Vector2f(m_followDistance + m_xOffset, m_followHeight);
    m_targetZoom = m_baseZoom + (m_zoomRange * speedRatio);

    // 2. Smooth movement (SmoothDamp)
    sf::Vector2f delta = m_targetPosition - m_currentPosition;

    if (dt > 0) {
        m_velocity = delta * (1.0f / m_smoothTime);
        float maxSpeed = 2000.f;
        float currentSpeed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
        if (currentSpeed > maxSpeed) {
            m_velocity = m_velocity * (maxSpeed / currentSpeed);
        }
    }

    m_currentPosition += m_velocity * dt;

    // 3. Smooth zoom
    float zoomLerpFactor = std::min(5.0f * dt, 1.0f);
    m_currentZoom += (m_targetZoom - m_currentZoom) * zoomLerpFactor;

    // 4. Apply parameters
    m_view.setSize(m_window.getDefaultView().getSize());
    m_view.zoom(m_currentZoom);
    m_view.setCenter(m_currentPosition);

    // 5. Clamp to world bounds
    sf::Vector2f viewHalfSize = m_view.getSize() / 2.f;

    float left = std::max(m_worldBounds.left + viewHalfSize.x, viewHalfSize.x);
    float right = std::max(m_worldBounds.left + m_worldBounds.width - viewHalfSize.x, viewHalfSize.x);
    float top = std::max(m_worldBounds.top + viewHalfSize.y, viewHalfSize.y);
    float bottom = std::max(m_worldBounds.top + m_worldBounds.height - viewHalfSize.y, viewHalfSize.y);

    sf::Vector2f center = m_view.getCenter();
    center.x = std::clamp(center.x, left, right);
    center.y = std::clamp(center.y, top, bottom);
    m_view.setCenter(center);
}

void Camera::applyToWindow() {
    m_window.setView(m_view);
}

const sf::View& Camera::getView() const {
    return m_view;
}