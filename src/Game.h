#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <deque>
#include <vector>
#include "Player.h"
#include "Level.h"
#include "AudioManager.h"
#include "ParallaxLayer.h"

// ============================================================
// GameState — maszyna stanów gry
//
// Gra zawsze jest w dokładnie jednym stanie. Stan kontroluje:
//   - co jest rysowane (render)
//   - co reaguje na klawiaturę/mysz (handleEvents)
//   - co jest aktualizowane (update)
//
// Diagram przejść:
//   MENU ──[Start]──► PLAYING ──[0 żyć]──► GAME_OVER ──[R]──► PLAYING
//     ▲                  │                     └──[Esc]──────────┘
//     │              [wszystkie monety]
//     │                  ▼
//     │              WINNING ──[Enter z imieniem]──► MENU
//     │
//     └──[Najlepsze wyniki]──► SCORES ──[Esc]──► MENU
// ============================================================
enum class GameState {
    MENU,      // menu główne
    PLAYING,   // rozgrywka
    GAME_OVER, // gracz stracił wszystkie życia
    WIN,       // (nieużywany aktualnie)
    WINNING,   // gracz wygrał — wpisuje imię do rankingu
    SCORES,    // ekran top 10 wyników
};

// Jeden wpis w rankingu (wczytany z scores.json)
struct ScoreEntry {
    std::string name;
    std::string time;  // format "M:SS" np. "1:05"
    int coins{0};
};

// ============================================================
// Game — główny kontroler gry
//
// Posiada (przez wartość, nie wskaźniki) wszystkie obiekty gry:
//   player_, level_, audio_, music_, bgLayers_
//
// Cykl życia: konstruktor → run() → (pętla) → destruktor
// ============================================================
class Game {
public:
    Game();    // ładuje zasoby, otwiera okno, ustawia stan MENU
    void run(); // blokuje; wraca gdy okno zostanie zamknięte

private:
    // ── Pętla gry ──────────────────────────────────────────────────
    void handleEvents(); // czyta zdarzenia systemu (klawiatura, mysz, zamknięcie)
    void update(float dt); // aktualizuje logikę (dt = czas klatki [s])
    void render();          // rysuje wszystko na ekranie

    // ── Rysowanie poszczególnych ekranów ───────────────────────────
    void drawHUD();           // pasek góry (monety/życia/czas) — tylko w PLAYING
    void drawGameOver();      // nakładka "GAME OVER" + klawisze
    void drawWin();           // nakładka "YOU WIN" (nieużywana aktualnie)
    void drawWinningScreen(); // nakładka + pole tekstowe do wpisania imienia
    void drawMenu();          // pełnoekranowe menu główne
    void drawScores();        // pełnoekranowy ranking top 10

    // ── Zarządzanie stanem ─────────────────────────────────────────
    void startGame();          // startuje/restartuje grę; resetuje gracza i poziom
    void updateCamera(float dt); // przesuwa gameView_ za graczem (lerp + clamp)
    void loadTopScores();      // wczytuje i sortuje scores.json → topScores_
    void saveScore(const std::string& playerName); // dopisuje wynik do scores.json

    // Zatrzymuje poprzednią muzykę i uruchamia nową
    // loop=false dla krótkich jingli (gameover, win)
    void playMusic(const std::string& filepath, bool loop = true, float volume = 50.f);
    void stopMusic();

    // ── Zasoby i obiekty gry ───────────────────────────────────────
    sf::RenderWindow window_;  // okno SFML 1280×720
    sf::Font         font_;    // czcionka Roboto używana dla całego UI

    GameState state_{GameState::MENU}; // aktualny stan (startujemy w MENU)

    Player       player_;  // gracz (fizyka, animacje, życia)
    Level        level_;   // platformy, monety, kolizje
    AudioManager audio_;   // SFX (skoki, monety, śmierć)
    sf::Music    music_;   // muzyka w tle (streamowana z dysku)

    // Widok kamery: porusza się za graczem w world space
    // defaultView = stały widok 1280×720 dla HUD, menu, parallax
    sf::View gameView_;

    // Warstwy tła parallax (indeks 0 = najdalsza/najwolniejsza)
    std::deque<ParallaxLayer> bgLayers_;

    // ── Elementy UI ────────────────────────────────────────────────
    sf::Text hudTextCoins_;  // "Coins: X / Y" w lewym górnym rogu
    sf::Text hudTextLives_;  // "Lives: X"
    sf::Text hudTextTime_;   // "Time: M:SS"
    sf::Text centerText_;    // środkowe napisy (GAME OVER, YOU WIN)
    sf::Text inputText_;     // pole wpisywania imienia gracza

    // ── Stan rozgrywki ─────────────────────────────────────────────
    float       elapsedTime_{0.f};    // czas od startu gry [s] → zapisywany w wyniku
    std::string playerInput_{""};     // imię wpisywane przez gracza po wygranej
    int         maxInputLength_{20};  // maksymalna długość imienia

    // ── Stan menu ──────────────────────────────────────────────────
    int   menuSelectedItem_{0};   // podświetlony element (0=Start, 1=Wyniki, 2=Wyjdź)
    float menuScrollOffset_{0.f}; // monotoniczny licznik do auto-scrollu parallax w menu

    // ── Wyniki ─────────────────────────────────────────────────────
    std::vector<ScoreEntry> topScores_;          // max 10 wpisów, posortowane po czasie
    std::string scoresPath_{"scores.json"};      // ścieżka pliku (obok .exe)
    std::string scoresStatus_{""};               // komunikat diagnostyczny dla UI
};
