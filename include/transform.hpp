#pragma once

#include "math.hpp"

namespace lve {
	struct Transform {
		public: // I don't care that these are public, im not dealing with getters and setters in a personal project

		glm::vec3 rotation;
		glm::vec3 position;
		glm::vec3 scale{1.f, 1.f, 1.f};
		glm::mat4 getLocalToWorld();
		glm::mat4 getWorldToLocal();
		glm::mat3 getNormalMatrix();
		void recalculateBasisDirs();

		
		glm::vec3 forward  {0.f, 0.f, 1.f};	// Do not modify this manually
		glm::vec3 right    {1.f, 0.f, 0.f};	// Do not modify this manually
		glm::vec3 up       {0.f,-1.f, 1.f};	// Do not modify this manually
		glm::vec3 backward = -forward;		// Do not modify this manually
		glm::vec3 left     = -right;		// Do not modify this manually
		glm::vec3 down     = -up;			// Do not modify this manually
	};
}