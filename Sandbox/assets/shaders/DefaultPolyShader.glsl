#type vertex
#version 430

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 UV;

out vec2 v_UV;
void main()
{
	gl_Position = vec4(Position, 1.0);		
	v_UV = UV;
}

#type geometry
#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;
uniform mat4 u_Transform;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;
uniform sampler2D u_Texture; 

out float f_Illumination;
out float f_Specular;
out vec4 f_Color;

in vec2 v_UV[];

void main() 
{
	mat4 vpMat = u_ProjectionMatrix * u_ViewMatrix * u_Transform;
	vec3 V1 = gl_in[0].gl_Position.xyz;
	vec3 V2 = gl_in[1].gl_Position.xyz;
	vec3 V3 = gl_in[2].gl_Position.xyz;
	vec3 Normal = normalize(cross(V2-V1, V3-V1));
	vec3 P = (V1 + V2 + V3) / 3.0;
	vec3 lightDir = normalize(u_LightPosition);
	vec3 viewDir = normalize(P - u_ViewPosition);
	f_Illumination = -dot(Normal, lightDir);
	f_Specular = clamp(dot(reflect(lightDir, Normal),-viewDir), 0.0, 1.0);

	f_Color = texture(u_Texture, (v_UV[0] + v_UV[1] + v_UV[2])/3);
	gl_Position = vpMat * gl_in[0].gl_Position;
	EmitVertex();
	gl_Position = vpMat * gl_in[1].gl_Position;
	EmitVertex();
	gl_Position = vpMat * gl_in[2].gl_Position;
	EmitVertex();

}

#type fragment
#version 430

layout(location = 0) out vec4 fragColor;

in vec4 f_Color;
in float f_Illumination;
in float f_Specular;

void main (void) 
{	
	float Brightness = 2 + 5 * f_Illumination + 1 * f_Specular;
	fragColor = Brightness * f_Color;
	fragColor.a = 1.0;
}