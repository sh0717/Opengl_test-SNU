#ifndef MATH_UTILS_H
#define MATH_UTILS_H
#include <iostream>
#include <vector>
struct AABB{
    float left;
    float right;
    float top;
    float bottom;
};
float getRandomValueBetween(float a, float b)
{
    float r = (float) rand() / (float) RAND_MAX;
    return a * (1-r) + b * r;
}
float lerp(float a, float b, float r)
{
    return a * (1 - r) + b * r;
}
float clamp(float x, float x_min, float x_max)
{
    return x < x_min ? x_min : (x > x_max ? x_max : x);
}


glm::mat4 BezierMat = glm::mat4(glm::vec4(-1, 3, -3, 1), glm::vec4(3, -6, 3, 0), glm::vec4(-3, 3, 0, 0), glm::vec4(1, 0, 0, 0));

#endif