#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// ============================================================
// ParallaxLayer — jedna warstwa tła z efektem głębi (parallax)
//
// Efekt parallax: warstwy "dalej" poruszają się wolniej niż
// warstwy "bliżej". Daje iluzję trójwymiarowości w 2D.
//
//   speedFactor = 0.2  → chmury (najdalej, najwolniej)
//   speedFactor = 0.4  → góry (środek)
//   speedFactor = 0.7  → drzewa (najbliżej, najszybciej)
//
// Technika seamless loop (bezszwowa pętla):
//   Dwa sprite'y ustawione obok siebie. Gdy lewy wyjedzie za lewy
//   brzeg ekranu, jest natychmiast przesuwany za prawy.
//
//   [sprite_0][sprite_1]  →  przesuwa się w lewo
//   [sprite_1][sprite_0]  →  sprite_0 przeskoczył za prawy
//
// WAŻNE: Metoda draw() musi być wywołana przy aktywnym
// window_.getDefaultView() (screen space), NIE przy gameView_.
// Sprite'y są pozycjonowane w screen space (0–scaledWidth_).
// Gdyby rysować je przy gameView_ (środek np. 1500), byłyby
// poza obszarem kamery i zniknęłyby z ekranu.
// ============================================================
class ParallaxLayer {
public:
    // filepath    — ścieżka do tileable tekstury (kafelkuje się poziomo)
    // speedFactor — współczynnik prędkości: 0.0 = statyczna, 1.0 = razem z kamerą
    // screenHeight — wysokość ekranu, do przeskalowania warstwy (wypełnia pion)
    ParallaxLayer(const std::string& filepath, float speedFactor, float screenHeight);

    // Aktualizuje pozycje sprite'ów na podstawie playerX (world space).
    // W menu przekazywany jest monotoniczny licznik menuScrollOffset_.
    void update(float playerX);

    // Rysuje dwa sprite'y — wywołuj przy aktywnym getDefaultView()
    void draw(sf::RenderWindow& window);

private:
    sf::Texture texture_;
    sf::Sprite  sprites_[2];  // dwa sprite'y do seamless loopingu
    float       speedFactor_;
    float       scaledWidth_; // szerokość jednego sprite'a po przeskalowaniu do screenHeight
};
