// Car.h
#ifndef CAR_H
#define CAR_H

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

class Car {
public:
    Car(b2World& world, float density, float friction);
    ~Car();

    void update(float dt, bool nitroActive);
    void draw(sf::RenderWindow& window) const;

    b2Vec2 getPosition() const;
    b2Body* getBody() const;
    bool isInAir() const;

    float getSpeed() const;
    float getMaxSpeed() const;

private:
    b2Body* carBody;
    b2Body* leftWheel;
    b2Body* rightWheel;
    b2WheelJoint* leftJoint;
    b2WheelJoint* rightJoint;

    sf::Texture wheelTexture;
    sf::Texture carTexture;  // Добавлено для текстуры машины
    sf::Sprite carSprite;    // Добавлено для спрайта машины
    sf::Sprite leftWheelSprite;
    sf::Sprite rightWheelSprite;

    float currentSpeed;
    float targetSpeed;
    bool inAir;
    const float maxSpeed = 50.0f; // Максимальная скорость

    void createCar(b2World& world, float density, float friction);
    void createWheels(b2World& world);
};

#endif // CAR_H