#include "Platform.h"

Platform::Platform(float x, float y, float width, float height) {
    shape_.setSize({width, height});          // szerokość × wysokość prostokąta
    shape_.setFillColor(sf::Color(80, 180, 80)); // zielony kolor wypełnienia
    shape_.setPosition(x, y);                // lewy górny róg w world space
}

void Platform::draw(sf::RenderWindow& window) {
    window.draw(shape_);
}

// getGlobalBounds() zwraca prostokąt uwzględniający transformacje (pozycja, skala).
// Używamy go zarówno do rysowania jak i kolizji — jeden prostokąt do obu celów.
sf::FloatRect Platform::getHitbox() const {
    return shape_.getGlobalBounds();
}
