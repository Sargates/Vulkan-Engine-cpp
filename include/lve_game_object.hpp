#pragma once

#include "lve_model.hpp"
#include "transform.hpp"



#include <memory>

namespace lve {


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
			Transform transform{};

		private:
			id_t id;
			LveGameObject(id_t objId) : id(objId) {}
	};
}