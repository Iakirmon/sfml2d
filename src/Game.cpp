#include "Game.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>

// ============================================================
// Logger — zapisuje diagnostykę do %TEMP%\game2d_debug.log
//
// Używamy zmiennej środowiskowej TEMP (Windows), żeby uzyskać
// ścieżkę do folderu zawsze zapisywalnego (np. C:\Users\Jan\AppData\Local\Temp).
// Gdybyśmy pisali do bieżącego katalogu, mogłoby to nie działać
// w środowiskach z ograniczonymi uprawnieniami zapisu.
//
// LOG_PATH obliczany jest RAZ przy starcie (static const).
// LOG() otwiera plik w trybie append (ios::app), dopisuje i zamyka.
// ============================================================
static std::string logPath() {
    const char* tmp = std::getenv("TEMP");
    if (tmp) return std::string(tmp) + "\\game2d_debug.log";
    const char* home = std::getenv("USERPROFILE");
    if (home) return std::string(home) + "\\game2d_debug.log";
    return "C:\\game2d_debug.log"; // ostateczny fallback
}
static const std::string LOG_PATH = logPath(); // obliczany raz przy ładowaniu biblioteki

static void LOG(const std::string& msg) {
    std::ofstream f(LOG_PATH, std::ios::app); // otwórz → dopisz → zamknij automatycznie
    if (f.is_open()) f << msg << "\n";
}

// ============================================================
// Konstruktor — inicjalizacja całej gry
//
// Lista inicjalizacyjna (po :) uruchamia się PRZED ciałem konstruktora.
// Kolejność inicjalizacji = kolejność deklaracji w Game.h, nie lista.
// ============================================================
Game::Game()
    : window_(sf::VideoMode(1280, 720), "Platformer"),
      gameView_(sf::FloatRect(0.f, 0.f, 1280.f, 720.f)) { // widok 1280×720 px, środek (640,360)

    // Wyczyść log z poprzedniej sesji i zapisz znacznik startu
    { std::ofstream f(LOG_PATH); f << "=== GAME START ===\n"; }
    LOG("LOG_PATH = " + LOG_PATH);

    window_.setFramerateLimit(60); // ogranicz do 60 FPS — stabilne dt ≈ 0.0167 s

    // Czcionka — używana przez WSZYSTKIE elementy UI
    // Próbujemy ścieżki względne (obok .exe) i fallback
    if (!font_.loadFromFile("assets/fonts/Roboto-Regular.ttf")) {
        LOG("FONT: nie znaleziono assets/fonts/Roboto-Regular.ttf, probuje fallback");
        font_.loadFromFile("Roboto-Regular.ttf");
    } else {
        LOG("FONT: zaladowano OK");
    }
    // Asercja: jeśli font nie załadowany, getInfo().family jest pustym stringiem
    assert(font_.getInfo().family != "" && "Font nie zostal zaladowany – sprawdz assets/fonts/");

    level_.load(0); // wczytaj pierwszy poziom kampanii (indeks 0)

    // Załaduj SFX (krótkie dźwięki efektów) — wczytywane do RAM
    audio_.load("jump",     "assets/sounds/jump.ogg");
    audio_.load("coin",     "assets/sounds/coin.ogg");
    audio_.load("death",    "assets/sounds/death.ogg");
    audio_.load("gameover", "assets/sounds/gameover.ogg");

    playMusic("assets/music/menu.ogg"); // muzyka startuje od razu w MENU

    // Warstwy tła parallax — kolejność: od najdalszej do najbliższej
    // (rysowane od tyłu do przodu, ostatnia przesłania poprzednie)
    bgLayers_.emplace_back("assets/textures/bg_clouds.png",    0.2f, 720.f); // najwolniejsza
    bgLayers_.emplace_back("assets/textures/bg_mountains.png", 0.4f, 720.f);
    bgLayers_.emplace_back("assets/textures/bg_trees.png",     0.7f, 720.f); // najszybsza

    // Pola sf::Text są inicjalizowane tutaj bo setFont musi być wywołane
    // po załadowaniu font_ — lista inicjalizacyjna byłaby za wcześnie
    // HUD: kolejność pionowa = kolejność rysowania → Coins / Level / Lives / Time
    hudTextCoins_.setFont(font_);
    hudTextCoins_.setCharacterSize(20);
    hudTextCoins_.setFillColor(sf::Color::White);
    hudTextCoins_.setPosition(10.f, 10.f); // lewy górny róg ekranu

    hudTextLevel_.setFont(font_);
    hudTextLevel_.setCharacterSize(20);
    hudTextLevel_.setFillColor(sf::Color::White);
    hudTextLevel_.setPosition(10.f, 40.f);

    hudTextLives_.setFont(font_);
    hudTextLives_.setCharacterSize(20);
    hudTextLives_.setFillColor(sf::Color::White);
    hudTextLives_.setPosition(10.f, 70.f);

    hudTextTime_.setFont(font_);
    hudTextTime_.setCharacterSize(20);
    hudTextTime_.setFillColor(sf::Color::White);
    hudTextTime_.setPosition(10.f, 100.f);

    centerText_.setFont(font_);
    centerText_.setCharacterSize(36);
    centerText_.setFillColor(sf::Color::White);

    inputText_.setFont(font_);
    inputText_.setCharacterSize(24);
    inputText_.setFillColor(sf::Color::Yellow);

    gameView_.setCenter(640.f, 360.f); // środek widoku na środku ekranu (redundantne, dla czytelności)
}

