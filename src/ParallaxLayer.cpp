#include "ParallaxLayer.h"
#include <cmath>

ParallaxLayer::ParallaxLayer(const std::string& filepath, float speedFactor, float screenHeight)
    : speedFactor_(speedFactor)
{
    texture_.loadFromFile(filepath);

    sf::Vector2u texSize = texture_.getSize();
    float scaleY = screenHeight / static_cast<float>(texSize.y);
    scaledWidth_ = static_cast<float>(texSize.x) * scaleY;

    for (int i = 0; i < 2; ++i) {
        sprites_[i].setTexture(texture_);
        sprites_[i].setScale(scaleY, scaleY);
        sprites_[i].setPosition(i * scaledWidth_, 0.f);
    }
}

void ParallaxLayer::update(float playerX) {
    float offset = std::fmod(playerX * speedFactor_, scaledWidth_);
    if (offset < 0.f) offset += scaledWidth_;

    sprites_[0].setPosition(-offset, 0.f);
    sprites_[1].setPosition(scaledWidth_ - offset, 0.f);
}

void ParallaxLayer::draw(sf::RenderWindow& window) {
    window.draw(sprites_[0]);
    window.draw(sprites_[1]);
}
