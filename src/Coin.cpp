#include "../include/Coin.h"
#include <cmath>

constexpr float SCALE = 100.0f;
constexpr float COIN_ANIM_SPEED = 5.0f;

Coin::Coin(b2World& world, const sf::Texture& texture, const b2Vec2& position, Type type)
    : collected(false), type(type) {
    // Создание тела
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = position;
    body = world.CreateBody(&bodyDef);

    // Форма монеты
    b2CircleShape circle;
    circle.m_radius = 0.3f;

    // Фикстура
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circle;
    fixtureDef.density = 0.1f;
    fixtureDef.friction = 0.3f;
    fixtureDef.isSensor = true;

    body->CreateFixture(&fixtureDef);

    // Настройка спрайта
    sprite.setTexture(texture);
    sf::IntRect textureRect;
    switch (type) {
    case Type::GOLD:
        textureRect = sf::IntRect(0, 0, 64, 64);
        break;
    case Type::SPECIAL:
        textureRect = sf::IntRect(64, 0, 64, 64);
        break;
    default:
        textureRect = sf::IntRect(128, 0, 64, 64);
    }
    sprite.setTextureRect(textureRect);
    sprite.setOrigin(32, 32);
    sprite.setScale(0.15f, 0.15f);
}

Coin::~Coin() {
    if (body) {
        body->GetWorld()->DestroyBody(body);
    }
}

void Coin::update() {
    if (!collected) {
        sprite.setPosition(
            body->GetPosition().x * SCALE,
            body->GetPosition().y * SCALE
        );

        // Анимация вращения
        animTime += 0.016f; // Примерное время кадра
        float rotation = std::sin(animTime * COIN_ANIM_SPEED) * 15.0f;
        sprite.setRotation(rotation);
    }
}

void Coin::draw(sf::RenderWindow& window) const {
    if (!collected) {
        window.draw(sprite);
    }
}

b2Vec2 Coin::getPosition() const {
    return body->GetPosition();
}

bool Coin::isCollected() const {
    return collected;
}

void Coin::collect() {
    collected = true;
}

int Coin::getValue() const {
    switch (type) {
    case Type::GOLD: return 5;
    case Type::SPECIAL: return 10;
    default: return 1;
    }
}