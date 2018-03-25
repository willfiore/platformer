#include "PlayerSystem.hpp"

#include <glm/glm.hpp>

#include "Player.hpp"
#include "Terrain.hpp"
#include "Joystick.hpp"

#include <glm/gtc/constants.hpp>

#include <iostream>

PlayerSystem::PlayerSystem(const Terrain* t) :
  terrain(t)
{
  Player player;
  player.position.x = 100.f;

  player.weapons.insert(Weapon::GRENADE);
  player.weapons.insert(Weapon::MISSILE);

  players.push_back(player);

  EventManager::Register(Event::EXPLOSION,
      std::bind(&PlayerSystem::onExplosion, this, _1));
}

void PlayerSystem::update(float dt, const float* _a)
{
  axes = _a;

  for (auto& p : players) {

    float terrainHeight = terrain->getHeight(p.position.x);
    float terrainAngle = terrain->getAngle(p.position.x);

    // Movement
    ////////////////////////////////////////////////////

    // -------- Acceleration --------
    float accel_factor = Player::ACCEL_X;
    if (p.airborne)
      accel_factor = Player::ACCEL_X_AIRBORNE;
    if (p.airborne && p.outOfControl)
      accel_factor = Player::ACCEL_X_NOCONTROL;

    // Acceleration due to player input
    if (!p.outOfControl) {
      p.acceleration.x = axes[0] * accel_factor;
    } else {
      p.acceleration.x = 0.f;
    }

    // Drag to oppose velocity
    p.acceleration.x -= accel_factor / Player::MAX_SPEED * p.velocity.x;

    // Gravity
    p.acceleration.y = Player::ACCEL_Y;

    // -------- Velocity --------
    p.velocity += p.acceleration * dt;

    // Reduce velocity uphill
    if (!p.airborne &&
	glm::sign(terrainAngle) == glm::sign(p.velocity.x)) {
      p.velocity *= glm::cos(terrainAngle);
    }

    // -------- Position --------
    p.position += p.velocity * dt;

    // Terrain collision
    if (p.position.y < terrainHeight) {
      // Movement
      p.acceleration.y = 0.f;
      p.velocity.y = 0.f;
      p.position.y = terrainHeight;

      // Flags
      p.airborne = false;
      p.outOfControl = false;
      p.jumpAvailable = true;
    }

    // Stick to terrain for shallow angles
    if (!p.airborne && p.position.y > terrainHeight) {
      if(abs(terrainAngle) < Player::MAX_DOWNHILL_ANGLE) {
        p.position.y = terrainHeight;
      }
      else {
        p.airborne = true;
      }
    }

    // -------- Angle --------
    float goalAngle = terrainAngle;

    float modifier = (p.position.y - terrainHeight) / 170.f;
    if (modifier < 0.f) modifier = 0.f;

    // Slightly tilt towards velocity direction
    goalAngle += modifier *
      (-glm::radians(15.f) * (p.velocity.x / Player::MAX_SPEED) - goalAngle);

    p.angle += 12.f * dt * (goalAngle - p.angle);

    // Aiming
    ////////////////////////////////////////////////////
    p.aimDirection = glm::atan(axes[1], axes[0]);
  }
}

void PlayerSystem::processInput(int playerID, int button, bool action)
{
  Player& player = players[playerID];

  // Press
  if (action) {
    switch (button) {
      case JOY_BUTTON_A:
	jump(player); break;
      case JOY_BUTTON_RB:
	fireWeapon(player); break;
      case JOY_BUTTON_Y:
	cycleWeapon(player); break;
      default: break;
    }
  }

  // Release
  else {
    switch (button) {
      default: break;
    }
  }
}

void PlayerSystem::jump(Player& p)
{
  if (!p.jumpAvailable) return;

  float terrainAngle = terrain->getAngle(p.position.x);

  p.velocity.y = Player::JUMP_VELOCITY;
  if (abs(terrainAngle) > Player::MIN_SIDEJUMP_ANGLE) {
    p.velocity.x += 0.4f * Player::JUMP_VELOCITY * -glm::sin(terrainAngle); 
  }
  p.airborne = true;
  p.jumpAvailable = false;
}



void PlayerSystem::fireWeapon(Player& p)
{
  // Can't fire if they have no weapons!
  if (p.weapons.size() == 0) return;

  Weapon currentWeapon = *std::next(p.weapons.begin(), p.currentWeaponIndex);

  Event e{Event::PLAYER_FIRE_WEAPON};
  e.data.push_back(p);
  e.data.push_back(currentWeapon);
  EventManager::Send(e);
}

void PlayerSystem::cycleWeapon(Player& p)
{
  if (p.weapons.size() <= 1) return;

  p.currentWeaponIndex++;

  // Wrap
  if(p.currentWeaponIndex >= p.weapons.size()) {
    p.currentWeaponIndex = 0;
  }
}

void PlayerSystem::onExplosion(Event e)
{
  glm::vec2 position = boost::any_cast<glm::vec2>(e.data[0]);
  float radius = boost::any_cast<float>(e.data[1]);

  for (auto& p : players) {
    glm::vec2 diff = p.position - position;
    float dist = glm::length(diff);

    if (dist < radius) {
      p.airborne = true;
      p.outOfControl = true;

      glm::vec2 launchVelocity;

      launchVelocity.x = 1500.f * glm::pow( (radius-dist)/radius, 0.5);
      launchVelocity.x *= glm::sign(diff.x);

      launchVelocity.y = 1500.f * glm::pow( (radius-dist)/radius, 0.5);

      // If we are very close in x, launch more upwards
      if (glm::abs(diff.x) < 1.5f * Player::SIZE) {
	launchVelocity.x *= 0.2f;
	launchVelocity.y *= 1.5f;
      }

      p.velocity += launchVelocity;
    }
  }
}
