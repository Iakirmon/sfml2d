#include "Game.h"

#include <algorithm>
#include <fstream>
#include <string>

Game::Game()
    : window_(sf::VideoMode(1280, 720), "Platformer") {
    window_.setFramerateLimit(60);

    if (!font_.loadFromFile("assets/fonts/Roboto-Regular.ttf")) {
        font_.loadFromFile("Roboto-Regular.ttf");
    }

    level_.load();

    audio_.load("jump",     "assets/sounds/jump.ogg");
    audio_.load("coin",     "assets/sounds/coin.ogg");
    audio_.load("death",    "assets/sounds/death.ogg");
    audio_.load("gameover", "assets/sounds/gameover.ogg");

    playMusic("assets/music/gameplay.ogg");

    // Kolejność: od najdalszej (najwolniejsza) do najbliższej (najszybsza)
    bgLayers_.emplace_back("assets/textures/bg_clouds.png",    0.2f, 720.f);
    bgLayers_.emplace_back("assets/textures/bg_mountains.png", 0.4f, 720.f);
    bgLayers_.emplace_back("assets/textures/bg_trees.png",     0.7f, 720.f);

    hudTextCoins_.setFont(font_);
    hudTextCoins_.setCharacterSize(20);
    hudTextCoins_.setFillColor(sf::Color::White);
    hudTextCoins_.setPosition(10.f, 10.f);

    hudTextLives_.setFont(font_);
    hudTextLives_.setCharacterSize(20);
    hudTextLives_.setFillColor(sf::Color::White);
    hudTextLives_.setPosition(10.f, 40.f);

    hudTextTime_.setFont(font_);
    hudTextTime_.setCharacterSize(20);
    hudTextTime_.setFillColor(sf::Color::White);
    hudTextTime_.setPosition(10.f, 70.f);

    centerText_.setFont(font_);
    centerText_.setCharacterSize(36);
    centerText_.setFillColor(sf::Color::White);

    inputText_.setFont(font_);
    inputText_.setCharacterSize(24);
    inputText_.setFillColor(sf::Color::Yellow);
}

void Game::run() {
    sf::Clock clock;
    while (window_.isOpen()) {
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.05f);

        handleEvents();

        if (state_ == GameState::PLAYING) {
            update(dt);
        }

        render();
    }
}

void Game::handleEvents() {
    sf::Event event{};
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
        }
        if (event.type == sf::Event::KeyPressed &&
            event.key.code == sf::Keyboard::R &&
            state_ == GameState::GAME_OVER) {
            reset();
        }

        if (state_ == GameState::WINNING) {
            if (event.type == sf::Event::TextEntered) {
                char c = static_cast<char>(event.text.unicode);
                if (c >= 32 && c < 127 && playerInput_.length() < maxInputLength_) {
                    playerInput_ += c;
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::BackSpace && !playerInput_.empty()) {
                    playerInput_.pop_back();
                }
                if (event.key.code == sf::Keyboard::Return && !playerInput_.empty()) {
                    saveScore(playerInput_);
                    reset();
                }
            }
        }
    }
}

void Game::update(float dt) {
    elapsedTime_ += dt;

    float playerX = player_.getHitbox().left;
    for (auto& layer : bgLayers_) {
        layer.update(playerX);
    }

    int coinsBefore = static_cast<int>(level_.getCollectedCoins());

    player_.handleInput();
    player_.update(dt);
    level_.checkCollisions(player_);

    if (player_.justJumped()) {
        audio_.play("jump");
    }

    if (static_cast<int>(level_.getCollectedCoins()) > coinsBefore) {
        audio_.play("coin");
    }

    if (player_.getHitbox().top > 720.f + 50.f) {
        player_.respawn();
        audio_.play("death");
    }

    if (player_.getLives() <= 0 && state_ == GameState::PLAYING) {
        audio_.play("gameover");
        state_ = GameState::GAME_OVER;
        playMusic("assets/music/gameover.ogg", false);
    }
    if (level_.allCoinsCollected() && state_ == GameState::PLAYING) {
        state_ = GameState::WINNING;
        playMusic("assets/music/win.ogg", false);
    }
}

