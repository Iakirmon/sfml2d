#pragma once

#include <SFML/Graphics.hpp>

class Coin {
public:
    Coin(float x, float y);

    void draw(sf::RenderWindow& window);
    sf::FloatRect getHitbox() const;
    bool isCollected() const;
    void collect();

private:
    sf::CircleShape shape_;
    bool collected_{false};
};

