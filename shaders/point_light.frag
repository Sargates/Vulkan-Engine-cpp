#version 450

layout(location = 0) in vec2 fragOffset;

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
	vec3 position;
	vec4 color;	// w is intensity, not alpha
	float radius;
} push;

void main() {
	// `fragOffset` gets passed from vertex shader
	
	// Using fragment interpolation, calculate distance from center of square
	float alpha = 1.0 - sqrt(dot(fragOffset, fragOffset));
	if (alpha <= 0.0) { discard; } // Discard any pixels with distance >= 1, this will only draw pixels in a circle

	// Set `outColor` based on alpha value, this will cause the circle to fade toward the edges
	outColor = vec4(push.color.xyz * push.color.w, alpha);
}
