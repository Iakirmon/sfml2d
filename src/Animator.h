#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <unordered_map>

// ============================================================
// Animation — dane jednej animacji spritesheet
//
// Spritesheet to jeden plik PNG z wieloma klatkami ułożonymi
// w siatce. Zamiast ładować N osobnych plików, wycinamy
// prostokąty (sf::IntRect) z jednej tekstury.
//
// Przykład: klatki animacji "run" w pliku 720×330px, klatka 80×110:
//   frames[0] = {320, 220, 80, 110}  ← 4. kolumna, 3. rząd
//   frames[1] = {400, 220, 80, 110}  ← 5. kolumna, 3. rząd
// ============================================================
struct Animation {
    std::vector<sf::IntRect> frames;  // prostokąty wycinające klatki z tekstury
    float frameDuration{0.1f};        // czas trwania jednej klatki [sekundy]
    bool  loop{true};                 // czy animacja ma się zapętlić
};

// ============================================================
// Animator — zarządza animacjami dla jednego sprite'a
//
// Przechowuje mapę nazw → Animation i śledzi:
//   - która animacja jest aktualnie aktywna (current_)
//   - który numer klatki jest teraz wyświetlany (frameIndex_)
//   - ile czasu upłynęło od ostatniej zmiany klatki (elapsed_)
//
// Użycie:
//   animator_.addAnimation("run", run);   // rejestracja
//   animator_.play("run");                 // przełączenie
//   sf::IntRect r = animator_.update(dt); // postęp + pobierz klatkę
//   sprite_.setTextureRect(r);            // zastosuj na sprite
// ============================================================
class Animator {
public:
    // Rejestruje animację pod podaną nazwą (np. "idle", "run", "jump")
    void addAnimation(const std::string& name, Animation anim);

    // Przełącza na podaną animację; jeśli już jest aktywna — brak efektu
    // (zapobiega resetowaniu klatki przy każdym wywołaniu co klatkę)
    void play(const std::string& name);

    // Przesuwa czas animacji o dt sekund i zwraca aktualny sf::IntRect
    // do ustawienia na sprite (sprite_.setTextureRect)
    sf::IntRect update(float dt);

    // Zwraca true gdy animacja non-loop dotarła do ostatniej klatki
    bool isFinished() const;

private:
    std::unordered_map<std::string, Animation> animations_; // mapa: nazwa → dane
    std::string current_;      // nazwa aktualnie aktywnej animacji
    int   frameIndex_{0};      // indeks bieżącej klatki w frames[]
    float elapsed_{0.f};       // akumulator czasu od ostatniej zmiany klatki
};
