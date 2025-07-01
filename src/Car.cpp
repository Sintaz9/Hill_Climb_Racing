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

    // Загрузка текстуры машины
    if (!carTexture.loadFromFile("imgs/car.png")) {
        // Если не удалось загрузить, создаем красный квадрат как запасной вариант
        carTexture.create(1560, 780);
        sf::Uint8* pixels = new sf::Uint8[1560 * 780 * 4];
        for (int y = 0; y < 780; y++) {
            for (int x = 0; x < 1560; x++) {
                int index = (y * 1560 + x) * 4;
                pixels[index] = 255;     // R
                pixels[index + 1] = 0;   // G
                pixels[index + 2] = 0;   // B
                pixels[index + 3] = 255; // A
            }
        }
        carTexture.update(pixels);
        delete[] pixels;
    }

    carSprite.setTexture(carTexture);
    carSprite.setOrigin(carTexture.getSize().x / 2, carTexture.getSize().y / 2);
    carSprite.setScale(0.29f, 0.29f);                                    // Масштабируем до нужного размера колеса

    leftWheelSprite.setTexture(wheelTexture);
    rightWheelSprite.setTexture(wheelTexture);
    leftWheelSprite.setOrigin(wheelTexture.getSize().x / 2, wheelTexture.getSize().y / 2);
    rightWheelSprite.setOrigin(wheelTexture.getSize().x / 2, wheelTexture.getSize().y / 2);

    createCar(world, density, friction);
    createWheels(world);

    // Уменьшаем колеса
    leftWheelSprite.setScale(0.115f, 0.115f);
    rightWheelSprite.setScale(0.115f, 0.115f);
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
        targetSpeed = nitroActive ? 80.0f : 42.0f;              //скорость машины
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

b2Vec2 Car::getPosition() const {
    return carBody->GetPosition();
}

b2Body* Car::getBody() const {
    return carBody;
}

bool Car::isInAir() const {
    return inAir;
}


void Car::draw(sf::RenderWindow& window) const {
    // Кузов машины (теперь с текстурой)
    sf::Sprite& mutableCarSprite = const_cast<sf::Sprite&>(carSprite);
    mutableCarSprite.setPosition(carBody->GetPosition().x * SCALE, carBody->GetPosition().y * SCALE);
    mutableCarSprite.setRotation(carBody->GetAngle() * 180.0f / b2_pi);

    // Колеса
    sf::Sprite& mutableLeftWheel = const_cast<sf::Sprite&>(leftWheelSprite);
    mutableLeftWheel.setPosition(leftWheel->GetPosition().x * SCALE, leftWheel->GetPosition().y * SCALE);
    mutableLeftWheel.setRotation(leftWheel->GetAngle() * 180.0f / b2_pi);
    window.draw(mutableLeftWheel);

    sf::Sprite& mutableRightWheel = const_cast<sf::Sprite&>(rightWheelSprite);
    mutableRightWheel.setPosition(rightWheel->GetPosition().x * SCALE, rightWheel->GetPosition().y * SCALE);
    mutableRightWheel.setRotation(rightWheel->GetAngle() * 180.0f / b2_pi);
    window.draw(mutableRightWheel);

    window.draw(mutableCarSprite);
}

