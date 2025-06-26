#include "../include/Game.h"
#include <sstream>
#include <cmath>
#include <random>

constexpr float SCALE = 100.0f;

Game::Game() :
    window(sf::VideoMode::getDesktopMode(), "Hill Climb Racing", sf::Style::Fullscreen),
    world(b2Vec2(0, 0)),
    currentTheme(EARTH),
    score(0),
    distance(0),
    nitroAmount(0),
    nitroActive(false),
    inMenu(true) {

    window.setFramerateLimit(60);

    // Загрузка текстур
    if (!backgroundTexture.loadFromFile("imgs/background.jpg") ||
        !coinTexture.loadFromFile("imgs/coin.png") ||
        !obstacleTexture.loadFromFile("imgs/Obstacle.png")) {
        window.close();
    }
    if (!cloudTexture.loadFromFile("imgs/cloud.png") ||
        !bushTexture.loadFromFile("imgs/bush.png") ||
        !treeTexture.loadFromFile("imgs/tree.png")) {
        window.close();
    }
    menu.setup(window.getSize());
    setupHUD();
    camera = std::make_unique<Camera>(window, 2.5f, 1.5f);
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

    for (size_t i = 0; i < coins.size(); ++i) {
        if (!coinsCollected[i]) {
            sf::Vector2f coinPos = coins[i].getPosition();
            float dist = std::hypot(carPos.x * SCALE - coinPos.x, carPos.y * SCALE - coinPos.y);
            if (dist < 50.0f) {
                coinsCollected[i] = true;
                score += 10;
            }
        }
    }

    for (auto& obstacle : obstacles) {
        obstacle.update();
        if (obstacle.shouldBreak(car->getBody())) {
            obstacle.breakApart(world);
        }
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

        // Облака
        for (auto& cloud : clouds) {
            cloud.update(camera->getView());
            cloud.draw(window);
        }

        // Земля
        window.draw(groundMesh, &groundTexture);

        // Декорации
        for (auto& deco : decorations) {
            deco.update(camera->getView());
            deco.draw(window);
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





void Game::setupWorld() {
    // Инициализация генератора случайных чисел
    std::random_device rd;
    std::mt19937 rng(rd());

    world.SetGravity(b2Vec2(0, currentTheme == EARTH ? 9.8f : 1.6f));

    // Генерация террейна
    terrain.generate(currentTheme == EARTH ? 1000 : 1500,
        currentTheme == EARTH ? 12.0f : 8.0f);

    // Создание физического тела земли
    terrain.createPhysics(world);

    // Создание графического меша
    groundMesh = terrain.createMesh(groundTexture);

    // Генерация облаков
    std::uniform_real_distribution<float> cloudXDist(0.f, terrain.getPoints().back().x * SCALE);
    std::uniform_real_distribution<float> cloudYDist(-800.f, -300.f);
    std::uniform_real_distribution<float> parallaxDist(0.1f, 0.9f);

    clouds.clear();
    for (int i = 0; i < 100; ++i) {
        clouds.emplace_back(
            cloudTexture,
            sf::Vector2f(cloudXDist(rng), cloudYDist(rng)),
            parallaxDist(rng)
        );
    }

    // Генерация декораций
    std::uniform_int_distribution<int> decType(0, 3);
    std::uniform_real_distribution<float> decXOffset(-0.5f, 0.5f);

    decorations.clear();
    for (size_t i = 0; i < terrain.getPoints().size(); i += 15) {
        if (i >= terrain.getPoints().size()) break;

        b2Vec2 pt = terrain.getPoints()[i];
        float x = pt.x * SCALE + decXOffset(rng) * 50.f;
        float y = pt.y * SCALE;

        if (decType(rng) <= 1) {
            decorations.emplace_back(
                bushTexture,
                sf::Vector2f(x, y),
                0.8f,
                y
            );
        }
        else {
            decorations.emplace_back(
                treeTexture,
                sf::Vector2f(x, y),
                0.7f,
                y
            );
        }
    }

    // Генерация монет
    std::uniform_int_distribution<int> coinInterval(50, 150);
    coins.clear();
    coinsCollected.clear();

    for (size_t i = 0; i < terrain.getPoints().size(); i += coinInterval(rng)) {
        if (i >= terrain.getPoints().size()) break;

        b2Vec2 pos = terrain.getPoints()[i];
        pos.y -= 1.0f;

        sf::Sprite coin(coinTexture);
        coin.setOrigin(coinTexture.getSize().x / 2, coinTexture.getSize().y / 2);
        coin.setScale(0.1f, 0.1f);
        coin.setPosition(pos.x * SCALE, pos.y * SCALE);

        coins.push_back(coin);
        coinsCollected.push_back(false);
    }

    // Создание машины
    auto& points = terrain.getPoints();
    float startX = points[0].x + 2.0f;
    float startY = points[0].y - 2.0f;

    car = std::make_unique<Car>(world,
        currentTheme == EARTH ? 1.5f : 0.8f,
        currentTheme == EARTH ? 1.2f : 0.6f);
    car->getBody()->SetTransform(b2Vec2(startX, startY), 0);

    // Настройка камеры
    camera->setZoom(currentTheme == EARTH ? 2.0f : 3.0f);
    camera->setWorldBounds(sf::FloatRect(
        0, -500,
        terrain.getPoints().back().x * SCALE, 5000
    ));
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
    }

    backgroundSprite.setTexture(backgroundTexture);
    backgroundSprite.setScale(
        static_cast<float>(window.getSize().x) / backgroundTexture.getSize().x,
        static_cast<float>(window.getSize().y) / backgroundTexture.getSize().y);
}