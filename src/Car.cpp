#include "../include/Car.h"

constexpr float SCALE = 100.0f;

Car::Car(b2World& world, float density, float friction) :
    currentSpeed(0), targetSpeed(0), inAir(false), maxSpeed(50.0f) {

    // Загрузка текстуры колеса
    if (!wheelTexture.loadFromFile("imgs/wheel.png")) {
        // Создаем простую текстуру колеса, если файл не найден
        wheelTexture.create(56, 56);
        sf::Uint8* pixels = new sf::Uint8[56 * 56 * 4];
        for (int y = 0; y < 56; y++) {
            for (int x = 0; x < 56; x++) {
                int index = (y * 56 + x) * 4;
                float dist = sqrt(pow(x - 28, 2) + pow(y - 28, 2));
                if (dist < 28) {
                    pixels[index] = 50;   // R
                    pixels[index + 1] = 50; // G
                    pixels[index + 2] = 50; // B
                    pixels[index + 3] = 255; // A

                    // Добавляем спицы
                    if ((x % 10 < 2 || y % 10 < 2) && dist > 10) {
                        pixels[index] = 100;
                        pixels[index + 1] = 100;
                        pixels[index + 2] = 100;
                    }
                }
                else {
                    pixels[index + 3] = 0; // Прозрачный
                }
            }
        }
        wheelTexture.update(pixels);
        delete[] pixels;
    }

    leftWheelSprite.setTexture(wheelTexture);
    rightWheelSprite.setTexture(wheelTexture);
    leftWheelSprite.setOrigin(wheelTexture.getSize().x / 2, wheelTexture.getSize().y / 2);
    rightWheelSprite.setOrigin(wheelTexture.getSize().x / 2, wheelTexture.getSize().y / 2);

    createCar(world, density, friction);
    createWheels(world);

    carShape.setFillColor(sf::Color::Red);
    carShape.setSize(sf::Vector2f(170, 80));
    carShape.setOrigin(85, 40);

   //уменьшаем колеса
    leftWheelSprite.setScale(0.1f, 0.1f);  
    rightWheelSprite.setScale(0.1f, 0.1f); 
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
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        carBody->SetAngularVelocity(1.f);
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        carBody->SetAngularVelocity(-1.f);
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
// Обновить метод draw:
void Car::draw(sf::RenderWindow& window) const {
    // Кузов машины
    sf::RectangleShape& mutableCarShape = const_cast<sf::RectangleShape&>(carShape);
    mutableCarShape.setPosition(carBody->GetPosition().x * SCALE, carBody->GetPosition().y * SCALE);
    mutableCarShape.setRotation(carBody->GetAngle() * 180.0f / b2_pi);
    window.draw(mutableCarShape);

    // Колеса
    sf::Sprite& mutableLeftWheel = const_cast<sf::Sprite&>(leftWheelSprite);
    mutableLeftWheel.setPosition(leftWheel->GetPosition().x * SCALE, leftWheel->GetPosition().y * SCALE);
    mutableLeftWheel.setRotation(leftWheel->GetAngle() * 180.0f / b2_pi);
    window.draw(mutableLeftWheel);

    sf::Sprite& mutableRightWheel = const_cast<sf::Sprite&>(rightWheelSprite);
    mutableRightWheel.setPosition(rightWheel->GetPosition().x * SCALE, rightWheel->GetPosition().y * SCALE);
    mutableRightWheel.setRotation(rightWheel->GetAngle() * 180.0f / b2_pi);
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
    carDef.angularDamping = .8f;    // Сопротивление вращению
    carBody = world.CreateBody(&carDef);

    b2PolygonShape carBox;
    carBox.SetAsBox(0.85f, 0.4f);

    //хар-ки машины(кузова)
    b2FixtureDef carFixture;
    carFixture.shape = &carBox;
    carFixture.density = density + 1.f;
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
    wheelFixture.restitution = 0.2f;

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
    jointDef.stiffness = 30.0f;       // Более жесткая подвеска
    jointDef.damping = 15.0f;         // Улучшенная амортизация
    jointDef.lowerTranslation = -0.05f;
    jointDef.upperTranslation = 0.05f;
    jointDef.enableLimit = true;
    leftJoint = (b2WheelJoint*)world.CreateJoint(&jointDef);

    // Аналогично для правого колеса
    jointDef.Initialize(carBody, rightWheel, rightWheel->GetPosition(), b2Vec2(0, 1));
    rightJoint = (b2WheelJoint*)world.CreateJoint(&jointDef);
}