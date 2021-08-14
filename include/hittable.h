#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;
using std::vector;

struct hit_record {
    glm::vec3 p;
    glm::vec3 normal;
    double t;
    bool front_face;

	void set_face_normal(const ray& r, const glm::vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
public:
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};

class sphere: public hittable
{
public:
    sphere(const glm::vec3&, double);
    virtual bool hit(const ray&, double, double, hit_record&) const override;
public:
    glm::vec3 center;
    double radius;
};

inline sphere::sphere(const glm::vec3& c, double r) : center(c), radius(r) {}

inline bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
    glm::vec3 oc = r.origin() - center;
    auto a = glm::dot(r.direction(), r.direction());
    auto b = 2 * glm::dot(oc, r.direction());
    auto c = glm::dot(oc, oc) - radius * radius;
    auto discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        return false;
    }
    else
    {
        auto root = (-b - sqrt(discriminant)) / (2.0 * a);
        auto p = r.at(root);
    	
        if (root < t_min || t_max < root) {
            root = (-b + sqrt(discriminant)) / (2.0 * a);
            if (root < t_min || t_max < root)
                return false;
        }
        rec.p = p;
        rec.t = root;
        glm::vec3 outward_normal = (p - center) / static_cast<float>(radius);
        rec.set_face_normal(r, outward_normal);
    }
    return true;
}

class hittable_list : public hittable
{
public:
    hittable_list() = default;
    hittable_list(shared_ptr<hittable> obj) { add(obj); }
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
    void add(shared_ptr<hittable> obj) { objects.push_back(obj); }
    void clear() { objects.clear(); }
private:
    vector<shared_ptr<hittable>> objects;
};

inline bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
    bool hitAnything = false;
    hit_record tempRecord;
    double far = t_max;
	for(auto obj : objects)
	{
		if(obj->hit(r, t_min, far, tempRecord))
		{
            hitAnything = true;
            rec = tempRecord;
            far = tempRecord.t;
		}
	}
    return hitAnything;
}


#endif