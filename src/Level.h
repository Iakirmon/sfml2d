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

private:
    std::vector<Platform> platforms_;
    std::vector<Coin> coins_;
};
