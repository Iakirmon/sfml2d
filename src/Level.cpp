#include "Level.h"

#include <algorithm>

void Level::load() {
    platforms_.clear();
    coins_.clear();

    // ── Podłoga (cały poziom) ────────────────────────────────────────
    platforms_.emplace_back(0.f, 680.f, 3840.f, 40.f);

    // ── Sekcja 1: x = 0 – 1280 (oryginalna) ─────────────────────────
    platforms_.emplace_back(100.f,  500.f, 200.f, 20.f);
    platforms_.emplace_back(350.f,  400.f, 150.f, 20.f);
    platforms_.emplace_back(550.f,  520.f, 200.f, 20.f);
    platforms_.emplace_back(800.f,  500.f, 180.f, 20.f);
    platforms_.emplace_back(1050.f, 520.f, 160.f, 20.f);

    coins_.emplace_back(200.f,  460.f);
    coins_.emplace_back(425.f,  360.f);
    coins_.emplace_back(650.f,  468.f);
    coins_.emplace_back(890.f,  360.f);
    coins_.emplace_back(1130.f, 460.f);

    // ── Sekcja 2: x = 1280 – 2100 (łatwa) ───────────────────────────
    platforms_.emplace_back(1300.f, 450.f, 200.f, 20.f);
    platforms_.emplace_back(1550.f, 360.f, 180.f, 20.f);
    platforms_.emplace_back(1780.f, 480.f, 160.f, 20.f);
    platforms_.emplace_back(2000.f, 380.f, 200.f, 20.f);

    coins_.emplace_back(1400.f, 410.f);
    coins_.emplace_back(1640.f, 320.f);
    coins_.emplace_back(1860.f, 440.f);
    coins_.emplace_back(2100.f, 340.f);

    // ── Sekcja 3: x = 2100 – 2950 (średnia) ─────────────────────────
    platforms_.emplace_back(2250.f, 500.f, 150.f, 20.f);
    platforms_.emplace_back(2450.f, 380.f, 130.f, 20.f);
    platforms_.emplace_back(2650.f, 460.f, 150.f, 20.f);
    platforms_.emplace_back(2850.f, 340.f, 120.f, 20.f);

    coins_.emplace_back(2325.f, 460.f);
    coins_.emplace_back(2515.f, 340.f);
    coins_.emplace_back(2725.f, 420.f);
    coins_.emplace_back(2910.f, 300.f);

    // ── Sekcja 4: x = 2950 – 3780 (trudna) ──────────────────────────
    platforms_.emplace_back(3050.f, 520.f, 200.f, 20.f);
    platforms_.emplace_back(3300.f, 400.f, 180.f, 20.f);
    platforms_.emplace_back(3500.f, 480.f, 160.f, 20.f);
    platforms_.emplace_back(3700.f, 360.f, 120.f, 20.f);

    coins_.emplace_back(3150.f, 480.f);
    coins_.emplace_back(3390.f, 360.f);
    coins_.emplace_back(3580.f, 440.f);
    coins_.emplace_back(3760.f, 320.f);
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
