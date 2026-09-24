// HLSL is the Direct3D language for making vertex and pixel shaders
// Its based on the C language.

// Globals for the Vertex Shader
cbuffer MatrixBuffer {
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

// Input structure for Vertex Shader
struct VertexInputType {
	float4 position : POSITION;
	float4 color : COLOR;
};

// Output structure for Pixel Shader
struct PixelInputType {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

// Vertex Shader
PixelInputType ColorVertexShader(VertexInputType input) {
	PixelInputType output;

	// Fill the W component to make a Point in 3-space
	input.position.w = 1.0f;

	// Project the point from World coordiantes to Screen coordinates
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Pass the input color verbatim
	output.color = input.color;

	// return the color
	return output;
}
