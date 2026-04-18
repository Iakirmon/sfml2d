#include "Level.h"

#include <algorithm>
#include <cassert>

void Level::load(int levelIndex) {
    // Kontrakt: indeks musi mieścić się w zakresie [0, LEVEL_COUNT)
    assert(levelIndex >= 0 && levelIndex < LEVEL_COUNT && "levelIndex poza zakresem");

    platforms_.clear();
    coins_.clear();

    switch (levelIndex) {
    case 0: {
        // ── Podłoga (cały poziom 3840px) ────────────────────────────────
        // Jedna szeroka platforma na y=680, zajmuje cały poziom
        platforms_.emplace_back(0.f, 680.f, 3840.f, 40.f);

        // ── Sekcja 1: x = 0 – 1280 (oryginalna, łatwa) ──────────────────
        platforms_.emplace_back(100.f,  500.f, 200.f, 20.f);
        platforms_.emplace_back(350.f,  400.f, 150.f, 20.f);
        platforms_.emplace_back(550.f,  520.f, 200.f, 20.f);
        platforms_.emplace_back(800.f,  500.f, 180.f, 20.f);
        platforms_.emplace_back(1050.f, 520.f, 160.f, 20.f);

        coins_.emplace_back(200.f,  460.f);
        coins_.emplace_back(425.f,  360.f);
        coins_.emplace_back(650.f,  468.f);
        coins_.emplace_back(890.f,  360.f);
        coins_.emplace_back(1130.f, 460.f);

        // ── Sekcja 2: x = 1280 – 2100 (łatwa) ───────────────────────────
        platforms_.emplace_back(1300.f, 450.f, 200.f, 20.f);
        platforms_.emplace_back(1550.f, 360.f, 180.f, 20.f);
        platforms_.emplace_back(1780.f, 480.f, 160.f, 20.f);
        platforms_.emplace_back(2000.f, 380.f, 200.f, 20.f);

        coins_.emplace_back(1400.f, 410.f);
        coins_.emplace_back(1640.f, 320.f);
        coins_.emplace_back(1860.f, 440.f);
        coins_.emplace_back(2100.f, 340.f);

        // ── Sekcja 3: x = 2100 – 2950 (średnia) ─────────────────────────
        platforms_.emplace_back(2250.f, 500.f, 150.f, 20.f);
        platforms_.emplace_back(2450.f, 380.f, 130.f, 20.f);
        platforms_.emplace_back(2650.f, 460.f, 150.f, 20.f);
        platforms_.emplace_back(2850.f, 340.f, 120.f, 20.f);

        coins_.emplace_back(2325.f, 460.f);
        coins_.emplace_back(2515.f, 340.f);
        coins_.emplace_back(2725.f, 420.f);
        coins_.emplace_back(2910.f, 300.f);

        // ── Sekcja 4: x = 2950 – 3780 (trudna) ──────────────────────────
        platforms_.emplace_back(3050.f, 520.f, 200.f, 20.f);
        platforms_.emplace_back(3300.f, 400.f, 180.f, 20.f);
        platforms_.emplace_back(3500.f, 480.f, 160.f, 20.f);
        platforms_.emplace_back(3700.f, 360.f, 120.f, 20.f);

        coins_.emplace_back(3150.f, 480.f);
        coins_.emplace_back(3390.f, 360.f);
        coins_.emplace_back(3580.f, 440.f);
        coins_.emplace_back(3760.f, 320.f);
        break;
    }

    case 1: {
        // ── Poziom 2 (indeks 1): schodkowe ścieżki, szybsze tempo ───────
        // Podłoga identyczna jak w poziomie 0 (LEVEL_WIDTH = 3840)
        platforms_.emplace_back(0.f, 680.f, 3840.f, 40.f);

        // 16 platform pływających: x ∈ [150, 3730], x+w ≤ 3830 — mieszczą się w świecie
        platforms_.emplace_back(150.f,  540.f, 180.f, 20.f);
        platforms_.emplace_back(400.f,  440.f, 150.f, 20.f);
        platforms_.emplace_back(620.f,  520.f, 170.f, 20.f);
        platforms_.emplace_back(850.f,  430.f, 150.f, 20.f);
        platforms_.emplace_back(1080.f, 510.f, 160.f, 20.f);
        platforms_.emplace_back(1300.f, 400.f, 140.f, 20.f);
        platforms_.emplace_back(1520.f, 500.f, 160.f, 20.f);
        platforms_.emplace_back(1740.f, 380.f, 150.f, 20.f);
        platforms_.emplace_back(1960.f, 480.f, 170.f, 20.f);
        platforms_.emplace_back(2200.f, 360.f, 140.f, 20.f);
        platforms_.emplace_back(2430.f, 470.f, 160.f, 20.f);
        platforms_.emplace_back(2650.f, 340.f, 150.f, 20.f);
        platforms_.emplace_back(2870.f, 460.f, 160.f, 20.f);
        platforms_.emplace_back(3100.f, 380.f, 170.f, 20.f);
        platforms_.emplace_back(3350.f, 490.f, 150.f, 20.f);
        platforms_.emplace_back(3580.f, 360.f, 150.f, 20.f);

        // 15 monet — środek koła (r=10) z zapasem ≥10 od brzegów poziomu
        coins_.emplace_back(240.f,  500.f);
        coins_.emplace_back(475.f,  400.f);
        coins_.emplace_back(705.f,  480.f);
        coins_.emplace_back(925.f,  390.f);
        coins_.emplace_back(1160.f, 470.f);
        coins_.emplace_back(1370.f, 360.f);
        coins_.emplace_back(1600.f, 460.f);
        coins_.emplace_back(1815.f, 340.f);
        coins_.emplace_back(2045.f, 440.f);
        coins_.emplace_back(2270.f, 320.f);
        coins_.emplace_back(2510.f, 430.f);
        coins_.emplace_back(2725.f, 300.f);
        coins_.emplace_back(2950.f, 420.f);
        coins_.emplace_back(3185.f, 340.f);
        coins_.emplace_back(3425.f, 450.f);
        break;
    }

    case 2: {
        // ── Poziom 3 (indeks 2): ostatni, trudniejsze skoki ─────────────
        platforms_.emplace_back(0.f, 680.f, 3840.f, 40.f);

        // 17 platform pływających — mniejsze i bardziej rozstawione pionowo
        platforms_.emplace_back(200.f,  560.f, 140.f, 20.f);
        platforms_.emplace_back(420.f,  460.f, 130.f, 20.f);
        platforms_.emplace_back(620.f,  380.f, 140.f, 20.f);
        platforms_.emplace_back(820.f,  500.f, 130.f, 20.f);
        platforms_.emplace_back(1020.f, 400.f, 140.f, 20.f);
        platforms_.emplace_back(1220.f, 320.f, 130.f, 20.f);
        platforms_.emplace_back(1420.f, 440.f, 140.f, 20.f);
        platforms_.emplace_back(1620.f, 360.f, 130.f, 20.f);
        platforms_.emplace_back(1820.f, 480.f, 140.f, 20.f);
        platforms_.emplace_back(2020.f, 380.f, 130.f, 20.f);
        platforms_.emplace_back(2220.f, 460.f, 140.f, 20.f);
        platforms_.emplace_back(2420.f, 340.f, 130.f, 20.f);
        platforms_.emplace_back(2620.f, 440.f, 140.f, 20.f);
        platforms_.emplace_back(2820.f, 360.f, 130.f, 20.f);
        platforms_.emplace_back(3020.f, 480.f, 140.f, 20.f);
        platforms_.emplace_back(3250.f, 380.f, 140.f, 20.f);
        platforms_.emplace_back(3500.f, 460.f, 140.f, 20.f);

        // 16 monet
        coins_.emplace_back(270.f,  520.f);
        coins_.emplace_back(485.f,  420.f);
        coins_.emplace_back(690.f,  340.f);
        coins_.emplace_back(885.f,  460.f);
        coins_.emplace_back(1090.f, 360.f);
        coins_.emplace_back(1285.f, 280.f);
        coins_.emplace_back(1490.f, 400.f);
        coins_.emplace_back(1685.f, 320.f);
        coins_.emplace_back(1890.f, 440.f);
        coins_.emplace_back(2085.f, 340.f);
        coins_.emplace_back(2290.f, 420.f);
        coins_.emplace_back(2485.f, 300.f);
        coins_.emplace_back(2690.f, 400.f);
        coins_.emplace_back(2885.f, 320.f);
        coins_.emplace_back(3090.f, 440.f);
        coins_.emplace_back(3570.f, 420.f);
        break;
    }

    default:
        // Nieosiągalne — assert na górze już by zadziałał. Zostaw jako siatka bezpieczeństwa.
        assert(false && "nieobsługiwany levelIndex");
    }
}

