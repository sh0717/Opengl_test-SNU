#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H
#include <glm/glm.hpp>
#include "math_utils.h"
struct Entity
{
   
    float scale;
    float rotate;
    float dropSpeed;
    float rotateSpeed;
    glm::vec3 position;
};
struct Bar
{   
    float length;
    float xPosition;
    float speed;
};
Entity getRandomEntity()
{
    Entity e;
    e.scale = getRandomValueBetween(0.1, 0.2f);
    e.rotate = getRandomValueBetween(0.1f, 3);
    e.dropSpeed = getRandomValueBetween(0.2f, 0.5f);
    e.rotateSpeed = getRandomValueBetween(0.1f, 0.4f);
    e.position = glm::vec3(getRandomValueBetween(-0.8f, 0.8f),1,0);
    return e;
}
#endif