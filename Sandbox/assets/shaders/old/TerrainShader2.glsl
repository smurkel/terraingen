#type vertex
#version 430

layout(location = 0) in vec2 index;
layout(binding = 0) uniform sampler2D height;
layout(binding = 1) uniform sampler2D erosion;

uniform float p_Gridsize;
uniform float N;
uniform float p_ErosionWeight;
uniform float p_Offset;
uniform float p_HeightScale;

out vec4 vecY;
out vec2 uv;
void main()
{
	vec2 UV = index / N;
	uv = UV;
	vec4 vecH = texture(height, UV);
	vec4 vecE = texture(erosion, UV);
	vecY = vecH + vecE * p_ErosionWeight;
	float X = (UV.r - 0.5) * p_Gridsize;
	float Z = (UV.g - 0.5) * p_Gridsize;
	float Y = vecY.r + vecY.g + vecY.b;
	Y *= p_HeightScale;
	Y += p_Offset;
	gl_Position = vec4(X, Y, Z, 1.0);
}

#type geometry
#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform float N;

uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;

out float f_Illumination;
out float f_Specular;
out float f_Steepness;
out float f_Height;

in vec4 vecY[];
in vec2 uv[];
out vec4 typeHeight;
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
	f_Illumination = clamp(dot(Normal, lightDir), 0.0, 1.0);
	f_Specular = clamp(dot(reflect(lightDir, Normal),-viewDir), 0.0, 1.0);
	float _dy = max(max(V1.y, V2.y), V3.y) - min(min(V1.y, V2.y), V3.y);
	float _dx = max(max(V1.x, V2.x), V3.x) - min(min(V1.x, V2.x), V3.x);
	float _dz = max(max(V1.z, V2.z), V3.z) - min(min(V1.z, V2.z), V3.z);
	f_Steepness = _dy / max(_dx, _dz);
	
	typeHeight = (vecY[0] + vecY[1] + vecY[2]) / 3.0;
	f_Height = V1.y;
	gl_Position = vpMat * gl_in[0].gl_Position;
	f_UV = uv[0];
	EmitVertex();
	f_Height = V2.y;
	gl_Position = vpMat * gl_in[1].gl_Position;
	f_UV = uv[1];
	EmitVertex();
	f_Height = V3.y;
	gl_Position = vpMat * gl_in[2].gl_Position;
	f_UV = uv[2];
	EmitVertex();
}

#type fragment
#version 430

layout(location = 0) out vec4 fragColor;
layout(binding = 2) uniform sampler2D heightMapTexture;
layout(binding = 3) uniform sampler2D humidityMap;

uniform float p_HeightScale;
uniform float p_LushScale;
uniform float p_LushOffset;
uniform float p_TextureOffset;

in float f_Illumination;
in float f_Specular;
in float f_Steepness;
in float f_Height;

in vec4 typeHeight;
in vec2 f_UV;
void main()
{
	vec4 cSand = vec4(1.0,0.83,0.36,1.0);
	vec4 cStone = vec4(0.47,0.40,0.32,1.0);
	vec4 cRock = vec4(0.33,0.33,0.33,1.0);
	
	vec4 rockCol = mix(vec4(0.0, 0.0, 0.0, 1.0), cRock, clamp(typeHeight.r, 0.0, 1.0));
	rockCol = mix(rockCol, cStone, clamp(typeHeight.g / 0.0005, 0.0, 1.0));
	rockCol = mix(rockCol, cSand, clamp(typeHeight.b / 0.0002, 0.0, 1.0));

	float humidity = clamp(texture(humidityMap, f_UV).r, 0.0, 1.0);
	float Brightness = f_Illumination;
	
	fragColor = Brightness * rockCol;
	fragColor.a = 1.0;
}