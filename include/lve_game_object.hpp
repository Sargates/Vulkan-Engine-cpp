#pragma once

#include "lve_model.hpp"
#include "transform.hpp"



#include <memory>

namespace lve {

	struct PointLightComponent {
		glm::vec4 color;
	};

	class LveGameObject {
		public:
			using id_t = unsigned int;
  			using Map = std::unordered_map<id_t, LveGameObject>;

			static LveGameObject createGameObject() {
				static id_t currentId = 0;
				return LveGameObject(currentId++);
			}
			static LveGameObject makePointLight(float intensity = 10.f, glm::vec3 color = glm::vec3(1.f)) {
				LveGameObject gameObj = LveGameObject::createGameObject();
				gameObj.pointLight = std::make_unique<PointLightComponent>();
				return gameObj;
			}



			LveGameObject(const LveGameObject&) = delete;
			LveGameObject& operator=(const LveGameObject&) = delete;
			LveGameObject(LveGameObject&&) = default;
			LveGameObject& operator=(LveGameObject&&) = default;

			id_t getId() const { return id; }

			std::shared_ptr<LveModel> model{};
			Transform transform{};

			std::unique_ptr<PointLightComponent> pointLight{};

		private:
			id_t id;
			LveGameObject(id_t objId) : id(objId) {}
	};
}