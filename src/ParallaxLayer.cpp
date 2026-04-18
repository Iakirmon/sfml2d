#include "ParallaxLayer.h"
#include <cmath>

ParallaxLayer::ParallaxLayer(const std::string& filepath, float speedFactor, float screenHeight)
    : speedFactor_(speedFactor)
{
    texture_.loadFromFile(filepath);

    sf::Vector2u texSize = texture_.getSize();

    // Oblicz skalę tak, by warstwa wypełniała cały ekran w pionie.
    // Ten sam współczynnik stosujemy do osi X, żeby zachować proporcje.
    float scaleY = screenHeight / static_cast<float>(texSize.y);
    scaledWidth_ = static_cast<float>(texSize.x) * scaleY;

    // Ustaw oba sprite'y obok siebie (od x=0 i od x=scaledWidth_)
    for (int i = 0; i < 2; ++i) {
        sprites_[i].setTexture(texture_);
        sprites_[i].setScale(scaleY, scaleY);
        sprites_[i].setPosition(i * scaledWidth_, 0.f);
    }
}

void ParallaxLayer::update(float playerX) {
    // Oblicz przesunięcie na podstawie pozycji gracza (lub menuScrollOffset_).
    // fmod zwraca resztę z dzielenia — gdy offset przekroczy szerokość,
    // wraca do zera, tworząc nieskończoną pętlę.
    float offset = std::fmod(playerX * speedFactor_, scaledWidth_);
    if (offset < 0.f) offset += scaledWidth_; // zabezpieczenie przed ujemnym fmod

    // Ustaw sprite'y tak, żeby razem zawsze pokrywały cały ekran:
    //   sprite_[0] zaczyna się od -offset (może wychodzić poza lewy brzeg)
    //   sprite_[1] zaczyna się od scaledWidth_ - offset
    // Gdy sprite_[0] całkowicie wyjdzie za lewy brzeg (-scaledWidth_),
    // sprite_[1] zajmuje jego miejsce, a przy następnej pętli role się odwracają.
    sprites_[0].setPosition(-offset, 0.f);
    sprites_[1].setPosition(scaledWidth_ - offset, 0.f);
}

void ParallaxLayer::draw(sf::RenderWindow& window) {
    window.draw(sprites_[0]);
    window.draw(sprites_[1]);
}
