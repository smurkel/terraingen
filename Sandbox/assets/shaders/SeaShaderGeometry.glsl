#type vertex
#version 330

layout(location = 0) in vec3 Position;

void main() 
{
	gl_Position = vec4(Position, 1.0);
}

#type geometry
#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;

out float Reflection;
out float Righton;
out float Fog;

void main()
{

	// calculate normal vector
	vec3 V1 = gl_in[0].gl_Position.xyz;
	vec3 V2 = gl_in[1].gl_Position.xyz;
	vec3 V3 = gl_in[2].gl_Position.xyz;
	vec3 Normal = normalize(cross(V2-V1, V3-V1));

	vec3 P = ((V1 + V2 + V3) / 3).xyz;
	Reflection = dot(normalize(reflect(P - u_LightPosition, Normal)),normalize(u_ViewPosition - P));
	Righton = dot(-Normal, normalize(P - u_ViewPosition));

	gl_Position = u_ViewMatrix * gl_in[0].gl_Position;	
	Fog = min(-gl_Position.z / 155.0, 1.0);
	gl_Position = u_ProjectionMatrix * gl_Position;
	EmitVertex();

	gl_Position = u_ViewMatrix * gl_in[1].gl_Position;	
	Fog = min(-gl_Position.z / 155.0, 1.0);
	gl_Position = u_ProjectionMatrix * gl_Position;
	EmitVertex();

	gl_Position = u_ViewMatrix * gl_in[2].gl_Position;	
	Fog = min(-gl_Position.z / 155.0, 1.0);
	gl_Position = u_ProjectionMatrix * gl_Position;
	EmitVertex();

}

#type fragment
#version 330

layout(location = 0) out vec4 fragColor;

in float Reflection;
in float Righton;
in float Fog;
uniform vec4 C_Emissive;
uniform vec4 C_Ambient;
uniform vec4 C_Diffuse;
uniform vec4 C_Specular;

void main (void) 
{	
	float F_Emissive = 1.0;
	float F_Ambient = 1.0;
	float F_Diffuse = 1.0;
	float F_Specular = 1.0;

	bool Facing = Reflection > 0.0;
	bool Right = Righton > 0.0;

	fragColor = C_Ambient * F_Ambient +
				C_Diffuse * F_Diffuse * max(Reflection, 0) +
				(Facing ?
					C_Specular * F_Specular * pow(Reflection,10) :
					vec4(0.0, 0.0, 0.0, 0.0)) +
				(Right ?
					C_Emissive * F_Emissive * pow(Righton, 0.5) :
					vec4(0.0, 0.0, 0.0, 0.0));
					
	
	fragColor = (1 - Fog) * fragColor + Fog * vec4(0.5, 0.7, 0.8, 1.0);
	fragColor.a = 1.0;
}