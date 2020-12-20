#type compute
#version 430 core
#define PI 3.1415926535897932384626433832795

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D erosionMap;

uniform sampler2D height;
uniform sampler2D xzNoise;

uniform float N;
uniform float VELOCITY;
uniform float VOLUME;
uniform float INERTIA;
uniform float CAPACITY;
uniform float EROSION;
uniform float GRAVITY;
uniform float EVAPORATION;
uniform float DEPOSITION;
uniform float MINSLOPE;
uniform int INVERTEROSION;
uniform int MAXSTEPS;
uniform float WATERLEVEL;

int RADIUS = 15;

vec2 localGradient(vec2 pos)
{
	int x = int(floor(pos.r));
	int z = int(floor(pos.g));
	float u = pos.r - x;
	float v = pos.g - z;

	float tl = texture(height, vec2(x, z) / N).y + imageLoad(erosionMap, ivec2(x, z)).y;
	float tr = texture(height, vec2(x + 1, z) / N).y + imageLoad(erosionMap, ivec2(x + 1, z)).y;
	float bl = texture(height, vec2(x, z + 1) / N).y + imageLoad(erosionMap, ivec2(x, z + 1)).y;
	float br = texture(height, vec2(x + 1, z + 1) / N).y + imageLoad(erosionMap, ivec2(x + 1, z + 1)).y;

	vec2 gradient = vec2((tr-tl)*(1-v) + (br-bl)*v, (bl-tl)*(1-u) + (br-tr)*u);
	return gradient;
}

float localHeight(vec2 pos)
{
	int x = int(floor(pos.r));
	int z = int(floor(pos.g));
	float u = pos.r - x;
	float v = pos.g - z;

	float tl = texture(height, vec2(x, z) / N).y + imageLoad(erosionMap, ivec2(x, z)).y;// - 0.5;
	float tr = texture(height, vec2(x + 1, z) / N).y + imageLoad(erosionMap, ivec2(x + 1, z)).y;// - 0.5;
	float bl = texture(height, vec2(x, z + 1) / N).y + imageLoad(erosionMap, ivec2(x, z + 1)).y;// - 0.5;
	float br = texture(height, vec2(x + 1, z + 1) / N).y + imageLoad(erosionMap, ivec2(x + 1, z + 1)).y;// - 0.5;

	float h = (1 - u) * (1 - v) * tl + u * (1 - v) * tr + (1 - u) * v * bl + u * v * br;
	return h;
}
vec2 randomVector(vec2 idx)
{
	float theta = texture(xzNoise, idx).b * PI;
	return vec2(sin(theta), cos(theta));
}

void erosionMapPixelAdd(ivec2 pxCoord, float val)
{
	vec4 _px = imageLoad(erosionMap, pxCoord);
	imageStore(erosionMap, pxCoord, _px + vec4(0.0, val, 0.0, 0.0));
}
void deposit(float toDrop, vec2 oldPos)
{
	int x0 = int(floor(oldPos.r));
	int z0 = int(floor(oldPos.g));
	int x1 = x0 + 1;
	int z1 = z0 + 1;
	float u = oldPos.r - x0;
	float v = oldPos.g - z0;

	float w00 = (1 - u) * (1 - v);
	float w10 = u * (1 - v);
	float w01 = v * (1 - u);
	float w11 = u * v;

	erosionMapPixelAdd(ivec2(x0, z0), toDrop * w00);
	erosionMapPixelAdd(ivec2(x1, z0), toDrop * w10);
	erosionMapPixelAdd(ivec2(x0, z1), toDrop * w01);
	erosionMapPixelAdd(ivec2(x1, z1), toDrop * w11);
}

void erode(float toTake, vec2 oldPos)
{
	int x = int(round(oldPos.r));
	int z = int(round(oldPos.g));
	float u = oldPos.r - floor(oldPos.r);
	float v = oldPos.g - floor(oldPos.g);
	// get weight normalisation factor:
	float wfac = 0;
	for (float i = -RADIUS; i < RADIUS; i++)
	{
		for (float j = -RADIUS; j < RADIUS; j++)
		{
			float r2 = (pow(i + u, 2) + pow(j + v, 2));
			if ((r2) < pow(RADIUS,2))
				wfac += RADIUS - sqrt(r2);
		}
	}
	for (float i = -RADIUS; i<RADIUS;i++)
	{
		for (float j = -RADIUS; j<RADIUS;j++)
		{
			float r2 = (pow(i + u, 2) + pow(j + v, 2));
			if ((x + i) <= N && (x + i) > 0 && (z + j) <= N && (z + j) > 0)
			{
				if ((r2) < pow(RADIUS,2))
				{
					float w = (RADIUS - sqrt(r2)) / wfac;
					erosionMapPixelAdd(ivec2(x + i, z + j), -toTake * w);
				}
			}
		}
	}
}

void main(void)
{
	float vel = VELOCITY;
	float vol = VOLUME;
	float ine = INERTIA;
	float cap = CAPACITY;
	float ero = EROSION;
	float gra = GRAVITY;
	float eva = EVAPORATION;
	float dep = DEPOSITION;
	float sed = 0.0;
	int steps = 0;
	vec2 dir = vec2(0.0,0.0);

	
	// MAYBE DROPLETS ONLY SPAWN IN X,Y = [0, 1] RANGE? DEPENDS ON HOW OPENGL SCALES THE FLOATS IN THE IMAGE...
	vec2 idx = vec2((gl_GlobalInvocationID.xy) / N);
	vec2 pos = texture(xzNoise, idx).rg * N;
	float h = localHeight(pos);

	// droplet loop
	while (true)
	{
		vec2 gradient = localGradient(pos);
		dir = dir * ine - gradient * (1 - ine);
		if (length(dir)==0.0)
		{
			dir = randomVector(idx);
		}
		dir = dir / length(dir);
		vec2 oldPos = vec2(pos.r, pos.g);
		pos += dir;

		// POSSIBLE BREAK 1
		if ((pos.x < 0) || (pos.x > N) || (pos.y < 0) || (pos.y > N))
			break;
		
		float hOld = h;
		float hDiff = localHeight(pos) - hOld;
		h = localHeight(pos);
		if (hDiff > 0.0)
		{
			float toDrop = min(hDiff, sed);
			deposit(toDrop, oldPos);
			sed = max(sed - toDrop, 0);
		}
		else
		{
			cap = max(-hDiff, MINSLOPE) * vel * vol * cap;
			if (cap > sed)
			{
				float toTake = min((cap - sed) * EROSION, -hDiff);
				erode(2 * (0.5 - INVERTEROSION) * toTake, oldPos);
				sed += toTake;
			}
			else
			{
				float toDrop = (sed - cap) * DEPOSITION;
				if (h < WATERLEVEL)
				{
					toDrop = (sed - cap) * 0.7;
				}
				deposit(toDrop, oldPos);
				sed = max(sed - toDrop, 0);
			}
		}

		vel = sqrt(max(pow(vel,2) + hDiff*GRAVITY, 0));
		vol = vol * (1 - EVAPORATION);
		steps++;
		// POSSIBLE BREAK 2
		if (steps >= MAXSTEPS)
			break;
	}
}