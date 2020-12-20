#type vertex
#version 430

layout(location = 0) in vec2 uv;
layout(binding = 0) uniform sampler2D heightMap;

uniform mat4 VPMatrix;
uniform float p_N;
uniform float p_Cellsize;
uniform float p_HeightOffset;

void main()
{
	vec2 XZ = (uv - 0.5) * p_N * p_Cellsize;
	vec4 Terrain = texture(heightMap, uv);
	float Y = Terrain.r + Terrain.g + Terrain.b + Terrain.a;
	gl_Position = VPMatrix * vec4(XZ.r, Y - p_HeightOffset, XZ.g, 1.0);
}

#type fragment
#version 430
layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = vec4(gl_FragCoord.z, 0.0, 0.0, 1.0);
}
