#include "GrenadeSystem.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/constants.hpp>

#include "imgui.h"
#include "Console.hpp"

#include "Terrain.hpp"
#include "EventManager.hpp"
#include "Player.hpp"
#include "Random.hpp"

GrenadeSystem::GrenadeSystem(const Terrain* t) :
  terrain(t)
{
  EventManager::Register(Event::PLAYER_FIRE_WEAPON,
      std::bind(&GrenadeSystem::onPlayerFireWeapon, this, _1));

  EventManager::Register(Event::PLAYER_SECONDARY_FIRE_WEAPON,
      std::bind(&GrenadeSystem::onPlayerSecondaryFireWeapon, this, _1));
}

void GrenadeSystem::update(double dt)
{
  // Remove dead grenades
  grenades.erase(std::remove_if(grenades.begin(), grenades.end(),
	[](const Grenade& g)->bool {
	return g.dirty_awaitingRemoval;
	}), grenades.end());
  
  // Add new grenades
  grenades.insert(grenades.end(),
      grenadesToSpawn.begin(), grenadesToSpawn.end());
  grenadesToSpawn.clear();

  for (auto& g : grenades) {
    g.age += dt;

    // ------------
    // Physics
    // ------------

    g.velocity += g.acceleration * (float)dt;
    glm::vec2 newPosition = g.position + g.velocity * (float)dt;

    if (g.dirty_justBounced) {
      g.dirty_justBounced = false;
    }
    else {
      std::vector<LineSegment> tSegments =
	terrain->getSegmentsInRange(g.position.x, newPosition.x);

      bool foundIntersection = false;
      for (auto& s : tSegments) {

	auto intersection =
	  geo::intersect(s.first, s.second, g.position, newPosition);

	if (intersection.first) {
	  foundIntersection = true;
	  g.dirty_justBounced = true;

	  newPosition = intersection.second;
	  grenadeHitGround(g, newPosition);
	  break;
	}
      }

      // Failsafe
      if (!foundIntersection &&
	  newPosition.y < terrain->getHeight(newPosition.x)) {
	newPosition.y = terrain->getHeight(newPosition.x);
	grenadeHitGround(g, newPosition);
      }
    }

    g.position = newPosition;

    // -------------
    // Timer explode
    // -------------
    if (g.properties.lifetime &&
	g.age >= g.properties.lifetime) {
      explodeGrenade(g);
    }
  }
}

void GrenadeSystem::grenadeHitGround(Grenade& g, glm::vec2 pos)
{
  if (g.properties.detonateOnLand) {
    explodeGrenade(g);
    return;
  }

  float terrainAngle = terrain->getAngle(g.position.x);
  float grenadeAngle = glm::atan(g.velocity.y, g.velocity.x);
  float rotateAngle = 2 * (terrainAngle - grenadeAngle);
  g.velocity = 0.5f * glm::rotate(g.velocity, rotateAngle);
  g.position = pos;
}

void GrenadeSystem::explodeGrenade(Grenade& g)
{
  g.dirty_awaitingRemoval = true;

  EvdExplosion d;
  d.position = g.position;
  d.damage = g.properties.damage;
  d.radius = g.properties.radius;
  d.knockback = g.properties.knockback;
  d.terrainDamageModifier = g.properties.terrainDamageModifier;
  d.terrainWobbleModifier = g.properties.terrainWobbleModifier;
  EventManager::Send(Event::EXPLOSION, d);

  for (int i = 0; i < g.properties.numClusterFragments; ++i) {
    Grenade& f = spawnGrenade(Grenade::Type::CLUSTER_FRAGMENT);
    f.owner = g.owner;
    f.position = g.position;
    f.velocity.x = 0.3f*g.velocity.x + 230.f*Random::randomFloat(-1.f, 1.f);
    f.velocity.y = 400.f*Random::randomFloat(0.5f, 1.f);
  }
}

void GrenadeSystem::onPlayerFireWeapon(Event e)
{
  auto d = boost::any_cast<EvdPlayerFireWeapon>(e.data);

  Grenade& p = spawnGrenade(Grenade::Type::CLUSTER);
  p.owner = d.player.id;

  float strength = 600.f;
  p.position = d.player.getCenterPosition();
  p.velocity = 0.33f * d.player.velocity;
  p.velocity.x += strength * glm::cos(d.player.aimDirection);
  p.velocity.y += strength * -glm::sin(d.player.aimDirection);
}

void GrenadeSystem::onPlayerSecondaryFireWeapon(Event e)
{
  auto d = boost::any_cast<EvdPlayerSecondaryFireWeapon>(e.data);

  for (auto& g : grenades) {
    // Grenade secondary fire explodes grenades
    if (g.owner == d.player.id &&
	g.properties.manualDetonate) {
      explodeGrenade(g);
      continue;
    }
  }
}

Grenade& GrenadeSystem::spawnGrenade(Grenade::Type type)
{
  grenadesToSpawn.emplace_back(Grenade(type));
  return grenadesToSpawn.back();
}
