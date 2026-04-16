# Specyfikacja projektu: Platformer 2D w C++ z SFML

## Cel projektu

Napisać grę platformową 2D w C++ używając biblioteki SFML. Gra ma klimat arcade – gracz porusza się po platformach i zbiera monety. Projekt jest zoptymalizowany pod osobę znającą podstawy C++, która chce nauczyć się grafiki z SFML.

---

## Stack technologiczny

- **Język**: C++17
- **Biblioteka graficzna**: SFML 2.5 lub nowszy (2.x) – **nie SFML 3.x**, API się różni
- **Build system**: CMake 3.20+
- **IDE**: dowolne (Cursor AI, VS Code, CLion)
- **System**: Windows / Linux / macOS

---

## Instalacja SFML

**Windows (zalecane przez vcpkg):**
```
git clone https://github.com/microsoft/vcpkg
cd vcpkg
bootstrap-vcpkg.bat          (lub bootstrap-vcpkg.exe na starszych systemach)
vcpkg install sfml:x64-windows
```
Następnie przy wywołaniu CMake dodaj flagę:
`-DCMAKE_TOOLCHAIN_FILE=[ścieżka_do_vcpkg]/scripts/buildsystems/vcpkg.cmake`

**Linux (Ubuntu/Debian):**
```
sudo apt install libsfml-dev
```

**macOS (Homebrew):**
```
brew install sfml
```

---

## Struktura projektu

```
platformer/
├── CMakeLists.txt
├── assets/
│   └── fonts/
│       └── Roboto-Regular.ttf    ← pobrać z fonts.google.com (darmowy, licencja OFL)
├── src/
│   ├── main.cpp
│   ├── Game.h / Game.cpp
│   ├── Player.h / Player.cpp
│   ├── Platform.h / Platform.cpp
│   ├── Coin.h / Coin.cpp
│   └── Level.h / Level.cpp
```

> Nie używaj Arial – jest licencjonowany przez Microsoft i nie istnieje domyślnie na Linuxie/macOS. Użyj Roboto lub innego fontu z Google Fonts.

---

## Parametry okna

```
rozdzielczość: 1280 x 720 px
tytuł: "Platformer"
framerate limit: 60 FPS  →  window.setFramerateLimit(60)
```

---

## Mechaniki gry (MVP)

### Gracz
- Ruch w lewo/prawo: A/D lub strzałki
- Skok: Spacja – tylko gdy `isOnGround == true`
- Spada pod wpływem grawitacji
- Stoi na platformach (kolizja AABB)
- Ma 3 życia
- Gdy pozycja Y gracza przekroczy `720 + 50` (zniknie za dolną krawędzią ekranu) → `respawn()`
- Gdy życia = 0 → stan `GAME_OVER`

### Monety
- Rozmieszczone ręcznie w `Level::load()` nad platformami
- Zbieranie przez dotknięcie: hitbox gracza ∩ hitbox monety
- Po zebraniu: `collected = true`, moneta przestaje być rysowana i wykrywana
- Licznik zebranych monet widoczny na HUD
- Cel: zebrać wszystkie monety → stan `WIN`
- Opcja rozszerzenia: moneta odradza się losowo nad jedną z platform po zebraniu (arcade)

### Platformy
- Statyczne prostokąty
- Pełna kolizja AABB ze wszystkich stron (nie jednostronna) – uproszczenie na MVP

### HUD
- Lewy górny róg, `sf::Text`, rozmiar 20px:
  - linia 1: `Coins: X / Y`
  - linia 2 (niżej o 30px): `Lives: X`

---

## Fizyka

```cpp
const float GRAVITY        = 1500.0f;   // px/s²
const float JUMP_VELOCITY  = -600.0f;   // px/s (ujemny = w górę w SFML)
const float MOVE_SPEED     = 250.0f;    // px/s
const float MAX_FALL_SPEED = 800.0f;    // px/s – bez tego gracz przebija platformy przy niskim FPS
```

Delta time musi być cappowany – bez tego fizyka eksploduje gdy okno jest zminimalizowane lub debugger zatrzyma program:

```cpp
// wymagany include: #include <algorithm>
float dt = clock.restart().asSeconds();
dt = std::min(dt, 0.05f);  // max 50ms (~20 FPS minimum)
```

---

## Architektura kodu

### Klasa `Game`

Pola:
- `sf::RenderWindow window`
- `sf::Font font` – **musi być polem klasy, nie zmienną lokalną!** (patrz: pułapka sf::Text poniżej)
- `GameState state` – `enum class GameState { PLAYING, GAME_OVER, WIN }`
- `Player player`
- `Level level`

