#pragma once

#include <vector>

#include "Player.hpp"
#include "EventManager.hpp"

class Terrain;
class ProjectileSystem;

class PlayerSystem
{
public:
  PlayerSystem(const Terrain*);

  void update(float dt, const float* axes);
  void processInput(int playerID, int button, bool action);

  void jump(Player&);
  void fireWeapon(Player&);
  void cycleWeapon(Player&);

  const std::vector<Player>& getPlayers() const { return players; };

private:
  std::vector<Player> players;
  const Terrain* terrain;

  const float* axes;

  void onExplosion(Event);
  void onPowerupPickup(Event);
};
