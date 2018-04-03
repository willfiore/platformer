#pragma once

#include <glm/vec2.hpp>

constexpr float GRENADE_LIFETIME = 3.f;
constexpr float GRENADE_EXPLOSION_RADIUS = 140.f;
constexpr float GRENADE_MAX_DAMAGE = 140.f;
constexpr float GRENADE_KNOCKBACK = 800.f;

struct Projectile
{
  enum Type {
    GRENADE,
    MISSILE,
    PROJECTILE_COUNT
  };

  Projectile();
  Projectile(Type);

  Type type;
  int owner;
  float age;

  glm::vec2 position;
  glm::vec2 velocity;
  glm::vec2 acceleration;

  bool dirty_justBounced;
  bool dirty_justExploded;
};
