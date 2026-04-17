#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <deque>
#include "Player.h"
#include "Level.h"
#include "AudioManager.h"
#include "ParallaxLayer.h"

enum class GameState {
    PLAYING,
    GAME_OVER,
    WIN,
    WINNING  // Waiting for player name input
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
    void drawWinningScreen();
    void reset();
    void saveScore(const std::string& playerName);
    void playMusic(const std::string& filepath, bool loop = true, float volume = 50.f);
    void stopMusic();

    sf::RenderWindow window_;
    sf::Font font_;

    GameState state_{GameState::PLAYING};

    Player player_;
    Level level_;
    AudioManager audio_;
    sf::Music music_;
    std::deque<ParallaxLayer> bgLayers_;

    sf::Text hudTextCoins_;
    sf::Text hudTextLives_;
    sf::Text hudTextTime_;
    sf::Text centerText_;
    sf::Text inputText_;
    
    float elapsedTime_{0.f};
    std::string playerInput_{""};
    int maxInputLength_{20};
};

