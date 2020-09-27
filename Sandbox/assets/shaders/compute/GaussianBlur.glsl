#type compute
#version 430

layout(local_size_x = 1) in;
layout(binding = 0, rgba32f) writeonly uniform image2D OUT;

uniform sampler2D IN;
uniform float N;
uniform int direction;

void main()
{
	const float kernel[7] = float[7](0.00598, 0.060626, 0.241843, 0.383103, 0.241843, 0.060626, 0.00598);
	vec2 fidx = vec2(gl_GlobalInvocationID.xy) / N;
	float pxs = 1.0 / N;
	float newVal = 0.0;
	float weightTotal = 0.0;
	if (direction == 0)
	{	
		for (int i = -3; i<=3; i++)
		{
			vec2 currIdx = fidx + vec2(pxs * i, 0.0);
			weightTotal += kernel[i + 3];
			newVal += kernel[i + 3] * texture(IN, currIdx).g;
		}
		imageStore(OUT, ivec2(gl_GlobalInvocationID.xy), vec4(0.0, newVal, 0.0, 1.0));
	}
	else
	{
		for (int i = -3; i<=3; i++)
		{
			vec2 currIdx = fidx + vec2(0.0, pxs * i);
			weightTotal += kernel[i + 3];
			newVal += kernel[i + 3] * texture(IN, currIdx).g;
		}
		imageStore(OUT, ivec2(gl_GlobalInvocationID.xy), vec4(0.0, newVal, 0.0, 1.0));
	}
}