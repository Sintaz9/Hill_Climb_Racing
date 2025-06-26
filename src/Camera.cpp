#include "../include/Camera.h"
#include <algorithm>

Camera::Camera(sf::RenderWindow& window, float baseZoom, float zoomRange)
    : window(window), baseZoom(baseZoom), zoomRange(zoomRange) {
    reset();
}

void Camera::reset() {
    view = window.getDefaultView();
    currentZoom = baseZoom;
    targetZoom = baseZoom;
    worldBounds = { 0, -500, 10000, 5000 };
}

void Camera::setWorldBounds(const sf::FloatRect& bounds) {
    worldBounds = bounds;
}

void Camera::setZoom(float zoom) {
    currentZoom = zoom;
    targetZoom = zoom;
    view.setSize(window.getDefaultView().getSize());
    view.zoom(currentZoom);
}

void Camera::update(const sf::Vector2f& targetPos, float speedRatio, float dt) {
    // Убедимся, что speedRatio в допустимых пределах
    speedRatio = std::clamp(speedRatio, 0.0f, 1.0f);

    // Целевая позиция камеры (машина смещена вправо на 30% экрана)
    float xOffset = view.getSize().x * 0.3f;
    targetPosition = targetPos + sf::Vector2f(xOffset, -100.f);

    // Плавное движение камеры
    float lerpFactor = std::min(5.0f * dt, 1.0f);
    currentPosition += (targetPosition - currentPosition) * lerpFactor;

    // Динамический зум
    targetZoom = baseZoom - zoomRange * speedRatio;
    currentZoom += (targetZoom - currentZoom) * lerpFactor;
    currentZoom = std::clamp(currentZoom, baseZoom - zoomRange, baseZoom);

    // Применение параметров
    view.setSize(window.getDefaultView().getSize());
    view.zoom(currentZoom);
    view.setCenter(currentPosition);

    // Ограничение камеры границами мира
    sf::Vector2f viewHalfSize = view.getSize() / 2.f;

    float left = std::max(worldBounds.left + viewHalfSize.x, viewHalfSize.x);
    float right = std::max(worldBounds.left + worldBounds.width - viewHalfSize.x, viewHalfSize.x);
    float top = std::max(worldBounds.top + viewHalfSize.y, viewHalfSize.y);
    float bottom = std::max(worldBounds.top + worldBounds.height - viewHalfSize.y, viewHalfSize.y);

    sf::Vector2f center = view.getCenter();
    center.x = std::clamp(center.x, left, right);
    center.y = std::clamp(center.y, top, bottom);
    view.setCenter(center);
}

void Camera::applyToWindow() {
    window.setView(view);
}

const sf::View& Camera::getView() const {
    return view;
}

float Camera::getZoom() const {
    return currentZoom;
}