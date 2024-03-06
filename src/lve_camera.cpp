#include "lve_camera.hpp"

#include <cassert>
#include <limits>


namespace lve {
	void LveCamera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
		projectionMatrix = glm::mat4{1.f};
		projectionMatrix[0][0] = 2.f / (right-left);
		projectionMatrix[1][1] = 2.f / (bottom-top);
		projectionMatrix[2][2] = 1.f / (far - near);
		projectionMatrix[3][0] = -(right + left) / (right - left);
		projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		projectionMatrix[3][2] = - near / (far - near);
	}

	void LveCamera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
		assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.f);
		const float tanHalfFovY = tan(fovy/2.f);
		projectionMatrix = glm::mat4{0.f};
		projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovY);
		projectionMatrix[1][1] = 1.f / (tanHalfFovY);
		projectionMatrix[2][2] = far / (far-near);
		projectionMatrix[2][3] = 1.f;
		projectionMatrix[3][2] = -(far*near) / (far - near);
	}

	void LveCamera::UpdateView() {
		transform.recalculateBasisDirs();
		
		viewMatrix = glm::mat4{1.f};
		viewMatrix[0][0] = transform.right.x;
		viewMatrix[1][0] = transform.right.y;
		viewMatrix[2][0] = transform.right.z;
		viewMatrix[0][1] = transform.up.x;
		viewMatrix[1][1] = transform.up.y;
		viewMatrix[2][1] = transform.up.z;
		viewMatrix[0][2] = transform.forward.x;
		viewMatrix[1][2] = transform.forward.y;
		viewMatrix[2][2] = transform.forward.z;
		viewMatrix[3][0] = -glm::dot(transform.right, transform.position);
		viewMatrix[3][1] = -glm::dot(transform.up, transform.position);
		viewMatrix[3][2] = -glm::dot(transform.forward, transform.position);

		// viewMatrix = glm::scale(viewMatrix, transform.scale); // Scaling
	}

}