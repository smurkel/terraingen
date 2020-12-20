#type compute
#version 430

layout(local_size_x = 1) in;
layout(binding = 0, rgba32f) writeonly uniform image2D BlurBuffer;

uniform sampler2D heightmap;
uniform vec2 wind_dir;
uniform float wind_mag;
uniform float p_N;
uniform float fraction;

void main()
{
	imageStore(BlurBuffer, ivec2(gl_GlobalInvocationID.xy), vec4(0.0, 0.0, 0.0, 1.0));
	vec2 pos = vec2(gl_GlobalInvocationID.xy) / p_N;

	vec4 home = texture(heightmap, pos);
	vec4 donor = texture(heightmap, pos + (1.0 / p_N) * wind_dir);
	vec4 acceptor = texture(heightmap, pos - (1.0 / p_N) * wind_dir);

	float H_home = home.r + home.g + home.b + home.a;
	float H_donor = donor.r + donor.g + donor.b + donor.a;
	float H_acceptor = acceptor.r + acceptor.g + acceptor.b + acceptor.a;
	
	// sand spreading home -> donor.
	// ONLY SPREADING TO LOWER PIXELS FOR NOW

	float volume_hd = min(fraction * home.b, (H_home - H_donor) / 2);
	volume_hd = volume_hd * wind_mag;
	float volume_ah = min(fraction * acceptor.b, (H_acceptor - H_home) / 2);
	volume_ah = volume_ah * wind_mag;

	volume_hd = max(volume_hd, 0.0);
	volume_ah = max(volume_ah, 0.0);

	imageStore(BlurBuffer, ivec2(gl_GlobalInvocationID.xy), vec4(0.0, 0.0, volume_ah - volume_hd, 1.0));
}