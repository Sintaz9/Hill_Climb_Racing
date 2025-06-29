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
        inMenu(true)  {

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

        checkFinish();
    }





    void Game::setupWorld() {
        std::random_device rd;
        std::mt19937 rng(rd());
        world.SetGravity(b2Vec2(0, currentTheme == EARTH ? 9.8f : 1.6f));

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
                float y = pt.y * SCALE +2.f;

                if (decType(rng) <= 2) {
                    decorations.emplace_back(bushTexture, sf::Vector2f(x, y));
                }
                else {
                    decorations.emplace_back(treeTexture, sf::Vector2f(x, y));
                }
            }
        }

        // Генерация монет с разными типами
        std::uniform_int_distribution<int> coinInterval(5, 15); // Уменьшили интервал для большего количества монет
        std::uniform_real_distribution<float> coinHeight(1.2f, 3.0f);
        std::uniform_real_distribution<float> coinOffset(-2.0f, 2.0f); // Увеличили диапазон смещения
        std::uniform_int_distribution<int> coinType(0, 100);

        coins.clear();
        const auto& points = terrain->getPoints();
        for (size_t i = 25; i < points.size() - 10; i += coinInterval(rng)) { // Добавили проверку на выход за границы
            b2Vec2 pos = points[i];
            pos.x += coinOffset(rng);
            pos.y -= std::max(coinHeight(rng), 1.5f);

            // Проверяем, чтобы монета не уходила под землю
            if (i > 0 && pos.y > points[i - 1].y - 0.5f) {
                pos.y = points[i - 1].y - 1.5f;
            }

            // Определяем тип монеты
            Coin::Type type = Coin::Type::NORMAL;
            int typeRoll = coinType(rng);
            if (typeRoll > 90) {
                type = Coin::Type::SPECIAL;
            }
            else if (typeRoll > 70) {
                type = Coin::Type::GOLD;
            }

            coins.push_back(std::make_unique<Coin>(world, coinTexture, pos, type));
            std::cout << "Coin generated at: " << pos.x << ", " << pos.y << std::endl; // Отладочный вывод
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

        // Создание машины
        float startX = points[5].x + 1.5f;
        float startY = points[5].y - 1.8f;
        car = std::make_unique<Car>(world,
            currentTheme == EARTH ? 1.5f : 0.8f,
            currentTheme == EARTH ? 1.2f : 0.6f);
        car->getBody()->SetTransform(b2Vec2(startX, startY), 2 * b2_pi);

        terrain->draw(window);
        terrain->getPoints();
        terrain->getFinishLineX();
        setupFinish();
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
        const float coinCollectionDistance = 130.0f; // Расстояние сбора в пикселях
        const float squaredDistance = coinCollectionDistance * coinCollectionDistance; // Квадрат расстояния для оптимизации

        // Сбор монет
        for (auto& coin : coins) {
            if (!coin->isCollected()) {
                b2Vec2 carPos = car->getPosition();
                b2Vec2 coinPos = coin->getPosition();

                float dx = carPos.x - coinPos.x;
                float dy = carPos.y - coinPos.y;
                float distSqr = dx * dx + dy * dy;

                if (distSqr < 0.5f * 0.5f) {
                    coin->collect();
                    score += coin->getValue();
                    std::cout << "Coin collected! Score: " << score << std::endl;

                    animatingCoins.push_back({
                        static_cast<size_t>(&coin - &coins[0]),
                        0.0f,
                        0.1f
                        });
                }
            }
        }

        const auto& points = terrain->getPoints();
        // Обновляем анимации монет
        for (auto it = animatingCoins.begin(); it != animatingCoins.end(); ) {
            it->timer += dt;
            float progress = it->timer / COIN_ANIM_TIME;

            if (progress >= 1.0f) {
                it = animatingCoins.erase(it);
            }
            //else {
            //    // Анимация увеличения и исчезновения
            //    float scale = it->startScale * (1.0f + progress); // Увеличиваем
            //    float alpha = 255 * (1.0f - progress); // Прозрачность
            //    coins[it->index].setScale(scale, scale);
            //    coins[it->index].setColor(sf::Color(255, 255, 255, alpha));
            //    ++it;
            //}
        }
        // Обновляем препятствия
        for (auto& obstacle : obstacles) {
            obstacle.update();
        }
        updateHUD();
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
            terrain->draw(window);

            // Декорации только для Земли
            if (currentTheme == EARTH) {
                for (auto& deco : decorations) {
                    deco.draw(window);
                }
            }

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
        if (!finishTexture.loadFromFile("imgs/finish_flag.png")) {
        }
        finishFlag.setTexture(finishTexture);
        finishLineX = terrain->getFinishLineX();
        finishFlag.setPosition(finishLineX * SCALE, terrain->getPoints().back().y * SCALE - 128);
        finishFlag.setScale(0.5f, 0.5f);

        gameFinished = false;

        victoryText.setFont(font);
        victoryText.setString("VICTORY!\nScore: " + std::to_string(score) +
            "\nDistance: " + std::to_string(static_cast<int>(distance)) + "m");
        victoryText.setCharacterSize(60);
        victoryText.setFillColor(sf::Color::Yellow);
        victoryText.setOutlineColor(sf::Color::White);
        victoryText.setOutlineThickness(3);
        victoryText.setPosition(window.getSize().x / 2 - 200, window.getSize().y / 2 - 100);
    }

    void Game::checkFinish() {
        if (!gameFinished && car->getPosition().x >= finishLineX) {
            gameFinished = true;
            showVictoryScreen();
        }
    }

    void Game::showVictoryScreen() {
        sf::RenderTexture renderTexture;
        renderTexture.create(window.getSize().x, window.getSize().y);
        renderTexture.clear(sf::Color(0, 0, 0, 180));

        render();

        renderTexture.display();
        sf::Sprite overlay(renderTexture.getTexture());
        window.draw(overlay);
        window.draw(victoryText);
        window.display();

        sf::Event event;
        while (window.waitEvent(event)) {
            if (event.type == sf::Event::KeyPressed ||
                event.type == sf::Event::MouseButtonPressed) {
                inMenu = true;
                break;
            }
        }
    }