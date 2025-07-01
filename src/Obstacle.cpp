#include "../include/Obstacle.h"

constexpr float SCALE = 100.0f;

Obstacle::Obstacle(b2World& world, const sf::Texture& texture, const b2Vec2& position, float density) {
    // Создание динамического тела
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = position;
    body = world.CreateBody(&bodyDef);

    // Создание фикстуры
    b2PolygonShape box;
    box.SetAsBox(0.65f, 0.65f); // Размер 1x1 метр

    b2FixtureDef fixture;
    fixture.shape = &box;
    fixture.density = density + 0.6f;
    fixture.friction = 0.4f;
    fixture.restitution = .5f; // Упругость

    body->CreateFixture(&fixture);

    // Настройка спрайта
    sprite.setTexture(texture);
    sprite.setOrigin(texture.getSize().x / 2.0f, texture.getSize().y / 2.0f);
    sprite.setScale(0.115f, 0.115f);
}

void Obstacle::update() {
    sprite.setPosition(
        body->GetPosition().x * SCALE,
        body->GetPosition().y * SCALE
    );
    sprite.setRotation(body->GetAngle() * 180.0f / b2_pi);
}

void Obstacle::draw(sf::RenderWindow& window) const {
    window.draw(sprite);
}

b2Vec2 Obstacle::getPosition() const {
    return body->GetPosition();
}

b2Body* Obstacle::getBody() const {
    return body;
}