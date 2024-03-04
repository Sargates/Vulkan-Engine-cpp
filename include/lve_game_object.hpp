#pragma once

#include "lve_model.hpp"

#include <memory>

namespace lve {

	struct Transform2DComponent {
		glm::vec2 translation{};
		glm::vec2 scale{1.f, 1.f};
		float rotation = 0.f;
		glm::mat2 getMatrix() {
			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);
			glm::mat2 scaleMat {{scale.x, .0f}, {.0f, scale.y}}; // glm::mat2 takes columns not rows
			glm::mat2 rotMat {{c, s}, {-s, c}};
			return rotMat * scaleMat;
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
			Transform2DComponent transform2D{};

		private:
			id_t id;
			LveGameObject(id_t objId) : id(objId) {}
	};
}