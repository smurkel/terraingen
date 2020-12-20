#type vertex
#version 430

layout(location = 0) in vec2 uv;
layout(binding = 0) uniform sampler2D heightMap;

uniform mat4 VPMatrix;
uniform mat4 DepthBiasMVP;
uniform float p_N;
uniform float p_Cellsize;
uniform float p_HeightOffset;

out vec4 ShadowCoord;
void main()
{
	vec2 XZ = (uv - 0.5) * p_N * p_Cellsize;
	vec4 Terrain = texture(heightMap, uv);
	float Y = Terrain.r + Terrain.g + Terrain.b + Terrain.a;
	vec4 vertexPosition = vec4(XZ.r, Y - p_HeightOffset, XZ.g, 1.0);
	ShadowCoord = DepthBiasMVP * vertexPosition;
	gl_Position = VPMatrix * vertexPosition;
}

#type fragment
#version 430
layout(location = 0) out vec4 fragColor;
layout(binding = 1) uniform sampler2D shadowMap;

in vec4 ShadowCoord;
void main()
{
	fragColor = vec4(1.0, 0.0, 1.0, 1.0);
	if (texture(shadowMap, ShadowCoord.xy).r < ShadowCoord.z)
	{
		fragColor = vec4(0.0, 1.0, 0.0, 1.0);
	}
}