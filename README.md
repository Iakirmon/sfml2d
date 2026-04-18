# Platformer 2D (SFML, C++)

> **[English version below](#english-version)**

---

Gra platformowa 2D napisana w C++17 z użyciem biblioteki SFML 2.6.2.  
Gracz porusza się po platformach, zbiera monety, skacze (triple jump), a całość uzupełniają animacje sprite'ów, efekty dźwiękowe, muzyka w tle i wielowarstwowe tło parallax.  
Gra posiada menu główne, ekran wyników (Top 10) oraz zapis wyników do pliku JSON.

---

## Wymagania

- C++17
- CMake >= 3.20
- SFML 2.5+ (gałąź 2.x — **nie 3.x**)

---

## Instalacja SFML

### macOS (Homebrew)

```bash
brew install sfml
```

### Linux (Debian/Ubuntu)

```bash
sudo apt install libsfml-dev
```

### Windows (vcpkg)

```bash
git clone https://github.com/microsoft/vcpkg
cd vcpkg
./bootstrap-vcpkg.bat
./vcpkg install sfml:x64-windows
```

Przy wywołaniu CMake dodaj:

```bash
-DCMAKE_TOOLCHAIN_FILE=/ścieżka/do/vcpkg/scripts/buildsystems/vcpkg.cmake
```

---

## Budowanie projektu

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Plik wykonywalny: `Platformer` (Windows: `Platformer.exe`).  
Folder `assets/` jest kopiowany automatycznie do katalogu z binarką po każdym buildzie.

---

## Uruchomienie

```bash
cd build
./Platformer        # macOS / Linux
Platformer.exe      # Windows
```

---

## Sterowanie

### Menu główne

| Klawisz / Akcja | Opis |
|---|---|
| `↑` / `↓` | Poruszanie po opcjach |
| Ruch myszy | Podświetlenie opcji pod kursorem |
| `Enter` / LPM | Wybór opcji |

### W grze

| Klawisz | Akcja |
|---|---|
| `A` / `←` | Ruch w lewo |
| `D` / `→` | Ruch w prawo |
| `Spacja` | Skok (do 3× w powietrzu) |

### Po grze

| Klawisz | Akcja |
|---|---|
| `R` | Natychmiastowy restart (Game Over) |
| `Escape` | Powrót do menu (Game Over) |
| `Enter` | Zatwierdź nick po wygranej → powrót do menu |

---

## Mechanika gry

- Gra startuje w **menu głównym** z animowanym tłem parallax.
- Gracz ma **3 życia** — wypadnięcie poza dolną krawędź ekranu powoduje respawn i utratę życia.
- Brak żyć → ekran **GAME OVER** (R = restart, Escape = menu).
- **Monety** na mapie — zebrane znikają i zwiększają licznik.
- Zebranie wszystkich → ekran **YOU WIN** z formularzem na nick gracza.
- Wyniki (nick, czas, monety) zapisywane są do pliku `scores.json`.
- **Ekran Top 10** — wyniki posortowane rosnąco po czasie (szybciej = wyżej).

### Menu główne

| Opcja | Akcja |
|---|---|
| **Start** | Rozpoczyna grę od nowa |
| **Najlepsze wyniki** | Ekran Top 10 z `scores.json` |
| **Wyjdź** | Zamknięcie gry |

### HUD (lewy górny róg)

- `Coins: X / Y`
- `Lives: X`
- `Time: MM:SS`

---

## Wyniki (`scores.json`)

Plik tworzony automatycznie po wygranej — jeden wpis na linię:

```json
{"name":"Jan","time":"1:23","coins":5}
```

Ekran Top 10 wczytuje plik przy każdym otwarciu, sortuje po czasie i wyświetla maksymalnie 10 wyników.

---

## Assety

Gra nie crashuje przy braku plików — działa bez dźwięku / bez tekstury.

### Tekstury

| Ścieżka | Opis |
|---|---|
| `assets/textures/player_sheet.png` | Spritesheet gracza (9 kolumn × 3 wiersze, klatki 80×110 px) |
| `assets/textures/bg_clouds.png` | Warstwa parallax — chmury |
| `assets/textures/bg_mountains.png` | Warstwa parallax — góry |
| `assets/textures/bg_trees.png` | Warstwa parallax — drzewa |

### Dźwięki SFX

| Ścieżka | Zdarzenie |
|---|---|
| `assets/sounds/jump.ogg` | Skok gracza |
| `assets/sounds/coin.ogg` | Zebranie monety |
| `assets/sounds/death.ogg` | Utrata życia / respawn |
| `assets/sounds/gameover.ogg` | Game over |

### Muzyka

| Ścieżka | Kiedy gra |
|---|---|
| `assets/music/menu.ogg` | Menu główne (pętla) |
| `assets/music/gameplay.ogg` | Podczas rozgrywki (pętla) |
| `assets/music/gameover.ogg` | Ekran game over |
| `assets/music/win.ogg` | Ekran wygranej |

Pliki muzyczne dołączone w repo pochodzą z paczki **Kenney Music Jingles** (CC0).

### Czcionka

| Ścieżka | Opis |
|---|---|
| `assets/fonts/Roboto-Regular.ttf` | Czcionka HUD i UI (Google Fonts, OFL) |

---

## Struktura katalogów

```
.
├── CMakeLists.txt
├── README.md
├── assets/
│   ├── fonts/
│   │   └── Roboto-Regular.ttf
│   ├── music/
│   │   ├── menu.ogg
│   │   ├── gameplay.ogg
│   │   ├── gameover.ogg
│   │   └── win.ogg
│   ├── sounds/
│   │   ├── jump.ogg
│   │   ├── coin.ogg
│   │   ├── death.ogg
│   │   └── gameover.ogg
│   └── textures/
│       ├── player_sheet.png
│       ├── bg_clouds.png
│       ├── bg_mountains.png
│       └── bg_trees.png
├── files/
│   ├── spec_01_sprite_gracza.md
│   ├── spec_02_animacje.md
│   ├── spec_03_dzwieki_sfx.md
│   ├── spec_04_muzyka.md
│   ├── spec_05_parallax.md
│   └── spec_06_menu_glowne.md
└── src/
    ├── main.cpp
    ├── Game.h / Game.cpp
    ├── Player.h / Player.cpp
    ├── Platform.h / Platform.cpp
    ├── Coin.h / Coin.cpp
    ├── Level.h / Level.cpp
    ├── Animator.h / Animator.cpp
    ├── AudioManager.h / AudioManager.cpp
    └── ParallaxLayer.h / ParallaxLayer.cpp
```

---

## Architektura klas

| Klasa | Odpowiedzialność |
|---|---|
| `Game` | Pętla gry, stany (MENU/PLAYING/GAME_OVER/WINNING/SCORES), HUD, muzyka, parallax, wyniki |
| `Player` | Fizyka, input, animacje, sprite, lives |
| `Level` | Ładowanie platform i monet, detekcja kolizji |
| `Platform` | Prostokąt kolizyjny platformy |
| `Coin` | Moneta — pozycja, stan zebrania |
| `Animator` | Spritesheet — zarządzanie klatkami animacji |
| `AudioManager` | Ładowanie i odtwarzanie krótkich SFX |
| `ParallaxLayer` | Jedna warstwa tła z efektem paralaksy |

---

## Licencja fontu

Projekt używa czcionki **Roboto** (Google Fonts) — licencja Open Font License (OFL).

---
---

# English version

# Platformer 2D (SFML, C++)

A 2D platformer game written in C++17 using the SFML 2.x library.  
The player moves across platforms, collects coins, jumps (triple jump), with sprite animations, sound effects, background music and a multi-layer parallax background.  
The game includes a main menu, a Top 10 leaderboard screen and score saving to a JSON file.

---

## Requirements

- C++17
- CMake >= 3.20
- SFML 2.5+ (branch 2.x — **not 3.x**)

---

## Installing SFML

### macOS (Homebrew)

```bash
brew install sfml
```

### Linux (Debian/Ubuntu)

```bash
sudo apt install libsfml-dev
```

### Windows (vcpkg)

```bash
git clone https://github.com/microsoft/vcpkg
cd vcpkg
./bootstrap-vcpkg.bat
./vcpkg install sfml:x64-windows
```

When calling CMake, add:

```bash
-DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

---

## Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Executable: `Platformer` (Windows: `Platformer.exe`).  
The `assets/` folder is copied automatically to the binary directory after each build.

---

## Running

```bash
cd build
./Platformer        # macOS / Linux
Platformer.exe      # Windows
```

---

## Controls

### Main menu

| Key / Action | Description |
|---|---|
| `↑` / `↓` | Navigate menu options |
| Mouse move | Highlight option under cursor |
| `Enter` / LMB | Select option |

### In-game

| Key | Action |
|---|---|
| `A` / `←` | Move left |
| `D` / `→` | Move right |
| `Space` | Jump (up to 3× in the air) |

### After game

| Key | Action |
|---|---|
| `R` | Instant restart (Game Over) |
| `Escape` | Return to menu (Game Over) |
| `Enter` | Confirm name after winning → back to menu |

---

## Gameplay

- The game starts at the **main menu** with an animated parallax background.
- The player has **3 lives** — falling off the bottom of the screen causes a respawn and costs a life.
- No lives left → **GAME OVER** screen (R = restart, Escape = menu).
- **Coins** are placed on the map — collected coins disappear and increase the counter.
- Collecting all coins → **YOU WIN** screen with a name input form.
- Results (name, time, coins) are saved to `scores.json`.
- **Top 10 screen** — scores sorted ascending by time (faster = higher rank).

### Main menu

| Option | Action |
|---|---|
| **Start** | Begin a new game |
| **Best scores** | Top 10 screen from `scores.json` |
| **Exit** | Close the game |

### HUD (top-left corner)

- `Coins: X / Y`
- `Lives: X`
- `Time: MM:SS`

---

## Scores (`scores.json`)

Created automatically after winning — one entry per line:

```json
{"name":"Jan","time":"1:23","coins":5}
```

The Top 10 screen loads the file on every open, sorts by time and displays at most 10 entries.

---

## Assets

The game does not crash on missing files — it runs without sound / without texture.

### Textures

| Path | Description |
|---|---|
| `assets/textures/player_sheet.png` | Player spritesheet (9 cols × 3 rows, 80×110 px frames) |
| `assets/textures/bg_clouds.png` | Parallax layer — clouds |
| `assets/textures/bg_mountains.png` | Parallax layer — mountains |
| `assets/textures/bg_trees.png` | Parallax layer — trees |

### Sound effects

| Path | Event |
|---|---|
| `assets/sounds/jump.ogg` | Player jump |
| `assets/sounds/coin.ogg` | Coin collected |
| `assets/sounds/death.ogg` | Life lost / respawn |
| `assets/sounds/gameover.ogg` | Game over |

### Music

| Path | When it plays |
|---|---|
| `assets/music/menu.ogg` | Main menu (loop) |
| `assets/music/gameplay.ogg` | During gameplay (loop) |
| `assets/music/gameover.ogg` | Game over screen |
| `assets/music/win.ogg` | Win screen |

Music files included in the repo come from the **Kenney Music Jingles** pack (CC0).

### Font

| Path | Description |
|---|---|
| `assets/fonts/Roboto-Regular.ttf` | HUD and UI font (Google Fonts, OFL) |

---

## Directory structure

```
.
├── CMakeLists.txt
├── README.md
├── assets/
│   ├── fonts/
│   │   └── Roboto-Regular.ttf
│   ├── music/
│   │   ├── menu.ogg
│   │   ├── gameplay.ogg
│   │   ├── gameover.ogg
│   │   └── win.ogg
│   ├── sounds/
│   │   ├── jump.ogg
│   │   ├── coin.ogg
│   │   ├── death.ogg
│   │   └── gameover.ogg
│   └── textures/
│       ├── player_sheet.png
│       ├── bg_clouds.png
│       ├── bg_mountains.png
│       └── bg_trees.png
├── files/
│   ├── spec_01_sprite_gracza.md
│   ├── spec_02_animacje.md
│   ├── spec_03_dzwieki_sfx.md
│   ├── spec_04_muzyka.md
│   ├── spec_05_parallax.md
│   └── spec_06_menu_glowne.md
└── src/
    ├── main.cpp
    ├── Game.h / Game.cpp
    ├── Player.h / Player.cpp
    ├── Platform.h / Platform.cpp
    ├── Coin.h / Coin.cpp
    ├── Level.h / Level.cpp
    ├── Animator.h / Animator.cpp
    ├── AudioManager.h / AudioManager.cpp
    └── ParallaxLayer.h / ParallaxLayer.cpp
```

---

## Class architecture

| Class | Responsibility |
|---|---|
| `Game` | Game loop, states (MENU/PLAYING/GAME_OVER/WINNING/SCORES), HUD, music, parallax, scores |
| `Player` | Physics, input, animations, sprite, lives |
| `Level` | Loading platforms and coins, collision detection |
| `Platform` | Platform collision rectangle |
| `Coin` | Coin — position and collected state |
| `Animator` | Spritesheet — animation frame management |
| `AudioManager` | Loading and playing short SFX |
| `ParallaxLayer` | Single background layer with parallax effect |

---

## Font license

This project uses the **Roboto** font (Google Fonts) — Open Font License (OFL).
