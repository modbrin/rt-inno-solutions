#include "ray_generation.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

RayGenerationApp::RayGenerationApp(short width, short height) :
	width(width),
	height(height)
{
	frame_buffer.resize(width * height);
}

RayGenerationApp::~RayGenerationApp()
{}

void RayGenerationApp::SetCamera(float3 position, float3 direction, float3 approx_up)
{
	camera.SetPosition(position);
	camera.SetDirection(direction);
	camera.SetUp(approx_up);
	camera.SetRenderTargetSize(width, height);
}

void RayGenerationApp::Clear()
{
	std::fill(frame_buffer.begin(), frame_buffer.end(), byte3());
}

void RayGenerationApp::DrawScene()
{
	for (short x = 0; x < width; ++x)
#pragma omp parallel for
		for (short y = 0; y < height; ++y) {
			Ray ray = camera.GetCameraRay(x, y);
			Payload payload = TraceRay(ray, raytracing_depth);
			SetPixel(x, y, payload.color);
		}
}

int RayGenerationApp::Save(std::string filename) const
{
	int result = stbi_write_png(filename.c_str(), width, height, 3, frame_buffer.data(), width * 3);

	if (result == 1)
	{
		system((std::string("start ") + filename).c_str());
	}

	return 1 - result;
}

Payload RayGenerationApp::TraceRay(const Ray& ray, const unsigned int max_raytrace_depth) const
{
	return Miss(ray);
}

Payload RayGenerationApp::Miss(const Ray& ray) const
{
	Payload payload;
	float t = 0.5f * (ray.direction.y + 1.f);
	payload.color = { 0.f, 0.2f, 0.7f + 0.3f * t };
	return payload;
}

void RayGenerationApp::SetPixel(unsigned short x, unsigned short y, float3 color)
{
	byte3 byte_color{ static_cast<uint8_t>(255.f * color.x), static_cast<uint8_t>(255.f * color.y), static_cast<uint8_t>(255.f * color.z) };

	frame_buffer[static_cast<size_t>(y)* static_cast<size_t>(width) + static_cast<size_t>(x)] = byte_color;
}

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::SetPosition(float3 position)
{
	this->position = position;
}

void Camera::SetDirection(float3 direction)
{
	this->direction = normalize(direction - position);
}

void Camera::SetUp(float3 approx_up)
{
	this->right = normalize(cross(this->direction, approx_up));
	this->up = normalize(cross(this->right, this->direction));
}

void Camera::SetRenderTargetSize(short width, short height)
{
	this->width = width;
	this->height = height;
}

Ray Camera::GetCameraRay(short x, short y) const
{
	float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	float u = (2.f * (x + .5f) / static_cast<float>(width) - 1.f)* aspect_ratio;
	float v = (2.f * (y + .5f) / static_cast<float>(height) - 1.f);

	float3 direction = this->direction + (u * this->right) - (v * this->up);
	return Ray(this->position, direction);
}

Ray Camera::GetCameraRay(short x, short y, float3 jitter) const
{
	return Ray({ 1,1,1 }, { 0,0,0 });
}
