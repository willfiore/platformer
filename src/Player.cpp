#include "Player.hpp"

#include <vector>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "ResourceManager.hpp"

Player::Player()
{
  id = -1;
  controllerID = -1;
  currentWeaponIndex = 0;

  // Init physics
  position = glm::vec2();
  velocity = glm::vec2();
  acceleration = glm::vec2();
  angle = 0.f;
  aimDirection = 0.f;

  // Init state
  health = 200;

  airborne = false;
  jumpAvailable = true;
  outOfControl = false;
}
