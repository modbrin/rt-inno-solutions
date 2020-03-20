#include "shadow_rays.h"
#include <algorithm>
#include <execution>


ShadowRays::ShadowRays(short width, short height) : Lighting(width, height)
{}

ShadowRays::~ShadowRays()
{}

Payload ShadowRays::TraceRay(const Ray& ray, const unsigned int max_raytrace_depth) const
{
	IntersectableData closest(t_max);
	MaterialTriangle* closest_tri = nullptr;

	for (auto& obj : material_objects)
	{
		IntersectableData data = obj->Intersect(ray);
		if (data.t > t_min&& data.t < closest.t) {
			closest = data;
			closest_tri = obj;
		}
	}

	if (closest.t < t_max) {
		return Hit(ray, closest, closest_tri, max_raytrace_depth);
	}

	return Miss(ray);
}


Payload ShadowRays::Hit(const Ray& ray, const IntersectableData& data, const MaterialTriangle* triangle, const unsigned int max_raytrace_depth) const
{
	if (max_raytrace_depth <= 0) return Miss(ray);

	if (triangle == nullptr) return Miss(ray);

	Payload payload;
	payload.color = triangle->emissive_color;

	float3 X = ray.position + ray.direction * data.t;
	float3 n = triangle->GetNormal(data.baricentric);

	for (auto light : lights) {
		Ray to_light(X, light->position - X);
		float to_light_distance = length(light->position - X);

		float t = TraceShadowRay(to_light, to_light_distance);
		if (fabs(t - to_light_distance) > .001f)
			continue;

		payload.color += light->color * triangle->diffuse_color * (std::max(0.f, dot(n, to_light.direction)));
		float3 reflection_direction = 2.f * dot(n, to_light.direction) * n - to_light.direction;
		payload.color += light->color * triangle->specular_color * powf(std::max(0.f, dot(ray.direction, reflection_direction)), triangle->specular_exponent);
	}

	return payload;
}

float ShadowRays::TraceShadowRay(const Ray& ray, const float max_t) const
{
	IntersectableData closest(max_t);
	for (auto& obj : material_objects)
	{
		IntersectableData data = obj->Intersect(ray);
		if (data.t > t_min&& data.t < closest.t) {
			return data.t;
		}
	}

	return max_t;
}

