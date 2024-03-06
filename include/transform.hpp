#pragma once

#include "math.hpp"

namespace lve {
	struct Transform {
		public: // I don't care that these are public, im not dealing with getters and setters in a personal project

		glm::vec3 rotation;
		glm::vec3 position;
		glm::vec3 scale{1.f, 1.f, 1.f};
		glm::mat4 getLocalToWorldMatrix() {
			recalculateBasisDirs(); // Probably bad to call this automatically, I don't care because ensuring a prior call each time would be annoying
			glm::mat4 transform = glm::translate({1.f}, position);

			// Rotation
			transform[0][0] = right.x;
			transform[0][1] = right.y;
			transform[0][2] = right.z;
			transform[1][0] = up.x;
			transform[1][1] = up.y;
			transform[1][2] = up.z;
			transform[2][0] = forward.x;
			transform[2][1] = forward.y;
			transform[2][2] = forward.z;

			transform = glm::scale(transform, scale); // Scaling
			return transform;
		}

		glm::mat3 getNormalMatrix() {
			// Rotation
			glm::mat3 out{};

			const glm::vec3 invScale = 1.0f / scale;

			out[0][0] = invScale.x * right.x;
			out[0][1] = invScale.x * right.y;
			out[0][2] = invScale.x * right.z;
			out[1][0] = invScale.y * up.x;
			out[1][1] = invScale.y * up.y;
			out[1][2] = invScale.y * up.z;
			out[2][0] = invScale.z * forward.x;
			out[2][1] = invScale.z * forward.y;
			out[2][2] = invScale.z * forward.z;

			return out;

			// return glm::mat3 {
			// 	{
			// 		invScale.x * (c1*c3+s1*s2*s3),
			// 		invScale.x * (c2*s3),
			// 		invScale.x * (c1*s2*s3-c3*s1)
			// 	},
			// 	{
			// 		invScale.y * (c3*s1*s2-c1*s3),
			// 		invScale.y * (c2*c3),
			// 		invScale.y * (c1*c3*s2+s1*s3)
			// 	},
			// 	{
			// 		invScale.z * (c2*s1),
			// 		invScale.z * (-s2),
			// 		invScale.z * (c1*c2)
			// 	},
			// };
		}

		void recalculateBasisDirs() {
			const float c1 = glm::cos(rotation.y); const float s1 = glm::sin(rotation.y);
			const float c2 = glm::cos(rotation.x); const float s2 = glm::sin(rotation.x);
			const float c3 = glm::cos(rotation.z); const float s3 = glm::sin(rotation.z);

			right = {c1*c3+s1*s2*s3, c2*s3, c1*s2*s3-c3*s1 };
			up   = {c3*s1*s2-c1*s3, c2*c3, c1*c3*s2+s1*s3};	// Up and Down are flipped in vulkan because I haven't fixed that yet, changing the variable names is the easiest fix for right now
			forward      = {c2*s1, -s2, c1*c2};
			backward = -forward;
			left     = -right;
			down     = -up;
		}

		
		glm::vec3 forward  {0.f, 0.f, 1.f};	// Do not modify this manually
		glm::vec3 right    {1.f, 0.f, 0.f};	// Do not modify this manually
		glm::vec3 up       {0.f,-1.f, 1.f};	// Do not modify this manually
		glm::vec3 backward = -forward;		// Do not modify this manually
		glm::vec3 left     = -right;		// Do not modify this manually
		glm::vec3 down     = -up;			// Do not modify this manually
	};
}