Metody:
- `void run()` – pętla główna
- `void handleEvents()`
- `void update(float dt)`
- `void render()`

Kolejność w `update(dt)`:
```
player.handleInput()
player.update(dt)          ← zeruje isOnGround wewnętrznie
level.checkCollisions(player)  ← ustawia isOnGround gdy gracz stoi na platformie
sprawdź śmierć / win
```

> **Kolejność jest krytyczna.** `isOnGround` jest zerowane w `player.update()` i ustawiane z powrotem w `checkCollisions()`. Zamiana kolejności sprawi że gracz nigdy nie będzie mógł skoczyć.

### Klasa `Player`

Pola:
- `sf::RectangleShape shape` – rozmiar `{40.f, 50.f}`, kolor `sf::Color::Blue`
- `sf::Vector2f velocity`
- `bool isOnGround = false`
- `int lives = 3`
- `sf::Vector2f spawnPosition = {100.f, 300.f}`

Metody (wszystkie publiczne):
- `void handleInput()` – `sf::Keyboard::isKeyPressed`; ustawia `velocity.x`, inicjuje skok
- `void update(float dt)` – aplikuje grawitację, cappuje `velocity.y`, przesuwa `shape`, zeruje `isOnGround`
- `void draw(sf::RenderWindow& window)`
- `sf::FloatRect getHitbox()` – zwraca `shape.getGlobalBounds()`
- `void moveShape(sf::Vector2f offset)` – opakowuje `shape.move(offset)`; potrzebne dla `Level::checkCollisions`
- `void setVelocityX(float vx)`
- `void setVelocityY(float vy)`
- `void setOnGround(bool val)`
- `int getLives() const`
- `void respawn()` – `shape.setPosition(spawnPosition)`, zeruje `velocity`, `lives--`

> `Player` nie ma metody `resolveCollision` – cała logika kolizji jest w `Level::checkCollisions`. Nie mieszaj tych dwóch podejść.

### Klasa `Platform`

Pola:
- `sf::RectangleShape shape` – kolor `sf::Color(80, 180, 80)` (zielony)

Metody:
- `void draw(sf::RenderWindow& window)`
- `sf::FloatRect getHitbox()` – zwraca `shape.getGlobalBounds()`

Konstruktor: `Platform(float x, float y, float width, float height)`

### Klasa `Coin`

Pola:
- `sf::CircleShape shape` – promień `10.f`, kolor `sf::Color::Yellow`
- `bool collected = false`

Konstruktor `Coin(float x, float y)`:
```cpp
shape.setRadius(10.f);
shape.setFillColor(sf::Color::Yellow);
shape.setOrigin(10.f, 10.f);   // ← konieczne! origin domyślnie w lewym górnym rogu
shape.setPosition(x, y);
```

Metody:
- `void draw(sf::RenderWindow& window)` – rysuje tylko gdy `!collected`
- `sf::FloatRect getHitbox()` – zwraca `shape.getGlobalBounds()` (tylko gdy `!collected`)
- `bool isCollected() const` – zwraca `collected`
- `void collect()` – ustawia `collected = true`

### Klasa `Level`

Pola:
- `std::vector<Platform> platforms`
- `std::vector<Coin> coins`

Metody:
- `void load()` – wypełnia `platforms` i `coins`
- `void checkCollisions(Player& player)`
- `bool allCoinsCollected()` – `std::all_of` po `coins`
- `void draw(sf::RenderWindow& window)`

---

## Kolizja AABB – pełna implementacja

```cpp
// wymagane includes: #include <algorithm>
void Level::checkCollisions(Player& player) {
    sf::FloatRect pBox = player.getHitbox();

    for (auto& platform : platforms) {
        sf::FloatRect platBox = platform.getHitbox();
        sf::FloatRect overlap;

        if (!pBox.intersects(platBox, overlap)) continue;

        if (overlap.width < overlap.height) {
            // Kolizja pozioma – odpychamy w osi X
            float pushX = (pBox.left < platBox.left) ? -overlap.width : overlap.width;
            player.moveShape({pushX, 0.f});
            player.setVelocityX(0.f);
        } else {
            // Kolizja pionowa – odpychamy w osi Y
            float pushY = (pBox.top < platBox.top) ? -overlap.height : overlap.height;
            player.moveShape({0.f, pushY});
            player.setVelocityY(0.f);
            if (pushY < 0.f) {
                player.setOnGround(true);   // gracz ląduje na górze platformy
            }
            // pushY > 0 → gracz uderzył głową w spód platformy, velocity.y = 0 wystarczy
        }

        pBox = player.getHitbox(); // odśwież po każdej platformie
    }

    // Zbieranie monet
    pBox = player.getHitbox(); // odśwież raz jeszcze po wszystkich platformach
    for (auto& coin : coins) {
        if (!coin.isCollected() && pBox.intersects(coin.getHitbox())) {
            coin.collect();
        }
    }
}
```

