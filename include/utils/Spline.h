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

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
struct CubicSpline {
    std::vector<glm::vec3> m_points;
    std::vector<std::vector<glm::vec3>> m_coeffs;
    std::vector<float> m_lengths;

    void InitializeSpline() {
        int n = m_points.size() - 1;
        std::vector<glm::vec3> a(m_points.size(), glm::vec3(0.0f)); // Initialize with zeros
        m_coeffs.resize(m_points.size())
        ;
        for (int i = 0; i < n + 1; i++) {
            m_coeffs[i].resize(4); // Ensure each segment has 4 coefficients
        }
        m_lengths.resize(m_points.size());

        if (n > 0) { // Ensure there are segments to process
            for (int i = 1; i < n; i++) {
                a[i] = 3.0f * (m_points[i + 1] - 2.0f * m_points[i] + m_points[i - 1]);
            }


            std::vector<float> l;
            std::vector<float> mu;
            std::vector<glm::vec3> z;

            l.resize(m_points.size());
            mu.resize(m_points.size());
            z.resize(m_points.size());
            for (int i = 1; i <= n; i++) {
                l[i] = 4.0f - mu[i - 1];
                mu[i] = 1.0f / l[i];
                z[i] = (a[i] - z[i - 1]) / l[i];
            }

            for (int i = 1; i <= n - 1; i++) {
                l[i] = 4 - mu[i - 1];
                mu[i] = 1 / l[i];
                z[i] = (a[i] - z[i - 1]) / l[i];
            }

            for (int i = 0; i < m_points.size(); i++) {
                m_coeffs[i][0] = m_points[i];
            }

            for (int j = n - 1; j >= 0; j--) {
                m_coeffs[j][2] = z[j] - mu[j] * m_coeffs[j + 1][2];
                m_coeffs[j][3] = (1.0f / 3.0f) * (m_coeffs[j + 1][2] - m_coeffs[j][2]);
                m_coeffs[j][1] = m_points[j + 1] - m_points[j] - m_coeffs[j][2] - m_coeffs[j][3];
            }

            for (int k = 0; k < m_points.size() - 1; k++)
                m_lengths[k] = Integrate(k, 1);
        }
    }
    glm::vec3 SplineAtTime(float t)
    {
        int numPoints = m_points.size();
        if (t >= numPoints)
            t = static_cast<float>(numPoints) - 0.0001f;

        int spline = static_cast<int>(t);
        float fractional = t - spline;
        spline = spline % (numPoints - 1);

        float x = fractional;
        float xx = x * x;
        float xxx = x * xx;

        return m_coeffs[spline][0] + m_coeffs[spline][1] * x + m_coeffs[spline][2] * xx + m_coeffs[spline][3] * xxx;
    }
    float ArcLengthIntegrand(int spline, float t)
    {
        float tt = t * t;
        glm::vec3 dv = m_coeffs[spline][1] + 2.0f * m_coeffs[spline][2] * t + 3.0f * m_coeffs[spline][3] * tt;
        return glm::length(dv);
    }

    float Integrate(int spline, float t)
    {
        int n = 16;
        float h = t / n;
        float XI0 = ArcLengthIntegrand(spline, 0);
        float XI1 = 0;
        float XI2 = 0;

        for (int i = 1; i <= n; i++) {
            float X = i * h;
            if (i % 2 == 0)
                XI2 += ArcLengthIntegrand(spline, X);
            else
                XI1 += ArcLengthIntegrand(spline, X);
        }

        float XI = h * (XI0 + 2 * XI2 + 4 * XI1) / 3.0f;
        return XI;
    }

    glm::vec3 ConstVelocitySplineAtTime(float t)
    {
        int spline = 0;
        while (t > m_lengths[spline] && spline < m_points.size()) {
            t -= m_lengths[spline];
            spline++;
        }

        float s = t / m_lengths[spline]; // Initial guess

        for (int i = 0; i < 6; i++) // Perform several Newton-Raphson iterations
            s -= (Integrate(spline, s) - t) / ArcLengthIntegrand(spline, s);

        return SplineAtTime(static_cast<float>(spline) + s);
    }
};

class Spline {
public:
    std::vector<glm::vec3> points;
    float alpha;
    int index = 0;
    float dt = 0;
    float transitionTime;
    Spline(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int noPoints, float alpha = 0.5, float transitionTime = 0.2)
        : alpha(alpha)
        , transitionTime(transitionTime)
    {
        points = generateSplinePoints(p0, p1, p2, p3, alpha, noPoints);
    }

    void addDelta(float delta)
    {
        dt += delta;
        if (dt > transitionTime) {
            index++;
            dt = 0;
        }
    }

    bool isAtEnd()
    {
        return index >= points.size();
    }

    glm::vec3 current()
    {
        return points[index];
    }
};
#endif // INCLUDE_UTILS_SPLINE_H_
