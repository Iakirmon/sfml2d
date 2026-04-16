#include "Level.h"

#include <algorithm>

void Level::load() {
    platforms_.clear();
    coins_.clear();

    // Floor
    platforms_.emplace_back(0.f,    680.f, 1280.f, 40.f);

    // Platforms
    platforms_.emplace_back(100.f,  500.f,  200.f, 20.f);
    platforms_.emplace_back(350.f,  400.f,  150.f, 20.f);
    platforms_.emplace_back(550.f,  520.f,  200.f, 20.f);
    platforms_.emplace_back(800.f,  500.f,  180.f, 20.f);
    platforms_.emplace_back(1050.f, 520.f,  160.f, 20.f);

    // Coins
    coins_.emplace_back(200.f,  460.f);
    coins_.emplace_back(425.f,  360.f);
    coins_.emplace_back(650.f,  468.f);
    coins_.emplace_back(890.f,  360.f);
    coins_.emplace_back(1130.f, 460.f);
}

void Level::checkCollisions(Player& player) {
    sf::FloatRect pBox = player.getHitbox();

    for (auto& platform : platforms_) {
        sf::FloatRect platBox = platform.getHitbox();
        sf::FloatRect overlap;

        if (!pBox.intersects(platBox, overlap)) continue;

        if (overlap.width < overlap.height) {
            float pushX = (pBox.left < platBox.left) ? -overlap.width : overlap.width;
            player.moveShape({pushX, 0.f});
            player.setVelocityX(0.f);
        } else {
            float pushY = (pBox.top < platBox.top) ? -overlap.height : overlap.height;
            player.moveShape({0.f, pushY});
            player.setVelocityY(0.f);
            if (pushY < 0.f) {
                player.setOnGround(true);
            }
        }

        pBox = player.getHitbox();
    }

    pBox = player.getHitbox();
    for (auto& coin : coins_) {
        if (!coin.isCollected() && pBox.intersects(coin.getHitbox())) {
            coin.collect();
        }
    }
}

bool Level::allCoinsCollected() const {
    return std::all_of(coins_.begin(), coins_.end(),
                       [](const Coin& c) { return c.isCollected(); });
}

void Level::draw(sf::RenderWindow& window) {
    for (auto& platform : platforms_) {
        platform.draw(window);
    }
    for (auto& coin : coins_) {
        coin.draw(window);
    }
}

std::size_t Level::getTotalCoins() const {
    return coins_.size();
}

std::size_t Level::getCollectedCoins() const {
    return static_cast<std::size_t>(std::count_if(
        coins_.begin(), coins_.end(),
        [](const Coin& c) { return c.isCollected(); }));
}

