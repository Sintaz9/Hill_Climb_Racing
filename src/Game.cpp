#include "../include/Game.h"
#include <sstream>
#include <cmath>
#include <random>
#include <iostream>

constexpr float SCALE = 100.0f;

Game::Game() :
    window(sf::VideoMode::getDesktopMode(), "Hill Climb Racing", sf::Style::Fullscreen),
    world(b2Vec2(0, 0)),
    terrain(world, groundTexture), // Инициализируем terrain здесь
    currentTheme(EARTH),
    score(0),
    distance(0),
    nitroAmount(0),
    nitroActive(false),
    inMenu(true) {

    window.setFramerateLimit(60);

    // Установка иконки окна
    sf::Image icon;
    if (icon.loadFromFile("imgs/icon.png")) {
        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    }
    // Загрузка текстур
    if (!backgroundTexture.loadFromFile("imgs/background.jpg") ||
        !coinTexture.loadFromFile("imgs/coin.png") ||
        !obstacleTexture.loadFromFile("imgs/Obstacle.png") || 
        !groundTexture.loadFromFile("imgs/grass.jpg")) {
        window.close();
    }
    if (!cloudTexture.loadFromFile("imgs/cloud.png") ||
        !bushTexture.loadFromFile("imgs/bush.png") ||
        !treeTexture.loadFromFile("imgs/tree.png")) {
        window.close();
    }
    menu.setup(window.getSize());
    setupHUD();
    camera = std::make_unique<Camera>(window, 1.0f, .6f);//Отдаление при движении     Первый параметр - базовый зум    Второй параметр - диапазон изменения зума

    groundTexture.setRepeated(true);
    groundTexture.setSmooth(true);
}

void Game::run() {
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        processEvents();

        if (!inMenu) {
            update(dt);
        }

        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed ||
            (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
            window.close();
        }

        if (inMenu) {
            Menu::MenuResult result = menu.handleEvent(event, window);
            if (result == Menu::EARTH_SELECTED) {
                changeTheme(EARTH);
                inMenu = false;
                resetGame();
            }
            else if (result == Menu::MOON_SELECTED) {
                changeTheme(MOON);
                inMenu = false;
                resetGame();
            }
            else if (result == Menu::EXIT) {
                window.close();
            }
        }
        else {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::M) {
                inMenu = true;
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::LShift) {
                if (nitroAmount > 0.1f) nitroActive = true;
            }

            if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::LShift) {
                nitroActive = false;
            }
        }
    }
}

