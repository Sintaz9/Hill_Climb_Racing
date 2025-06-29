#include "../include/Camera.h"
#include <algorithm>

Camera::Camera(sf::RenderWindow& window, float baseZoom, float zoomRange)
    : m_window(window),
    m_position(window.getDefaultView().getCenter()),
    m_currentZoom(baseZoom) {
    m_view = window.getDefaultView();
    setZoomParameters(baseZoom, zoomRange);
}

void Camera::update(const sf::Vector2f& targetPos, float speedRatio, float dt) {
    m_position = calculateTargetPosition(targetPos, speedRatio);
    m_currentZoom = calculateTargetZoom(speedRatio);

    m_view.setSize(m_window.getDefaultView().getSize());
    m_view.zoom(m_currentZoom);
    m_view.setCenter(m_position);
}

void Camera::applyToWindow() {
    m_window.setView(m_view);
}

const sf::View& Camera::getView() const {
    return m_view;
}

void Camera::setFollowParameters(float distance, float height, float smoothness) {
    m_followParams.distance = distance;
    m_followParams.height = height;
    m_followParams.smoothness = smoothness;
}

void Camera::setZoomParameters(float baseZoom, float zoomRange) {
    m_zoomParams.base = baseZoom;
    m_zoomParams.range = zoomRange;
}

sf::Vector2f Camera::calculateTargetPosition(const sf::Vector2f& targetPos, float speedRatio) const {
    return {
        targetPos.x + m_followParams.distance,
        targetPos.y + m_followParams.height - speedRatio * 100.f
    };
}

float Camera::calculateTargetZoom(float speedRatio) const {
    return m_zoomParams.base + (m_zoomParams.range * speedRatio);
}