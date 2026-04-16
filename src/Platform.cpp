#include "Platform.h"

Platform::Platform(float x, float y, float width, float height) {
    shape_.setSize({width, height});
    shape_.setFillColor(sf::Color(80, 180, 80));
    shape_.setPosition(x, y);
}

void Platform::draw(sf::RenderWindow& window) {
    window.draw(shape_);
}

sf::FloatRect Platform::getHitbox() const {
    return shape_.getGlobalBounds();
}

