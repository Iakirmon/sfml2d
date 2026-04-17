#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class ParallaxLayer {
public:
    ParallaxLayer(const std::string& filepath, float speedFactor, float screenHeight);

    void update(float playerX);
    void draw(sf::RenderWindow& window);

private:
    sf::Texture texture_;
    sf::Sprite  sprites_[2];
    float       speedFactor_;
    float       scaledWidth_;
};