---

## Game loop (pełna logika)

```cpp
// wymagane includes: #include <algorithm>
sf::Clock clock;
while (window.isOpen()) {
    float dt = clock.restart().asSeconds();
    dt = std::min(dt, 0.05f);

    handleEvents();  // obsługa sf::Event, w tym sf::Event::Closed i restart przez R

    if (state == GameState::PLAYING) {
        player.handleInput();
        player.update(dt);              // zeruje isOnGround
        level.checkCollisions(player);  // ustawia isOnGround

        // Wykrycie śmierci (gracz zniknął za dolną krawędzią)
        if (player.getHitbox().top > 720.f + 50.f) {
            player.respawn();  // respawn odejmuje życie wewnętrznie
        }

        if (player.getLives() <= 0)
            state = GameState::GAME_OVER;
        if (level.allCoinsCollected())
            state = GameState::WIN;
    }

    window.clear(sf::Color(30, 30, 30));
    level.draw(window);
    player.draw(window);
    drawHUD(window);         // rysuje sf::Text z wynikiem i życiami
    if (state == GameState::GAME_OVER) drawGameOver(window);
    if (state == GameState::WIN)       drawWin(window);
    window.display();
}
```

---

## Pułapka: sf::Text i sf::Font

`sf::Text` trzyma **wskaźnik** do `sf::Font`, nie kopię. Jeśli `sf::Font` zostanie zniszczony (np. był zmienną lokalną), tekst wyświetli się jako czarne kwadraty zamiast liter – i nie dostaniesz żadnego błędu.

**Rozwiązanie:** `sf::Font font` musi być **polem klasy `Game`**, nie zmienną lokalną żadnej metody. Przekazuj go do `sf::Text` przez `text.setFont(font)` gdzie `font` żyje przez cały czas działania programu.

```cpp
// ŹLE – font ginie po wyjściu z funkcji
void Game::render() {
    sf::Font font;
    font.loadFromFile("assets/fonts/Roboto-Regular.ttf");
    sf::Text text("Lives: 3", font, 20);
    window.draw(text);
}

// DOBRZE – font jako pole klasy
class Game {
    sf::Font font;   // ← żyje przez cały czas działania Game
    // ...
};
```

---

## Przykładowa mapa (Level::load)

```cpp
void Level::load() {
    platforms.clear();
    coins.clear();

    // Podłoga (pełna szerokość)
    platforms.emplace_back(0.f,    680.f, 1280.f, 40.f);

    // Platformy pośrednie – gracz zaczyna na y=300, spada na podłogę lub platformy
    platforms.emplace_back(100.f,  500.f,  200.f, 20.f);
    platforms.emplace_back(350.f,  400.f,  150.f, 20.f);
    platforms.emplace_back(550.f,  520.f,  200.f, 20.f);
    platforms.emplace_back(800.f,  350.f,  180.f, 20.f);
    platforms.emplace_back(1050.f, 470.f,  160.f, 20.f);

    // Monety – ustawione 40px nad górną krawędzią każdej platformy
    // platforma y=500 → moneta y=500-40=460, środek na x=200 (środek platformy 100+100)
    coins.emplace_back(200.f,  460.f);
    coins.emplace_back(425.f,  360.f);
    coins.emplace_back(650.f,  480.f);
    coins.emplace_back(890.f,  310.f);
    coins.emplace_back(1130.f, 430.f);
}
```

> Gracz startuje na pozycji `{100.f, 300.f}` i spada swobodnie aż do pierwszej platformy lub podłogi – to zamierzone zachowanie.

---

## Plan implementacji (kolejność)

