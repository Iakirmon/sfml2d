#include "Game.h"

#include <algorithm>
#include <fstream>
#include <sstream>
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
                // Aktualizuj zaznaczenie na podstawie pozycji kliknięcia
                sf::Vector2f click(static_cast<float>(event.mouseButton.x),
                                   static_cast<float>(event.mouseButton.y));
                const float itemY[3] = {320.f, 390.f, 460.f};
                for (int i = 0; i < 3; ++i) {
                    if (click.y >= itemY[i] - 30.f && click.y <= itemY[i] + 30.f)
                        menuSelectedItem_ = i;
                }
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

        // --- PLAYING ---
        if (state_ == GameState::PLAYING) {
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Escape) {
                state_ = GameState::MENU;
                playMusic("assets/music/menu.ogg");
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
    // --- Ciemny panel dla czytelnosci ---
    sf::RectangleShape panel(sf::Vector2f(780.f, 622.f));
    panel.setFillColor(sf::Color(8, 8, 24, 215));
    panel.setOutlineColor(sf::Color(200, 170, 50, 180));
    panel.setOutlineThickness(2.f);
    panel.setPosition(250.f, 49.f);
    window_.draw(panel);

    // Pomocnicze lambdy do rysowania
    auto drawText = [&](const std::string& str, float x, float y,
                        unsigned sz, sf::Color col) {
        sf::Text t;
        t.setFont(font_);
        t.setCharacterSize(sz);
        t.setFillColor(col);
        t.setString(str);
        t.setPosition(x, y);
        window_.draw(t);
    };
    auto drawTextC = [&](const std::string& str, float y,
                         unsigned sz, sf::Color col) {
        sf::Text t;
        t.setFont(font_);
        t.setCharacterSize(sz);
        t.setFillColor(col);
        t.setString(str);
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.width / 2.f, 0.f);
        t.setPosition(640.f, y);
        window_.draw(t);
    };
    auto drawLine = [&](float y, sf::Color col = sf::Color(100, 100, 100, 180)) {
        sf::RectangleShape r(sf::Vector2f(740.f, 1.f));
        r.setFillColor(col);
        r.setPosition(270.f, y);
        window_.draw(r);
    };

    // Tytul
    drawTextC("TOP 10 WYNIKOW", 66.f, 40, sf::Color(255, 215, 0));
    drawLine(122.f, sf::Color(200, 170, 50, 200));

    // Naglowki kolumn
    const float COL_RANK  = 272.f;
    const float COL_NAME  = 318.f;
    const float COL_TIME  = 680.f;
    const float COL_COINS = 782.f;
    const sf::Color HDR_COL(160, 160, 160);

    drawText("#",      COL_RANK,  130.f, 16, HDR_COL);
    drawText("IMIE",   COL_NAME,  130.f, 16, HDR_COL);
    drawText("CZAS",   COL_TIME,  130.f, 16, HDR_COL);
    drawText("MONETY", COL_COINS, 130.f, 16, HDR_COL);
    drawLine(156.f);

    // Wiersze wynikow
    if (topScores_.empty()) {
        drawTextC("Brak wynikow – zagraj i wygraj, aby zobaczyc ranking!",
                  310.f, 19, sf::Color(160, 160, 160));
        // Diagnostyka – pokazuje gdzie szukano pliku i co sie stalo
        drawTextC("Status: " + scoresStatus_,
                  346.f, 14, sf::Color(255, 160, 50));
        drawTextC(scoresPath_,
                  372.f, 12, sf::Color(100, 120, 100));
    } else {
        for (int i = 0; i < static_cast<int>(topScores_.size()); ++i) {
            const ScoreEntry& s = topScores_[i];
            const float ry = 162.f + i * 34.f;

            // Lekkie tlo co drugiego wiersza
            if (i % 2 == 1) {
                sf::RectangleShape rowBg(sf::Vector2f(740.f, 33.f));
                rowBg.setFillColor(sf::Color(255, 255, 255, 10));
                rowBg.setPosition(270.f, ry);
                window_.draw(rowBg);
            }

            // Zloto / srebro / braz / biel
            sf::Color col;
            if      (i == 0) col = sf::Color(255, 215,   0);
            else if (i == 1) col = sf::Color(192, 192, 192);
            else if (i == 2) col = sf::Color(210, 140,  50);
            else             col = sf::Color(220, 220, 220);

            drawText(std::to_string(i + 1) + ".", COL_RANK,  ry + 7.f, 18, col);
            drawText(s.name,                       COL_NAME,  ry + 7.f, 18, col);
            drawText(s.time,                       COL_TIME,  ry + 7.f, 18, col);
            drawText(std::to_string(s.coins),      COL_COINS, ry + 7.f, 18, col);
        }
    }

    drawLine(618.f);
    drawTextC("Escape / Enter / klik - powrot do menu",
              626.f, 15, sf::Color(130, 130, 130));
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
    scoresPath_   = "scores.json";
    scoresStatus_ = "";

    try {
        // Wczytaj cały plik jako jeden string (odporne na \r\n / \n / \r)
        std::ifstream file(scoresPath_, std::ios::binary);
        if (!file.is_open()) {
            scoresStatus_ = "BRAK PLIKU scores.json obok Platformer.exe";
            return;
        }

        std::ostringstream buf;
        buf << file.rdbuf();
        const std::string content = buf.str();

        if (content.empty()) {
            scoresStatus_ = "PLIK PUSTY";
            return;
        }

        // Szukaj kolejnych obiektów { ... } – niezależnie od końcówek linii
        std::size_t pos = 0;
        while (pos < content.size()) {
            const auto objStart = content.find('{', pos);
            if (objStart == std::string::npos) break;
            const auto objEnd = content.find('}', objStart);
            if (objEnd == std::string::npos) break;

            const std::string obj = content.substr(objStart, objEnd - objStart + 1);
            pos = objEnd + 1;

            // Wyciąga wartość klucza: "key":"val" lub "key":val
            auto field = [&](const std::string& key, bool quoted) -> std::string {
                const std::string kStr = "\"" + key + "\":";
                auto kp = obj.find(kStr);
                if (kp == std::string::npos) return "";
                auto vs = kp + kStr.size();
                while (vs < obj.size() && (obj[vs] == ' ' || obj[vs] == '\t')) ++vs;
                if (vs >= obj.size()) return "";
                if (quoted) {
                    if (obj[vs] != '"') return "";
                    ++vs;
                    const auto ve = obj.find('"', vs);
                    if (ve == std::string::npos) return "";
                    return obj.substr(vs, ve - vs);
                } else {
                    auto ve = obj.find_first_of(",}", vs);
                    if (ve == std::string::npos) ve = obj.size();
                    std::string val = obj.substr(vs, ve - vs);
                    const auto t1 = val.find_first_not_of(" \t\r\n");
                    const auto t2 = val.find_last_not_of(" \t\r\n");
                    return (t1 == std::string::npos) ? "" : val.substr(t1, t2 - t1 + 1);
                }
            };

            ScoreEntry e;
            e.name        = field("name",  true);
            e.time        = field("time",  true);
            const auto cs = field("coins", false);
            if (!cs.empty()) { try { e.coins = std::stoi(cs); } catch (...) {} }

            if (!e.name.empty() && !e.time.empty())
                topScores_.push_back(e);
        }

        scoresStatus_ = "Wczytano: " + std::to_string(topScores_.size()) + " wynikow";

        auto toSec = [](const std::string& t) -> int {
            const auto col = t.find(':');
            if (col == std::string::npos) return 99999;
            try { return std::stoi(t.substr(0, col)) * 60 + std::stoi(t.substr(col + 1)); }
            catch (...) { return 99999; }
        };

        std::sort(topScores_.begin(), topScores_.end(),
                  [&](const ScoreEntry& a, const ScoreEntry& b) {
                      return toSec(a.time) < toSec(b.time);
                  });

        if (topScores_.size() > 10)
            topScores_.resize(10);

    } catch (...) {
        scoresStatus_ = "BLAD WCZYTYWANIA";
        topScores_.clear();
    }
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
