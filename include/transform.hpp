#pragma once

#include "math.hpp"

namespace lve {
	struct Transform {
		public: // I don't care that these are public, im not dealing with getters and setters in a personal project

		glm::vec3 rotation;
		glm::vec3 position;
		glm::vec3 scale{1.f, 1.f, 1.f};
		glm::mat4 getLocalToWorldMatrix() {
			glm::mat4 transform = glm::translate(glm::mat4{1.f}, position); // Translation


			// Rotation
			float c1, s1, c2, s2, c3, s3;
			c1 = glm::cos(rotation.y); s1 = glm::sin(rotation.y);
			c2 = glm::cos(rotation.x); s2 = glm::sin(rotation.x);
			c3 = glm::cos(rotation.z); s3 = glm::sin(rotation.z);
			transform[0][0] = c1*c3+s1*s2*s3;
			transform[0][1] = c2*s3;
			transform[0][2] = c1*s2*s3-c3*s1;
			transform[1][0] = c3*s1*s2-c1*s3;
			transform[1][1] = c2*c3;
			transform[1][2] = c1*c3*s2+s1*s3;
			transform[2][0] = c2*s1;
			transform[2][1] = -s2;
			transform[2][2] = c1*c2;

			transform = glm::scale(transform, scale); // Scaling
			return transform;
		}

		glm::mat3 getNormalMatrix() {
			glm::mat3 transform{0.f}; // Translation
			// Rotation
			const float c1 = glm::cos(rotation.y); const float s1 = glm::sin(rotation.y);
			const float c2 = glm::cos(rotation.x); const float s2 = glm::sin(rotation.x);
			const float c3 = glm::cos(rotation.z); const float s3 = glm::sin(rotation.z);
			
			const glm::vec3 invScale = 1.0f / scale;

			return glm::mat3 {
				{
					invScale.x * (c1*c3+s1*s2*s3),
					invScale.x * (c2*s3),
					invScale.x * (c1*s2*s3-c3*s1)
				},
				{
					invScale.y * (c3*s1*s2-c1*s3),
					invScale.y * (c2*c3),
					invScale.y * (c1*c3*s2+s1*s3)
				},
				{
					invScale.z * (c2*s1),
					invScale.z * (-s2),
					invScale.z * (c1*c2)
				},
			};
		}
	};
}