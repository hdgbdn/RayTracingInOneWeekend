#include <random>
#include "glm/glm.hpp"

using namespace glm;

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

vec3 random_in_unit_sphere() {
    while (true) {
        auto p = vec3(random_double(-1.0, 1.0), random_double(-1.0, 1.0), random_double(-1.0, 1.0));
        if (length(p) >= 1) continue;
        return p;
    }
}

vec3 random_unit_vector() {
    return normalize(random_in_unit_sphere());
}