void Game::update(float dt) {
    world.Step(dt, 10, 8);
    car->update(dt, nitroActive);

    if (car->isInAir()) {
        nitroAmount = std::min(nitroAmount + 0.1f * dt, 1.0f);
    }

    if (nitroActive && nitroAmount > 0) {
        nitroAmount = std::max(nitroAmount - 0.3f * dt, 0.0f);
        if (nitroAmount <= 0) nitroActive = false;
    }

    b2Vec2 carPos = car->getPosition();
    distance = carPos.x;
    float speedRatio = car->getSpeed() / car->getMaxSpeed();
    camera->update(sf::Vector2f(carPos.x * SCALE, carPos.y * SCALE), speedRatio, dt);

                                                                                        // Настройка сбора монет
    const float coinCollectionDistance = 130.0f; // Расстояние сбора в пикселях
    const float squaredDistance = coinCollectionDistance * coinCollectionDistance; // Квадрат расстояния для оптимизации

    //Сбор монет:
    for (size_t i = 0; i < coins.size(); ++i) {
        if (!coinsCollected[i]) {
            sf::Vector2f coinPos = coins[i].getPosition();
            b2Vec2 carPos = car->getPosition();

            float dx = carPos.x * SCALE - coinPos.x;
            float dy = carPos.y * SCALE - coinPos.y;
            float distSqr = dx * dx + dy * dy;

            if (distSqr < squaredDistance) {
                coinsCollected[i] = true;
                score += 1;

                // Запускаем анимацию
                animatingCoins.push_back({ i, 2.0f, coins[i].getScale().x });
            }
        }
    }

    // Обновляем анимации монет
    for (auto it = animatingCoins.begin(); it != animatingCoins.end(); ) {
        it->timer += dt;
        float progress = it->timer / COIN_ANIM_TIME;

        if (progress >= 1.0f) {
            it = animatingCoins.erase(it);
        }
        else {
            // Анимация увеличения и исчезновения
            float scale = it->startScale * (1.0f + progress); // Увеличиваем
            float alpha = 255 * (1.0f - progress); // Прозрачность
            coins[it->index].setScale(scale, scale);
            coins[it->index].setColor(sf::Color(255, 255, 255, alpha));
            ++it;
        }
    }
    // Обновляем препятствия
    for (auto& obstacle : obstacles) {
        obstacle.update();
    }
    updateHUD();
}
void Game::drawHUD() {
    // Фиксированный HUD
    sf::View hudView = window.getDefaultView();
    window.setView(hudView);

    // Позиционируем элементы относительно размеров окна
    sf::Vector2f windowSize = hudView.getSize();

    scoreText.setPosition(20, 20);
    distanceText.setPosition(20, 70);
    timeText.setPosition(20, 120);

    nitroText.setPosition(windowSize.x - 220, 20);
    nitroBarBackground.setPosition(windowSize.x - 220, 60);
    nitroBar.setPosition(windowSize.x - 220, 60);

    window.draw(scoreText);
    window.draw(distanceText);
    window.draw(timeText);
    window.draw(nitroText);
    window.draw(nitroBarBackground);
    window.draw(nitroBar);

    // Восстанавливаем игровой вид
    window.setView(camera->getView());
}





