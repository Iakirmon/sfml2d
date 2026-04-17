#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>

class AudioManager {
public:
    // Załaduj dźwięk z pliku i zarejestruj pod nazwą
    bool load(const std::string& name, const std::string& filepath);

    // Odtwórz zarejestrowany dźwięk (nie blokuje – async)
    void play(const std::string& name);

    // Globalna głośność SFX (0–100)
    void setVolume(float volume);

private:
    // WAŻNE: buffer i sound muszą być w tej samej strukturze.
    // sf::Sound trzyma wskaźnik do sf::SoundBuffer – jeśli buffer
    // zostanie przeniesiony w pamięci (np. przy realokacji mapy),
    // wskaźnik staje się nieprawidłowy → crash lub UB.
    struct SoundEntry {
        sf::SoundBuffer buffer;
        sf::Sound       sound;
    };
    std::unordered_map<std::string, SoundEntry> sounds_;
};
