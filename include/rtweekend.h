#include <random>
#include "glm/glm.hpp"

namespace rtweekend
{
    inline double random_double() {
        static std::uniform_real_distribution<double> distribution(0.0, 1.0);
        static std::mt19937 generator;
        return distribution(generator);
    }

    inline double random_double(double min, double max) {
        static std::uniform_real_distribution<double> distribution(min, max);
        static std::mt19937 generator;
        return distribution(generator);
    }

    glm::vec3 random_in_unit_sphere() {
        while (true) {
            auto p = glm::vec3(random_double(-1.0, 1.0), random_double(-1.0, 1.0), random_double(-1.0, 1.0));
            if (length(p) >= 1) continue;
            return p;
        }
    }

    glm::vec3 random_unit_vector() {
        return normalize(random_in_unit_sphere());
    }

    glm::vec3 random_in_hemisphere(const glm::vec3& normal) {
        glm::vec3 in_unit_sphere = random_in_unit_sphere();
        if (dot(in_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
            return in_unit_sphere;
        else
            return -in_unit_sphere;
    }

    glm::vec3 reflect(const glm::vec3& v, const glm::vec3& n)
    {
        return v - 2 * dot(v, n) * n;
    }

    glm::vec3 refract(const glm::vec3& uv, const glm::vec3& n, float etai_over_etat) {
        float cos_theta = fmin(glm::dot(-uv, n), 1.0);
        glm::vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
        glm::vec3 r_out_parallel = - static_cast<float>(sqrt(fabs(1.0 - powf(glm::length(r_out_perp), 2)))) * n;
        return r_out_perp + r_out_parallel;
    }
}
