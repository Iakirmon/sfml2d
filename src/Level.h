#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

#include "Platform.h"
#include "Coin.h"
#include "Player.h"

class Level {
public:
    void load();
    void checkCollisions(Player& player);
    bool allCoinsCollected() const;
    void draw(sf::RenderWindow& window);

    std::size_t getTotalCoins() const;
    std::size_t getCollectedCoins() const;

    static constexpr float LEVEL_WIDTH = 3840.f;
    float getLevelWidth() const { return LEVEL_WIDTH; }

private:
    std::vector<Platform> platforms_;
    std::vector<Coin> coins_;
};
