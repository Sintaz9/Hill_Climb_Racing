#include "../include/Game.h"
#include <sstream>
#include <cmath>
#include <random>
#include <iostream>

constexpr float SCALE = 100.0f;

Game::Game() :
    window(sf::VideoMode::getDesktopMode(), "Hill Climb Racing", sf::Style::Fullscreen),
    world(b2Vec2(0, 0)),
    currentTheme(EARTH),
    score(0),
    distance(0),
    nitroAmount(0),
    nitroActive(false),
    inMenu(true),
    gameFinished(false) {

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
    camera = std::make_unique<Camera>(window, 1.0f, 0.6f);//Отдаление при движении     Первый параметр - базовый зум    Второй параметр - диапазон изменения зума
    camera->setFollowParameters(300.f, -150.f, 0.1f); 
    // Инициализация текста победы
    if (!font.loadFromFile("imgs/arial/arialmt.ttf")) {
        font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
    }
    victoryText.setFont(font);
    victoryText.setCharacterSize(60);
    victoryText.setFillColor(sf::Color::Yellow);
    victoryText.setOutlineColor(sf::Color::Black);
    victoryText.setOutlineThickness(3);

    groundTexture.setRepeated(true);
    groundTexture.setSmooth(true);
}

void Game::run() {
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        processEvents();

        if (!inMenu && !gameFinished) {
            update(dt);
        }
        if (gameFinished) {
            showVictoryScreen();
            gameFinished = false; // Чтобы showVictoryScreen() не вызывалась снова
            continue;
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
            else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
                inMenu = true;  // Теперь по нажатию R сразу в меню
            }
            else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::LShift) {
                if (nitroAmount > 0.1f) nitroActive = true;
            }
            else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::LShift) {
                nitroActive = false;
            }
        }
    }
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
    world.SetGravity(b2Vec2(0, currentTheme == EARTH ? 9.8f : 4.6f));

    // Генерация террейна
    terrain = std::make_unique<Terrain>(world, groundTexture);

    // Настройка камеры
    float lastX = terrain->getPoints().back().x * SCALE;

    // Генерация облаков только для Земли
    if (currentTheme == EARTH) {
        std::uniform_real_distribution<float> cloudXDist(0.f, lastX * 1.5f);
        std::uniform_real_distribution<float> cloudYDist(-2000.f, -100.f); //Высота генерации облаков
        std::uniform_real_distribution<float> parallaxDist(0.1f, 0.9f);
        std::uniform_real_distribution<float> cloudScale(0.2f, 0.6f);

        clouds.clear();
        for (int i = 0; i < 400; ++i) {
            Cloud cloud(
                cloudTexture,
                sf::Vector2f(cloudXDist(rng), cloudYDist(rng)),
                parallaxDist(rng)
            );
            float scale = cloudScale(rng);
            cloud.sprite.setScale(scale, scale);
            clouds.push_back(cloud);
        }
    }

    // Генерация декораций только для Земли
    if (currentTheme == EARTH) {
        std::uniform_int_distribution<int> decType(0, 3);
        std::uniform_real_distribution<float> decXOffset(-1.5f, 1.5f);

        decorations.clear();
        for (size_t i = 20; i < terrain->getPoints().size(); i += 10) {
            const auto& pt = terrain->getPoints()[i];
            float x = pt.x * SCALE + decXOffset(rng) * 70.f;
            float y = pt.y * SCALE +3.1f;

            if (decType(rng) <= 2) {
                decorations.emplace_back(bushTexture, sf::Vector2f(x, y));
            }
            else {
                decorations.emplace_back(treeTexture, sf::Vector2f(x, y));
            }
        }
    }

    // Генерация монет:
    std::uniform_int_distribution<int> coinInterval(5, 15);
    std::uniform_real_distribution<float> coinOffset(-1.5f, 1.5f);
    std::uniform_real_distribution<float> heightVariation(0.5f, 1.2f);

    coins.clear();
    const auto& points = terrain->getPoints();
    for (size_t i = 25; i < points.size() - 10; i += coinInterval(rng)) {
        b2Vec2 pos = points[i];
        pos.x += coinOffset(rng);

        // Гарантированное размещение над дорогой
        float minY = points[i].y;
        if (i > 0) minY = std::min(minY, points[i - 1].y);
        if (i < points.size() - 1) minY = std::min(minY, points[i + 1].y);

        pos.y = minY - heightVariation(rng); // Всегда выше самой низкой точки

        // Дополнительная проверка для крутых склонов
        if (i > 1 && std::abs(points[i].y - points[i - 1].y) > 0.5f) {
            pos.y = minY - 1.0f; // Больше отступ на склонах
        }

        coins.push_back(std::make_unique<Coin>(world, coinTexture, pos));
    }



    // Генерация препятствий
    std::uniform_int_distribution<int> obstacleInterval(80, 150);
    obstacles.clear();
    for (size_t i = 30; i < points.size(); i += obstacleInterval(rng)) {
        b2Vec2 pos = points[i];
        pos.y -= 0.7f;
        obstacles.emplace_back(world, obstacleTexture, pos,
            currentTheme == EARTH ? 0.8f : 0.4f);
    }

    // Создание машины - сдвигаем начальную позицию правее
    float startX = points[10].x + 3.0f;  // Было points[5].x + 1.5f
    float startY = points[10].y - 1.8f;
    car = std::make_unique<Car>(world,
        currentTheme == EARTH ? 1.5f : 0.8f,
        currentTheme == EARTH ? 1.2f : 0.6f);
    car->getBody()->SetTransform(b2Vec2(startX, startY), 13 * b2_pi / 6);

    setupFinish();
    std::cout << "Finish line at x = " << finishLineX << std::endl;
}