// ============================================================
// run() — główna pętla gry
//
// Wzorzec: mierzenie czasu → zdarzenia → logika → rysowanie
//
// dt (delta time) = czas od poprzedniej klatki [sekundy]
// Mnożenie przez dt w fizyce sprawia, że ruch jest niezależny
// od FPS: przy 30fps dt≈0.033, przy 60fps dt≈0.017 — gracz
// pokonuje taką samą odległość na sekundę w obu przypadkach.
//
// Ograniczenie dt do 0.05s (20fps equiv.) zapobiega "tunelowaniu"
// — obiekt mógłby przeskoczyć przez cienką platformę gdyby
// jedno dt było bardzo duże (np. chwilowe zawieszenie systemu).
// ============================================================
void Game::run() {
    sf::Clock clock;
    while (window_.isOpen()) {
        float dt = clock.restart().asSeconds(); // czas od poprzedniego restart()
        dt = std::min(dt, 0.05f); // cap: max 50ms / klatka — zapobiega tunelowaniu

        handleEvents(); // 1. zdarzenia wejścia
        update(dt);     // 2. logika i fizyka
        render();       // 3. rysowanie
    }
}

// ============================================================
// handleEvents() — obsługa zdarzeń systemu
//
// KLUCZOWE: blok if/else if dla stanów, NIE osobne if-y.
//
// Gdyby były osobne if-y, jedno zdarzenie mogłoby zostać
// obsłużone przez dwa stany jednocześnie.
// Przykład błędu: kliknięcie "Najlepsze wyniki" ustawia
// state_=SCORES, a następny if (state_==SCORES) natychmiast
// by je cofnął do MENU. else if zapobiega temu.
// ============================================================
void Game::handleEvents() {
    sf::Event event{};
    while (window_.pollEvent(event)) { // pollEvent pobiera jedno zdarzenie z kolejki

        if (event.type == sf::Event::Closed) {
            window_.close(); // zamknij okno → window_.isOpen() zwróci false → wyjście z run()
        }

        // Każdy stan obsługuje TYLKO swoje zdarzenia (łańcuch else if)
        if (state_ == GameState::MENU) {

            if (event.type == sf::Event::KeyPressed) {
                // Strzałka Up: wróć o jeden element (modulo 3 zachowuje zakres 0-2)
                // +2 zamiast -1 bo ujemne modulo w C++ może dać wynik ujemny
                if (event.key.code == sf::Keyboard::Up)
                    menuSelectedItem_ = (menuSelectedItem_ + 2) % 3;
                if (event.key.code == sf::Keyboard::Down)
                    menuSelectedItem_ = (menuSelectedItem_ + 1) % 3;
                if (event.key.code == sf::Keyboard::Return) {
                    if      (menuSelectedItem_ == 0) startGame();
                    else if (menuSelectedItem_ == 1) { loadTopScores(); state_ = GameState::SCORES; }
                    else if (menuSelectedItem_ == 2) window_.close();
                }
            } else if (event.type == sf::Event::MouseMoved) {
                // Hover: gdy kursor jest w obszarze ±20px od środka elementu → zaznacz
                sf::Vector2f mouse(static_cast<float>(event.mouseMove.x),
                                   static_cast<float>(event.mouseMove.y));
                const float itemY[3] = {320.f, 390.f, 460.f}; // środki Y elementów
                for (int i = 0; i < 3; ++i) {
                    if (mouse.y >= itemY[i] - 20.f && mouse.y <= itemY[i] + 20.f)
                        menuSelectedItem_ = i;
                }
            } else if (event.type == sf::Event::MouseButtonPressed &&
                       event.mouseButton.button == sf::Mouse::Left) {
                // Klik: sprawdź pozycję kliknięcia (nie polegaj na hover — mógł się nie zaktualizować)
                sf::Vector2f click(static_cast<float>(event.mouseButton.x),
                                   static_cast<float>(event.mouseButton.y));
                const float itemY[3] = {320.f, 390.f, 460.f};
                for (int i = 0; i < 3; ++i) {
                    if (click.y >= itemY[i] - 30.f && click.y <= itemY[i] + 30.f)
                        menuSelectedItem_ = i; // ±30px dla łatwiejszego klikania
                }
                if      (menuSelectedItem_ == 0) startGame();
                else if (menuSelectedItem_ == 1) { loadTopScores(); state_ = GameState::SCORES; }
                else if (menuSelectedItem_ == 2) window_.close();
            }

        } else if (state_ == GameState::SCORES) {
            // Dowolny klawisz/klik wraca do menu
            if (event.type == sf::Event::KeyPressed &&
                (event.key.code == sf::Keyboard::Escape ||
                 event.key.code == sf::Keyboard::Return)) {
                state_ = GameState::MENU;
            } else if (event.type == sf::Event::MouseButtonPressed &&
                       event.mouseButton.button == sf::Mouse::Left) {
                state_ = GameState::MENU;
            }

        } else if (state_ == GameState::PLAYING) {
            // Escape przerywa grę i wraca do menu (gra nie jest kasowana — tylko stan)
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Escape) {
                state_ = GameState::MENU;
                playMusic("assets/music/menu.ogg");
            }
        }

        else if (state_ == GameState::GAME_OVER) {
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::R)
                    startGame(); // restart od zera
                else if (event.key.code == sf::Keyboard::Escape) {
                    state_ = GameState::MENU;
                    playMusic("assets/music/menu.ogg");
                }
            }

        } else if (state_ == GameState::WINNING) {
            // TextEntered: zdarzenie dla wpisywania tekstu (obsługuje layout klawiatury,
            // znaki specjalne, itp.) — lepsze niż KeyPressed dla pola tekstowego
            if (event.type == sf::Event::TextEntered) {
                char c = static_cast<char>(event.text.unicode);
                // Akceptuj tylko drukowalne znaki ASCII (32–126), ignoruj backspace (8), enter (13) itd.
                if (c >= 32 && c < 127 && playerInput_.length() < maxInputLength_) {
                    playerInput_ += c;
                }
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::BackSpace && !playerInput_.empty()) {
                    playerInput_.pop_back(); // usuń ostatni znak
                } else if (event.key.code == sf::Keyboard::Return && !playerInput_.empty()) {
                    saveScore(playerInput_); // zapisz wynik do scores.json
                    state_ = GameState::MENU;
                    playerInput_ = "";
                    playMusic("assets/music/menu.ogg");
                }
            }
        }
    }
}

