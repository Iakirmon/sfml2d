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

    playMusic("assets/music/menu.ogg");

    // Kolejnosc: od najdalszej (najwolniejsza) do najblizszej (najszybsza)
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
        update(dt);
        render();
    }
}

void Game::handleEvents() {
    sf::Event event{};
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
        }

        // --- MENU (klawiatura) ---
        if (state_ == GameState::MENU) {
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up)
                    menuSelectedItem_ = (menuSelectedItem_ + 2) % 3;
                if (event.key.code == sf::Keyboard::Down)
                    menuSelectedItem_ = (menuSelectedItem_ + 1) % 3;
                if (event.key.code == sf::Keyboard::Return) {
                    if      (menuSelectedItem_ == 0) startGame();
                    else if (menuSelectedItem_ == 1) { loadTopScores(); state_ = GameState::SCORES; }
                    else if (menuSelectedItem_ == 2) window_.close();
                }
            }
        }

        // --- MENU (mysz) ---
        if (state_ == GameState::MENU) {
            if (event.type == sf::Event::MouseMoved) {
                sf::Vector2f mouse(static_cast<float>(event.mouseMove.x),
                                   static_cast<float>(event.mouseMove.y));
                const float itemY[3] = {320.f, 390.f, 460.f};
                for (int i = 0; i < 3; ++i) {
                    if (mouse.y >= itemY[i] - 20.f && mouse.y <= itemY[i] + 20.f)
                        menuSelectedItem_ = i;
                }
            }
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {
                if      (menuSelectedItem_ == 0) startGame();
                else if (menuSelectedItem_ == 1) { loadTopScores(); state_ = GameState::SCORES; }
                else if (menuSelectedItem_ == 2) window_.close();
            }
        }

        // --- SCORES ---
        if (state_ == GameState::SCORES) {
            if (event.type == sf::Event::KeyPressed &&
                (event.key.code == sf::Keyboard::Escape ||
                 event.key.code == sf::Keyboard::Return)) {
                state_ = GameState::MENU;
            }
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {
                state_ = GameState::MENU;
            }
        }

        // --- GAME_OVER ---
        if (state_ == GameState::GAME_OVER) {
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::R)
                    startGame();
                if (event.key.code == sf::Keyboard::Escape) {
                    state_ = GameState::MENU;
                    playMusic("assets/music/menu.ogg");
                }
            }
        }

        // --- WINNING ---
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
                    state_ = GameState::MENU;
                    playerInput_ = "";
                    playMusic("assets/music/menu.ogg");
                }
            }
        }
    }
}

void Game::update(float dt) {
    // Auto-scroll tla w menu i ekranie wynikow
    if (state_ == GameState::MENU || state_ == GameState::SCORES) {
        menuScrollOffset_ += 60.f * dt;
        for (auto& layer : bgLayers_)
            layer.update(menuScrollOffset_);
        return;
    }

    // GAME_OVER, WIN, WINNING - gra wstrzymana
    if (state_ != GameState::PLAYING) return;

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

    for (auto& layer : bgLayers_)
        layer.draw(window_);

    if (state_ == GameState::MENU) {
        drawMenu();
    } else if (state_ == GameState::SCORES) {
        drawScores();
    } else {
        level_.draw(window_);
        player_.draw(window_);
        drawHUD();

        if (state_ == GameState::GAME_OVER)    drawGameOver();
        else if (state_ == GameState::WINNING)  drawWinningScreen();
        else if (state_ == GameState::WIN)      drawWin();
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
    centerText_.setString("GAME OVER\nR - restart   Escape - menu");
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

void Game::drawMenu() {
    sf::Text title;
    title.setFont(font_);
    title.setCharacterSize(64);
    title.setFillColor(sf::Color::White);
    title.setString("PLATFORMER");
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2.f, tb.height / 2.f);
    title.setPosition(1280.f / 2.f, 180.f);
    window_.draw(title);

    const char* labels[3] = {"START", "NAJLEPSZE WYNIKI", "WYJDZ"};
    const float itemY[3]  = {320.f, 390.f, 460.f};

    for (int i = 0; i < 3; ++i) {
        sf::Text item;
        item.setFont(font_);
        item.setCharacterSize(36);
        item.setFillColor(i == menuSelectedItem_ ? sf::Color::Yellow : sf::Color::White);
        item.setString(labels[i]);
        sf::FloatRect ib = item.getLocalBounds();
        item.setOrigin(ib.width / 2.f, ib.height / 2.f);
        item.setPosition(1280.f / 2.f, itemY[i]);
        window_.draw(item);
    }

    sf::Text hint;
    hint.setFont(font_);
    hint.setCharacterSize(16);
    hint.setFillColor(sf::Color(200, 200, 200));
    hint.setString("strzalki / mysz - wybor   Enter / klik - potwierdzenie");
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width / 2.f, hb.height / 2.f);
    hint.setPosition(1280.f / 2.f, 680.f);
    window_.draw(hint);
}

