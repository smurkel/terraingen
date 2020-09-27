#type compute
#version 430 core
#define M_PI 3.1415926535897932384626433832795

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) writeonly uniform image2D h0_a;
layout(binding = 1, rgba32f) writeonly uniform image2D h0_b;

uniform sampler2D noise_r0; 
uniform sampler2D noise_i0; 
uniform sampler2D noise_r1; 
uniform sampler2D noise_i1;

uniform int N; 
uniform int L; 
uniform float A; 
uniform vec2 _windDirection; 
uniform int windexponent;

const float windspeed = length(_windDirection);
const vec2 windDirection = normalize(_windDirection);

const float g = 9.81;

// Box-Muller-Method 
vec4 gaussRND()
{ 
	vec2 texCoord = vec2(gl_GlobalInvocationID.xy) / float(N);

	float noise00 = clamp(texture(noise_r0, texCoord).r,	0.001,	1.0); 
	float noise01 = clamp(texture(noise_i0, texCoord).r,	0.001,	1.0); 
	float noise02 = clamp(texture(noise_r1, texCoord).r,	0.001,	1.0); 
	float noise03 = clamp(texture(noise_i1, texCoord).r,	0.001,	1.0);

	float u0 = 2.0 * M_PI * noise00;
	float v0 = sqrt(-2.0 * log(noise01));
	float u1 = 2.0 * M_PI * noise02;
	float v1 = sqrt(-2.0 * log(noise03));

	vec4 rnd = vec4(v0 * cos(u0), v0 * sin(u0), v1 * cos(u1), v1 * sin(u1));
	return rnd;
}

void main(void)
{
	vec2 x = vec2(gl_GlobalInvocationID.xy) - float(N) / 2.0;
	vec2 k = vec2(2.0 * M_PI * x.x/L, 2.0 * M_PI * x.y/L);

	float L_ = (windspeed * windspeed) / g;
	float mag = length(k);
	if (mag < 0.00001) mag = 0.00001;
	float magSq = mag * mag;

	float h0k = sqrt(
				(A / (magSq * magSq * 2))
				* pow(abs(dot(normalize(k), windDirection)), windexponent)
				* exp(-(1.0 / (magSq * L_ * L_)))
				* exp(-magSq*pow(L/5000.0,2.0))
				);

	float h0minusk = sqrt(
				(A / (magSq * magSq * 2))
				* pow(abs(dot(normalize(-k), windDirection)), windexponent)
				* exp(-(1.0 / (magSq * L_ * L_)))
				* exp(-magSq*pow(L/5000.0,2.0))
				);

	h0k = clamp(h0k, -8000, 8000);
	h0minusk = clamp(h0minusk, -8000, 8000);

	if (mag < 0.0001)
	{
		h0k = 0;
		h0minusk = 0;
	}

	vec4 gauss_random = gaussRND();
	imageStore(h0_a, ivec2(gl_GlobalInvocationID.xy), vec4(gauss_random.xy * h0k, 0, 1));
	imageStore(h0_b, ivec2(gl_GlobalInvocationID.xy), vec4(gauss_random.zw * h0minusk, 0, 1));
}
							
