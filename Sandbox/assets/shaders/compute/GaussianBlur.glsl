#type compute
#version 430

layout(local_size_x = 1) in;
layout(binding = 0, rgba32f) writeonly uniform image2D OUT;

uniform sampler2D IN;
uniform float N;
uniform int direction;

void main()
{
	const int _kernelwidth = 3;
	//const float kernel[1 + 2 * _kernelwidth] = float[1 + 2 * _kernelwidth](0.27901, 0.44198, 0.27901);
	const float kernel[1 + 2 * _kernelwidth] = float[1 + 2 * _kernelwidth](0.00598, 0.060626, 0.241843, 0.383103, 0.241843, 0.060626, 0.00598);
	vec2 fidx = vec2(gl_GlobalInvocationID.xy) / N;
	float pxs = 1.0 / N;
	vec4 newVal = vec4(0.0, 0.0, 0.0, 0.0);
	if (direction == 0)
	{	
		for (int i = -_kernelwidth; i<=_kernelwidth; i++)
		{
			vec2 currIdx = fidx + vec2(pxs * i, 0.0);
			newVal += kernel[i + _kernelwidth] * texture(IN, currIdx);
		}
		imageStore(OUT, ivec2(gl_GlobalInvocationID.xy), newVal);
	}
	else
	{
		for (int i = -_kernelwidth; i<=_kernelwidth; i++)
		{
			vec2 currIdx = fidx + vec2(0.0, pxs * i);
			newVal += kernel[i + _kernelwidth] * texture(IN, currIdx);
		}
		imageStore(OUT, ivec2(gl_GlobalInvocationID.xy), newVal);
	}
}