void Level::checkCollisions(Player& player) {
    sf::FloatRect pBox = player.getHitbox(); // bieżący hitbox gracza

    // ── Kolizje z platformami (AABB) ────────────────────────────────
    for (auto& platform : platforms_) {
        sf::FloatRect platBox = platform.getHitbox();
        sf::FloatRect overlap; // prostokąt nachodzenia hitboxów

        if (!pBox.intersects(platBox, overlap)) continue; // brak nachodzenia → pomiń

        // Wyznacz kierunek wypychania na podstawie mniejszego wymiaru nachodzenia:
        //   overlap.width < overlap.height → gracz zahaczył bokiem (ściana)
        //   overlap.width ≥ overlap.height → gracz trafił od dołu/góry (podłoga/sufit)
        if (overlap.width < overlap.height) {
            // Kolizja boczna: wypchnij w osi X
            float pushX = (pBox.left < platBox.left) ? -overlap.width : overlap.width;
            player.moveShape({pushX, 0.f});
            player.setVelocityX(0.f); // zatrzymaj ruch poziomy
        } else {
            // Kolizja pionowa: wypchnij w osi Y
            float pushY = (pBox.top < platBox.top) ? -overlap.height : overlap.height;
            player.moveShape({0.f, pushY});
            player.setVelocityY(0.f); // zatrzymaj ruch pionowy
            if (pushY < 0.f) {
                // Wypychanie w górę → gracz ląduje na górnej powierzchni platformy
                player.setOnGround(true);
            }
        }

        // Pobierz zaktualizowany hitbox po wypychaniu — następna platforma
        // musi sprawdzać aktualną pozycję, nie pozycję sprzed wypchania
        pBox = player.getHitbox();
    }

    // ── Zbieranie monet ─────────────────────────────────────────────
    pBox = player.getHitbox(); // odśwież po kolizjach z platformami
    for (auto& coin : coins_) {
        // Coin::getHitbox() zwraca pusty rect gdy zebrana → intersects = false
        if (!coin.isCollected() && pBox.intersects(coin.getHitbox())) {
            coin.collect();
        }
    }
}

bool Level::allCoinsCollected() const {
    // std::all_of: zwraca true gdy KAŻDY element spełnia warunek
    return std::all_of(coins_.begin(), coins_.end(),
                       [](const Coin& c) { return c.isCollected(); });
}

void Level::draw(sf::RenderWindow& window) {
    for (auto& platform : platforms_) platform.draw(window);
    for (auto& coin     : coins_)     coin.draw(window);
}

std::size_t Level::getTotalCoins() const {
    return coins_.size();
}

std::size_t Level::getCollectedCoins() const {
    // std::count_if: liczy elementy spełniające warunek
    return static_cast<std::size_t>(std::count_if(
        coins_.begin(), coins_.end(),
        [](const Coin& c) { return c.isCollected(); }));
}
