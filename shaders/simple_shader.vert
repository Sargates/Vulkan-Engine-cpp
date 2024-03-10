#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;

layout (set=0, binding=0) uniform GlobalUbo {
	mat4 projectionViewMatrix;
	vec4 ambientLightColor;
	vec3 lightPosition;
	vec4 lightColor;
} ubo;


layout (push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;


const float AMBIENT_LIGHT_LEVEL = .01;


void main() {
	vec4 positionWorldSpace = push.modelMatrix * vec4(position, 1.f);
	gl_Position = ubo.projectionViewMatrix * positionWorldSpace;

	vec3 normalInWorldSpace = normalize(mat3(push.normalMatrix) * normal);

	vec3 differencePositionLight = ubo.lightPosition - positionWorldSpace.xyz;
	vec3 lightDirection = normalize(differencePositionLight);
	float attenuation = 1.0f / dot(differencePositionLight, differencePositionLight);

	// vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation; // I don't like the attenuation effect, need to look into better solutions
	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w;
	vec3 ambientColor = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;


	float lightIntensity = max(dot(normalInWorldSpace, lightDirection), 0);
	vec3 diffuseLight = lightColor * lightIntensity;
	
	fragColor = (diffuseLight + ambientColor) * color;
}