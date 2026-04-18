#pragma once
#include <SFML/Graphics.hpp>

// ============================================================
// Platform — pojedyncza platforma (prostokąt) w poziomie
//
// Rysowana jako zielony sf::RectangleShape.
// Kolizje z graczem są obsługiwane w Level::checkCollisions()
// metodą AABB (Axis-Aligned Bounding Box) — sprawdzamy czy
// dwa prostokąty na siebie nachodzą, a potem wypychamy gracza.
// ============================================================
class Platform {
public:
    // Tworzy platformę o lewym-górnym rogu (x, y) i rozmiarze (width × height)
    Platform(float x, float y, float width, float height);

    void draw(sf::RenderWindow& window);

    // Zwraca prostokąt kolizji w world space — używane przez Level::checkCollisions()
    sf::FloatRect getHitbox() const;

private:
    sf::RectangleShape shape_; // wewnętrzny kształt SFML do rysowania i hitboxa
};
