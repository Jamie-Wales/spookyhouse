#ifndef INCLUDE_UTILS_SPLINE_H_
#define INCLUDE_UTILS_SPLINE_H_

#include "glm/glm.hpp"
#include <vector>

float getT(float t, float alpha, const glm::vec3& p0, const glm::vec3& p1)
{
    auto d = p1 - p0;
    float a = glm::dot(d, d);
    float b = glm::pow(a, alpha * .5f);
    return b + t;
}

glm::vec3 catMullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t, float alpha = 0.5)
{
    float t0 = 0.0f;
    float t1 = getT(t0, alpha, p0, p1);
    float t2 = getT(t1, alpha, p1, p2);
    float t3 = getT(t2, alpha, p2, p3);
    t = glm::mix(t1, t2, t);
    auto A1 = (t1 - t) / (t1 - t0) * p0 + (t - t0) / (t1 - t0) * p1;
    auto A2 = (t2 - t) / (t2 - t1) * p1 + (t - t1) / (t2 - t1) * p2;
    auto A3 = (t3 - t) / (t3 - t2) * p2 + (t - t2) / (t3 - t2) * p3;
    auto B1 = (t2 - t) / (t2 - t0) * A1 + (t - t0) / (t2 - t0) * A2;
    auto B2 = (t3 - t) / (t3 - t1) * A2 + (t - t1) / (t3 - t1) * A3;
    auto C = (t2 - t) / (t2 - t1) * B1 + (t - t1) / (t2 - t1) * B2;
    return C;
}

std::vector<glm::vec3> generateSplinePoints(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float alpha, int numPoints)
{
    std::vector<glm::vec3> points;
    for (int i = 0; i <= numPoints; i++) {
        float t = i / (float)numPoints;
        glm::vec3 point = catMullRom(p0, p1, p2, p3, t, alpha);
        points.push_back(point);
    }
    return points;
}

class Spline {
public:
    std::vector<glm::vec3> points;
    float alpha;
    int index = 0;
    float dt = 0;
    float transitionTime;
    Spline(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int noPoints, float alpha = 0.5, float transitionTime = 0.2)
        : alpha(alpha), transitionTime(transitionTime)
    {
        points = generateSplinePoints(p0, p1, p2, p3, alpha, noPoints);
    }


    void addDelta(float delta) {
        dt += delta;
        if (dt > transitionTime) {
            index++;
            dt = 0;
        }
    }

    bool isAtEnd()
    {
        return index >=  points.size();
    }

    glm::vec3 current()
    {
        return points[index];
    }
};
#endif // INCLUDE_UTILS_SPLINE_H_
