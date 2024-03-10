#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;

layout (set=0, binding=0) uniform GlobalUbo {
	mat4 projectViewMatrix;
	vec3 directionToLight;
} ubo;


layout (push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;


const float AMBIENT_LIGHT_LEVEL = .01;


void main() {
	gl_Position = ubo.projectViewMatrix * push.modelMatrix * vec4(position, 1.f);

	vec3 normalInWorldSpace = normalize(mat3(push.normalMatrix) * normal);


	// Light direction is negated because of a sign issue, `direction` symbolizes the direction
	// the light is coming from, whereas this formula is for the direction TO the light
	float lightIntensity = max(dot(normalInWorldSpace, -ubo.directionToLight), AMBIENT_LIGHT_LEVEL);
	
	fragColor = lightIntensity * color;
}