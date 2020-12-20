#type compute
#version 430 core
#define PI 3.1415926535897932384626433832795

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D erosionMap;
layout(binding = 1, rgba32f) uniform image2D humidityMap;

uniform sampler2D height;
uniform sampler2D xzNoise;

uniform float N;
uniform float VELOCITY;
uniform float VOLUME;
uniform float INERTIA;
uniform float CAPACITY;
uniform float EROSION;
uniform vec3 HARDNESS;
uniform float GRAVITY;
uniform float EVAPORATION;
uniform float DEPOSITION;
uniform float MINSLOPE;
uniform int INVERTEROSION;
uniform int MAXSTEPS;
uniform float WATERLEVEL;

int RADIUS = 5;

vec2 localGradient(vec2 pos)
{
	int x = int(floor(pos.r));
	int z = int(floor(pos.g));
	float u = pos.r - x;
	float v = pos.g - z;

	vec3 tl = texture(height, vec2(x, z) / N).rgb + imageLoad(erosionMap, ivec2(x, z)).rgb;
	vec3 tr = texture(height, vec2(x + 1, z) / N).rgb + imageLoad(erosionMap, ivec2(x + 1, z)).rgb;
	vec3 bl = texture(height, vec2(x, z + 1) / N).rgb + imageLoad(erosionMap, ivec2(x, z + 1)).rgb;
	vec3 br = texture(height, vec2(x + 1, z + 1) / N).rgb + imageLoad(erosionMap, ivec2(x + 1, z + 1)).rgb;
	float TL = tl.x + tl.y + tl.z;
	float TR = tr.x + tr.y + tr.z;
	float BL = bl.x + bl.y + bl.z;
	float BR = br.x + br.y + br.z;

	vec2 gradient = vec2((TR-TL)*(1-v) + (BR-BL)*v, (BL-TL)*(1-u) + (BR-TR)*u);
	return gradient;
}

vec3 localHeight(vec2 pos)
{
	int x = int(floor(pos.r));
	int z = int(floor(pos.g));
	float u = pos.r - x;
	float v = pos.g - z;

	vec3 tl = texture(height, vec2(x, z) / N).rgb + imageLoad(erosionMap, ivec2(x, z)).rgb;
	vec3 tr = texture(height, vec2(x + 1, z) / N).rgb + imageLoad(erosionMap, ivec2(x + 1, z)).rgb;
	vec3 bl = texture(height, vec2(x, z + 1) / N).rgb + imageLoad(erosionMap, ivec2(x, z + 1)).rgb;
	vec3 br = texture(height, vec2(x + 1, z + 1) / N).rgb + imageLoad(erosionMap, ivec2(x + 1, z + 1)).rgb;

	vec3 h = (1 - u) * (1 - v) * tl + u * (1 - v) * tr + (1 - u) * v * bl + u * v * br;
	return h;
}

vec2 randomVector(vec2 idx)
{
	float theta = texture(xzNoise, idx).b * 2 * PI;
	return vec2(sin(theta), cos(theta));
}

void erosionMapPixelAdd(ivec2 pxCoord, vec3 val)
{
	vec4 _px = imageLoad(erosionMap, pxCoord);
	imageStore(erosionMap, pxCoord, vec4(_px.rgb + val, 1.0));
}
vec3 deposit(float toDrop, vec2 oldPos, vec3 SED)
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

	erosionMapPixelAdd(ivec2(x0, z0), toDrop * w00 * SED);
	erosionMapPixelAdd(ivec2(x1, z0), toDrop * w10 * SED);
	erosionMapPixelAdd(ivec2(x0, z1), toDrop * w01 * SED);
	erosionMapPixelAdd(ivec2(x1, z1), toDrop * w11 * SED);

	return max(SED * (1 - toDrop), vec3(0.0,0.0,0.0));
}


vec3 erodePixel(ivec2 pixel, float toTake)
{
	int x = pixel.r;
	int z = pixel.g;
	vec3 locheight = texture(height, vec2(x, z) / N).rgb + imageLoad(erosionMap, ivec2(x, z)).rgb;
	vec3 memory = vec3(locheight);
	vec3 taken = vec3(0.0, 0.0, 0.0);

	float damage = toTake;

	taken.b += min(damage / HARDNESS.b, locheight.b);
	damage -= taken.b * HARDNESS.b;
	taken.g += min(damage / HARDNESS.g, locheight.g);
	damage -= taken.g * HARDNESS.g;
	taken.r += min(damage / HARDNESS.r, locheight.r);
	damage -= taken.r * HARDNESS.r;

	erosionMapPixelAdd(pixel, -taken);
	// downgrade rock types:
	taken = vec3(taken.r * 0.75, taken.r * 0.25 + taken.g * 0.75, taken.g * 0.25 + taken.b);
	
	return taken;
}