// ============================================================
// update(dt) — logika i fizyka gry
//
// Wywoływane dla WSZYSTKICH stanów (patrz run()).
// Stany nieaktywne obsługują tylko niezbędne minimum (parallax).
// ============================================================
void Game::update(float dt) {
    // W MENU i SCORES: tylko auto-scroll tła (gra jest wstrzymana)
    // menuScrollOffset_ rośnie monotoniczne → parallax płynnie przewija się w pętli
    if (state_ == GameState::MENU || state_ == GameState::SCORES) {
        menuScrollOffset_ += 60.f * dt; // prędkość auto-scrollu: 60 jednostek/s
        for (auto& layer : bgLayers_)
            layer.update(menuScrollOffset_);
        return; // nie wykonuj dalszej logiki
    }

    // GAME_OVER, WIN, WINNING: fizyka zatrzymana — ekran nakładkowy aktywny
    if (state_ != GameState::PLAYING) return;

    elapsedTime_ += dt; // licznik czasu → zapisywany do wyniku po wygranej

    // Aktualizuj parallax w oparciu o pozycję gracza (X w world space)
    float playerX = player_.getHitbox().left;
    for (auto& layer : bgLayers_) {
        layer.update(playerX);
    }

    // Zapiszmy liczbę monet przed update() — porównamy po, by wykryć zebranie
    int coinsBefore = static_cast<int>(level_.getCollectedCoins());

    player_.handleInput();           // 1. wczytaj klawiaturę → ustaw velocity_
    player_.update(dt);              // 2. aplikuj grawitację, przesuń sprite
    level_.checkCollisions(player_); // 3. wykryj kolizje, wypchnij gracza z platform

    // Dźwięk skoku (justJumped_ = true tylko przez jedną klatkę)
    if (player_.justJumped()) {
        audio_.play("jump");
    }

    // Dźwięk monety: porównaj stan przed i po update
    if (static_cast<int>(level_.getCollectedCoins()) > coinsBefore) {
        audio_.play("coin");
    }

    // Wykryj wypadnięcie z poziomu: hitbox.top > dół ekranu + margines
    if (player_.getHitbox().top > 720.f + 50.f) {
        player_.respawn();   // wróć na spawn, odejmij życie
        audio_.play("death");
    }

    // Sprawdź warunki zakończenia gry (obie sprawdzamy z guardiem state_==PLAYING
    // bo updateCamera() poniżej może być jeszcze wywołane w tej klatce)
    if (player_.getLives() <= 0 && state_ == GameState::PLAYING) {
        audio_.play("gameover");
        state_ = GameState::GAME_OVER;
        playMusic("assets/music/gameover.ogg", false); // loop=false: jingle, nie pętla
    }
    // Zebrane wszystkie monety bieżącego poziomu:
    //   - jeśli jest jeszcze kolejny poziom → załaduj go (bez ekranu imienia, bez resetu czasu i żyć),
    //   - jeśli to był ostatni poziom (indeks == LEVEL_COUNT-1) → stan WINNING + jingle.
    if (level_.allCoinsCollected() && state_ == GameState::PLAYING) {
        if (currentLevelIndex_ + 1 < Level::LEVEL_COUNT) {
            // Dorzuć monety ukończonego poziomu do sumy na wynik końcowy
            coinsFromCompletedLevels_ += static_cast<int>(level_.getTotalCoins());
            ++currentLevelIndex_;
            level_.load(currentLevelIndex_);
            player_.prepareForNextLevel();     // reset pozycji/flag bez dotykania żyć
            gameView_.setCenter(640.f, 360.f); // kamera na początek nowego poziomu
            // Muzyka gameplay leci dalej — NIE wołamy playMusic (to by ją restartowało)
        } else {
            state_ = GameState::WINNING;
            playMusic("assets/music/win.ogg", false); // jingle, loop=false
        }
    }

    // Aktualizuj kamerę po całej logice — używa aktualnej pozycji gracza
    updateCamera(dt);
}

