#include "Coin.h"

Coin::Coin(float x, float y) {
    shape_.setRadius(10.f);              // promień koła = 10px
    shape_.setFillColor(sf::Color::Yellow);
    shape_.setOrigin(10.f, 10.f);        // origin na środku koła (promień, promień)
                                         // dzięki temu (x,y) to środek monety,
                                         // a nie lewy górny róg bounding boxa
    shape_.setPosition(x, y);
}

void Coin::draw(sf::RenderWindow& window) {
    if (!collected_) {           // rysuj tylko jeśli niezebrana
        window.draw(shape_);
    }
}

sf::FloatRect Coin::getHitbox() const {
    if (collected_) {
        return sf::FloatRect(); // pusty prostokąt (0,0,0,0) — brak kolizji
    }
    return shape_.getGlobalBounds(); // aktywny hitbox tylko dla niezebrane
}

bool Coin::isCollected() const {
    return collected_;
}

void Coin::collect() {
    collected_ = true; // "usuwa" monetę bez niszczenia obiektu
}