vec3 erode(float toTake, vec2 oldPos)
{
	vec3 newlyTaken = vec3(0.0, 0.0, 0.0);
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
					vec3 tempTaken = erodePixel(ivec2(x+i, z+j), toTake * w);
					newlyTaken += tempTaken;
				}
			}
		}
	}
	return newlyTaken;
}

void addHumidity(vec2 oldPos, float volume)
{
	int x0 = int(floor(oldPos.r));
	int z0 = int(floor(oldPos.g));
	int x1 = x0 + 1;
	int z1 = z0 + 1;
	float u = oldPos.r - x0;
	float v = oldPos.g - z0;

	float w00 = (1 - u) * (1 - v) / 100.0;
	float w10 = u * (1 - v) / 100.0;
	float w01 = v * (1 - u) / 100.0;
	float w11 = u * v / 100.0;

	vec4 _pxd = imageLoad(humidityMap, ivec2(x0, z0)) + vec4(volume * w00, 0.0, 0.0, 0.0);
	_pxd.a = 1.0;
	imageStore(humidityMap, ivec2(x0, z0), _pxd);
	_pxd = imageLoad(humidityMap, ivec2(x1, z0)) + vec4(volume * w10, 0.0, 0.0, 0.0);
	_pxd.a = 1.0;
	imageStore(humidityMap, ivec2(x1, z0), _pxd);
	_pxd = imageLoad(humidityMap, ivec2(x0, z1)) + vec4(volume * w01, 0.0, 0.0, 0.0);
	_pxd.a = 1.0;
	imageStore(humidityMap, ivec2(x0, z1), _pxd);
	_pxd = imageLoad(humidityMap, ivec2(x1, z1)) + vec4(volume * w11, 0.0, 0.0, 0.0);
	_pxd.a = 1.0;
	imageStore(humidityMap, ivec2(x1, z1), _pxd);
}

void main(void)
{
	float vel = VELOCITY;
	float vol = VOLUME;
	float ine = INERTIA;
	float cap = CAPACITY;
	float gra = GRAVITY;
	vec3 sed = vec3(0.0, 0.0, 0.0);
	int steps = 0;
	vec2 dir = vec2(0.0,0.0);

	
	
	vec2 idx = vec2((gl_GlobalInvocationID.xy) / N);
	vec2 pos = texture(xzNoise, idx).rg * N;
	vec3 hVec = localHeight(pos);
	float h = hVec.x + hVec.y + hVec.z;

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

		
		if ((pos.x < 0) || (pos.x > N) || (pos.y < 0) || (pos.y > N))
			break;
		
		float hOld = h;
		vec3 hVecOld = hVec;
		hVec = localHeight(pos);
		h = hVec.x + hVec.y + hVec.z;
		float hDiff = h - hOld;
		float sumsed = sed.r + sed.g + sed.b;
		if (hDiff > 0.0)
		{
			
			float toDrop = min(hDiff, sumsed);
			sed = deposit(toDrop, oldPos, sed);
		}
		else
		{
			cap = max(-hDiff, MINSLOPE) * vel * vol * cap;
			if (cap > sumsed)
			{
				float toTake = min((cap - sumsed) * EROSION, -hDiff);
				vec3 taken = erode(2 * (0.5 - INVERTEROSION) * toTake, oldPos);
				sed += taken;
			}
			else
			{
				float toDrop = (sumsed - cap) * DEPOSITION;
				if (h < WATERLEVEL)
				{
					toDrop = (sumsed - cap) * 0.7;
				}
				sed = deposit(toDrop, oldPos, sed);
			}
		}

		vel = sqrt(max(pow(vel,2) + hDiff*GRAVITY, 0));
		vol = vol * (1 - EVAPORATION);
		addHumidity(pos, vol);
		steps++;
		
		if (steps >= MAXSTEPS)
			break;
	}
}