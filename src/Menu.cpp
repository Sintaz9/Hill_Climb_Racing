#include "../include/Menu.h"

Menu::MenuResult Menu::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            return getMenuResponse(window.mapPixelToCoords(
                sf::Vector2i(event.mouseButton.x, event.mouseButton.y)));
        }
    }

    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::E) return EARTH_SELECTED;
        if (event.key.code == sf::Keyboard::M) return MOON_SELECTED;
        if (event.key.code == sf::Keyboard::Escape) return EXIT;
    }

    return NOTHING;
}

void Menu::draw(sf::RenderWindow& window) const {
    window.draw(background);
    window.draw(title);
    window.draw(earthButton);
    window.draw(moonButton);
    window.draw(exitButton);
}

void Menu::setup(const sf::Vector2u& windowSize) {
    if (!backgroundTexture.loadFromFile("imgs/background.jpg")) {
        // Обработка ошибки
    }
    background.setTexture(backgroundTexture);
    background.setScale(
        static_cast<float>(windowSize.x) / backgroundTexture.getSize().x,
        static_cast<float>(windowSize.y) / backgroundTexture.getSize().y);

    if (!font.loadFromFile("imgs/arial/arialmt.ttf")) {
        font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
    }

    title.setFont(font);
    title.setString("HILL CLIMB RACING");
    title.setCharacterSize(80);
    title.setFillColor(sf::Color::Red);
    title.setOutlineColor(sf::Color::Black);
    title.setOutlineThickness(3);
    title.setPosition(windowSize.x / 2 - title.getGlobalBounds().width / 2, 100);

    earthButton.setFont(font);
    earthButton.setString("EARTH (Press E)");
    earthButton.setCharacterSize(50);
    earthButton.setFillColor(sf::Color::Green);
    earthButton.setOutlineColor(sf::Color::Black);
    earthButton.setOutlineThickness(2);
    earthButton.setPosition(windowSize.x / 2 - earthButton.getGlobalBounds().width / 2, 300);

    moonButton.setFont(font);
    moonButton.setString("MOON (Press M)");
    moonButton.setCharacterSize(50);
    moonButton.setFillColor(sf::Color::Cyan);
    moonButton.setOutlineColor(sf::Color::Black);
    moonButton.setOutlineThickness(2);
    moonButton.setPosition(windowSize.x / 2 - moonButton.getGlobalBounds().width / 2, 400);

    exitButton.setFont(font);
    exitButton.setString("EXIT (Press Esc)");
    exitButton.setCharacterSize(50);
    exitButton.setFillColor(sf::Color::Red);
    exitButton.setOutlineColor(sf::Color::Black);
    exitButton.setOutlineThickness(2);
    exitButton.setPosition(windowSize.x / 2 - exitButton.getGlobalBounds().width / 2, 500);
}

Menu::MenuResult Menu::getMenuResponse(const sf::Vector2f& mousePos) const {
    if (earthButton.getGlobalBounds().contains(mousePos)) return EARTH_SELECTED;
    if (moonButton.getGlobalBounds().contains(mousePos)) return MOON_SELECTED;
    if (exitButton.getGlobalBounds().contains(mousePos)) return EXIT;
    return NOTHING;
}