void Game::setupWorld() {
    std::random_device rd;
    std::mt19937 rng(rd());
    world.SetGravity(b2Vec2(0, currentTheme == EARTH ? 9.8f : 1.6f));

    // Генерация террейна
    const int pointCount = currentTheme == EARTH ? 1000 : 1500;
    const float baseY = currentTheme == EARTH ? 12.0f : 8.0f;
    const float stepX = 2.0f;

    std::vector<b2Vec2> terrainPoints;
    // Начальная плоская платформа (20 точек)
    for (int i = 0; i < 20; ++i) {
        terrainPoints.emplace_back(i * stepX, baseY);
    }

    // Генерация основного рельефа
    std::uniform_real_distribution<float> baseStep(-0.6f, 0.6f);
    std::uniform_real_distribution<float> extraStep(-4.0f, 4.0f);
    std::uniform_real_distribution<float> spikeChance(0.f, 1.f);

    float y = baseY;
    for (int i = 20; i < pointCount; ++i) {
        float x = i * stepX;
        float t = static_cast<float>(i) / pointCount;
        float difficulty = std::pow(t, 2.0f);

        float spike = (spikeChance(rng) < 0.05f + 0.1f * difficulty)
            ? extraStep(rng) * (0.5f + 2.0f * difficulty) : 0.f;

        y += std::clamp(baseStep(rng) + spike, -3.0f, 3.0f);
        y = std::clamp(y, baseY - 25.f, baseY + 30.f);

        terrainPoints.emplace_back(x, y);
    }

    // Настройка камеры
    float lastX = terrainPoints.back().x * SCALE;
    camera->setWorldBounds(sf::FloatRect(0, -500, lastX, 5000));

    // Генерация облаков только для Земли
    if (currentTheme == EARTH) {
        std::uniform_real_distribution<float> cloudXDist(0.f, lastX * 1.5f);
        std::uniform_real_distribution<float> cloudYDist(-800.f, -200.f);
        std::uniform_real_distribution<float> parallaxDist(0.1f, 0.9f);

        clouds.clear();
        for (int i = 0; i < 80; ++i) {
            clouds.emplace_back(
                cloudTexture,
                sf::Vector2f(cloudXDist(rng), cloudYDist(rng)),
                parallaxDist(rng)
            );
        }
    }

    // Генерация декораций только для Земли
    if (currentTheme == EARTH) {
        std::uniform_int_distribution<int> decType(0, 3);
        std::uniform_real_distribution<float> decXOffset(-1.5f, 1.5f);

        decorations.clear();
        for (size_t i = 20; i < terrain.getPoints().size(); i += 10) {
            const auto& pt = terrain.getPoints()[i];
            float x = pt.x * SCALE + decXOffset(rng) * 70.f;
            float y = pt.y * SCALE - 25.f;

            if (decType(rng) <= 2) {
                decorations.emplace_back(bushTexture, sf::Vector2f(x, y));
            }
            else {
                decorations.emplace_back(treeTexture, sf::Vector2f(x, y));
            }
        }
    }

    // Генерация препятствий
    std::uniform_int_distribution<int> obstacleInterval(80, 150);
    obstacles.clear();
    for (size_t i = 30; i < terrainPoints.size(); i += obstacleInterval(rng)) {
        b2Vec2 pos = terrainPoints[i];
        pos.y -= 0.7f;
        obstacles.emplace_back(world, obstacleTexture, pos,
            currentTheme == EARTH ? 0.8f : 0.4f);
    }

    // Улучшенная генерация монет
    std::uniform_int_distribution<int> coinInterval(30, 80);
    std::uniform_real_distribution<float> coinHeight(1.2f, 2.0f);
    std::uniform_real_distribution<float> coinOffset(-0.5f, 0.5f);

    coins.clear();
    coinsCollected.clear();
    for (size_t i = 25; i < terrainPoints.size(); i += coinInterval(rng)) {
        b2Vec2 pos = terrainPoints[i];

        // Высота монеты над поверхностью с небольшим случайным смещением
        float height = coinHeight(rng);
        pos.y -= height;
        pos.x += coinOffset(rng);

        // Проверка наклона поверхности
        if (i + 1 < terrainPoints.size()) {
            float groundSlope = (terrainPoints[i + 1].y - terrainPoints[i].y) /
                (terrainPoints[i + 1].x - terrainPoints[i].x);
            // Корректировка позиции в зависимости от наклона
            pos.y -= 0.2f * groundSlope;
        }

        sf::Sprite coin(coinTexture);
        coin.setOrigin(coinTexture.getSize().x / 2, coinTexture.getSize().y / 2);
        coin.setScale(0.1f, 0.1f);
        coin.setPosition(pos.x * SCALE, pos.y * SCALE);

        coins.push_back(coin);
        coinsCollected.push_back(false);
    }

    // Создание машины
    float startX = terrainPoints[5].x + 1.5f;
    float startY = terrainPoints[5].y - 1.8f;
    car = std::make_unique<Car>(world,
        currentTheme == EARTH ? 1.5f : 0.8f,
        currentTheme == EARTH ? 1.2f : 0.6f);
    car->getBody()->SetTransform(b2Vec2(startX, startY), 2 * b2_pi);
    // После создания можно принудительно остановить вращение:
    car->getBody()->SetAngularVelocity(0.0f);
    car->getBody()->SetFixedRotation(true); // Если нужно полностью запретить вращение
}


void Game::render() {
    window.clear();

    if (inMenu) {
        menu.draw(window);
    }
    else {
        camera->applyToWindow();

        // Фон
        sf::View fixedView = window.getDefaultView();
        window.setView(fixedView);
        window.draw(backgroundSprite);
        window.setView(camera->getView());

        // Облака только для Земли
        if (currentTheme == EARTH) {
            for (auto& cloud : clouds) {
                cloud.update(camera->getView());
                cloud.draw(window);
            }
        }

        // Земля
        terrain.draw(window);

        // Декорации только для Земли
        if (currentTheme == EARTH) {
            for (auto& deco : decorations) {
                deco.draw(window);
            }
        }

        // Монеты
        for (size_t i = 0; i < coins.size(); ++i) {
            if (!coinsCollected[i]) {
                window.draw(coins[i]);
            }
        }

        // Препятствия
        for (auto& obstacle : obstacles) {
            obstacle.draw(window);
        }

        // Машина
        car->draw(window);

        // Интерфейс
        drawHUD();
    }

    window.display();
}








