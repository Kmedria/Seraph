#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "CommonIncludes.h"
#include <array>

struct Vec3 {
	float x = 0, y = 0, z = 0;

	float& operator[](int index) {

		switch (index) {
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			throwError("Tried to access out of vector length", logLevelError);
		}
	}

	Vec3 operator+=(const Vec3& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	Vec3 operator+(const Vec3& rhs) {
		Vec3 temp = { x, y, z };
		temp += rhs;
		return temp;
	}

	Vec3 operator-(const Vec3& rhs) {
		Vec3 temp = -rhs;
		return *this + temp;
	}

	Vec3 operator-() const {
		Vec3 temp = { -x, -y, -z };
		return temp;
	}

	static Vec3 cross(const Vec3& lhs, const Vec3& rhs) {
		Vec3 temp = { lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x };
		return temp;
	}

	static float dot(const Vec3& lhs, const Vec3 rhs) {
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
	}

	float abs() const {
		return sqrtf(x * x + y * y + z * z);
	}

};

Vec3 operator*(double scale, Vec3& rhs);
Vec3 operator/(Vec3& lhs, double scale);
Vec3& operator/=(Vec3& lhs, double scale);

struct Vertex {
	Vec3 pos;
	float color[4];

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};