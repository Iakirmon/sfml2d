#include "Game.h"

#include <algorithm>

Game::Game()
    : window_(sf::VideoMode(1280, 720), "Platformer") {
    window_.setFramerateLimit(60);

    // Load font (user must ensure file exists)
    if (!font_.loadFromFile("assets/fonts/Roboto-Regular.ttf")) {
        // fallback: try without subdir, but keep font_ alive
        font_.loadFromFile("Roboto-Regular.ttf");
    }

    level_.load();

    hudTextCoins_.setFont(font_);
    hudTextCoins_.setCharacterSize(20);
    hudTextCoins_.setFillColor(sf::Color::White);
    hudTextCoins_.setPosition(10.f, 10.f);

    hudTextLives_.setFont(font_);
    hudTextLives_.setCharacterSize(20);
    hudTextLives_.setFillColor(sf::Color::White);
    hudTextLives_.setPosition(10.f, 40.f);

    centerText_.setFont(font_);
    centerText_.setCharacterSize(36);
    centerText_.setFillColor(sf::Color::White);
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
            state_ != GameState::PLAYING) {
            reset();
        }
    }
}

void Game::update(float dt) {
    player_.handleInput();
    player_.update(dt);
    level_.checkCollisions(player_);

    if (player_.getHitbox().top > 720.f + 50.f) {
        player_.respawn();
    }

    if (player_.getLives() <= 0) {
        state_ = GameState::GAME_OVER;
    }
    if (level_.allCoinsCollected()) {
        state_ = GameState::WIN;
    }
}

void Game::render() {
    window_.clear(sf::Color(30, 30, 30));

    level_.draw(window_);
    player_.draw(window_);
    drawHUD();

    if (state_ == GameState::GAME_OVER) {
        drawGameOver();
    } else if (state_ == GameState::WIN) {
        drawWin();
    }

    window_.display();
}

void Game::drawHUD() {
    const auto total = level_.getTotalCoins();
    const auto collected = level_.getCollectedCoins();

    hudTextCoins_.setString(
        "Coins: " + std::to_string(collected) + " / " + std::to_string(total));
    hudTextLives_.setString("Lives: " + std::to_string(player_.getLives()));

    window_.draw(hudTextCoins_);
    window_.draw(hudTextLives_);
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

void Game::reset() {
    *this = Game(); // reconstructs game (simple reset)
}