void Game::drawScores() {
    sf::Text title;
    title.setFont(font_);
    title.setCharacterSize(48);
    title.setFillColor(sf::Color::White);
    title.setString("TOP 10 WYNIKOW");
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2.f, tb.height / 2.f);
    title.setPosition(1280.f / 2.f, 80.f);
    window_.draw(title);

    const float COL_RANK  = 300.f;
    const float COL_NAME  = 350.f;
    const float COL_TIME  = 660.f;
    const float COL_COINS = 760.f;

    auto makeHeader = [&](const std::string& str, float x) {
        sf::Text h;
        h.setFont(font_);
        h.setCharacterSize(20);
        h.setFillColor(sf::Color(180, 180, 180));
        h.setString(str);
        h.setPosition(x, 160.f);
        window_.draw(h);
    };
    makeHeader("#",      COL_RANK);
    makeHeader("IMIE",   COL_NAME);
    makeHeader("CZAS",   COL_TIME);
    makeHeader("MONETY", COL_COINS);

    if (topScores_.empty()) {
        sf::Text empty;
        empty.setFont(font_);
        empty.setCharacterSize(24);
        empty.setFillColor(sf::Color(180, 180, 180));
        empty.setString("Brak wynikow. Zagraj i wygraj!");
        sf::FloatRect eb = empty.getLocalBounds();
        empty.setOrigin(eb.width / 2.f, eb.height / 2.f);
        empty.setPosition(1280.f / 2.f, 360.f);
        window_.draw(empty);
    } else {
        for (int i = 0; i < static_cast<int>(topScores_.size()); ++i) {
            const auto& s = topScores_[i];
            float y = 195.f + i * 30.f;
            sf::Color color = (i == 0) ? sf::Color::Yellow : sf::Color::White;

            auto makeCell = [&](const std::string& str, float x) {
                sf::Text t;
                t.setFont(font_);
                t.setCharacterSize(22);
                t.setFillColor(color);
                t.setString(str);
                t.setPosition(x, y);
                window_.draw(t);
            };

            makeCell(std::to_string(i + 1) + ".", COL_RANK);
            makeCell(s.name,                       COL_NAME);
            makeCell(s.time,                       COL_TIME);
            makeCell(std::to_string(s.coins),      COL_COINS);
        }
    }

    sf::Text hint;
    hint.setFont(font_);
    hint.setCharacterSize(16);
    hint.setFillColor(sf::Color(200, 200, 200));
    hint.setString("Escape / Enter / klik - powrot do menu");
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width / 2.f, hb.height / 2.f);
    hint.setPosition(1280.f / 2.f, 680.f);
    window_.draw(hint);
}

void Game::startGame() {
    state_ = GameState::PLAYING;
    player_.resetState();
    level_.load();
    elapsedTime_ = 0.f;
    playerInput_ = "";
    playMusic("assets/music/gameplay.ogg");
}

void Game::loadTopScores() {
    topScores_.clear();
    std::ifstream file("scores.json");
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        ScoreEntry e;

        auto extract = [&](const std::string& key, bool quoted) -> std::string {
            auto pos = line.find("\"" + key + "\":");
            if (pos == std::string::npos) return "";
            pos += key.size() + 3;
            if (quoted) {
                pos++;
                auto end = line.find('"', pos);
                return line.substr(pos, end - pos);
            } else {
                auto end = line.find_first_of(",}", pos);
                return line.substr(pos, end - pos);
            }
        };

        e.name  = extract("name",  true);
        e.time  = extract("time",  true);
        auto cs = extract("coins", false);
        if (!cs.empty()) e.coins = std::stoi(cs);
        topScores_.push_back(e);
    }

    auto toSec = [](const std::string& t) -> int {
        auto col = t.find(':');
        if (col == std::string::npos) return 99999;
        return std::stoi(t.substr(0, col)) * 60 + std::stoi(t.substr(col + 1));
    };

    std::sort(topScores_.begin(), topScores_.end(),
              [&](const ScoreEntry& a, const ScoreEntry& b) {
                  return toSec(a.time) < toSec(b.time);
              });

    if (topScores_.size() > 10)
        topScores_.resize(10);
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
