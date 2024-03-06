#pragma once
#include <memory>

#include "transform.hpp"
#include "math.hpp"


namespace lve {
	class LveCamera {
		public:

		Transform transform{};

		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void setPerspectiveProjection(float fovy, float aspect, float near, float far);

		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0, -1, 0});
		void setViewTarget(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0, -1, 0});
		void UpdateView();

		const glm::mat4& getProjection() const { return projectionMatrix; }
		const glm::mat4& getView() const { return viewMatrix; }

		private:
		
		glm::mat4 projectionMatrix{1.f};
		glm::mat4 viewMatrix{1.f};
	};
}