// ============================================================
// render() — rysowanie klatki
//
// Kolejność rysowania = kolejność warstw (późniejsze = na wierzchu):
//   1. clear (czyste niebieskie tło)
//   2. parallax (defaultView — screen space) — zawsze
//   3. level + player (gameView_ — world space)  ← we wszystkich stanach poza MENU/SCORES
//      (PLAYING, GAME_OVER, WINNING, WIN — świat gry jest wciąż widoczny pod nakładką)
//   4. HUD + nakładki (defaultView — screen space) — w tych samych stanach co pkt 3
//   5. display (zamień bufor → ekran)
//
// Uwaga: update() ma wczesny `return` dla stanów innych niż PLAYING, więc
// czas w HUD i parallax powiązany z pozycją gracza są „zamrożone” poza PLAYING.
// ============================================================
void Game::render() {
    window_.clear(sf::Color(135, 206, 235)); // kolor nieba (jasnoniebieski)

    // 1. Parallax — MUSI być z defaultView (sprite'y pozycjonowane w screen space)
    //    Gdybyśmy używali gameView_, sprite'y byłyby w world space i zniknęłyby z ekranu
    window_.setView(window_.getDefaultView());
    for (auto& layer : bgLayers_)
        layer.draw(window_);

    if (state_ == GameState::MENU) {
        drawMenu();   // dziedziczy defaultView — poprawnie w screen space
    } else if (state_ == GameState::SCORES) {
        drawScores(); // dziedziczy defaultView — poprawnie w screen space
    } else {
        // 2. Świat gry — gameView_ przesuwa "okno kamery" za graczem
        //    Obiekty w world space (x=0–3840) są wycinane do obszaru 1280×720 ekranu
        window_.setView(gameView_);
        level_.draw(window_);
        player_.draw(window_);

        // 3. HUD i nakładki — wróć do screen space (stałe współrzędne ekranu)
        //    BEZ tego przełączenia tekst HUD "jeździłby" razem z kamerą
        window_.setView(window_.getDefaultView());
        drawHUD();

        if      (state_ == GameState::GAME_OVER) drawGameOver();
        else if (state_ == GameState::WINNING)   drawWinningScreen();
        else if (state_ == GameState::WIN)       drawWin();
    }

    window_.display(); // zamień tylny bufor z przednim (double buffering)
}

