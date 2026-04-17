#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <unordered_map>

struct Animation {
    std::vector<sf::IntRect> frames;
    float frameDuration{0.1f};
    bool  loop{true};
};

class Animator {
public:
    void addAnimation(const std::string& name, Animation anim);
    void play(const std::string& name);
    sf::IntRect update(float dt);
    bool isFinished() const;

private:
    std::unordered_map<std::string, Animation> animations_;
    std::string current_;
    int   frameIndex_{0};
    float elapsed_{0.f};
};