void Game::render() {
    window_.clear(sf::Color(135, 206, 235));

    for (auto& layer : bgLayers_) {
        layer.draw(window_);
    }

    level_.draw(window_);
    player_.draw(window_);
    drawHUD();

    if (state_ == GameState::GAME_OVER) {
        drawGameOver();
    } else if (state_ == GameState::WINNING) {
        drawWinningScreen();
    } else if (state_ == GameState::WIN) {
        drawWin();
    }

    window_.display();
}

void Game::drawHUD() {
    const auto total = level_.getTotalCoins();
    const auto collected = level_.getCollectedCoins();

    int minutes = static_cast<int>(elapsedTime_) / 60;
    int seconds = static_cast<int>(elapsedTime_) % 60;

    hudTextCoins_.setString(
        "Coins: " + std::to_string(collected) + " / " + std::to_string(total));
    hudTextLives_.setString("Lives: " + std::to_string(player_.getLives()));
    hudTextTime_.setString("Time: " + std::to_string(minutes) + ":" +
                           (seconds < 10 ? "0" : "") + std::to_string(seconds));

    window_.draw(hudTextCoins_);
    window_.draw(hudTextLives_);
    window_.draw(hudTextTime_);
}

void Game::drawGameOver() {
    centerText_.setString("GAME OVER\nPress R to restart");
    sf::FloatRect bounds = centerText_.getLocalBounds();
    centerText_.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    centerText_.setPosition(1280.f / 2.f, 720.f / 2.f);
    window_.draw(centerText_);
}

void Game::drawWin() {
    centerText_.setString("YOU WIN!\nPress R to restart");
    sf::FloatRect bounds = centerText_.getLocalBounds();
    centerText_.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    centerText_.setPosition(1280.f / 2.f, 720.f / 2.f);
    window_.draw(centerText_);
}

void Game::drawWinningScreen() {
    centerText_.setString("YOU WIN!\nEnter your name:");
    sf::FloatRect bounds = centerText_.getLocalBounds();
    centerText_.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    centerText_.setPosition(1280.f / 2.f, 250.f);
    window_.draw(centerText_);

    inputText_.setString(playerInput_ + "_");
    sf::FloatRect inputBounds = inputText_.getLocalBounds();
    inputText_.setOrigin(inputBounds.width / 2.f, inputBounds.height / 2.f);
    inputText_.setPosition(1280.f / 2.f, 400.f);
    window_.draw(inputText_);

    sf::Text pressEnterText;
    pressEnterText.setFont(font_);
    pressEnterText.setCharacterSize(16);
    pressEnterText.setFillColor(sf::Color::White);
    pressEnterText.setString("Press ENTER to confirm");
    sf::FloatRect enterBounds = pressEnterText.getLocalBounds();
    pressEnterText.setOrigin(enterBounds.width / 2.f, enterBounds.height / 2.f);
    pressEnterText.setPosition(1280.f / 2.f, 500.f);
    window_.draw(pressEnterText);
}

void Game::saveScore(const std::string& playerName) {
    std::ofstream file("scores.json", std::ios::app);
    if (file.is_open()) {
        int minutes = static_cast<int>(elapsedTime_) / 60;
        int seconds = static_cast<int>(elapsedTime_) % 60;
        std::string timeStr = std::to_string(minutes) + ":" +
                              (seconds < 10 ? "0" : "") + std::to_string(seconds);

        file << "{\"name\":\"" << playerName << "\",\"time\":\"" << timeStr << "\",\"coins\":"
             << level_.getCollectedCoins() << "}\n";
        file.close();
    }
}

void Game::reset() {
    state_ = GameState::PLAYING;
    player_.resetState();
    level_.load();
    elapsedTime_ = 0.f;
    playerInput_ = "";
    playMusic("assets/music/gameplay.ogg");
}

void Game::playMusic(const std::string& filepath, bool loop, float volume) {
    music_.stop();
    if (!music_.openFromFile(filepath)) return;
    music_.setLoop(loop);
    music_.setVolume(volume);
    music_.play();
}

void Game::stopMusic() {
    music_.stop();
}
