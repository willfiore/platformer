#pragma once

#include <glm/vec2.hpp>

struct Grenade
{
  enum Type {
    STANDARD,
    CLUSTER,
    CLUSTER_FRAGMENT,
    INERTIA,
    TYPE_COUNT
  };

  Grenade();
  Grenade(Type);

  Type type;
  int owner;
  double age;
  double localTimescale;

  glm::vec2 position;
  glm::vec2 velocity;
  glm::vec2 acceleration;

  bool dirty_justBounced;
  bool dirty_awaitingRemoval;
  int dirty_justCollidedWithPlayer;

  // Properties
  struct {

    double lifetime;
    float knockback;
    float damage;
    float radius;
    float terrainDamageModifier;
    float terrainWobbleModifier;

    bool manualDetonate;
    bool detonateOnLand;
    bool detonateOnPlayerHit;
    bool bounceOnPlayerHit;
    bool slowBeforeDetonate;
    int numClusterFragments;

  } properties;

};
