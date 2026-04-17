#include "Player.h"
#include "Animator.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float GRAVITY        = 1500.0f;
constexpr float JUMP_VELOCITY  = -600.0f;
constexpr float MOVE_SPEED     = 250.0f;
constexpr float MAX_FALL_SPEED = 800.0f;
}

Player::Player() {
    // Rozmiar jednej klatki w tileshecie Kenney Player (720×330, siatka 9×3)
    constexpr int W = 80, H = 110;

    if (!texture_.loadFromFile("assets/textures/player_sheet.png")) {
        texture_.loadFromFile("assets/textures/player.png");
    }

    sprite_.setTexture(texture_);

    // texSize_ przechowuje rozmiar JEDNEJ klatki – używamy do skalowania i flipowania
    texSize_ = sf::Vector2u(W, H);

    sprite_.setScale(
        40.f / static_cast<float>(W),
        50.f / static_cast<float>(H)
    );
    sprite_.setPosition(spawnPosition_);

    // Układ tileshetu Kenney (alfabetyczny, 9 kolumn × 3 rzędy po 80×110 px):
    // Rząd 0: action1 action2 back cheer1 cheer2 climb1 climb2 duck fall
    // Rząd 1: hang hold1 hold2 hurt idle jump kick skid slide
    // Rząd 2: stand swim1 swim2 talk walk1 walk2 (3×puste)

    // idle – kol 4, rząd 1
    Animation idle;
    idle.frameDuration = 0.5f;
    idle.loop = true;
    idle.frames.push_back({4 * W, 1 * H, W, H});
    animator_.addAnimation("idle", idle);

    // run – walk1 (kol 4, rząd 2) + walk2 (kol 5, rząd 2)
    Animation run;
    run.frameDuration = 0.15f;
    run.loop = true;
    run.frames.push_back({4 * W, 2 * H, W, H});
    run.frames.push_back({5 * W, 2 * H, W, H});
    animator_.addAnimation("run", run);

    // jump – kol 5, rząd 1
    Animation jump;
    jump.frameDuration = 0.15f;
    jump.loop = false;
    jump.frames.push_back({5 * W, 1 * H, W, H});
    animator_.addAnimation("jump", jump);

    // fall – kol 8, rząd 0
    Animation fall;
    fall.frameDuration = 0.2f;
    fall.loop = true;
    fall.frames.push_back({8 * W, 0 * H, W, H});
    animator_.addAnimation("fall", fall);

    animator_.play("idle");
}

void Player::handleInput() {
    justJumped_ = false;

    float vx = 0.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        vx -= MOVE_SPEED;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        vx += MOVE_SPEED;
    }

    bool spacePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
    if (spacePressed && !spaceWasPressed_) {
        if (isOnGround_) {
            velocity_.y = JUMP_VELOCITY;
            isOnGround_ = false;
            canDoubleJump_ = true;
            justJumped_ = true;
        } else if (canDoubleJump_) {
            velocity_.y = JUMP_VELOCITY;
            canDoubleJump_ = false;
            canTripleJump_ = true;
            justJumped_ = true;
        } else if (canTripleJump_) {
            velocity_.y = JUMP_VELOCITY;
            canTripleJump_ = false;
            justJumped_ = true;
        }
    }
    spaceWasPressed_ = spacePressed;

    velocity_.x = vx;
}

void Player::update(float dt) {
    // Wybór animacji
    if (!isOnGround_) {
        animator_.play(velocity_.y < 0.f ? "jump" : "fall");
    } else if (std::abs(velocity_.x) > 1.f) {
        animator_.play("run");
    } else {
        animator_.play("idle");
    }

    // Flip sprite w zależności od kierunku
    float scaleX = 40.f / static_cast<float>(texSize_.x);
    float scaleY = 50.f / static_cast<float>(texSize_.y);

    if (velocity_.x < 0.f) facingRight_ = false;
    if (velocity_.x > 0.f) facingRight_ = true;

    if (facingRight_) {
        sprite_.setScale(scaleX, scaleY);
        sprite_.setOrigin(0.f, 0.f);
    } else {
        sprite_.setScale(-scaleX, scaleY);
        sprite_.setOrigin(static_cast<float>(texSize_.x), 0.f);
    }

    sprite_.setTextureRect(animator_.update(dt));

    velocity_.y += GRAVITY * dt;
    velocity_.y = std::min(velocity_.y, MAX_FALL_SPEED);

    sprite_.move(velocity_ * dt);

    isOnGround_ = false;
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(sprite_);
}

sf::FloatRect Player::getHitbox() const {
    return sprite_.getGlobalBounds();
}

void Player::moveShape(sf::Vector2f offset) {
    sprite_.move(offset);
}

void Player::setVelocityX(float vx) {
    velocity_.x = vx;
}

void Player::setVelocityY(float vy) {
    velocity_.y = vy;
}

void Player::setOnGround(bool val) {
    isOnGround_ = val;
    if (val) {
        canDoubleJump_ = false;
    }
}

int Player::getLives() const {
    return lives_;
}

void Player::respawn() {
    sprite_.setPosition(spawnPosition_);
    velocity_ = {0.f, 0.f};
    --lives_;
}

void Player::resetState() {
    sprite_.setPosition(spawnPosition_);
    velocity_ = {0.f, 0.f};
    lives_ = 3;
    isOnGround_ = false;
    canDoubleJump_ = false;
    canTripleJump_ = false;
    spaceWasPressed_ = false;
    justJumped_ = false;
    facingRight_ = true;
    sprite_.setOrigin(0.f, 0.f);
    animator_.play("idle");
}
