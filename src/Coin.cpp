#include "../include/Coin.h"

// Конструктор монеты
Coin::Coin(b2World& world, const sf::Texture& texture, const b2Vec2& position)
    : collected(false), body(nullptr) {

    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.position = position;
    body = world.CreateBody(&bodyDef);

    b2CircleShape circle;
    circle.m_radius = 1.4f; // Увеличиваем радиус коллизии

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circle;
    fixtureDef.isSensor = true;
    fixtureDef.density = 0.0f;

    // Добавляем фильтр коллизий
    b2Filter filter;
    filter.categoryBits = 0x0002; // Категория монет
    filter.maskBits = 0x0001;    // Коллизии только с машиной
    fixtureDef.filter = filter;

    body->CreateFixture(&fixtureDef);

    sprite.setTexture(texture);
    sprite.setOrigin(texture.getSize().x / 2.f, texture.getSize().y / 2.f);
    sprite.setScale(0.1f, 0.1f); // Увеличиваем визуальный размер
    sprite.setPosition(position.x * SCALE, position.y * SCALE);
}

// Деструктор - очистка ресурсов
Coin::~Coin() {
    if (body) {
        body->GetWorld()->DestroyBody(body);  // Удаление тела из физического мира
    }
}

// Обновление состояния монеты (вызывается каждый кадр)
void Coin::update(float dt) {
    if (!collected) {
        animTime += dt;
        // Более плавная анимация с контролируемой скоростью
        sprite.setRotation(animTime * 90.f); // 90 градусов в секунду

        if (body) {
            sprite.setPosition(
                body->GetPosition().x * SCALE,
                body->GetPosition().y * SCALE
            );
        }
    }
}

// Отрисовка монеты
void Coin::draw(sf::RenderWindow& window) const {
    if (!collected) {  // Рисуем только не собранные монеты
        window.draw(sprite);
    }
}

// Получение текущей позиции в физическом мире
b2Vec2 Coin::getPosition() const {
    return body ? body->GetPosition() : b2Vec2(0, 0);  // Защита от nullptr
}

// Проверка, собрана ли монета
bool Coin::isCollected() const {
    return collected;
}

// Обработка сбора монеты
void Coin::collect() {
    collected = true;  // Помечаем как собранную

    // Удаляем физическое тело
    if (body) {
        body->GetWorld()->DestroyBody(body);
        body = nullptr;  // Обнуляем указатель
    }
}
