#type vertex
#version 430

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 UV;


uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_ModelMatrix;

out vec2 f_UV;
float CLIPPING_MARGIN_REFLECTION = -1.5;
float CLIPPING_MARGIN_REFRACTION = 1.5;

void main()
{
	gl_ClipDistance[0] = CLIPPING_MARGIN_REFLECTION + Position.y;
	gl_ClipDistance[1] = CLIPPING_MARGIN_REFRACTION - Position.y;
	gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * vec4(Position, 1.0);
	f_UV = UV;
}

#type fragment
#version 430

layout(location = 0) out vec4 fragColor;
layout(location = 0) uniform sampler2D MainTexture;

in vec2 f_UV;

void main (void) 
{	
	fragColor = texture(MainTexture, f_UV);
	fragColor.a = 1.0;
}