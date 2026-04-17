#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>

class AudioManager {
public:
    bool load(const std::string& name, const std::string& filepath);
    void play(const std::string& name);
    void setVolume(float volume);

private:
    struct SoundEntry {
        sf::SoundBuffer buffer;
        sf::Sound       sound;
    };
    std::unordered_map<std::string, SoundEntry> sounds_;
};