void Car::createCar(b2World& world, float density, float friction) {
    b2BodyDef carDef;
    carDef.type = b2_dynamicBody;
    carDef.position.Set(5.0f, 5.0f); // Стартовая позиция
    carDef.linearDamping = 0.2f;     // Сопротивление движению
    carDef.angularDamping = .8f;    // Сопротивление вращению
    carBody = world.CreateBody(&carDef);

    // Хитбокс из 12 точек — сбалансированная точность и производительность
    b2Vec2 vertices1[8]; // первые 8 точек
    b2Vec2 vertices2[8]; // следующие 7 (или меньше)

    float width = 3.f;
    float height = 2.f;

    // Первая часть хитбокса (верхняя + передняя часть)
    vertices1[0].Set(-width * 0.66f, height * 0.3f);
    vertices1[1].Set(-width * 0.65f, height * 0.1f);
    vertices1[2].Set(-width * 0.62f, -height * 0.05f);
    vertices1[3].Set(-width * 0.58f, -height * 0.03f);
    vertices1[4].Set(-width * 0.48f, -height * 0.14f);
    vertices1[5].Set(-width * 0.36f, -height * 0.4f);
    vertices1[6].Set(width * 0.25f, -height * 0.4f);
    vertices1[7].Set(width * 0.28f, -height * 0.12f);

    // Вторая часть хитбокса (задняя часть и низ)
    vertices2[0].Set(width * 0.64f, -height * 0.11f);
    vertices2[1].Set(width * 0.68f, height * 0.3f);//низ бампера
    vertices2[2].Set(width * 0.45f, height * 0.3f);
    vertices2[3].Set(width * 0.45f, height * 0.05f);
    vertices2[4].Set(-width * 0.30f, height * 0.05f);
    vertices2[5].Set(-width * 0.48f, height * 0.3f);
    vertices2[6].Set(-width * 0.52f, height * 0.3f);
    vertices2[7].Set(-width * 0.66f, height * 0.3f); // замыкаем


    b2PolygonShape shape1;
    shape1.Set(vertices1, 8);

    b2FixtureDef fixture1;
    fixture1.shape = &shape1;
    fixture1.density = density;
    fixture1.friction = friction;
    carBody->CreateFixture(&fixture1);

    b2PolygonShape shape2;
    shape2.Set(vertices2, 8);

    b2FixtureDef fixture2;
    fixture2.shape = &shape2;
    fixture2.density = density;
    fixture2.friction = friction;
    carBody->CreateFixture(&fixture2);  


}

void Car::createWheels(b2World& world) {
    b2CircleShape wheelShape;
    wheelShape.m_radius = 0.47f; //радиус колес

    b2FixtureDef wheelFixture;
    wheelFixture.shape = &wheelShape;
    wheelFixture.density = 1.7f;
    wheelFixture.friction = 1.3f;
    wheelFixture.restitution = 0.25f;

    // Левое колесо
    b2BodyDef leftWheelDef;
    leftWheelDef.type = b2_dynamicBody;
    leftWheelDef.position = carBody->GetWorldPoint(b2Vec2(-1.16f, 0.67f));
    leftWheel = world.CreateBody(&leftWheelDef);
    leftWheel->CreateFixture(&wheelFixture);

    // Правое колесо
    b2BodyDef rightWheelDef;
    rightWheelDef.type = b2_dynamicBody;
    rightWheelDef.position = carBody->GetWorldPoint(b2Vec2(1.47f, 0.62f));
    rightWheel = world.CreateBody(&rightWheelDef);
    rightWheel->CreateFixture(&wheelFixture);

    // Улучшенная подвеска
    b2WheelJointDef jointDef;
    jointDef.Initialize(carBody, leftWheel, leftWheel->GetPosition(), b2Vec2(0, 1));
    jointDef.motorSpeed = 0.0f;
    jointDef.maxMotorTorque = 35.0f;  // Увеличенный крутящий момент
    jointDef.enableMotor = true;
    jointDef.stiffness = 31.0f;       // Более жесткая подвеска
    jointDef.damping = 15.0f;         // Улучшенная амортизация
    jointDef.lowerTranslation = -0.05f;
    jointDef.upperTranslation = 0.05f;
    jointDef.enableLimit = true;
    leftJoint = (b2WheelJoint*)world.CreateJoint(&jointDef);

    // Аналогично для правого колеса
    jointDef.Initialize(carBody, rightWheel, rightWheel->GetPosition(), b2Vec2(0, 1));
    rightJoint = (b2WheelJoint*)world.CreateJoint(&jointDef);
}