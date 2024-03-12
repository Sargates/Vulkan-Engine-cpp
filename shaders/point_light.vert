#version 450
//! This vertex shader gets run 6 times (see point_light_system.cpp), one for each vertex specified in OFFSETS

// These 6 vertices construct a square in 3 space
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
	// Get vertex offset based on `gl_VertexIndex` variable, getting current vertex on the square
	vec3 offset = OFFSETS[gl_VertexIndex];

	// Calculate the camera's right and up directions in world space
	vec3 cameraRightWorld = {ubo.cameraMatrix[0][0], ubo.cameraMatrix[1][0], ubo.cameraMatrix[2][0]};
	vec3 cameraUpWorld = {ubo.cameraMatrix[0][1], ubo.cameraMatrix[1][1], ubo.cameraMatrix[2][1]};

	// These three lines effectively turn the square generated from `OFFSETS` into a billboard sprite translating
	// each point on the square so that the surface is perpendicular to the camera's forward direction
	vec3 positionWorld = push.position
		+ offset.x * push.radius * cameraRightWorld
		+ offset.y * push.radius * cameraUpWorld;

	// We need to force update of `gl_Position` because we didn't bind a vertex buffer
	// for point lights, so there's no vertex data that's passed to this shader
	
	// This gets the billboard's vertex position in world space
	gl_Position = ubo.projectionMatrix * ubo.cameraMatrix * vec4(positionWorld, 1.0);
	fragOffset = offset.xy; // Pass offset value to frag shader
}
