#type compute
#version 430

layout(local_size_x = 256, local_size_y = 1) in;
layout(binding = 0) uniform sampler2D _X;
layout(binding = 1) uniform sampler2D _Y;
layout(binding = 2) uniform sampler2D _Z;

struct probe
{
	vec3 position;
	float mass;
	vec4 meta;
};

layout(std430, binding = 3) buffer Probes
{
	probe Floaters[ ];
};

uniform vec3 rb_V;
uniform vec3 rb_W;
uniform vec3 rb_Position;
uniform mat4 rb_ModelMatrix;

uniform float u_Scale;
uniform float u_HeightScale;

const float g = 9.81;
const float water_density = 1.0;
const float floater_height = 0.1;
const float floater_density = 0.8;

void main(void)
{
	uint idx = gl_LocalInvocationID.x;
	probe floater = Floaters[idx];
	
	vec3 position = (rb_ModelMatrix * vec4(floater.position.xyz, 1.0)).xyz;

	vec2 uv = position.xz / u_Scale;
	// Iterate towards the texture coordinates of the vertex that is currently at the floater position;
	float dx = texture(_X, uv).r;
	float dz = texture(_Z, uv).r;
	uv = (position.xz + vec2(dx, dz)) / u_Scale;

	// FLOATER POSITION W.R.T. WATER
	float floater_depth = u_HeightScale * (texture(_Y, uv).r) - position.y;
	float fracSubmerged = clamp(floater_depth / floater_height, 0.0, 1.0);
	vec3 Arm = position - rb_Position;

	// COMPUTING FORCES
		// GRAVITY
	float Fgrav = g * (water_density * fracSubmerged - floater_density);	
	vec3 Force = vec3(0, Fgrav, 0);

		// DRAG DUE TO ANGULAR VELOCITY
	vec3 Velocity = 1.0 * cross(rb_W, Arm) + 1.0 * rb_V;
	Velocity = Velocity * clamp((0.1 + dot(Velocity, vec3(0.0,-1.0,0.0))), 0.0, 1.0);
	vec3 Drag = -Velocity * fracSubmerged;
	Force += Drag;
	vec3 Torque	= cross(Arm, Force);

	Floaters[idx].position.xyz = Force.xyz;
	Floaters[idx].meta.xyz = Torque.xyz;
}
