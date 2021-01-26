#type vertex
#version 330 core

layout (location = 0) in vec3 position;

out vec3 f_UV;

uniform mat4 u_ViewProjectionMatrix;

void main()
{
    f_UV = position;
    vec4 _pos = u_ViewProjectionMatrix * vec4(position, 1.0);
    gl_Position = _pos.xyww;
}  

#type fragment
#version 330 core

out vec4 fragColor;

in vec3 f_UV;

uniform samplerCube skybox;

void main()
{    
    fragColor = texture(skybox, f_UV);
}