void Game::setupHUD() {
    if (!font.loadFromFile("imgs/arial/arialmt.ttf")) {
        font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
    }

    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color::Yellow);
    scoreText.setOutlineColor(sf::Color::Black);
    scoreText.setOutlineThickness(2);

    distanceText.setFont(font);
    distanceText.setCharacterSize(30);
    distanceText.setFillColor(sf::Color::White);
    distanceText.setOutlineColor(sf::Color::Black);
    distanceText.setOutlineThickness(2);

    timeText.setFont(font);
    timeText.setCharacterSize(30);
    timeText.setFillColor(sf::Color::Cyan);
    timeText.setOutlineColor(sf::Color::Black);
    timeText.setOutlineThickness(2);

    nitroText.setFont(font);
    nitroText.setCharacterSize(30);
    nitroText.setString("NITRO");
    nitroText.setFillColor(sf::Color::Green);
    nitroText.setOutlineColor(sf::Color::Black);
    nitroText.setOutlineThickness(2);

    nitroBarBackground.setSize(sf::Vector2f(200, 20));
    nitroBarBackground.setFillColor(sf::Color(50, 50, 50));

    nitroBar.setSize(sf::Vector2f(200, 20));
    nitroBar.setFillColor(sf::Color(0, 255, 255));
}

void Game::updateHUD() {
    std::ostringstream ss;
    ss << "Score: " << score;
    scoreText.setString(ss.str());

    ss.str("");
    ss << "Distance: " << static_cast<int>(distance) << "m";
    distanceText.setString(ss.str());

    int seconds = static_cast<int>(gameClock.getElapsedTime().asSeconds());
    int minutes = seconds / 60;
    seconds %= 60;
    ss.str("");
    ss << "Time: " << minutes << ":" << (seconds < 10 ? "0" : "") << seconds;
    timeText.setString(ss.str());

    nitroBar.setSize(sf::Vector2f(200 * nitroAmount, 20));

    sf::Vector2f viewSize = window.getView().getSize();
    sf::Vector2f viewCenter = window.getView().getCenter();

    scoreText.setPosition(viewCenter.x - viewSize.x / 2 + 20, viewCenter.y - viewSize.y / 2 + 20);
    distanceText.setPosition(viewCenter.x - viewSize.x / 2 + 20, viewCenter.y - viewSize.y / 2 + 60);
    timeText.setPosition(viewCenter.x - viewSize.x / 2 + 20, viewCenter.y - viewSize.y / 2 + 100);

    nitroText.setPosition(viewCenter.x + viewSize.x / 2 - 220, viewCenter.y - viewSize.y / 2 + 20);
    nitroBarBackground.setPosition(viewCenter.x + viewSize.x / 2 - 220, viewCenter.y - viewSize.y / 2 + 60);
    nitroBar.setPosition(viewCenter.x + viewSize.x / 2 - 220, viewCenter.y - viewSize.y / 2 + 60);
}

void Game::resetGame() {
    score = 0;
    distance = 0;
    nitroAmount = 0;
    nitroActive = false;
    gameClock.restart();
    setupWorld();
}

void Game::changeTheme(Theme newTheme) {
    currentTheme = newTheme;

    if (currentTheme == EARTH) {
        if (!backgroundTexture.loadFromFile("imgs/background.jpg") ||
            !groundTexture.loadFromFile("imgs/grass.jpg")) {
            window.close();
        }
    }
    else {
        if (!backgroundTexture.loadFromFile("imgs/moon_background.jpg") ||
            !groundTexture.loadFromFile("imgs/moon_ground.jpg")) {
            window.close();
        }
        // Очищаем декорации для Луны
        clouds.clear();
        decorations.clear();
    }

    groundTexture.setRepeated(true);
    groundTexture.setSmooth(true);

    backgroundSprite.setTexture(backgroundTexture);
    backgroundSprite.setScale(
        static_cast<float>(window.getSize().x) / backgroundTexture.getSize().x,
        static_cast<float>(window.getSize().y) / backgroundTexture.getSize().y);
}
