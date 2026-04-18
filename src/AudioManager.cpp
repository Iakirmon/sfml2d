#include "AudioManager.h"
#include <iostream>

bool AudioManager::load(const std::string& name, const std::string& filepath) {
    // try_emplace (C++17): tworzy nowy wpis TYLKO jeśli klucz nie istnieje.
    // Zwraca parę {iterator, czy_wstawiono}.
    // Nie przenosi ani nie kopiuje istniejących elementów mapy — ważne
    // dla stabilności wskaźników sf::Sound → sf::SoundBuffer.
    auto [it, inserted] = sounds_.try_emplace(name);
    if (!inserted) {
        return true; // dźwięk był już załadowany — nic nie rób
    }

    // Wczytaj dane audio z pliku do bufora (RAM)
    if (!it->second.buffer.loadFromFile(filepath)) {
        std::cerr << "[AudioManager] Nie można załadować: " << filepath << "\n";
        sounds_.erase(it); // usuń puste wejście — nie zostawiaj śmieciowych wpisów
        return false;
    }

    // Powiąż odtwarzacz z buforem — sf::Sound przechowuje wewnętrznie
    // wskaźnik do przekazanego bufora. Dlatego buffer musi przeżyć sound.
    it->second.sound.setBuffer(it->second.buffer);
    return true;
}

void AudioManager::play(const std::string& name) {
    auto it = sounds_.find(name);
    if (it != sounds_.end()) {
        // play() jest asynchroniczne — wraca natychmiast,
        // dźwięk jest odtwarzany w osobnym wątku SFML
        it->second.sound.play();
    }
    // brak else: celowo ignorujemy nieznane nazwy (łatwiejsze debugowanie)
}

void AudioManager::setVolume(float volume) {
    // Przeiteruj po wszystkich wpisach i ustaw głośność każdego odtwarzacza
    for (auto& [name, entry] : sounds_) {
        entry.sound.setVolume(volume); // zakres: 0.0 (cisza) – 100.0 (max)
    }
}
