#pragma once

#include <SFML/Graphics.hpp>

class Platform {
public:
    Platform(float x, float y, float width, float height);

    void draw(sf::RenderWindow& window);
    sf::FloatRect getHitbox() const;

private:
    sf::RectangleShape shape_;
};

