#pragma once

#include "lve_model.hpp"

#include "glm/gtc/matrix_transform.hpp"


#include <memory>

namespace lve {

	struct TransformComponent {
		glm::vec3 translation{};
		glm::vec3 scale{1.f, 1.f, 1.f};
		glm::vec3 rotation{};
		glm::mat4 getMatrix() {
			auto transform = glm::translate(glm::mat4{1.f}, translation);
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

	class LveGameObject {
		public:
			using id_t = unsigned int;

			static LveGameObject createGameObject() {
				static id_t currentId = 0;
				return LveGameObject(currentId++);
			}

			LveGameObject(const LveGameObject&) = delete;
			LveGameObject& operator=(const LveGameObject&) = delete;
			LveGameObject(LveGameObject&&) = default;
			LveGameObject& operator=(LveGameObject&&) = default;

			id_t getId() const { return id; }

			std::shared_ptr<LveModel> model{};
			glm::vec3 color{};
			TransformComponent transform{};

		private:
			id_t id;
			LveGameObject(id_t objId) : id(objId) {}
	};
}