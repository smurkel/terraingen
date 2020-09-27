#type vertex
#version 430

layout(location = 0) in vec2 index;
layout(binding = 0) uniform sampler2D height;
layout(binding = 1) uniform sampler2D erosion;


uniform float N;
uniform float p_ErosionWeight;
uniform float p_Offset;
uniform float p_HeightScale;
void main()
{
	vec2 UV = index / N;
	vec3 xyz = texture(height, UV).rgb;
	float dy = texture(erosion, UV).g;
	xyz.y += dy * p_ErosionWeight;
	xyz.y *= p_HeightScale;
	xyz.y += p_Offset;
	gl_Position = vec4(xyz, 1.0);
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
out vec2 f_XZ;

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
	f_Specular = clamp(dot(reflect(lightDir, Normal),-viewDir), 0.0, 1.0);
	float _dy = max(max(V1.y, V2.y), V3.y) - min(min(V1.y, V2.y), V3.y);
	float _dx = max(max(V1.x, V2.x), V3.x) - min(min(V1.x, V2.x), V3.x);
	float _dz = max(max(V1.z, V2.z), V3.z) - min(min(V1.z, V2.z), V3.z);
	f_Steepness = _dy / max(_dx, _dz);
	
	f_Height = V1.y;
	f_XZ = V1.xz;
	gl_Position = vpMat * gl_in[0].gl_Position;
	EmitVertex();
	f_Height = V2.y;
	f_XZ = V2.xz;
	gl_Position = vpMat * gl_in[1].gl_Position;
	EmitVertex();
	f_Height = V3.y;
	f_XZ = V3.xz;
	gl_Position = vpMat * gl_in[2].gl_Position;
	EmitVertex();
}

#type fragment
#version 430

layout(location = 0) out vec4 fragColor;
layout(binding = 2) uniform sampler2D heightMapTexture;

uniform float p_HeightScale;
uniform float p_LushScale;
uniform float p_LushOffset;
uniform float p_TextureOffset;

in float f_Illumination;
in float f_Specular;
in float f_Steepness;
in float f_Height;

in vec2 f_XZ;
void main()
{
	float remapHeight = clamp(((f_Height - p_TextureOffset) / p_HeightScale + 3) / 6, 0.01, 0.99);

	vec4 grassColor = texture(heightMapTexture, vec2(remapHeight, 0.6));
	vec4 rockColor = texture(heightMapTexture, vec2(remapHeight, 0.9));
	float specularFac = texture(heightMapTexture, vec2(remapHeight, 0.3)).r;

	float Brightness = (0.6 + 2 * f_Illumination + 2 * pow(f_Specular, 10) * specularFac) / 2.5;

	fragColor = Brightness * mix(grassColor, rockColor, clamp(pow((f_Steepness * p_LushScale + p_LushOffset),2), 0.0, 1.0));
	fragColor.a = 1.0;
}