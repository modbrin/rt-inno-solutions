#include "refraction.h"

Refraction::Refraction(short width, short height) :Reflection(width, height)
{
	raytracing_depth = 10;
}

Refraction::~Refraction()
{
}

Payload Refraction::Hit(const Ray& ray, const IntersectableData& data, const MaterialTriangle* triangle, const unsigned int max_raytrace_depth) const
{
	if (max_raytrace_depth <= 0) {
		return Miss(ray);
	}
	if (triangle == nullptr) {
		return Miss(ray);
	}
	Payload payload;
	payload.color = triangle->emissive_color;

	float3 X = ray.position + ray.direction * data.t;
	float3 n = triangle->GetNormal(data.baricentric);

	if (triangle->reflectiveness) {
		float3 reflection_direction = ray.direction - 2.f * dot(n, ray.direction) * n;
		Ray reflection_ray(X, reflection_direction);
		return TraceRay(reflection_ray, max_raytrace_depth - 1);
	}

	if (triangle->reflectiveness_and_transparency) {
		float kr;
		float cosi = std::max(-1.f, std::min(1.f, dot(ray.direction, n)));
		float etai = 1.f;
		float etat = triangle->ior;

		if (cosi > 0) {
			std::swap(etai, etat);
		}

		float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
		if (sint >= 1.f) {
			kr = 1.f;
		}
		else {
			float cost = sqrtf(std::max(0.f, 1 - sint * sint));
			cosi = fabs(cosi);
			float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
			float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
			kr = (Rs * Rs + Rp * Rp) / 2.f;
		}

		bool outside = dot(ray.direction, n) < 0;
		float3 bias = .001f * n;
		Payload refraction_paylaod;

		if (kr < 1.f) {
			float cosi = std::max(-1.f, std::min(1.f, dot(ray.direction, n)));
			float etai = 1.f;
			float etat = triangle->ior;

			if (cosi < 0) {
				cosi = -cosi;
			}
			else {
				std::swap(etai, etat);
			}

			cosi = fabs(cosi);
			float eta = etai / etat;
			float k = 1 - eta * eta * (1 - cosi * cosi);
			float3 refraction_direction{ 0, 0, 0 };
			if (k >= 0) {
				refraction_direction = eta * ray.direction + (eta * cosi - sqrtf(k)) * n;
			}
			Ray refraction_ray(outside ? X - bias : X + bias, refraction_direction);
			refraction_paylaod = TraceRay(refraction_ray, max_raytrace_depth - 1);

			float3 reflection_direction = ray.direction - 2.f * dot(n, ray.direction) * n;
			Ray reflection_ray(outside ? X + bias : X - bias, reflection_direction);
			Payload reflection_payload = TraceRay(reflection_ray, max_raytrace_depth - 1);

			Payload final;
			final.color = reflection_payload.color * kr + refraction_paylaod.color;
			return final;
		}
	}

	for (auto light : lights) {
		Ray to_light(X, light->position - X);
		float to_light_distance = length(light->position - X);

		float t = TraceShadowRay(to_light, to_light_distance);
		if (fabs(t - to_light_distance) > .001f) {
			continue;
		}

		payload.color += light->color * triangle->diffuse_color * std::max(.0f, dot(n, to_light.direction));

		float3 reflection_direction = 2.f * dot(n, to_light.direction) * n - to_light.direction;
		payload.color += light->color * triangle->specular_color * powf(std::max(.0f, dot(ray.direction, reflection_direction)), triangle->specular_exponent);
	}

	return payload;
}
