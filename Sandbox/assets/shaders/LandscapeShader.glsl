#type vertex
#version 430

layout(location = 0) in vec2 uv;
layout(binding = 0) uniform sampler2D heightMap;

uniform float p_N;
uniform float p_Cellsize;
uniform float p_HeightOffset;
out vec4 Terrain;
out vec2 g_UV;

void main()
{
	vec2 XZ = (uv - 0.5) * p_N * p_Cellsize;
	Terrain = texture(heightMap, uv);
	float Y = Terrain.r + Terrain.g + Terrain.b + Terrain.a - p_HeightOffset;
	g_UV = uv;
	XZ /= 1000;
	gl_Position = vec4(XZ.r, Y, XZ.g, 1.0);
}

#type geometry
#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;

out float f_Illumination;
out float f_Specular;
out float f_Steepness;

in vec4 Terrain[];
out vec4 f_Terrain;

in vec2 g_UV[];
out vec2 f_UV;

void main()
{
	// calculate normal vector
	mat4 vpMat = u_ProjectionMatrix * u_ViewMatrix;
	vec3 V1 = gl_in[0].gl_Position.xyz;
	vec3 V2 = gl_in[1].gl_Position.xyz;
	vec3 V3 = gl_in[2].gl_Position.xyz;
	vec3 Normal = normalize(cross(V2-V1, V3-V1));
	vec3 P = (V1 + V2 + V3) / 3.0;
	vec3 lightDir = -normalize(u_LightPosition);
	vec3 viewDir = normalize(P - u_ViewPosition);
	f_Illumination = -dot(Normal, lightDir);
	f_Specular = clamp(dot(reflect(lightDir, Normal),viewDir), 0.0, 1.0);
	float _dy = max(max(V1.y, V2.y), V3.y) - min(min(V1.y, V2.y), V3.y);
	float _dx = max(max(V1.x, V2.x), V3.x) - min(min(V1.x, V2.x), V3.x);
	float _dz = max(max(V1.z, V2.z), V3.z) - min(min(V1.z, V2.z), V3.z);
	f_Steepness = _dy / max(_dx, _dz);
	f_UV = (g_UV[0] + g_UV[1] + g_UV[2]) / 3.0;
	gl_Position = vpMat * gl_in[0].gl_Position;
	f_Terrain = Terrain[0];
	EmitVertex();
	gl_Position = vpMat * gl_in[1].gl_Position;
	f_Terrain = Terrain[1];
	EmitVertex();
	gl_Position = vpMat * gl_in[2].gl_Position;
	f_Terrain = Terrain[2];
	EmitVertex();
}

#type fragment
#version 430

layout(location = 0) out vec4 fragColor;
layout(binding = 1) uniform sampler2D moistureMap;

in float f_Illumination;
in float f_Specular;
in float f_Steepness;
in vec4 f_Terrain;
in vec2 f_UV;

void main()
{
	vec4 terrainColorR = vec4(1.0, 0.0, 0.0, 1.0);
	vec4 terrainColorG = vec4(0.0, 1.0, 0.0, 1.0);
	vec4 terrainColorB = vec4(0.0, 0.0, 1.0, 1.0);
	vec4 terrainColorA = vec4(1.0, 1.0, 1.0, 1.0);

	vec4 terrainColor = mix(terrainColorR, terrainColorG, clamp(f_Terrain.g / 0.001, 0.0, 1.0));
	terrainColor = mix(terrainColor, terrainColorB, clamp(f_Terrain.b / 0.001, 0.0, 1.0));
	terrainColor = mix(terrainColor, terrainColorA, clamp(f_Terrain.a / 0.002, 0.0, 1.0));

	//vec4 grassColor = vec4(0.0, 0.4, 0.0, 1.0);

	float Brightness = (0.6 + 2 * f_Illumination + 0.5 * pow(f_Specular, 60)) / 2.0;
	fragColor = Brightness * terrainColor;
	fragColor.a = 1.0;
}