1. **Krok 1** – Okno SFML 1280x720, minimalna pętla z `sf::Clock`, cappowany dt, zamykanie przez X
2. **Krok 2** – Klasa `Player`: prostokąt, ruch lewo/prawo przez `handleInput`
3. **Krok 3** – Grawitacja i skok w `Player::update`, cap prędkości, flaga `isOnGround`
4. **Krok 4** – Klasa `Platform`, jedna testowa platforma, kolizja AABB w `Level::checkCollisions`
5. **Krok 5** – Klasa `Level::load()` z pełną mapą, wszystkie platformy
6. **Krok 6** – Klasa `Coin` z `setOrigin`, zbieranie w `checkCollisions`, licznik
7. **Krok 7** – HUD (`sf::Text`, font jako pole klasy `Game`)
8. **Krok 8** – Śmierć przez wypadnięcie, respawn, `GAME_OVER`, `WIN`, restart przez R
9. **Krok 9** – Dopracowanie: kolory, ekran startowy, drobne efekty

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(Platformer)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2 COMPONENTS graphics window system REQUIRED)

add_executable(Platformer
    src/main.cpp
    src/Game.cpp
    src/Player.cpp
    src/Platform.cpp
    src/Coin.cpp
    src/Level.cpp
)

target_link_libraries(Platformer sfml-graphics sfml-window sfml-system)

# Kopiuj assets do katalogu build przy każdym buildzie
add_custom_command(TARGET Platformer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Platformer>/assets
)
```

> Poprzednie `file(COPY ...)` kopiowało assets tylko przy `cmake ..`, nie przy `cmake --build`. Ta wersja z `add_custom_command` działa przy każdym buildzie.

---

## Wskazówki dla Cursor AI

Wklej tę specyfikację jako kontekst (`@specyfikacja_gra_platformer.md` w Cursor) i używaj poniższych promptów **po kolei, jeden krok na raz**. Nie dawaj Cursorowi wszystkiego naraz.

**Krok 1–2:**
> "Create a Game class with an SFML 1280x720 window, framerate limit 60, and a game loop using sf::Clock with dt capped at 0.05f (include `<algorithm>`). Create a Player class with a blue sf::RectangleShape (40x50 px), spawn at {100, 300}. handleInput() reads A/D keys and moves at 250 px/s by setting velocity.x. update(dt) moves shape by velocity*dt. Follow the spec."

**Krok 3:**
> "Add gravity and jumping to Player. In update(dt): add GRAVITY(1500)*dt to velocity.y, cap it at MAX_FALL_SPEED(800), then call shape.move(velocity * dt), then set isOnGround=false. Add jump: if Space pressed and isOnGround==true, set velocity.y = -600. Use delta time throughout."

**Krok 4–5:**
> "Create a Platform class (sf::RectangleShape, green color, constructor takes x,y,width,height, has getHitbox() returning getGlobalBounds()). Create a Level class with vector<Platform>. Implement Level::checkCollisions(Player&) using AABB overlap: for each platform, call pBox.intersects(platBox, overlap), resolve by pushing player via player.moveShape() based on smaller overlap axis. Player needs public methods: moveShape(sf::Vector2f), setVelocityX, setVelocityY, setOnGround. Use the map from the spec."

**Krok 6–7:**
> "Create a Coin class: sf::CircleShape radius 10, yellow, setOrigin(10,10) in constructor so hitbox matches visual. Fields: bool collected. Methods: draw() only if !collected, getHitbox(), isCollected(), collect(). Add coins to Level. In checkCollisions, call coin.collect() on intersection. Add sf::Font as a field of Game class (not local variable!), load Roboto-Regular.ttf. Draw HUD with sf::Text: 'Coins: X/Y' and 'Lives: X' at top-left."

**Krok 8:**
> "In Game::update: after checkCollisions, if player.getHitbox().top > 770, call player.respawn() which decrements lives and resets position to {100,300} and zeroes velocity. If getLives()<=0 set state=GAME_OVER. If level.allCoinsCollected() set state=WIN. In GAME_OVER/WIN states draw centered sf::Text. In handleEvents, if R pressed and state!=PLAYING: reset player lives to 3, call level.load(), set state=PLAYING."

---

## Rozszerzenia po MVP (opcjonalne)

- Animowane sprite'y gracza (klasa `Animation` z `sf::Sprite` i arkuszem tekstur)
- Poruszające się platformy (patrol między dwoma punktami, `sf::Vector2f patrolA, patrolB`)
- Wróg poruszający się wahadłowo – kolizja z graczem = `respawn()`
- Wiele poziomów – `Level::load(int levelIndex)` z różnymi mapami
- Zapis highscore do pliku (`std::ofstream`, `std::ifstream`)
- Efekty dźwiękowe (`sf::SoundBuffer`, `sf::Sound`)
- Tło paralaksy – kilka `sf::Sprite` przesuwanych z różną prędkością w zależności od pozycji gracza