// ============================================================
// drawHUD() — pasek informacyjny podczas gry
// ============================================================
void Game::drawHUD() {
    const auto total     = level_.getTotalCoins();     // monety TYLKO bieżącego poziomu
    const auto collected = level_.getCollectedCoins();

    // Konwersja czasu [s] na format M:SS
    int minutes = static_cast<int>(elapsedTime_) / 60;
    int seconds = static_cast<int>(elapsedTime_) % 60;

    hudTextCoins_.setString(
        "Coins: " + std::to_string(collected) + " / " + std::to_string(total));
    // Numer poziomu liczymy od 1 dla użytkownika; w kodzie indeks 0..LEVEL_COUNT-1
    hudTextLevel_.setString(
        "Level: " + std::to_string(currentLevelIndex_ + 1) + " / " +
        std::to_string(Level::LEVEL_COUNT));
    hudTextLives_.setString("Lives: " + std::to_string(player_.getLives()));
    hudTextTime_.setString("Time: " + std::to_string(minutes) + ":" +
                           (seconds < 10 ? "0" : "") + std::to_string(seconds)); // zero-padding

    // Kolejność rysowania = kolejność pionowa etykiet: Coins → Level → Lives → Time
    window_.draw(hudTextCoins_);
    window_.draw(hudTextLevel_);
    window_.draw(hudTextLives_);
    window_.draw(hudTextTime_);
}

// Wyśrodkowanie tekstu: setOrigin na połowie bounding boxa → setPosition na środku ekranu
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
    // Tytuł "YOU WIN!"
    centerText_.setString("YOU WIN!\nEnter your name:");
    sf::FloatRect bounds = centerText_.getLocalBounds();
    centerText_.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    centerText_.setPosition(1280.f / 2.f, 250.f);
    window_.draw(centerText_);

    // Pole tekstowe — dodajemy "_" jako wizualny kursor
    inputText_.setString(playerInput_ + "_");
    sf::FloatRect inputBounds = inputText_.getLocalBounds();
    inputText_.setOrigin(inputBounds.width / 2.f, inputBounds.height / 2.f);
    inputText_.setPosition(1280.f / 2.f, 400.f);
    window_.draw(inputText_);

    // Podpowiedź
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

// ============================================================
// drawMenu() — menu główne z trzema opcjami
//
// Zaznaczony element (menuSelectedItem_) rysowany żółtym kolorem.
// Technika wyśrodkowania: getLocalBounds() → setOrigin → setPosition(640, y)
// ============================================================
void Game::drawMenu() {
    sf::Text title;
    title.setFont(font_);
    title.setCharacterSize(64);
    title.setFillColor(sf::Color::White);
    title.setString("PLATFORMER");
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2.f, tb.height / 2.f); // origin = środek tekstu
    title.setPosition(1280.f / 2.f, 180.f);
    window_.draw(title);

    const char* labels[3] = {"START", "NAJLEPSZE WYNIKI", "WYJDZ"};
    const float itemY[3]  = {320.f, 390.f, 460.f};

    for (int i = 0; i < 3; ++i) {
        sf::Text item;
        item.setFont(font_);
        item.setCharacterSize(36);
        // Zaznaczony element = żółty, pozostałe = biały
        item.setFillColor(i == menuSelectedItem_ ? sf::Color::Yellow : sf::Color::White);
        item.setString(labels[i]);
        sf::FloatRect ib = item.getLocalBounds();
        item.setOrigin(ib.width / 2.f, ib.height / 2.f);
        item.setPosition(1280.f / 2.f, itemY[i]);
        window_.draw(item);
    }

    // Podpowiedź sterowania na dole ekranu
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

