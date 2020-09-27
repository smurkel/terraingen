#type compute
#version 430

layout(local_size_x = 256, local_size_y = 1) in;
layout(binding = 0, rgba32f) readonly uniform image2D _X;
layout(binding = 1, rgba32f) readonly uniform image2D _Y;
layout(binding = 2, rgba32f) readonly uniform image2D _Z;

struct probe
{
	vec4 XYZd;
	vec4 NxNyNzV;
};

layout(std430, binding = 3) buffer Probes
{
	probe Floaters[ ];
};


uniform mat3 rb_Rot;
uniform vec3 rb_Pos;
uniform vec3 rb_V;
uniform vec3 rb_W;

uniform float u_L;
uniform int u_N;

const float g = 9.81;
const float water_density = 1.0;
const float floater_height = 0.5;

void main(void)
{
	
	uint idx = gl_LocalInvocationID.x;
	probe floater = Floaters[idx];
	
	float V = floater.NxNyNzV.w;
	float floater_density = floater.XYZd.w;
	vec3 position = rb_Pos + rb_Rot * floater.XYZd.xyz;
	vec3 normal = rb_Rot * floater.NxNyNzV.xyz;

	ivec2 pixelCoords = ivec2(position.xz / u_L + 0.5) * u_N;
	float _xOffset = imageLoad(_X, pixelCoords).r;
	float _zOffset = imageLoad(_Z, pixelCoords).r;
	position.x += _xOffset;
	position.z += _zOffset;

	pixelCoords = ivec2((position.xz / u_L + 0.5) * u_N);
	
	// FLOATER POSITION W.R.T. WATER
	float floater_depth = imageLoad(_Y, pixelCoords).r - position.y;
	float fracSubmerged = floater_depth / floater_height;
	fracSubmerged = clamp(fracSubmerged, 0.0, 1.0);
	vec3 Arm = position.xyz - rb_Pos;

	// COMPUTING FORCES
	// GRAVITY
	float Fgrav = g * V * (water_density * fracSubmerged - floater_density);	
	vec3 Force = vec3(0, Fgrav, 0);
	// DRAG DUE TO ANGULAR VELOCITY
	vec3 Velocity = 1.0 * cross(rb_W, Arm) + 1.0 * rb_V;
	Velocity = Velocity * clamp((0.1 + dot(Velocity, vec3(0.0,-1.0,0.0))), 0.0, 1.0);
	vec3 Drag = -Velocity * V * fracSubmerged * 10;
	Force += Drag;
	vec3 Torque	= cross(Arm, Force);

	Floaters[idx].XYZd.x = Torque.x;
	Floaters[idx].XYZd.y = Torque.y;
	Floaters[idx].XYZd.z = Torque.z;
	Floaters[idx].NxNyNzV.x = Force.x;
	Floaters[idx].NxNyNzV.y = Force.y;
	Floaters[idx].NxNyNzV.z = Force.z;
}
