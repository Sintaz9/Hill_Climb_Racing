#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <vector>
#include <memory>
#include "Car.h"
#include "Terrain.h"
#include "Obstacle.h"
#include "Menu.h"
#include "Camera.h"

struct Cloud {
    sf::Sprite sprite;
    float parallaxFactor;
    sf::Vector2f originalPosition;

    Cloud(const sf::Texture& texture, const sf::Vector2f& position, float parallax)
        : parallaxFactor(parallax), originalPosition(position) {
        sprite.setTexture(texture);
        sprite.setPosition(position);
        sprite.setScale(0.4f, 0.4f);
    }

    void update(const sf::View& view) {
        float x = originalPosition.x - (view.getCenter().x - view.getSize().x / 2) * (1.0f - parallaxFactor);
        sprite.setPosition(x, originalPosition.y);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(sprite);
    }
};

struct Decoration {
    sf::Sprite sprite;
    sf::Vector2f position;

    Decoration(const sf::Texture& texture, const sf::Vector2f& pos)
        : position(pos) {
        sprite.setTexture(texture);
        sprite.setPosition(pos);

        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width / 2, bounds.height);

        float scale = 0.4f + (std::rand() % 20) * 0.01f;
        sprite.setScale(scale, scale);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(sprite);
    }
};

class Game {
public:
    enum Theme { EARTH, MOON };

    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void setupWorld();
    void setupHUD();
    void updateHUD();
    void drawHUD();
    void resetGame();
    void changeTheme(Theme newTheme);

    sf::RenderWindow window;
    b2World world;

    // Игровые объекты
    std::unique_ptr<Car> car;
    Terrain terrain;
    std::vector<Obstacle> obstacles;
    std::vector<sf::Sprite> coins;
    std::vector<bool> coinsCollected;
    std::unique_ptr<Camera> camera;


    std::vector<Cloud> clouds;
    std::vector<Decoration> decorations;
    sf::Texture cloudTexture;
    sf::Texture bushTexture;
    sf::Texture treeTexture;
    // Графика
    sf::Texture backgroundTexture;
    sf::Texture groundTexture;
    sf::Texture coinTexture;
    sf::Texture obstacleTexture;
    sf::Sprite backgroundSprite;

    // Интерфейс
    sf::Font font;
    sf::Text scoreText;
    sf::Text distanceText;
    sf::Text timeText;
    sf::Text nitroText;
    sf::RectangleShape nitroBar;
    sf::RectangleShape nitroBarBackground;

    // Физические параметры
    Theme currentTheme;
    float gravityScale;
    float carDensity;
    float carFriction;
    float obstacleDensity;

    //константы
    const float MAX_CAMERA_X = 10000.0f;
    const float MIN_CAMERA_X = 0.0f;
    const float MAX_CAMERA_Y = 1000.0f;
    const float MIN_CAMERA_Y = -500.0f;
    const float TEXTURE_REPEAT_FACTOR = 10.0f;
    const sf::FloatRect WORLD_BOUNDS = { 0, -500, 10000, 5000 };
    // Игровые параметры
    int score;
    float distance;
    sf::Clock gameClock;
    float nitroAmount;
    bool nitroActive;
    struct CoinAnim {
        size_t index;
        float timer;
        float startScale;
    };
    std::vector<CoinAnim> animatingCoins;
    const float COIN_ANIM_TIME = 0.3f; // Длительность анимации в секундах
    // Меню
    Menu menu;
    bool inMenu;
    //Удаляем копирование
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
};

#endif // GAME_H