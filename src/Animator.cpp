#include "Animator.h"

// std::move przenosi obiekt Animation do mapy bez kopiowania wektora klatek
void Animator::addAnimation(const std::string& name, Animation anim) {
    animations_[name] = std::move(anim);
}

// Przełącza animację tylko jeśli to nowa nazwa — zapobiega resetowaniu
// frameIndex_ gdy Player co klatkę wywołuje play("run")
void Animator::play(const std::string& name) {
    if (current_ == name) return;  // już aktywna — nie ruszaj klatki
    current_    = name;
    frameIndex_ = 0;   // zacznij od pierwszej klatki nowej animacji
    elapsed_    = 0.f; // wyzeruj akumulator czasu
}

// Przesuwa animację o dt sekund i zwraca sf::IntRect bieżącej klatki.
// Wywołuj co klatkę i podstawiaj wynik do sprite_.setTextureRect().
sf::IntRect Animator::update(float dt) {
    auto it = animations_.find(current_);
    if (it == animations_.end()) return {}; // nieznana animacja → pusty rect

    const Animation& anim = it->second;
    elapsed_ += dt;

    // Sprawdź czy minął czas jednej klatki
    if (elapsed_ >= anim.frameDuration) {
        elapsed_ -= anim.frameDuration; // odejmij, nie zeruj — zachowuje resztę
        if (frameIndex_ + 1 < static_cast<int>(anim.frames.size())) {
            ++frameIndex_;           // następna klatka
        } else if (anim.loop) {
            frameIndex_ = 0;         // koniec pętli → wróć do klatki 0
        }
        // jeśli !loop i ostatnia klatka → zostań na niej (isFinished() = true)
    }

    return anim.frames[frameIndex_]; // prostokąt do sf::Sprite::setTextureRect
}

// Animacja zakończyła się gdy: nie zapętla się I jesteśmy na ostatniej klatce
bool Animator::isFinished() const {
    auto it = animations_.find(current_);
    if (it == animations_.end()) return true;
    const Animation& anim = it->second;
    return !anim.loop && frameIndex_ == static_cast<int>(anim.frames.size()) - 1;
}
