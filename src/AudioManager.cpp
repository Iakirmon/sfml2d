#include "AudioManager.h"
#include <iostream>

bool AudioManager::load(const std::string& name, const std::string& filepath) {
    auto [it, inserted] = sounds_.try_emplace(name);
    if (!inserted) return true;

    if (!it->second.buffer.loadFromFile(filepath)) {
        std::cerr << "[AudioManager] Nie można załadować: " << filepath << "\n";
        sounds_.erase(it);
        return false;
    }

    it->second.sound.setBuffer(it->second.buffer);
    return true;
}

void AudioManager::play(const std::string& name) {
    auto it = sounds_.find(name);
    if (it != sounds_.end()) {
        it->second.sound.play();
    }
}

void AudioManager::setVolume(float volume) {
    for (auto& [name, entry] : sounds_) {
        entry.sound.setVolume(volume);
    }
}
