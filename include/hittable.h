#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;
using std::vector;

class material;

struct hit_record {
    glm::vec3 p;
    glm::vec3 normal;
    shared_ptr<material> pMat;
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
    sphere(const glm::vec3&, double, shared_ptr<material>);
    virtual bool hit(const ray&, double, double, hit_record&) const override;
public:
    glm::vec3 center;
    double radius;
    shared_ptr<material> pMat;
};

inline sphere::sphere(const glm::vec3& c, double r, shared_ptr<material> pm)
		: center(c), radius(r), pMat(pm) {}

inline bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
    glm::vec3 oc = r.origin() - center;
    auto a = glm::dot(r.direction(), r.direction());
	auto halfB = glm::dot(oc, r.direction());
    auto c = glm::dot(oc, oc) - radius * radius;
    auto discriminant = halfB * halfB - a * c;
    if (discriminant < 0) { return false; }
    auto sqrtd = sqrt(discriminant);
    auto root = (-halfB - sqrtd) / a;

    if (root < t_min || t_max < root) {
        root = (-halfB + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }
    rec.t = root;
    rec.p = r.at(root);
    glm::vec3 outward_normal = (rec.p - center) / static_cast<float>(radius);
    rec.set_face_normal(r, outward_normal);
    rec.pMat = pMat;
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