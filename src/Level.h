#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

#include "Platform.h"
#include "Coin.h"
#include "Player.h"

// ============================================================
// Level — zarządza platformami i monetami; wykrywa kolizje
//
// Kolizje z platformami: metoda AABB (Axis-Aligned Bounding Box)
//   1. Sprawdź czy hitboxy gracza i platformy na siebie nachodzą
//      (sf::FloatRect::intersects zwraca prostokąt nachodzenia)
//   2. Porównaj wymiary nachodzenia: mniejsza oś → kierunek wypychania
//      overlap.width < overlap.height → pchnij w osi X (ściana boczna)
//      overlap.width ≥ overlap.height → pchnij w osi Y (podłoga/sufit)
//   3. Gdy pchnięcie jest w górę (pushY < 0) → gracz ląduje → setOnGround(true)
//
// LEVEL_WIDTH definiuje szerokość poziomu — używana przez kamerę (Game::updateCamera)
// do obliczania granic przesuwania widoku.
// ============================================================
class Level {
public:
    // Liczba poziomów w kampanii (jedno źródło prawdy). Poziomy indeksowane 0..LEVEL_COUNT-1.
    static constexpr int LEVEL_COUNT = 3;

    // Czyści i wypełnia vectors platform i monet układem o podanym indeksie.
    // Kontrakt: 0 <= levelIndex < LEVEL_COUNT (asercja w .cpp)
    void load(int levelIndex);

    void checkCollisions(Player& player); // AABB: kolizje gracz↔platformy i monety
    bool allCoinsCollected() const;       // Warunek wygranej
    void draw(sf::RenderWindow& window);  // Rysuj platformy i monety

    std::size_t getTotalCoins() const;     // Łączna liczba monet (do HUD)
    std::size_t getCollectedCoins() const; // Liczba zebranych monet (do HUD i wyniku)

    // Szerokość poziomu [px] — kamera (updateCamera) klamuje do [halfW, LEVEL_WIDTH-halfW]
    static constexpr float LEVEL_WIDTH = 3840.f;
    float getLevelWidth() const { return LEVEL_WIDTH; }

private:
    std::vector<Platform> platforms_; // podłoga + platformy pływające
    std::vector<Coin>     coins_;     // monety do zebrania
};
