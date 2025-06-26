#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>

class Menu {
public:
    enum MenuResult { NOTHING, EARTH_SELECTED, MOON_SELECTED, EXIT };

    MenuResult handleEvent(const sf::Event& event, sf::RenderWindow& window); // Добавлен window
    void draw(sf::RenderWindow& window) const;
    void setup(const sf::Vector2u& windowSize);

private:
    MenuResult getMenuResponse(const sf::Vector2f& mousePos) const;

    sf::Texture backgroundTexture;
    sf::Sprite background;
    sf::Font font;
    sf::Text title;
    sf::Text earthButton;
    sf::Text moonButton;
    sf::Text exitButton;
};

#endif // MENU_H