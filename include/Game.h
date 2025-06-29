#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <vector>
#include <memory>
#include "Car.h"
#include "Terrain.h"
#include "Obstacle.h"
#include "Coin.h"
#include "Menu.h"
#include "Camera.h"

struct Cloud {
    sf::Sprite sprite;
    float parallaxFactor;
    sf::Vector2f originalPosition;

    Cloud(const sf::Texture& texture, const sf::Vector2f& position, float parallax);
    void update(const sf::View& view);
    void draw(sf::RenderWindow& window) const;
};

struct Decoration {
    sf::Sprite sprite;
    sf::Vector2f position;

    Decoration(const sf::Texture& texture, const sf::Vector2f& pos);
    void draw(sf::RenderWindow& window) const;
};

class Game {
public:
    enum Theme { EARTH, MOON };

    Game();
    void run();

    bool gameFinished;
    sf::Sprite finishFlag;
    sf::Texture finishTexture;
    sf::Text victoryText;
    float finishLineX;

    void setupFinish();
    void checkFinish();
    void showVictoryScreen();

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
    std::unique_ptr<Terrain> terrain;
    std::vector<Obstacle> obstacles;
    std::vector<std::unique_ptr<Coin>> coins;
    std::unique_ptr<Camera> camera;
    std::vector<Cloud> clouds;
    std::vector<Decoration> decorations;

    // Текстуры
    sf::Texture backgroundTexture;
    sf::Texture groundTexture;
    sf::Texture coinTexture;
    sf::Texture obstacleTexture;
    sf::Texture cloudTexture;
    sf::Texture bushTexture;
    sf::Texture treeTexture;
    sf::Sprite backgroundSprite;

    // Интерфейс
    sf::Font font;
    sf::Text scoreText;
    sf::Text distanceText;
    sf::Text timeText;
    sf::Text nitroText;
    sf::RectangleShape nitroBar;
    sf::RectangleShape nitroBarBackground;

    // Игровые параметры
    Theme currentTheme;
    int score;
    float distance;
    sf::Clock gameClock;
    float nitroAmount;
    bool nitroActive;
    bool inMenu;
    Menu menu;

    struct CoinAnim {
        size_t index;
        float timer;
        float startScale;
    };
    std::vector<CoinAnim> animatingCoins;
    const float COIN_ANIM_TIME = 0.3f;

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
};

#endif // GAME_H