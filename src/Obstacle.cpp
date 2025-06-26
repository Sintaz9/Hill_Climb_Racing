#include "../include/Obstacle.h"
#include <random>
#include <algorithm>

constexpr float SCALE = 100.0f;

Obstacle::Obstacle(b2World& world, const sf::Texture& texture, const b2Vec2& position, float density) :
    broken(false) {

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = position;
    mainBody = world.CreateBody(&bodyDef);

    b2PolygonShape box;
    box.SetAsBox(0.5f, 0.5f);

    b2FixtureDef fixture;
    fixture.shape = &box;
    fixture.density = density;
    fixture.friction = 0.7f;
    fixture.restitution = 0.3f;

    mainBody->CreateFixture(&fixture);

    sprite.setTexture(texture);
    sf::Vector2u texSize = texture.getSize();
    sprite.setOrigin(static_cast<float>(texSize.x) / 2.0f,
        static_cast<float>(texSize.y) / 2.0f);
    sprite.setScale(0.2f, 0.2f);
}

void Obstacle::update() {
    if (!broken) {
        sprite.setPosition(
            mainBody->GetPosition().x * SCALE,
            mainBody->GetPosition().y * SCALE
        );
        sprite.setRotation(mainBody->GetAngle() * 180.0f / static_cast<float>(b2_pi));
    }
    else {
        for (size_t i = 0; i < fragments.size(); ++i) {
            fragmentSprites[i].setPosition(
                fragments[i]->GetPosition().x * SCALE,
                fragments[i]->GetPosition().y * SCALE
            );
            fragmentSprites[i].setRotation(fragments[i]->GetAngle() * 180.0f / static_cast<float>(b2_pi));
        }
    }
}

bool Obstacle::shouldBreak(b2Body* carBody) const {
    if (broken) return false;

    for (b2ContactEdge* edge = mainBody->GetContactList(); edge; edge = edge->next) {
        b2Contact* contact = edge->contact;

        if (contact->IsTouching()) {
            b2Body* bodyA = contact->GetFixtureA()->GetBody();
            b2Body* bodyB = contact->GetFixtureB()->GetBody();

            if (bodyA == carBody || bodyB == carBody) {
                // ѕолучаем данные о контакте
                b2Manifold* manifold = contact->GetManifold();

                // ќцениваем силу удара по количеству точек контакта и их глубине
                float impact = 0.0f;
                for (int i = 0; i < manifold->pointCount; ++i) {
                    impact += manifold->points[i].normalImpulse;
                }

                return impact > 5.0f;
            }
        }
    }
    return false;
}

// ќстальные методы остаютс€ без изменений
void Obstacle::draw(sf::RenderWindow& window) const {
    if (!broken) {
        window.draw(sprite);
    }
    else {
        for (const auto& fragment : fragmentSprites) {
            window.draw(fragment);
        }
    }
}

void Obstacle::breakApart(b2World& world) {
    if (broken) return;
    broken = true;
    createFragments(world);
    world.DestroyBody(mainBody);
}

void Obstacle::createFragments(b2World& world) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

    for (int i = 0; i < 4; ++i) {
        b2BodyDef fragmentDef;
        fragmentDef.type = b2_dynamicBody;
        fragmentDef.position = mainBody->GetPosition();
        fragmentDef.linearVelocity.Set(dist(rng), dist(rng));
        fragmentDef.angularVelocity = dist(rng) * 0.5f;

        b2Body* fragment = world.CreateBody(&fragmentDef);

        b2PolygonShape fragmentShape;
        fragmentShape.SetAsBox(0.25f, 0.25f,
            b2Vec2(i % 2 ? 0.25f : -0.25f, i / 2 ? 0.25f : -0.25f), 0);

        b2FixtureDef fragmentFixture;
        fragmentFixture.shape = &fragmentShape;
        fragmentFixture.density = 0.5f;
        fragmentFixture.friction = 0.5f;
        fragmentFixture.restitution = 0.2f;

        fragment->CreateFixture(&fragmentFixture);
        fragments.push_back(fragment);

        sf::Sprite fragmentSprite;
        fragmentSprite.setTexture(*sprite.getTexture());

        sf::Vector2u texSize = sprite.getTexture()->getSize();
        fragmentSprite.setTextureRect(sf::IntRect(
            i % 2 * texSize.x / 2,
            i / 2 * texSize.y / 2,
            texSize.x / 2,
            texSize.y / 2
        ));

        fragmentSprite.setOrigin(
            static_cast<float>(fragmentSprite.getTextureRect().width) / 2.0f,
            static_cast<float>(fragmentSprite.getTextureRect().height) / 2.0f
        );
        fragmentSprite.setScale(0.2f, 0.2f);
        fragmentSprites.push_back(fragmentSprite);
    }
}