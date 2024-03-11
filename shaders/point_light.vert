#version 450

// These 6 vertices construct a square that represents where the light gets drawn
const vec3 OFFSETS[6] = vec3[](
	vec3(-1.0, 1.0, 0.0),
	vec3(-1.0, -1.0, 0.0),
	vec3(1.0, -1.0, 0.0),
	vec3(-1.0, 1.0, 0.0),
	vec3(1.0, -1.0, 0.0),
	vec3(1.0, 1.0, 0.0)
);

struct PointLight {
	vec3 position;
	vec4 color; // w is intensity, not alpha
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionMatrix;
	mat4 cameraMatrix;
	mat4 invCameraMatrix; 
	vec3 ambientLightColor;
	int numLights;
	PointLight pointLights[10];
} ubo;

layout(push_constant) uniform Push {
	vec3 position;
	vec4 color; // w is intensity, not alpha
	float radius;
} push;

layout(location = 0) out vec2 fragOffset;

void main() {
	vec3 offset = OFFSETS[gl_VertexIndex];
	fragOffset = 2.0 * offset.xy;
	vec3 cameraRightWorld = {ubo.cameraMatrix[0][0], ubo.cameraMatrix[1][0], ubo.cameraMatrix[2][0]};
	vec3 cameraUpWorld = {ubo.cameraMatrix[0][1], ubo.cameraMatrix[1][1], ubo.cameraMatrix[2][1]};

	vec3 positionWorld = push.position
		+ offset.x * 2.0 * push.radius * cameraRightWorld
		+ offset.y * 2.0 * push.radius * cameraUpWorld;

	gl_Position = ubo.projectionMatrix * ubo.cameraMatrix * vec4(positionWorld, 1.0);
}
