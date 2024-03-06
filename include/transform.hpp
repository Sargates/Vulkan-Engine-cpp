#pragma once

#include "math.hpp"

namespace lve {
	struct Transform {
		public: // I don't care that these are public, im not dealing with getters and setters in a personal project

		glm::vec3 rotation;
		glm::vec3 position;
		glm::vec3 scale{1.f, 1.f, 1.f};
		glm::mat4 getLocalToWorldMatrix() {
			glm::mat4 transform = glm::translate(glm::mat4{1.f}, position);
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

			transform = glm::scale(transform, scale);
			return transform;
		}
	};
}