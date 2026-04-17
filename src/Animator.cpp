#include "Animator.h"

void Animator::addAnimation(const std::string& name, Animation anim) {
    animations_[name] = std::move(anim);
}

void Animator::play(const std::string& name) {
    if (current_ == name) return;
    current_    = name;
    frameIndex_ = 0;
    elapsed_    = 0.f;
}

sf::IntRect Animator::update(float dt) {
    auto it = animations_.find(current_);
    if (it == animations_.end()) return {};

    const Animation& anim = it->second;
    elapsed_ += dt;

    if (elapsed_ >= anim.frameDuration) {
        elapsed_ -= anim.frameDuration;
        if (frameIndex_ + 1 < static_cast<int>(anim.frames.size())) {
            ++frameIndex_;
        } else if (anim.loop) {
            frameIndex_ = 0;
        }
    }

    return anim.frames[frameIndex_];
}

bool Animator::isFinished() const {
    auto it = animations_.find(current_);
    if (it == animations_.end()) return true;
    const Animation& anim = it->second;
    return !anim.loop && frameIndex_ == static_cast<int>(anim.frames.size()) - 1;
}
