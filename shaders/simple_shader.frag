#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormal;

layout (location = 0) out vec4 outColor;

struct PointLight {
	vec3 position;
	vec4 color;	// w is intensity, not alpha
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionMatrix;
	mat4 cameraMatrix;
	mat4 invCameraMatrix; 
	vec4 ambientLightColor;
	int numLights;
	PointLight pointLights[10];
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;


void main() {
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 specularLighting = vec3(0);
	vec3 normal = normalize(fragNormal);

	vec3 cameraPosWorld = ubo.invCameraMatrix[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

	for (int i=0; i<ubo.numLights; i++) {
		PointLight light = ubo.pointLights[i];
		
		vec3 lightDirection = normalize(light.position - fragPosWorld);
		float sqDstToLight = dot(light.position - fragPosWorld, light.position - fragPosWorld);
		float attenuation = 1.0 / sqDstToLight;
		float cosAngOfInc = max(dot(normal, normalize(lightDirection)), 0);
		float intensity = light.color.w * attenuation;

		diffuseLight += light.color.xyz * intensity * cosAngOfInc;

		vec3 halfwayAngle = normalize(lightDirection + viewDirection);
		float binnTerm = max(0, dot(halfwayAngle, normal));
		binnTerm = pow(binnTerm, 512.0);

		specularLighting += light.color.xyz * binnTerm * intensity;

	}

	outColor = vec4((diffuseLight + specularLighting) * fragColor, 1.0);
}