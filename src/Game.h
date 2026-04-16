#pragma once

#include <SFML/Graphics.hpp>
#include "Player.h"
#include "Level.h"

enum class GameState {
    PLAYING,
    GAME_OVER,
    WIN
};

class Game {
public:
    Game();
    void run();

private:
    void handleEvents();
    void update(float dt);
    void render();

    void drawHUD();
    void drawGameOver();
    void drawWin();
    void reset();

    sf::RenderWindow window_;
    sf::Font font_;

    GameState state_{GameState::PLAYING};

    Player player_;
    Level level_;

    sf::Text hudTextCoins_;
    sf::Text hudTextLives_;
    sf::Text centerText_;
};

