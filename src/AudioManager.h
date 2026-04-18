#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>

// ============================================================
// AudioManager — zarządza krótkimi dźwiękami SFX (efekty)
//
// Różnica między SFX a muzyką w SFML:
//   sf::Sound + sf::SoundBuffer  → cały plik wczytany do RAM
//                                   odtwarzanie natychmiastowe (<1s)
//                                   dobre dla krótkich efektów
//
//   sf::Music                    → streamowany z dysku na bieżąco
//                                   niskie użycie RAM
//                                   dobre dla długich utworów
//
// AudioManager używa sf::Sound (SFX). sf::Music jest polem w Game.
// ============================================================
class AudioManager {
public:
    // Wczytuje plik audio do pamięci i rejestruje pod nazwą klucza.
    // Zwraca false jeśli plik nie istnieje lub format jest nieobsługiwany.
    bool load(const std::string& name, const std::string& filepath);

    // Odtwarza zarejestrowany dźwięk asynchronicznie (nie blokuje pętli gry).
    // Jeśli dźwięk o danej nazwie nie istnieje — cicho ignoruje.
    void play(const std::string& name);

    // Ustawia głośność (0–100) dla wszystkich załadowanych dźwięków.
    void setVolume(float volume);

private:
    // KRYTYCZNE: SoundEntry musi trzymać buffer i sound RAZEM w jednej strukturze.
    //
    // sf::Sound wewnętrznie przechowuje WSKAŹNIK do sf::SoundBuffer.
    // Gdyby buffer i sound były w osobnych mapach (std::unordered_map),
    // przy realokacji tablicy haszującej obiekty byłyby przenoszone
    // w pamięci → wskaźnik w sf::Sound stałby się nieprawidłowy → crash / UB.
    //
    // Rozwiązanie: buffer i sound w tej samej strukturze — gwarantuje
    // że wskaźnik pozostaje ważny tak długo jak SoundEntry istnieje.
    struct SoundEntry {
        sf::SoundBuffer buffer; // dane audio (PCM) w RAM
        sf::Sound       sound;  // odtwarzacz trzymający wskaźnik do buffer
    };

    // unordered_map: O(1) wyszukiwanie po nazwie klucza ("jump", "coin" itp.)
    std::unordered_map<std::string, SoundEntry> sounds_;
};
