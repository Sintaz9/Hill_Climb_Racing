#include "../include/Car.h"

constexpr float SCALE = 100.0f;

Car::Car(b2World& world, float density, float friction) :
    currentSpeed(0), targetSpeed(0), inAir(false), maxSpeed(50.0f) {
    createCar(world, density, friction);
    createWheels(world);
    carShape.setFillColor(sf::Color::Red);
    carShape.setSize(sf::Vector2f(170, 80));
    carShape.setOrigin(85, 40);

    leftWheelShape.setFillColor(sf::Color::Black);
    leftWheelShape.setRadius(28);
    leftWheelShape.setOrigin(28, 28);

    rightWheelShape.setFillColor(sf::Color::Black);
    rightWheelShape.setRadius(28);
    rightWheelShape.setOrigin(28, 28);
}

// Добавленные методы
float Car::getSpeed() const {
    return currentSpeed;
}

float Car::getMaxSpeed() const {
    return maxSpeed;
}

Car::~Car() {
    // Уничтожение физических тел происходит в мире Box2D
}

void Car::update(float dt, bool nitroActive) {
    // Управление
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        targetSpeed = nitroActive ? 80.0f : 50.0f;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        targetSpeed = -8.0f;
    }
    else {
        targetSpeed = 0;
    }

    // Плавное изменение скорости
    if (currentSpeed < targetSpeed) {
        currentSpeed += 0.5f;
    }
    else if (currentSpeed > targetSpeed) {
        currentSpeed -= 0.5f;
    }

    // Применение скорости к колесам
    leftJoint->SetMotorSpeed(currentSpeed);
    rightJoint->SetMotorSpeed(currentSpeed);

    // Управление наклоном
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        carBody->SetAngularVelocity(1.2f);
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        carBody->SetAngularVelocity(-1.2f);
    }

    // Проверка нахождения в воздухе
    inAir = true;
        for (b2ContactEdge* edge = carBody->GetContactList(); edge; edge = edge->next) {
            if (edge->contact->IsTouching()) {
                inAir = false;
                break;
            }
        }
}
void Car::draw(sf::RenderWindow& window) const {
    // Изменяем объекты через const_cast, так как SFML методы не const-correct
    sf::RectangleShape& mutableCarShape = const_cast<sf::RectangleShape&>(carShape);
    mutableCarShape.setPosition(carBody->GetPosition().x * SCALE, carBody->GetPosition().y * SCALE);
    mutableCarShape.setRotation(carBody->GetAngle() * 180.0f / b2_pi);
    window.draw(mutableCarShape);

    sf::CircleShape& mutableLeftWheel = const_cast<sf::CircleShape&>(leftWheelShape);
    mutableLeftWheel.setPosition(leftWheel->GetPosition().x * SCALE, leftWheel->GetPosition().y * SCALE);
    window.draw(mutableLeftWheel);

    sf::CircleShape& mutableRightWheel = const_cast<sf::CircleShape&>(rightWheelShape);
    mutableRightWheel.setPosition(rightWheel->GetPosition().x * SCALE, rightWheel->GetPosition().y * SCALE);
    window.draw(mutableRightWheel);
}

b2Vec2 Car::getPosition() const {
    return carBody->GetPosition();
}

b2Body* Car::getBody() const {
    return carBody;
}

bool Car::isInAir() const {
    return inAir;
}

void Car::createCar(b2World& world, float density, float friction) {
    b2BodyDef carDef;
    carDef.type = b2_dynamicBody;
    carDef.position.Set(5.0f, 5.0f); // Стартовая позиция
    carDef.linearDamping = 0.2f;     // Сопротивление движению
    carDef.angularDamping = 0.5f;    // Сопротивление вращению
    carBody = world.CreateBody(&carDef);

    b2PolygonShape carBox;
    carBox.SetAsBox(0.85f, 0.4f);

    //хар-ки машины(кузова)
    b2FixtureDef carFixture;
    carFixture.shape = &carBox;
    carFixture.density = density;
    carFixture.friction = friction;
    carBody->CreateFixture(&carFixture);
}

void Car::createWheels(b2World& world) {
    b2CircleShape wheelShape;
    wheelShape.m_radius = 0.28f;

    b2FixtureDef wheelFixture;
    wheelFixture.shape = &wheelShape;
    wheelFixture.density = 1.5f;
    wheelFixture.friction = 2.0f;
    wheelFixture.restitution = 0.05f;

    // Левое колесо
    b2BodyDef leftWheelDef;
    leftWheelDef.type = b2_dynamicBody;
    leftWheelDef.position = carBody->GetWorldPoint(b2Vec2(-0.6f, 0.7f));
    leftWheel = world.CreateBody(&leftWheelDef);
    leftWheel->CreateFixture(&wheelFixture);

    // Правое колесо
    b2BodyDef rightWheelDef;
    rightWheelDef.type = b2_dynamicBody;
    rightWheelDef.position = carBody->GetWorldPoint(b2Vec2(0.6f, 0.7f));
    rightWheel = world.CreateBody(&rightWheelDef);
    rightWheel->CreateFixture(&wheelFixture);

    // Улучшенная подвеска
    b2WheelJointDef jointDef;
    jointDef.Initialize(carBody, leftWheel, leftWheel->GetPosition(), b2Vec2(0, 1));
    jointDef.motorSpeed = 0.0f;
    jointDef.maxMotorTorque = 50.0f;  // Увеличенный крутящий момент
    jointDef.enableMotor = true;
    jointDef.stiffness = 20.0f;       // Более жесткая подвеска
    jointDef.damping = 10.0f;         // Улучшенная амортизация
    jointDef.lowerTranslation = -0.1f;
    jointDef.upperTranslation = 0.1f;
    jointDef.enableLimit = true;
    leftJoint = (b2WheelJoint*)world.CreateJoint(&jointDef);

    // Аналогично для правого колеса
    jointDef.Initialize(carBody, rightWheel, rightWheel->GetPosition(), b2Vec2(0, 1));
    rightJoint = (b2WheelJoint*)world.CreateJoint(&jointDef);
}