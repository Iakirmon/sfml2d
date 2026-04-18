#include "Player.h"
#include "Animator.h"

#include <algorithm>
#include <cmath>

// Stałe fizyki — w anonimowym namespace (widoczne tylko w tym pliku)
namespace {
constexpr float GRAVITY        = 1500.0f;  // przyspieszenie grawitacyjne [px/s²]
constexpr float JUMP_VELOCITY  = -600.0f;  // prędkość na starcie skoku (ujemna = góra)
constexpr float MOVE_SPEED     = 250.0f;   // prędkość pozioma [px/s]
constexpr float MAX_FALL_SPEED = 800.0f;   // ograniczenie prędkości opadania [px/s]
}

Player::Player() {
    // Rozmiar JEDNEJ klatki w spritesheecie Kenney Player Pack (720×330, siatka 9×3)
    constexpr int W = 80, H = 110;

    // Spróbuj załadować spritesheet; fallback do pojedynczego pliku
    if (!texture_.loadFromFile("assets/textures/player_sheet.png")) {
        texture_.loadFromFile("assets/textures/player.png");
    }

    sprite_.setTexture(texture_);

    // texSize_ to rozmiar klatki (W×H), NIE całego spritesheeta.
    // Używamy do obliczania skali i punktu origin przy flipowaniu.
    texSize_ = sf::Vector2u(W, H);

    // Skaluj sprite do rozmiaru 40×50px na ekranie (niezależnie od rozmiaru klatki)
    sprite_.setScale(
        40.f / static_cast<float>(W),
        50.f / static_cast<float>(H)
    );
    sprite_.setPosition(spawnPosition_);

    // Układ spritesheeta Kenney (alfabetyczny, 9 kolumn × 3 rzędy po 80×110 px):
    // Rząd 0 (y=0):   action1 action2 back cheer1 cheer2 climb1 climb2 duck fall
    // Rząd 1 (y=110): hang hold1 hold2 hurt idle jump kick skid slide
    // Rząd 2 (y=220): stand swim1 swim2 talk walk1 walk2 (3×puste)
    //
    // sf::IntRect(x, y, szerokość, wysokość) — wycina fragment z tekstury

    // "idle" — kol 4 (x=320), rząd 1 (y=110): stanie w miejscu
    Animation idle;
    idle.frameDuration = 0.5f;
    idle.loop = true;
    idle.frames.push_back({4 * W, 1 * H, W, H});
    animator_.addAnimation("idle", idle);

    // "run" — walk1 (kol 4, rząd 2) + walk2 (kol 5, rząd 2): chód/bieg
    Animation run;
    run.frameDuration = 0.15f;
    run.loop = true;
    run.frames.push_back({4 * W, 2 * H, W, H});
    run.frames.push_back({5 * W, 2 * H, W, H});
    animator_.addAnimation("run", run);

    // "jump" — kol 5 (x=400), rząd 1 (y=110): wznoszenie
    Animation jump;
    jump.frameDuration = 0.15f;
    jump.loop = false; // nie pętluj — zostań na tej klatce aż zmieni się stan
    jump.frames.push_back({5 * W, 1 * H, W, H});
    animator_.addAnimation("jump", jump);

    // "fall" — kol 8 (x=640), rząd 0 (y=0): opadanie
    Animation fall;
    fall.frameDuration = 0.2f;
    fall.loop = true;
    fall.frames.push_back({8 * W, 0 * H, W, H});
    animator_.addAnimation("fall", fall);

    animator_.play("idle");
}

void Player::handleInput() {
    justJumped_ = false; // flaga jednoklatkowa — reset na początku każdej klatki

    // Prędkość pozioma: bezpośrednio z klawiatury (brak przyspieszania/tarcia)
    float vx = 0.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        vx -= MOVE_SPEED;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        vx += MOVE_SPEED;
    }

    // Edge detection: skok tylko na zboczu narastającym (pressed, nie held)
    // spaceWasPressed_ zapamiętuje stan z poprzedniej klatki
    bool spacePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
    if (spacePressed && !spaceWasPressed_) {
        if (isOnGround_) {
            // Skok 1: z ziemi
            velocity_.y     = JUMP_VELOCITY;
            isOnGround_     = false;
            canDoubleJump_  = true;  // odblokuj skok 2
            justJumped_     = true;
        } else if (canDoubleJump_) {
            // Skok 2: w powietrzu
            velocity_.y     = JUMP_VELOCITY;
            canDoubleJump_  = false;
            canTripleJump_  = true;  // odblokuj skok 3
            justJumped_     = true;
        } else if (canTripleJump_) {
            // Skok 3: ostatnia szansa w powietrzu
            velocity_.y     = JUMP_VELOCITY;
            canTripleJump_  = false;
            justJumped_     = true;
        }
    }
    spaceWasPressed_ = spacePressed; // zapamiętaj stan na następną klatkę

    velocity_.x = vx;
}

