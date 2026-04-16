# Platformer 2D (SFML, C++)

Prosty projekt gry platformowej 2D napisany w C++17 z użyciem biblioteki SFML 2.x.  
Gracz porusza się po platformach, zbiera monety i ma ograniczoną liczbę żyć.

## Wymagania

- C++17
- CMake >= 3.20
- SFML 2.5 (lub inna wersja z gałęzi 2.x – **nie 3.x**)

## Instalacja SFML (skrót)

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
./bootstrap-vcpkg.sh        # lub bootstrap-vcpkg.bat
./vcpkg install sfml:x64-windows
```

Przy wywołaniu CMake dodaj:

```bash
-DCMAKE_TOOLCHAIN_FILE=/ścieżka/do/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Budowanie projektu

W katalogu projektu:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Plik wykonywalny: `Platformer` (na Windows `Platformer.exe`).

Assets (font, itp.) są kopiowane automatycznie do katalogu z binarką (`assets/...`).

## Uruchomienie

Z katalogu `build`:

```bash
./Platformer   # lub Platformer.exe na Windows
```

## Sterowanie

- Ruch w lewo/prawo: `A` / `D` lub strzałki
- Skok: `Spacja` (tylko gdy gracz stoi na ziemi)
- Restart po wygranej/przegranej: `R`

## Mechanika gry (MVP)

- Gracz ma 3 życia.
- Gdy wypadnie poza dolną krawędź ekranu, traci życie i respawnuje na pozycji startowej.
- Gdy życia spadną do 0 → ekran `GAME OVER`.
- Na mapie rozmieszczone są monety – zebrane znikają i zwiększają licznik.
- Po zebraniu wszystkich monet → ekran `YOU WIN`.

HUD w lewym górnym rogu pokazuje:

- `Coins: X / Y`
- `Lives: X`

## Struktura katalogów

```text
.
├── CMakeLists.txt
├── assets/
│   └── fonts/
│       └── Roboto-Regular.ttf
├── src/
│   ├── main.cpp
│   ├── Game.h / Game.cpp
│   ├── Player.h / Player.cpp
│   ├── Platform.h / Platform.cpp
│   ├── Coin.h / Coin.cpp
│   └── Level.h / Level.cpp
└── specyfikacja_gra_platformer.md   # pełna specyfikacja gry (PL)
```

## Licencja fontu

Projekt używa fontu **Roboto** (Google Fonts), dołączony jest plik `OFL.txt` z licencją Open Font License.

