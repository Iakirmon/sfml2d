# Platformer 2D (SFML, C++)

> **[English version below](#english-version)**

---

Gra platformowa 2D napisana w C++17 z użyciem biblioteki SFML 2.x.  
Gracz porusza się po platformach, zbiera monety, skacze (triple jump), a całość uzupełniają animacje sprite'ów, efekty dźwiękowe, muzyka w tle i wielowarstwowe tło parallax.

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

| Klawisz | Akcja |
|---|---|
| `A` / `←` | Ruch w lewo |
| `D` / `→` | Ruch w prawo |
| `Spacja` | Skok (do 3× w powietrzu) |
| `R` | Restart po game over |
| `Enter` | Zatwierdź nick po wygranej |

---

## Mechanika gry

- Gracz ma **3 życia** — wypadnięcie poza dolną krawędź ekranu powoduje respawn i utratę życia.
- Brak żyć → ekran **GAME OVER**.
- **Monety** na mapie — zebrane znikają i zwiększają licznik.
- Zebranie wszystkich → ekran **YOU WIN** z formularzem na nick gracza.
- Wyniki (nick, czas, monety) zapisywane są do pliku `scores.json`.

### HUD (lewy górny róg)

- `Coins: X / Y`
- `Lives: X`
- `Time: MM:SS`

---

## Assety (wymagane do pełnego działania)

Gra nie crashuje przy braku plików — wyświetla placeholder lub gra w ciszy.  
Poniższe pliki należy umieścić we wskazanych ścieżkach przed buildem:

### Tekstury

| Ścieżka | Opis |
|---|---|
| `assets/textures/player_sheet.png` | Spritesheet gracza — klatki **32×32 px**, min. 4 wiersze: idle, run, jump, fall |
| `assets/textures/bg_clouds.png` | Warstwa parallax — chmury (tileable, PNG z alpha) |
| `assets/textures/bg_mountains.png` | Warstwa parallax — góry |
| `assets/textures/bg_trees.png` | Warstwa parallax — drzewa |

**Polecane darmowe źródła:**
- [kenney.nl/assets/platformer-characters-1](https://kenney.nl/assets/platformer-characters-1) — spritesheet gracza (CC0)
- [kenney.nl/assets/background-elements](https://kenney.nl/assets/background-elements) — elementy tła (CC0)
- [craftpix.net/freebies](https://craftpix.net/freebies/) — paczki tła platformerów

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
| `assets/music/gameplay.ogg` | Podczas rozgrywki (pętla) |
| `assets/music/gameover.ogg` | Ekran game over |
| `assets/music/win.ogg` | Ekran wygranej |

**Polecane darmowe źródła audio:**
- [kenney.nl/assets/interface-sounds](https://kenney.nl/assets/interface-sounds) — SFX (CC0)
- [opengameart.org/content/platformer-game-music](https://opengameart.org/content/platformer-game-music) — muzyka (CC-BY 3.0)
- [freesound.org](https://freesound.org) — filtrem CC0

### Czcionka

| Ścieżka | Opis |
|---|---|
| `assets/fonts/Roboto-Regular.ttf` | Czcionka HUD (Google Fonts, OFL) |

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
│   └── spec_05_parallax.md
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
| `Game` | Pętla gry, stany, HUD, muzyka, parallax |
| `Player` | Fizyka, input, animacje, sprite, lives |
| `Level` | Ładowanie platform i monet, detekcja kolizji |
| `Platform` | Prostokąt kolizyjny platformy |
| `Coin` | Moneta — pozycja, stan zebrania |
| `Animator` | Spritesheet — zarządzanie klatkami animacji |
| `AudioManager` | Ładowanie i odtwarzanie krótkich SFX |
| `ParallaxLayer` | Jedna warstwa tła z efektem paralaksy |

---

## Licencja fontu

Projekt używa czcionki **Roboto** (Google Fonts) — licencja Open Font License (OFL), plik `assets/fonts/OFL.txt`.

---

---

# English version

# Platformer 2D (SFML, C++)

A 2D platformer game written in C++17 using the SFML 2.x library.  
The player moves across platforms, collects coins, jumps (triple jump), with sprite animations, sound effects, background music and a multi-layer parallax background.

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

| Key | Action |
|---|---|
| `A` / `←` | Move left |
| `D` / `→` | Move right |
| `Space` | Jump (up to 3× in the air) |
| `R` | Restart after game over |
| `Enter` | Confirm name after winning |

---

## Gameplay

- The player has **3 lives** — falling off the bottom of the screen causes a respawn and costs a life.
- No lives left → **GAME OVER** screen.
- **Coins** are placed on the map — collected coins disappear and increase the counter.
- Collecting all coins → **YOU WIN** screen with a name input form.
- Results (name, time, coins) are saved to `scores.json`.

### HUD (top-left corner)

- `Coins: X / Y`
- `Lives: X`
- `Time: MM:SS`

---

## Assets (required for full functionality)

The game does not crash on missing files — it shows a placeholder or runs silently.  
Place the files below at the indicated paths before building:

### Textures

| Path | Description |
|---|---|
| `assets/textures/player_sheet.png` | Player spritesheet — **32×32 px** frames, min. 4 rows: idle, run, jump, fall |
| `assets/textures/bg_clouds.png` | Parallax layer — clouds (tileable PNG with alpha) |
| `assets/textures/bg_mountains.png` | Parallax layer — mountains |
| `assets/textures/bg_trees.png` | Parallax layer — trees |

**Recommended free sources:**
- [kenney.nl/assets/platformer-characters-1](https://kenney.nl/assets/platformer-characters-1) — player spritesheet (CC0)
- [kenney.nl/assets/background-elements](https://kenney.nl/assets/background-elements) — background elements (CC0)
- [craftpix.net/freebies](https://craftpix.net/freebies/) — platformer background packs

### Sound Effects

| Path | Event |
|---|---|
| `assets/sounds/jump.ogg` | Player jump |
| `assets/sounds/coin.ogg` | Coin collected |
| `assets/sounds/death.ogg` | Life lost / respawn |
| `assets/sounds/gameover.ogg` | Game over |

### Music

| Path | When it plays |
|---|---|
| `assets/music/gameplay.ogg` | During gameplay (loop) |
| `assets/music/gameover.ogg` | Game over screen |
| `assets/music/win.ogg` | Win screen |

**Recommended free audio sources:**
- [kenney.nl/assets/interface-sounds](https://kenney.nl/assets/interface-sounds) — SFX (CC0)
- [opengameart.org/content/platformer-game-music](https://opengameart.org/content/platformer-game-music) — music (CC-BY 3.0)
- [freesound.org](https://freesound.org) — filter by CC0

### Font

| Path | Description |
|---|---|
| `assets/fonts/Roboto-Regular.ttf` | HUD font (Google Fonts, OFL) |

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
│   └── spec_05_parallax.md
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
| `Game` | Game loop, states, HUD, music, parallax |
| `Player` | Physics, input, animations, sprite, lives |
| `Level` | Loading platforms and coins, collision detection |
| `Platform` | Platform collision rectangle |
| `Coin` | Coin — position and collected state |
| `Animator` | Spritesheet — animation frame management |
| `AudioManager` | Loading and playing short SFX |
| `ParallaxLayer` | Single background layer with parallax effect |

---

## Font license

This project uses the **Roboto** font (Google Fonts) — Open Font License (OFL), see `assets/fonts/OFL.txt`.
