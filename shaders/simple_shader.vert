#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;


layout (push_constant) uniform Push {
	mat4 transform; // projection * view * model
	mat4 normalMatrix;
} push;


const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, 3.0, -1.0));
const float AMBIENT_LIGHT_LEVEL = 0.01;


void main() {
	gl_Position = push.transform * vec4(position, 1.f);

	// vec3 normalInWorldSpace = normalize(mat3(push.modelMatrix) * normal);

	// mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
	// vec3 normalInWorldSpace = normalize(normalMatrix * normal);

	vec3 normalInWorldSpace = normalize(mat3(push.normalMatrix) * normal);


	float lightIntensity = max(dot(normalInWorldSpace, DIRECTION_TO_LIGHT), AMBIENT_LIGHT_LEVEL);
	
	fragColor = lightIntensity * color;
}