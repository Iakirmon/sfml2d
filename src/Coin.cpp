#include "Coin.h"

Coin::Coin(float x, float y) {
    shape_.setRadius(10.f);
    shape_.setFillColor(sf::Color::Yellow);
    shape_.setOrigin(10.f, 10.f);
    shape_.setPosition(x, y);
}

void Coin::draw(sf::RenderWindow& window) {
    if (!collected_) {
        window.draw(shape_);
    }
}

sf::FloatRect Coin::getHitbox() const {
    if (collected_) {
        return sf::FloatRect(); // empty
    }
    return shape_.getGlobalBounds();
}

bool Coin::isCollected() const {
    return collected_;
}

void Coin::collect() {
    collected_ = true;
}

