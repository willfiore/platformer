#include "Terrain.hpp"
#include "Random.hpp"

#include <vector>
#include <map>
#include <iostream>

#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include "geo.hpp"

#include "EventManager.hpp"
#include "ResourceManager.hpp"
#include "Grenade.hpp"
#include "Powerup.hpp"
#include "Console.hpp"

Terrain::Terrain() :
  maxDepth(-400.f),
  maxWidth(10000.f)
{
  for (float i = 0.f; i < maxWidth; i += PRECISION) {
    basePoints.push_back({i, -Random::randomFloat(0.f, 10.f)});
  }
  maxWidth = basePoints.back().x;
  points = basePoints;

  EventManager::Register(Event::Type::EXPLOSION,
      std::bind(&Terrain::onExplosion, this, _1));
  EventManager::Register(Event::Type::POWERUP_LAND,
      std::bind(&Terrain::onPowerupLand, this, _1));
}

float Terrain::getHeight(float x) const
{
  for (size_t i = 0; i < points.size(); ++i) {
    glm::vec2 p1 = points[i];

    if (x < p1.x) {

      // Before first point
      if (i == 0) return -1000.f;
      glm::vec2 p2 = points[i-1];

      float a = (x - p1.x) / (p2.x - p1.x);
      float y = p1.y + (p2.y - p1.y) * a;

      return y;
    }
  }

  return -1000.f;
}

float Terrain::getAngle(float x) const
{
  for (size_t i = 0; i < points.size(); ++i) {
    glm::vec2 p1 = points[i];

    if (x < p1.x) {
      if (i == 0) return 0.f;
      glm::vec2 p2 = points[i-1];

      return glm::atan((p2.y - p1.y) / (p2.x - p1.x));
    }
  }

  return 0.f;
}

std::vector<LineSegment> Terrain::getSegmentsInRange(float x1, float x2) const
{
  std::vector<LineSegment> ret;

  bool reverse = false;
  if (x1 > x2) {
    reverse = true;
    float t = x1;
    x1 = x2;
    x2 = t;
  }

  for (size_t i = 1; i < points.size(); ++i) {
    glm::vec2 a = points[i-1];
    glm::vec2 b = points[i];

    if ((x1 > a.x && x1 < b.x) ||
	(x2 > a.x && x2 < b.x)) {
	ret.push_back({a, b});
	}
  }

  if (reverse)
    std::reverse(ret.begin(), ret.end());

  return ret;
}

std::pair<bool, glm::vec2> Terrain::intersect(glm::vec2 p1, glm::vec2 p2) const
{
  std::vector<LineSegment> tSegments = getSegmentsInRange(p1.x, p2.x);
  for (auto& s : tSegments) {
    auto intersection = geo::intersect(s.first, s.second, p1, p2);
    if (intersection.first)
      return intersection;
  }

  return std::make_pair(false, glm::vec2());
}

void Terrain::update(double t, double dt) {
  time = t;

  points = basePoints;

  // Remove old modifiers
  modifiers.erase(std::remove_if(modifiers.begin(), modifiers.end(),
	[](const TerrainPointModifier& m) -> bool {
	return m.age > m.lifetime;
	}), modifiers.end());

  for (auto& m : modifiers) {
    m.age += dt;

    for (auto& p : points) {
      p.y += m.func(p.x, t);
    }
  }
}

void Terrain::addFunc(
    const std::function<float(float, double)>& func,
    double lifetime)
{
  while (modifiers.size() > MAX_MODIFIERS) {
    modifiers.pop_front();
  }

  TerrainPointModifier m;
  m.lifetime = lifetime;
  m.age = 0.0f;
  m.func = func;

  modifiers.push_back(m);
}

void Terrain::wobble(float xpos, float amplitude)
{
  // Wobble terrain
  double currentTime = time;
  addFunc([=](float x, double t) -> float {

      float dt = t - currentTime;

      // Oscillate up and down over time
      float r = amplitude * glm::cos(15.f*dt + glm::half_pi<float>());

      // Fade out over time
      float mt = glm::exp(-3.5f*dt);

      // Fade out over distance
      float dx = glm::abs(x - xpos);
      float mx = glm::exp(-dx / 200.f);

      return r * mx * mt;
      }, 4.0);
}

void Terrain::deform(glm::vec2 pos, float radius, float depthModifier)
{
  for (auto& p : basePoints) {
    float distance = glm::distance(pos, p);
    if (distance < radius) {
      // Create hole in ground
      p.y -= 0.1f * radius * depthModifier *
	glm::cos(glm::half_pi<float>() * (distance/radius)) *
	(1 - p.y / maxDepth);

      if (p.y < maxDepth) p.y = maxDepth;
    }
  }
}

void Terrain::onExplosion(const Event& e)
{
  const auto* g = boost::any_cast<EvdGrenadeExplosion>(e.data).grenade;

  if (g->properties.radius == 0.f) return;

  deform(g->position, g->properties.radius, g->properties.terrainDamageModifier);

  float wobbleAmount = 15.f * g->properties.terrainWobbleModifier;
  if ((g->position.y - getHeight(g->position.x)) / g->properties.radius < 1.f) {
    wobble(g->position.x, wobbleAmount);
  }
}

void Terrain::onPowerupLand(const Event& e)
{
  const Powerup* p = boost::any_cast<EvdPowerupLand>(e.data).powerup;
  deform(p->position, 90.f, 1.f);
  wobble(p->position.x, 15.f);
}
