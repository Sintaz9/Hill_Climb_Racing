#include "../include/Obstacle.h"

constexpr float SCALE = 100.0f;

Obstacle::Obstacle(b2World& world, const sf::Texture& texture, const b2Vec2& position, float density) {
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody; // Изменено с dynamic на static
    bodyDef.position = position;
    mainBody = world.CreateBody(&bodyDef);

    b2PolygonShape box;
    box.SetAsBox(0.5f, 0.5f);//размер препятствия

    b2FixtureDef fixture;
    fixture.shape = &box;
    fixture.density = density;
    fixture.friction = 0.8f; // Увеличено трение
    fixture.restitution = 0.1f; // Уменьшен отскок

    mainBody->CreateFixture(&fixture);
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