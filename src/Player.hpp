#pragma once

#include <set>
#include <glm/vec2.hpp>
#include <glm/glm.hpp>

enum class Weapon {
  GRENADE,
  MISSILE,
  NUM_WEAPONS
};

struct Player {
  Player();

  static constexpr float MAX_SPEED = 380.f;
  static constexpr float ACCEL_X = 10000.f;
  static constexpr float ACCEL_X_AIRBORNE = 2000.f;
  static constexpr float ACCEL_X_NOCONTROL = 100.f;
  static constexpr float ACCEL_Y = -4000.f;
  static constexpr float MAX_DOWNHILL_ANGLE = glm::radians(45.f);
  static constexpr float MAX_UPHILL_ANGLE = glm::radians(60.f);
  static constexpr float MIN_SIDEJUMP_ANGLE = glm::radians(20.f);
  static constexpr float JUMP_VELOCITY = 1200.f;
  static constexpr float SIZE = 10.f;
  static constexpr float INITIAL_ATTACK_STRENGTH = 60.f;
  static constexpr float MAX_ATTACK_STRENGTH = 150.f;
  static constexpr float ATTACK_STRENGTH_GROWTH = 4.f;

  // State
  //////////////////////////////////////////////

  glm::vec2 position {0.f, 0.f};
  glm::vec2 velocity {0.f, 0.f};
  glm::vec2 acceleration {0.f, 0.f};

  float angle {0.f};
  float aimDirection {0.f};

  std::set<Weapon> weapons;
  std::set<Weapon>::size_type currentWeaponIndex;

  // Flags
  bool airborne;
  bool jumpAvailable;
  bool outOfControl;
};
