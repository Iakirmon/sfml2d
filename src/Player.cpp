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
    constexpr int W = 32, H = 32;

    if (!texture_.loadFromFile("assets/textures/player_sheet.png")) {
        texture_.loadFromFile("assets/textures/player.png");
    }

    sprite_.setTexture(texture_);
    texSize_ = sf::Vector2u(W, H);

    sprite_.setScale(
        40.f / static_cast<float>(W),
        50.f / static_cast<float>(H)
    );
    sprite_.setPosition(spawnPosition_);

    Animation idle;
    idle.frameDuration = 0.2f;
    idle.loop = true;
    for (int i = 0; i < 4; ++i)
        idle.frames.push_back({i * W, 0 * H, W, H});
    animator_.addAnimation("idle", idle);

    Animation run;
    run.frameDuration = 0.1f;
    run.loop = true;
    for (int i = 0; i < 4; ++i)
        run.frames.push_back({i * W, 1 * H, W, H});
    animator_.addAnimation("run", run);

    Animation jump;
    jump.frameDuration = 0.15f;
    jump.loop = false;
    for (int i = 0; i < 2; ++i)
        jump.frames.push_back({i * W, 2 * H, W, H});
    animator_.addAnimation("jump", jump);

    Animation fall;
    fall.frameDuration = 0.2f;
    fall.loop = true;
    for (int i = 0; i < 2; ++i)
        fall.frames.push_back({i * W, 3 * H, W, H});
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
        canDoubleJump_ = false;  // Reset double jump when on ground
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