void Player::update(float dt) {
    // Wybór animacji na podstawie stanu fizyki
    if (!isOnGround_) {
        // W powietrzu: jump gdy wznosi się (velocity_.y < 0), fall gdy opada
        animator_.play(velocity_.y < 0.f ? "jump" : "fall");
    } else if (std::abs(velocity_.x) > 1.f) {
        animator_.play("run");  // porusza się poziomo
    } else {
        animator_.play("idle"); // stoi w miejscu
    }

    // Flip sprite w zależności od kierunku patrzenia
    // Ujemna skala X odbija sprite — origin musi być na prawej krawędzi klatki
    float scaleX = 40.f / static_cast<float>(texSize_.x);
    float scaleY = 50.f / static_cast<float>(texSize_.y);

    if (velocity_.x < 0.f) facingRight_ = false;
    if (velocity_.x > 0.f) facingRight_ = true;

    if (facingRight_) {
        sprite_.setScale(scaleX, scaleY);
        sprite_.setOrigin(0.f, 0.f);                        // origin lewy-górny
    } else {
        sprite_.setScale(-scaleX, scaleY);                  // ujemna skala = odbicie
        sprite_.setOrigin(static_cast<float>(texSize_.x), 0.f); // origin prawy-górny
    }

    // Pobierz prostokąt bieżącej klatki od Animatora i ustaw na sprite
    sprite_.setTextureRect(animator_.update(dt));

    // Grawitacja: co klatkę zwiększa prędkość opadania (V = V0 + G*t)
    velocity_.y += GRAVITY * dt;
    // Ogranicz prędkość opadania (terminal velocity) — bez tego gracz
    // mógłby przebić się przez cienkie platformy przy niskich FPS
    velocity_.y = std::min(velocity_.y, MAX_FALL_SPEED);

    // Przesuń sprite o przebytą drogę (droga = prędkość × czas)
    sprite_.move(velocity_ * dt);

    // Zeruj flagę isOnGround_ co klatkę — Level::checkCollisions() ustawi ją
    // z powrotem jeśli gracz stoi na platformie. Dzięki temu gdy gracz
    // zejdzie z krawędzi platformy, isOnGround_ automatycznie staje się false.
    isOnGround_ = false;
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(sprite_);
}

// getGlobalBounds() = hitbox uwzględniający aktualną pozycję i skalę sprite'a
sf::FloatRect Player::getHitbox() const {
    return sprite_.getGlobalBounds();
}

// Przesuwa sprite (np. wypychanie z kolizji w Level::checkCollisions())
void Player::moveShape(sf::Vector2f offset) {
    sprite_.move(offset);
}

void Player::setVelocityX(float vx) { velocity_.x = vx; }
void Player::setVelocityY(float vy) { velocity_.y = vy; }

void Player::setOnGround(bool val) {
    isOnGround_ = val;
    if (val) {
        // Wylądowanie na ziemi: resetuj dostępność skoków
        canDoubleJump_ = false;
        // Uwaga: canTripleJump_ pozostaje false (resetowany był przy skoku 2)
    }
}

int Player::getLives() const { return lives_; }

void Player::respawn() {
    // Wróć na punkt startowy, wyzeruj prędkość, odejmij życie
    sprite_.setPosition(spawnPosition_);
    velocity_ = {0.f, 0.f};
    --lives_;
}

void Player::resetState() {
    // Pełny reset na początek nowej gry — przywróć wszystkie wartości domyślne
    sprite_.setPosition(spawnPosition_);
    velocity_        = {0.f, 0.f};
    lives_           = 3;
    isOnGround_      = false;
    canDoubleJump_   = false;
    canTripleJump_   = false;
    spaceWasPressed_ = false;
    justJumped_      = false;
    facingRight_     = true;
    sprite_.setOrigin(0.f, 0.f); // przywróć origin po możliwym flipie
    animator_.play("idle");
}
