#pragma once
#include <SFML/Graphics.hpp>

// ============================================================
// Coin — zbieralna moneta (żółte kółko)
//
// Stan `collected_` jest flagą logiczną:
//   false → moneta widoczna, hitbox aktywny
//   true  → moneta niewidoczna, hitbox pusty (sf::FloatRect())
//
// Pusta sf::FloatRect (0,0,0,0) nigdy nie nachodzi na hitbox gracza,
// więc zebranej monety nie można zebrać ponownie bez specjalnego
// sprawdzenia — to najprostszy mechanizm "usunięcia" obiektu.
// ============================================================
class Coin {
public:
    // Tworzy monetę w punkcie (x, y); origin ustawiony na środek koła
    Coin(float x, float y);

    // Rysuje monetę tylko jeśli nie jest zebrana
    void draw(sf::RenderWindow& window);

    // Zwraca pusty prostokąt gdy zebrana (wyklucza z kolizji)
    sf::FloatRect getHitbox() const;

    bool isCollected() const;
    void collect(); // ustawia collected_ = true

private:
    sf::CircleShape shape_;
    bool collected_{false};
};
