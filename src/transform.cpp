#include "transform.hpp"
#include <cassert>

namespace lve {
	
		glm::mat4 Transform::getLocalToWorld() {
			recalculateBasisDirs(); // Probably bad to call this automatically, I don't care because ensuring a prior call each time would be annoying

			// Translate
			glm::mat4 transform = glm::translate({1.f}, position);

			// Rotate
			transform[0][0] = right.x; transform[1][0] =  up.x; transform[2][0] = forward.x;
			transform[0][1] = right.y; transform[1][1] =  up.y; transform[2][1] = forward.y;
			transform[0][2] = right.z; transform[1][2] =  up.z; transform[2][2] = forward.z;

			// Scale
			transform = glm::scale(transform, scale);

			return transform;
		}
		glm::mat4 Transform::getWorldToLocal() {
			// Order of operations for Local->World is: Translate -> Rotate -> Scale
			// So reverse the order for World->Local: Scale -> Rotate -> Translate

			recalculateBasisDirs(); // Probably bad to call this automatically, I don't care because ensuring a prior call each time would be annoying

			// Scale
			// glm::mat4 transform{1.f};
			assert((scale.x != 0.f && scale.y != 0.f && scale.z != 0.f) && "Component in `scale` vector cannot be 0 when retrieving the worldToLocal matrix");
			glm::mat4 transform = glm::scale({1.f}, 1.f/scale);

			// Rotate
			transform[0][0] =   right.x; transform[1][0] =   right.y; transform[2][0] =   right.z;
			transform[0][1] =      up.x; transform[1][1] =      up.y; transform[2][1] =      up.z;
			transform[0][2] = forward.x; transform[1][2] = forward.y; transform[2][2] = forward.z;

			// Translate
			
			transform[3][0] = -glm::dot(  right, position);
			transform[3][1] = -glm::dot(     up, position);
			transform[3][2] = -glm::dot(forward, position);

			return transform;
		}

		glm::mat3 Transform::getNormalMatrix() {
			// Rotation
			glm::mat3 out{};

			assert((scale.x != 0.f && scale.y != 0.f && scale.z != 0.f) && "Component in `scale` vector cannot be 0 when retrieving the normal matrix");

			const glm::vec3 invScale = 1.0f / scale;

			out[0][0] = invScale.x * right.x; out[1][0] = invScale.x * up.x; out[2][0] = invScale.x * forward.x;
			out[0][1] = invScale.y * right.y; out[1][1] = invScale.y * up.y; out[2][1] = invScale.y * forward.y;
			out[0][2] = invScale.z * right.z; out[1][2] = invScale.z * up.z; out[2][2] = invScale.z * forward.z;

			return out;
		}

		void Transform::recalculateBasisDirs() {
			const float c1 = glm::cos(rotation.y); const float s1 = glm::sin(rotation.y);
			const float c2 = glm::cos(rotation.x); const float s2 = glm::sin(rotation.x);
			const float c3 = glm::cos(rotation.z); const float s3 = glm::sin(rotation.z);

			right    = {c1*c3+s1*s2*s3, c2*s3, c1*s2*s3-c3*s1 };
			up       = {c3*s1*s2-c1*s3, c2*c3, c1*c3*s2+s1*s3};
			forward  = {c2*s1, -s2, c1*c2};
			backward = -forward;
			left     = -right;
			down     = -up;
		}
}