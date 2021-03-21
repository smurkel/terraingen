#type vertex
#version 330

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;

uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;

out float Reflection;
out float Righton;
out float Fog;

void main()
{
	gl_Position = u_ViewMatrix * vec4(Position, 1.0);
	Fog = min(-gl_Position.z / 100.0, 1.0);
	Reflection = dot(normalize(reflect(Position - u_LightPosition, Normal)), normalize(u_ViewPosition - Position));
	Righton = dot(-Normal, normalize(Position - u_ViewPosition));
	gl_Position = u_ProjectionMatrix * gl_Position;
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
uniform float F_Emissive;
uniform float F_Ambient;
uniform float F_Diffuse;
uniform float F_Specular;

void main(void)
{
	bool Facing = Reflection > 0.0;
	bool Right = Righton > 0.0;

	fragColor = C_Ambient * F_Ambient +
				C_Diffuse * F_Diffuse * max(Reflection, 0) +
				(Facing ?
					C_Specular * F_Specular * pow(Reflection, 10) :
					vec4(0.0, 0.0, 0.0, 0.0)) +
				(Right ?
					C_Emissive * F_Emissive * pow(Righton, 0.5) :
					vec4(0.0, 0.0, 0.0, 0.0));


	fragColor = (1 - Fog) * fragColor + Fog * vec4(0.5, 0.7, 0.8, 1.0);
	fragColor.a = 1.0;
}