// ============================================================
// drawScores() — ekran top 10 wyników
//
// Lambdy lokalne (drawText, drawTextC, drawLine) upraszczają
// powtarzający się kod rysowania tekstu bez potrzeby tworzenia
// prywatnych metod tylko dla tego jednego miejsca.
// ============================================================
void Game::drawScores() {
    // Ciemny panel dla czytelności tekstu na tle parallax
    sf::RectangleShape panel(sf::Vector2f(780.f, 622.f));
    panel.setFillColor(sf::Color(8, 8, 24, 215));    // bardzo ciemny, prawie nieprzezroczysty
    panel.setOutlineColor(sf::Color(200, 170, 50, 180)); // złota ramka
    panel.setOutlineThickness(2.f);
    panel.setPosition(250.f, 49.f);
    window_.draw(panel);

    // Lambda: rysuj tekst od podanej pozycji (x, y)
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

    // Lambda: rysuj tekst wyśrodkowany poziomo na ekranie (origin na środku)
    auto drawTextC = [&](const std::string& str, float y,
                         unsigned sz, sf::Color col) {
        sf::Text t;
        t.setFont(font_);
        t.setCharacterSize(sz);
        t.setFillColor(col);
        t.setString(str);
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.width / 2.f, 0.f);
        t.setPosition(640.f, y); // 640 = środek ekranu 1280px
        window_.draw(t);
    };

    // Lambda: rysuj poziomą linię separatora
    auto drawLine = [&](float y, sf::Color col = sf::Color(100, 100, 100, 180)) {
        sf::RectangleShape r(sf::Vector2f(740.f, 1.f));
        r.setFillColor(col);
        r.setPosition(270.f, y);
        window_.draw(r);
    };

    drawTextC("TOP 10 WYNIKOW", 66.f, 40, sf::Color(255, 215, 0)); // złoty tytuł
    drawLine(122.f, sf::Color(200, 170, 50, 200));

    // Nagłówki kolumn z ustalonymi pozycjami X dla prawidłowego wyrównania
    // (proporcjonalny font — padding spacjami nie działa, stałe X są konieczne)
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

    if (topScores_.empty()) {
        // Pokaż diagnostykę gdy brak wyników — scoresStatus_ i scoresPath_
        // informują użytkownika dlaczego lista jest pusta
        drawTextC("Brak wynikow – zagraj i wygraj, aby zobaczyc ranking!",
                  310.f, 19, sf::Color(160, 160, 160));
        drawTextC("Status: " + scoresStatus_,
                  346.f, 14, sf::Color(255, 160, 50));
        drawTextC(scoresPath_,
                  372.f, 12, sf::Color(100, 120, 100));
    } else {
        for (int i = 0; i < static_cast<int>(topScores_.size()); ++i) {
            const ScoreEntry& s = topScores_[i];
            const float ry = 162.f + i * 34.f; // Y pozycja wiersza (34px na wiersz)

            // Lekkie tło co drugiego wiersza dla czytelności
            if (i % 2 == 1) {
                sf::RectangleShape rowBg(sf::Vector2f(740.f, 33.f));
                rowBg.setFillColor(sf::Color(255, 255, 255, 10)); // prawie przezroczysty
                rowBg.setPosition(270.f, ry);
                window_.draw(rowBg);
            }

            // Kolory medali: złoto → srebro → brąz → biały
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

// ============================================================
// startGame() — reset i start nowej gry
// ============================================================
void Game::startGame() {
    assert(menuSelectedItem_ >= 0 && menuSelectedItem_ < 3 &&
           "menuSelectedItem_ poza zakresem 0-2");
    state_                    = GameState::PLAYING;
    // Reset kampanii: zawsze od poziomu 0 i wyzerowany akumulator monet
    currentLevelIndex_        = 0;
    coinsFromCompletedLevels_ = 0;
    player_.resetState();                      // pozycja, życia, flagi
    level_.load(currentLevelIndex_);           // załaduj pierwszy poziom kampanii
    elapsedTime_              = 0.f;           // wyzeruj licznik czasu
    playerInput_              = "";
    playMusic("assets/music/gameplay.ogg");
    gameView_.setCenter(640.f, 360.f);         // kamera na początku poziomu
}

// ============================================================
// updateCamera(dt) — płynna kamera śledząca gracza
//
// Lerp (Linear Interpolation): każda klatka kamera pokonuje
// ułamek (5*dt) pozostałej odległości do gracza.
// Przy 60fps: 5 * 0.0167 ≈ 8.3% dystansu / klatkę.
// Kamera "dogania" gracza w ~0.5s — ani za wolno, ani szarpiąc.
//
// Clamp: kamera nie wychodzi poza granice poziomu.
//   min center X = halfW    (640) → widać od x=0
//   max center X = LEVEL_WIDTH - halfW (3200) → widać do x=3840
// ============================================================
void Game::updateCamera(float dt) {
    const float levelWidth = level_.getLevelWidth(); // 3840.f
    const float halfW      = 640.f;  // połowa szerokości ekranu
    const float halfH      = 360.f;  // środek Y — kamera nie przewija się pionowo
    assert(level_.getLevelWidth() > 1280.f &&
           "Poziom musi byc szerszy niz szerokosc ekranu");

    // Cel: środek gracza w world space
    sf::FloatRect hitbox = player_.getHitbox();
    float targetX = hitbox.left + hitbox.width / 2.f;

    // Lerp: smoothX = current + (target - current) * speed * dt
    sf::Vector2f cur = gameView_.getCenter();
    float smoothX = cur.x + (targetX - cur.x) * 5.f * dt;

    // Clamp: ogranicz do granic poziomu
    smoothX = std::max(halfW, std::min(smoothX, levelWidth - halfW));

    gameView_.setCenter(smoothX, halfH); // Y zawsze stały (brak pionowego scrollu)
}

// ============================================================
// loadTopScores() — wczytuje scores.json i ładuje top 10
//
// Format pliku: jedna linia JSON na wynik (newline-delimited JSON):
//   {"name":"Wojtek","time":"0:05","coins":5}
//   {"name":"Jan","time":"0:08","coins":3}
//
// Strategia parsowania: znajdź każdy obiekt { ... } w stringu.
// Odporność na: \r\n (Windows), \n (Unix), brak nagłówków HTTP,
// spacje po dwukropku, dodatkowe białe znaki.
// ============================================================
void Game::loadTopScores() {
    topScores_.clear();
    scoresPath_   = "scores.json";
    scoresStatus_ = "";

    LOG("loadTopScores: " + scoresPath_);

    try {
        // Tryb binarny: wczytaj dokładnie to co jest w pliku
        // (tryb tekstowy na Windows konwertuje \r\n → \n, co może powodować problemy)
        std::ifstream file(scoresPath_, std::ios::binary);
        if (!file.is_open()) {
            scoresStatus_ = "BRAK PLIKU scores.json obok Platformer.exe";
            LOG("loadTopScores: " + scoresStatus_);
            return;
        }

        // Wczytaj cały plik do jednego stringa przez rdbuf()
        // (szybsze niż getline() w pętli, odporne na różne końcówki linii)
        std::ostringstream buf;
        buf << file.rdbuf();
        const std::string content = buf.str();

        if (content.empty()) {
            scoresStatus_ = "PLIK PUSTY";
            LOG("loadTopScores: " + scoresStatus_);
            return;
        }

        // Iteracyjnie szukaj par { ... } w całym stringu
        std::size_t pos = 0;
        while (pos < content.size()) {
            const auto objStart = content.find('{', pos);
            if (objStart == std::string::npos) break; // brak więcej obiektów
            const auto objEnd = content.find('}', objStart);
            if (objEnd == std::string::npos) break;   // niezamknięty obiekt

            const std::string obj = content.substr(objStart, objEnd - objStart + 1);
            pos = objEnd + 1; // szukaj dalej od następnego znaku po }

            // Lambda pomocnicza: wyciągnij wartość pola "key" z obiektu JSON
            // quoted=true  → wartość w cudzysłowach: "name":"Wojtek" → "Wojtek"
            // quoted=false → wartość liczbowa:        "coins":5       → "5"
            auto field = [&](const std::string& key, bool quoted) -> std::string {
                const std::string kStr = "\"" + key + "\":";
                auto kp = obj.find(kStr);
                if (kp == std::string::npos) return ""; // brak klucza
                auto vs = kp + kStr.size();
                // Pomiń opcjonalne spacje po dwukropku
                while (vs < obj.size() && (obj[vs] == ' ' || obj[vs] == '\t')) ++vs;
                if (vs >= obj.size()) return "";
                if (quoted) {
                    if (obj[vs] != '"') return ""; // spodziewamy się cudzysłowu
                    ++vs;
                    const auto ve = obj.find('"', vs); // szukaj zamykającego "
                    if (ve == std::string::npos) return "";
                    return obj.substr(vs, ve - vs);
                } else {
                    // Wartość liczbowa kończy się na , lub }
                    auto ve = obj.find_first_of(",}", vs);
                    if (ve == std::string::npos) ve = obj.size();
                    std::string val = obj.substr(vs, ve - vs);
                    // Usuń białe znaki z obu końców
                    const auto t1 = val.find_first_not_of(" \t\r\n");
                    const auto t2 = val.find_last_not_of(" \t\r\n");
                    return (t1 == std::string::npos) ? "" : val.substr(t1, t2 - t1 + 1);
                }
            };

            ScoreEntry e;
            e.name = field("name",  true);
            e.time = field("time",  true);
            const auto cs = field("coins", false);
            // try/catch dla stoi: zabezpieczenie przed nieprawidłową wartością coins
            if (!cs.empty()) { try { e.coins = std::stoi(cs); } catch (...) {} }

            // Dodaj wpis tylko jeśli ma imię i czas (coins opcjonalne)
            if (!e.name.empty() && !e.time.empty())
                topScores_.push_back(e);
        }

        scoresStatus_ = "Wczytano: " + std::to_string(topScores_.size()) + " wynikow";
        LOG("loadTopScores: " + scoresStatus_);

        // Lambda konwersji czasu "M:SS" → sekundy (do sortowania)
        auto toSec = [](const std::string& t) -> int {
            const auto col = t.find(':');
            if (col == std::string::npos) return 99999; // nieprawidłowy format → na koniec
            try { return std::stoi(t.substr(0, col)) * 60 + std::stoi(t.substr(col + 1)); }
            catch (...) { return 99999; }
        };

        // Sortuj rosnąco po czasie (najszybszy = najlepszy = pierwszy)
        std::sort(topScores_.begin(), topScores_.end(),
                  [&](const ScoreEntry& a, const ScoreEntry& b) {
                      return toSec(a.time) < toSec(b.time);
                  });

        // Ogranicz do top 10
        if (topScores_.size() > 10)
            topScores_.resize(10);
        assert(topScores_.size() <= 10 && "topScores_ przekroczyl limit 10 po resize");

    } catch (const std::exception& ex) {
        scoresStatus_ = "BLAD: " + std::string(ex.what());
        LOG("loadTopScores: WYJATEK = " + scoresStatus_);
        topScores_.clear();
    } catch (...) {
        // Łapie każdy wyjątek (w tym rzucane przez kod C/systemowy)
        scoresStatus_ = "NIEZNANY BLAD";
        LOG("loadTopScores: NIEZNANY WYJATEK");
        topScores_.clear();
    }
}

// ============================================================
// saveScore() — dopisuje wynik do scores.json
//
// Tryb append (ios::app): nowe wpisy są dodawane na końcu pliku.
// Każdy wynik to osobna linia — newline-delimited JSON.
// ============================================================
void Game::saveScore(const std::string& playerName) {
    std::ofstream file("scores.json", std::ios::app); // otwiera lub tworzy plik
    if (file.is_open()) {
        int minutes = static_cast<int>(elapsedTime_) / 60;
        int seconds = static_cast<int>(elapsedTime_) % 60;
        std::string timeStr = std::to_string(minutes) + ":" +
                              (seconds < 10 ? "0" : "") + std::to_string(seconds);

        // Łączna liczba monet z CAŁEJ kampanii (3 poziomy):
        //   coinsFromCompletedLevels_ = suma getTotalCoins() z poziomów 0 i 1 (zebrana przy przejściach),
        //   level_.getCollectedCoins() = monety ostatniego poziomu (==getTotalCoins() przy WINNING).
        const int totalCoins = coinsFromCompletedLevels_ +
                               static_cast<int>(level_.getCollectedCoins());

        // Ręcznie formatujemy JSON (bez zewnętrznej biblioteki)
        file << "{\"name\":\"" << playerName
             << "\",\"time\":\"" << timeStr
             << "\",\"coins\":" << totalCoins << "}\n";
        file.close();
    }
}

// ============================================================
// playMusic() — zamiana muzyki w tle
//
// sf::Music streamuje z dysku (nie wczytuje całości do RAM).
// Zatrzymujemy poprzednią przed otwarciem nowej — SFML
// pozwala mieć tylko jeden aktywny strumień na sf::Music.
// loop=false: jingle (gameover, win) — odtwarza się raz.
// ============================================================
void Game::playMusic(const std::string& filepath, bool loop, float volume) {
    music_.stop();
    if (!music_.openFromFile(filepath)) return; // jeśli plik nie istnieje — cicho ignoruj
    music_.setLoop(loop);
    music_.setVolume(volume); // 0–100
    music_.play();
}

void Game::stopMusic() {
    music_.stop();
}
