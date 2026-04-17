#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class ParallaxLayer {
public:
    // filepath – tekstura tileable; speedFactor – 0.0 (statyczna) do 1.0 (razem z graczem)
    // screenHeight – do przeskalowania warstwy
    ParallaxLayer(const std::string& filepath, float speedFactor, float screenHeight);

    // playerX – aktualna pozycja X gracza
    void update(float playerX);

    void draw(sf::RenderWindow& window);

private:
    sf::Texture texture_;
    sf::Sprite  sprites_[2];   // dwa sprite'y obok siebie do seamless loopingu
    float       speedFactor_;
    float       scaledWidth_;  // szerokość jednego sprite'a po przeskalowaniu
};
