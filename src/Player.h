#pragma once

#include <SFML/Graphics.hpp>
#include "Animator.h"

// ============================================================
// Player — gracz sterowany przez użytkownika
//
// Fizyka:
//   - Grawitacja dodaje do velocity_.y każdą klatkę (velocity_.y += G*dt)
//   - Kolizje z platformami (w Level) zerują velocity_.y i ustawiają isOnGround_
//   - Prędkość X ustawiana jest bezpośrednio z klawiatury (brak przyspieszania)
//
// System skoków (triple jump):
//   - Skok 1: z ziemi (isOnGround_ = true) → uruchamia, canDoubleJump_ = true
//   - Skok 2: w powietrzu jeśli canDoubleJump_ → uruchamia, canTripleJump_ = true
//   - Skok 3: w powietrzu jeśli canTripleJump_ → uruchamia
//   - spaceWasPressed_ zapobiega wielokrotnemu skokowi przy jednym wciśnięciu
//
// Animacje (spritesheet Kenney, siatka 9×3 klatek 80×110px):
//   "idle", "run", "jump", "fall" — zarządzane przez Animator
// ============================================================
class Player {
public:
    Player();

    // Czyta klawiaturę i ustawia velocity_; wywołuj przed update()
    void handleInput();

    // Aplikuje grawitację, przesuwa sprite, wybiera animację; wywołuj po handleInput()
    void update(float dt);

    void draw(sf::RenderWindow& window);

    // Zwraca bounding box sprite'a w world space — używane przez Level::checkCollisions()
    sf::FloatRect getHitbox() const;

    // Przesuwa sprite'a o offset — używane przez Level do wypchnięcia z kolizji
    void moveShape(sf::Vector2f offset);

    // Settery wywoływane przez Level::checkCollisions() po wykryciu kolizji
    void setVelocityX(float vx);
    void setVelocityY(float vy);
    void setOnGround(bool val); // ustawia też canDoubleJump_ = false gdy true

    int  getLives() const;
    void respawn();     // cofa na spawnPosition_, odejmuje życie
    void resetState();  // pełny reset na nową grę (pozycja, życia, flagi)

    // Przygotowanie gracza na kolejny poziom kampanii — jak resetState,
    // ale BEZ zmiany liczby żyć (życia przechodzą między poziomami).
    void prepareForNextLevel();

    // Zwraca true przez jedną klatkę zaraz po wykonaniu skoku (do odtworzenia SFX)
    bool justJumped() const { return justJumped_; }

private:
    sf::Texture  texture_;
    sf::Sprite   sprite_;
    sf::Vector2u texSize_;   // rozmiar JEDNEJ klatki (nie całego spritesheeta)
    Animator     animator_;

    sf::Vector2f velocity_{0.f, 0.f}; // prędkość [px/s] w osiach X i Y
    bool isOnGround_{false};    // czy stoi na platformie (zerowane każdą klatkę)
    bool canDoubleJump_{false}; // czy dostępny skok 2.
    bool canTripleJump_{false}; // czy dostępny skok 3.
    bool spaceWasPressed_{false}; // poprzedni stan spacji (edge detection)
    bool facingRight_{true};      // kierunek patrzenia (do flipowania sprite'a)
    bool justJumped_{false};      // flaga jednoklatkowa dla dźwięku skoku

    int lives_{3};
    sf::Vector2f spawnPosition_{100.f, 300.f}; // punkt odrodzenia po śmierci
};