void Game::update(float dt) {
    world.Step(dt, 10, 8);
    car->update(dt, nitroActive);

    if (gameFinished) return;

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
    const float collectionRadius = 2.3f;
    for (auto it = coins.begin(); it != coins.end(); ) {
        if (!(*it)->isCollected()) {
            b2Vec2 carPos = car->getPosition();
            b2Vec2 coinPos = (*it)->getPosition();

            float dx = carPos.x - coinPos.x;
            float dy = carPos.y - coinPos.y;
            float distSqr = dx * dx + dy * dy;

            if (distSqr < collectionRadius * collectionRadius) {
                (*it)->collect();
                score += (*it)->getValue();
                it = coins.erase(it); // Немедленно удаляем собранную монету
                continue;
            }
        }
        ++it;
    }
    for (auto& coin : coins) {
        coin->update(dt); // Это вызовет вращение монет
    }
    const auto& points = terrain->getPoints();
       
    // Обновляем препятствия
    for (auto& obstacle : obstacles) {
        obstacle.update();
    }
    updateHUD();
    checkFinish();
    std::cout << "Car X = " << car->getPosition().x << std::endl;

}



void Game::render() {
    window.clear();

    if (inMenu) {
        window.setView(window.getDefaultView());
        menu.draw(window);
    }
    else {
        camera->applyToWindow();

        // Фон
        sf::View fixedView = window.getDefaultView();
        window.setView(fixedView);
        window.draw(backgroundSprite);
        window.setView(camera->getView());

        // Финишный флаг
        window.draw(finishFlag);


        // Облака только для Земли
        if (currentTheme == EARTH) {
            for (auto& cloud : clouds) {
                cloud.update(camera->getView());
                cloud.draw(window);
            }
        }


        // Декорации только для Земли
        if (currentTheme == EARTH) {
            for (auto& deco : decorations) {
                deco.draw(window);
            }
        }

        // Земля
        terrain->draw(window);

        // Монеты
        for (auto& coin : coins) {
            coin->draw(window);
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






// Реализация структуры Cloud
Cloud::Cloud(const sf::Texture& texture, const sf::Vector2f& position, float parallax) :
    parallaxFactor(parallax),
    originalPosition(position)
{
    sprite.setTexture(texture);
    sprite.setPosition(position);
}

void Cloud::update(const sf::View& view) {
    originalPosition.x -= 0.5f * (1.0f - parallaxFactor);

    float x = originalPosition.x - (view.getCenter().x - view.getSize().x / 2) * (1.0f - parallaxFactor);
    sprite.setPosition(x, originalPosition.y);
}

void Cloud::draw(sf::RenderWindow& window) const {
    window.draw(sprite);
}



Decoration::Decoration(const sf::Texture& texture, const sf::Vector2f& pos) :
    position(pos)
{
    sprite.setTexture(texture);
    sprite.setPosition(pos);

    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2, bounds.height);

    float scale = 0.4f + (std::rand() % 20) * 0.01f;
    sprite.setScale(scale, scale);
}

void Decoration::draw(sf::RenderWindow& window) const {
    window.draw(sprite);
}




void Game::setupFinish() {
    // Пытаемся загрузить текстуру флага
    if (!finishTexture.loadFromFile("imgs/finish_flag.png")) {
        // Создаем простую текстуру для отладки
        finishTexture.create(128, 256);
        sf::Uint8* pixels = new sf::Uint8[128 * 256 * 4];
        for (int y = 0; y < 256; y++) {
            for (int x = 0; x < 128; x++) {
                int index = (y * 128 + x) * 4;
                bool stripe = (y / 16) % 2 == 0;
                pixels[index] = stripe ? 255 : 0;
                pixels[index + 1] = stripe ? 0 : 255;
                pixels[index + 2] = 0;
                pixels[index + 3] = 255;
            }
        }
        finishTexture.update(pixels);
        delete[] pixels;
    }
    finishFlag.setTexture(finishTexture);
    finishLineX = terrain->getFinishLineX();

    // Получаем Y-координату дороги в точке финиша
    const auto& points = terrain->getPoints();
    float roadY = points.back().y * SCALE;

    // Критическое исправление: устанавливаем флаг перед последним сегментом трассы
    finishFlag.setPosition(finishLineX * SCALE, roadY);

    finishFlag.setScale(0.5f, 0.5f);
    finishFlag.setOrigin(0, finishTexture.getSize().y);


    gameFinished = false;

}

void Game::checkFinish() {
    if (!gameFinished && car && car->getPosition().x >= finishLineX) {
        gameFinished = true;
    }
}
void Game::showVictoryScreen() {
    // Убедимся, что окно активно
    if (!window.isOpen()) return;

    // Сохраняем текущую позицию камеры
    sf::View currentView = window.getView();
    window.setView(window.getDefaultView());

    // Подготовка текста
    int seconds = static_cast<int>(gameClock.getElapsedTime().asSeconds());
    int minutes = seconds / 60;
    seconds %= 60;

    victoryText.setString(
        "VICTORY!\n\n"
        "Score: " + std::to_string(score) + "\n"
        "Distance: " + std::to_string(static_cast<int>(distance)) + "m\n"
        "Time: " + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds)
    );

    // Центрирование текста
    sf::FloatRect textBounds = victoryText.getLocalBounds();
    victoryText.setOrigin(textBounds.width / 2, textBounds.height / 2);
    victoryText.setPosition(window.getSize().x / 2, window.getSize().y / 2 - 50);

    // Оверлей и текст продолжения
    sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));

    sf::Text continueText;
    continueText.setFont(font);
    continueText.setString("Press any key to continue");
    continueText.setCharacterSize(30);
    continueText.setFillColor(sf::Color::White);
    continueText.setPosition(window.getSize().x / 2, window.getSize().y / 2 + 100);
    sf::FloatRect contBounds = continueText.getLocalBounds();
    continueText.setOrigin(contBounds.width / 2, contBounds.height / 2);

    // Основной цикл экрана победы
    sf::Clock displayClock;
    bool waiting = true;

    // ОЧИСТКА очереди событий перед началом
    sf::Event flush;
    while (window.pollEvent(flush)) {}

    while (waiting && window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
            if (event.type == sf::Event::KeyPressed ||
                event.type == sf::Event::MouseButtonPressed) {
                waiting = false;
            }
        }

        // Отрисовка
        window.clear();
        render(); // Основная сцена
        window.draw(overlay);
        window.draw(victoryText);
        window.draw(continueText);
        window.display();
    }

    inMenu = true;
    menu.setup(window.getSize());
    window.setView(currentView);
}