#include "mt_algorithm.h"


Sphere::Sphere(float3 center, float radius) :
	center(center), radius(radius)
{
}

Sphere::~Sphere()
{
}

IntersectableData Sphere::Intersect(const Ray& ray) const
{
	float3 oc = center - ray.position;
	float a = dot(ray.direction, ray.direction);
	float b = -2.f * dot(oc, ray.direction);
	float c = dot(oc, oc) - radius * radius;
	float disc = b * b - 4.f * a * c;
	if (disc < 0.f)
		return { disc };
	float x0 = (-b - sqrtf(disc) / 2.f * a);
	float x1 = (-b + sqrtf(disc) / 2.f * a);

	float t = std::min(x0, x1);
	if (t < 0.f) t = std::max(x0, x1);
	
	return { t };
}


MTAlgorithm::MTAlgorithm(short width, short height) : RayGenerationApp(width, height)
{
}

MTAlgorithm::~MTAlgorithm()
{
}

int MTAlgorithm::LoadGeometry(std::string filename)
{
	objects.push_back(new Sphere(float3{ 2.f, 0.f, -1.f }, .4f));

	Vertex a(float3{ -0.5f, -0.5f, -1.f });
	Vertex b(float3{ .5f, -0.5, -1.f });
	Vertex c(float3{ .0f, .5f, -1.f });
	objects.push_back(new Triangle(a, b, c));
	return 0;
}

Payload MTAlgorithm::TraceRay(const Ray& ray, const unsigned int max_raytrace_depth) const
{
	IntersectableData closest_data(t_max);

	for (auto& object : objects)
	{
		IntersectableData data = object->Intersect(ray);
		if (data.t > t_min&& data.t < closest_data.t)
		{
			closest_data = data;
		}
	}

	if (closest_data.t < t_max)
	{
		return Hit(ray, closest_data);
	}

	return Miss(ray);
}

Payload MTAlgorithm::Hit(const Ray& ray, const IntersectableData& data) const
{
	Payload payload;
	payload.color = data.baricentric;
	return payload;
}

Triangle::Triangle(Vertex a, Vertex b, Vertex c) :
	a(a), b(b), c(c)
{
	ba = b.position - a.position;
	ca = c.position - a.position;
}

Triangle::Triangle() :
	a(float3{ 0, 0, 0 }), b(float3{ 0, 0, 0 }), c(float3{ 0, 0, 0 })
{
}

Triangle::~Triangle()
{
}

IntersectableData Triangle::Intersect(const Ray& ray) const
{
	float3 pvec = cross(ray.direction, ca);
	float det = dot(ba, pvec);
	if (det > -1.e-8 && det < 1e-8) return IntersectableData(-1.f);
	float3 tvec = ray.position - a.position;

	float u = dot(tvec, pvec) / det;
	if (u < 0 || u > 1) return IntersectableData(-1.0f);
	float3 qvec = cross(tvec, ba);
	
	float v = dot(ray.direction, qvec) / det;
	if (v < 0 || u + v > 1)
		return IntersectableData(-1.0f);
	float t = dot(ca, qvec) / det;
	return IntersectableData(t, float3{ 1 - u - v , u, v });
}
