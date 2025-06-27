#include "../include/Obstacle.h"

constexpr float SCALE = 100.0f;

Obstacle::Obstacle(b2World& world, const sf::Texture& texture, const b2Vec2& position, float density) {
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = position;
    mainBody = world.CreateBody(&bodyDef);

    b2PolygonShape box;
    box.SetAsBox(0.5f, 0.5f); // Размер 1x1 метр

    b2FixtureDef fixture;
    fixture.shape = &box;
    fixture.density = density;
    fixture.friction = 0.4f;
    fixture.restitution = 0.3f;

    mainBody->CreateFixture(&fixture);

    sprite.setTexture(texture);
    sf::Vector2u texSize = texture.getSize();
    sprite.setOrigin(texSize.x / 2.0f, texSize.y / 2.0f);
    sprite.setScale(0.15f, 0.15f);
}

void Obstacle::update() {
    sprite.setPosition(
        mainBody->GetPosition().x * SCALE,
        mainBody->GetPosition().y * SCALE
    );
    sprite.setRotation(mainBody->GetAngle() * 180.0f / b2_pi);
}

void Obstacle::draw(sf::RenderWindow& window) const {
    window.draw(sprite);
}