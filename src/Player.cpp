#include "Player.h"

#include <algorithm>

namespace {
constexpr float GRAVITY        = 1500.0f;  // px/s^2
constexpr float JUMP_VELOCITY  = -600.0f;  // px/s
constexpr float MOVE_SPEED     = 250.0f;   // px/s
constexpr float MAX_FALL_SPEED = 800.0f;   // px/s
}

Player::Player() {
    shape_.setSize({40.f, 50.f});
    shape_.setFillColor(sf::Color::Blue);
    shape_.setPosition(spawnPosition_);
}

void Player::handleInput() {
    float vx = 0.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        vx -= MOVE_SPEED;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        vx += MOVE_SPEED;
    }

    // Jump or Double Jump or Triple Jump (detect NEW key press, not held)
    bool spacePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
    if (spacePressed && !spaceWasPressed_) {
        if (isOnGround_) {
            // First jump
            velocity_.y = JUMP_VELOCITY;
            isOnGround_ = false;
            canDoubleJump_ = true;  // Allow double jump
        } else if (canDoubleJump_) {
            // Double jump
            velocity_.y = JUMP_VELOCITY;
            canDoubleJump_ = false;  // Only one double jump
            canTripleJump_ = true;   // Allow triple jump after double jump
        } else if (canTripleJump_){
            // Triple jump
            velocity_.y = JUMP_VELOCITY;
            canTripleJump_ = false;  // Only one triple jump
        }
    }
    spaceWasPressed_ = spacePressed;

    velocity_.x = vx;
}

void Player::update(float dt) {
    // Gravity
    velocity_.y += GRAVITY * dt;
    velocity_.y = std::min(velocity_.y, MAX_FALL_SPEED);

    shape_.move(velocity_ * dt);

    // will be reset by collisions if standing on ground
    isOnGround_ = false;
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(shape_);
}

sf::FloatRect Player::getHitbox() const {
    return shape_.getGlobalBounds();
}

void Player::moveShape(sf::Vector2f offset) {
    shape_.move(offset);
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
    shape_.setPosition(spawnPosition_);
    velocity_ = {0.f, 0.f};
    --lives_;
}

