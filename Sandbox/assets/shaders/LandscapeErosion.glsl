#type compute
#version 430 core
#define PI 3.1415926535897932384626433832795

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D heightMap;
layout(binding = 1, rgba32f) uniform image2D moistureMap;
layout(binding = 2, rgba32f) uniform image2D vegetationMap;
layout(binding = 3, rgba32f) uniform image2D heatMap;

uniform sampler2D random;

uniform int HEIGHT_MAP_RESOLUTION;
uniform float INITIAL_VOLUME;
uniform vec3 ABSORPTION_COEFFICIENT_PLANT;
uniform float ABSORPTION_RESISTANCE_PLANT;
uniform vec3 SHIELDING_COEFFICIENT_PLANT;
uniform float SHIELDING_RESISTANCE_PLANT;
uniform vec3 DEPOSITION_COEFFICIENT_PLANT;
uniform float DEPOSITION_RESISTANCE_PLANT;
uniform vec4 ABSORPTION_COEFFICIENT_SOIL;
uniform float ABSORPTION_RESISTANCE_SOIL;
uniform float ABSORPTION_SOIL_FLATNESS_THRESHOLD;
uniform float INERTIA;
uniform float GRAVITY;
uniform float CAPACITY;
uniform float DEPOSITION_RATE;
uniform float EROSION_RATE;
uniform float EVAPORATION_COEFFICIENT;
uniform vec4 DURABILITY_COEFFICIENT_SOIL;
uniform int MAX_STEPS;

int RADIUS = 21;



vec2 localGradient(vec2 pos)
{
	int x = int(floor(pos.r));
	int z = int(floor(pos.g));
	float u = pos.r - x;
	float v = pos.g - z;

	vec4 tl = imageLoad(heightMap, ivec2(x, z));
	vec4 tr = imageLoad(heightMap, ivec2(x+1, z));
	vec4 bl = imageLoad(heightMap, ivec2(x, z+1));
	vec4 br = imageLoad(heightMap, ivec2(x+1, z+1));

	vec4 gX = (1 - v) * (tr - tl) + v * (br - bl);
	vec4 gY = (1 - u) * (bl - tl) + u * (br - tr);
	vec2 gradient = vec2(gX.r + gX.g + gX.b + gX.a, gY.r + gY.g + gY.b + gY.a);
	return gradient;
}

vec4 localHeight(vec2 pos)
{
	int x = int(floor(pos.r));
	int z = int(floor(pos.g));
	float u = pos.r - x;
	float v = pos.g - z;

	vec4 tl = imageLoad(heightMap, ivec2(x, z));
	vec4 tr = imageLoad(heightMap, ivec2(x+1, z));
	vec4 bl = imageLoad(heightMap, ivec2(x, z+1));
	vec4 br = imageLoad(heightMap, ivec2(x+1, z+1));

	vec4 h = (1 - u) * (1 - v) * tl + u * (1 - v) * tr + (1 - u) * v * bl + u * v * br;
	return h;
}

float localMoisture(vec2 pos)
{
	int x = int(floor(pos.r));
	int z = int(floor(pos.g));
	float u = pos.r - x;
	float v = pos.g - z;

	float tl = imageLoad(moistureMap, ivec2(x, z)).r;
	float tr = imageLoad(moistureMap, ivec2(x+1, z)).r;
	float bl = imageLoad(moistureMap, ivec2(x, z+1)).r;
	float br = imageLoad(moistureMap, ivec2(x+1, z+1)).r;

	float m = (1 - u) * (1 - v) * tl + u * (1 - v) * tr + (1 - u) * v * bl + u * v * br;
	return m;
}

vec4 localVegetation(vec2 pos)
{
	int x = int(floor(pos.r));
	int z = int(floor(pos.g));
	float u = pos.r - x;
	float v = pos.g - z;

	vec4 tl = imageLoad(vegetationMap, ivec2(x, z));
	vec4 tr = imageLoad(vegetationMap, ivec2(x+1, z));
	vec4 bl = imageLoad(vegetationMap, ivec2(x, z+1));
	vec4 br = imageLoad(vegetationMap, ivec2(x+1, z+1));

	vec4 veg = (1 - u) * (1 - v) * tl + u * (1 - v) * tr + (1 - u) * v * bl + u * v * br;
	return veg;
}

float AbsorptionByPlants(float volume, vec2 pos)
{
	vec4 localplant = localVegetation(pos);
	float newVolume = (1 - dot(localplant.rgb, ABSORPTION_COEFFICIENT_PLANT) / ABSORPTION_RESISTANCE_PLANT) * volume;
	return max(newVolume, 0.0);
}

void moistureMapPixelAdd(ivec2 pixelPos, float toAdd)
{
	vec4 temp = imageLoad(moistureMap, pixelPos);
	imageStore(moistureMap, pixelPos, temp + vec4(toAdd, 0.0, 0.0, 0.0));
}

void heatMapPixelAdd(ivec2 pixelPos, vec4 toAdd)
{
	vec4 temp = imageLoad(heatMap, pixelPos);
	imageStore(heatMap, pixelPos, temp + toAdd);
}

float absorbMoisture(float volume, float hDiff, vec4 h, vec2 pos)
{
	vec4 soil = h;
	float waterCapacity = dot(h, ABSORPTION_COEFFICIENT_SOIL);
	float waterLevel = localMoisture(pos);
	float flatnessFactor = 1.0 - clamp(hDiff / ABSORPTION_SOIL_FLATNESS_THRESHOLD, 0.0, 1.0);
	float toAbsorb = (waterCapacity - waterLevel) / ABSORPTION_RESISTANCE_SOIL * flatnessFactor;
	toAbsorb = min(volume, toAbsorb);
	
	int x = int(floor(pos.r));
	int z = int(floor(pos.g));
	float u = pos.r - x;
	float v = pos.g - z;

	moistureMapPixelAdd(ivec2(x, z), (1 - u) * (1 - v) * toAbsorb);
	moistureMapPixelAdd(ivec2(x + 1, z), u * (1 - v) * toAbsorb);
	moistureMapPixelAdd(ivec2(x, z + 1), (1 - u) * v * toAbsorb);
	moistureMapPixelAdd(ivec2(x + 1, z + 1), u * v * toAbsorb);
	return (volume - toAbsorb);
}

void heightMapPixelAdd(ivec2 pixelPos, vec4 toAdd)
{
	vec4 temp = imageLoad(heightMap, pixelPos);
	imageStore(heightMap, pixelPos, temp + toAdd);
}

vec4 deposit(float volumeToDeposit, vec4 sed, vec2 pos)
{
	vec4 sediment = vec4(sed);
	vec4 toDeposit = vec4(0.0,0.0,0.0,0.0);
	// deposit heavy materials first.
	toDeposit.r += min(volumeToDeposit, sediment.r);
	sediment.r -= toDeposit.r;
	volumeToDeposit -= toDeposit.r;
	toDeposit.g += min(volumeToDeposit, sediment.g);
	sediment.g -= toDeposit.g;
	volumeToDeposit -= toDeposit.g;
	toDeposit.a += min(volumeToDeposit, sediment.a);
	sediment.a -= toDeposit.a;
	volumeToDeposit -= toDeposit.a;
	toDeposit.b += min(volumeToDeposit, sediment.b);
	sediment.b -= toDeposit.b;
	volumeToDeposit -= toDeposit.b;

	int x = int(floor(pos.r));
	int z = int(floor(pos.g));
	float u = pos.r - x;
	float v = pos.g - z;
	heightMapPixelAdd(ivec2(x, z), (1-u) * (1-v) * toDeposit);
	heightMapPixelAdd(ivec2(x + 1, z), u * (1-v) * toDeposit);
	heightMapPixelAdd(ivec2(x, z + 1), (1-u) * v * toDeposit);
	heightMapPixelAdd(ivec2(x + 1, z + 1), u * v * toDeposit);
	return sediment;
}

vec4 erodePixel(ivec2 pixelPos, float damage)
{
	// adjust damage value w.r.t local vegetation:
	heatMapPixelAdd(pixelPos, vec4(damage,0.0,0.0,0.0));
	vec3 plant = imageLoad(vegetationMap, pixelPos).rgb;
	//float protection = min(dot(plant, SHIELDING_COEFFICIENT_PLANT) / SHIELDING_RESISTANCE_PLANT, 1.0);
	//damage = damage * (1 - protection);
	vec4 soil = imageLoad(heightMap, pixelPos);
	vec4 soilFake = soil * DURABILITY_COEFFICIENT_SOIL;
	vec4 newSediment = vec4(0.0,0.0,0.0,0.0);
	// erode from the top down. sand, humus, rock, bedrock.
	newSediment.b += min(damage, soilFake.b);
	soilFake.b -= newSediment.b;
	damage -= newSediment.b;
	newSediment.a += min(damage, soilFake.a);
	soilFake.a -= newSediment.a;
	damage -= newSediment.a;
	newSediment.g += min(damage, soilFake.g);
	soilFake.g -= newSediment.g;
	damage -= newSediment.g;
	newSediment.r += min(damage, soilFake.r);
	soilFake.r -= newSediment.r;
	damage -= newSediment.r;
	vec4 newSedimentReal = newSediment / DURABILITY_COEFFICIENT_SOIL;
	heightMapPixelAdd(pixelPos, -newSedimentReal);
	return newSedimentReal;
}
vec4 erode(float damage, vec2 pos)
{
	vec4 newSediment = vec4(0.0, 0.0, 0.0, 0.0);
	// distribute damage over the neighbouring pixels.
	int x = int(round(pos.r));
	int z = int(round(pos.g));
	float u = pos.r - floor(pos.r);
	float v = pos.g - floor(pos.g);
	float normalizationFactor = 0.0;
	for (float i = -RADIUS; i < RADIUS; i++)
	{
		for (float j = -RADIUS; j < RADIUS; j++)
		{
			float r2 = (pow(i + u, 2) + pow(j + v, 2));
			if ((r2) < RADIUS*RADIUS)
				normalizationFactor += (RADIUS - sqrt(r2));
		}
	}
	for (float i = -RADIUS; i<RADIUS;i++)
	{
		for (float j = -RADIUS; j<RADIUS;j++)
		{
			float r2 = (pow(i + u, 2) + pow(j + v, 2));
			if ((x + i) <= HEIGHT_MAP_RESOLUTION && (x + i) > 0 && (z + j) <= HEIGHT_MAP_RESOLUTION && (z + j) > 0)
			{
				if ((r2) < pow(RADIUS,2))
				{
					float w = (RADIUS - sqrt(r2)) / normalizationFactor;
					newSediment += erodePixel(ivec2(x + i, z + j), damage * w);
				}
			}
		}
	}
	vec4 newSedimentDegraded = vec4(0.9 * newSediment.r, 0.1 * newSediment.r + 0.9 * newSediment.g, 0.1 * newSediment.g + newSediment.b, newSediment.a);
	return newSedimentDegraded;
}
vec2 randomVector(vec2 idx)
{
	float theta = texture(random, idx).b * 2 * PI;
	return vec2(sin(theta), cos(theta));
}
void main(void)
{
	// 0. set up
	vec2 idx = vec2(gl_GlobalInvocationID.xy);	// index of current invocation
	//vec2 pos = idx + texture(random, idx / HEIGHT_MAP_RESOLUTION).rg; // position in pixel value.
	vec2 pos = texture(random, idx / HEIGHT_MAP_RESOLUTION).rg * HEIGHT_MAP_RESOLUTION;
	vec2 dir = vec2(0.0,0.0);
	vec4 sed = vec4(0.0,0.0,0.0,0.0);
	float cap = CAPACITY;
	vec4 h = localHeight(pos);
	float velocity = 0.0;
	int steps = 0;
	// 1. Rain falls, creating a droplet with volume V0.
	float volume = INITIAL_VOLUME;
	// 2. Some of the volume is absorbed by plants.
	volume = AbsorptionByPlants(volume, pos);
	// NOW ENTER THE DROPLET LOOP
	while (true)
	{
		// 3. The droplet moves to a neighbouring cell.
		vec2 gradient = localGradient(pos);
		dir = dir * INERTIA - gradient * (1 - INERTIA);
		if (length(dir) == 0.0)
			dir = randomVector(pos / HEIGHT_MAP_RESOLUTION);
		dir = dir / length(dir);
		vec2 oldPos = vec2(pos.r, pos.g);
		pos += dir;

		// POSSIBLE BREAK 1
		if ((pos.x < 0) || (pos.x > HEIGHT_MAP_RESOLUTION) || (pos.y < 0) || (pos.y > HEIGHT_MAP_RESOLUTION))
			break;


		// 4. The height difference between the new and previous cell determines whether material is taken or left.
		vec4 hOld = h;
		vec4 hNew = localHeight(pos);
		vec4 _hDiff = hNew - hOld;
		float hDiff = _hDiff.r + _hDiff.g + _hDiff.b + _hDiff.a;
		h = hNew;

		// if the droplet has moved up, the pit that it apparently was in is filled.
		if (hDiff > 0.0)
		{
			sed = deposit(hDiff, sed, oldPos); // fills up hDiff or drops all there is.
		}
		else
		{
			// change the capacity of the droplet:
			cap = velocity * volume * cap;
			float sedimentTotal = sed.r + sed.g + sed.b + sed.a;
			if (cap > sedimentTotal)
			{
				// erode
				// 5. Local vegetation protects the soil from erosion (see 'erodePixel' function).
				float damage = velocity * volume * EROSION_RATE / 100.0;
				sed += erode(damage, oldPos);
			}
			else
			{
				// 6. Local vegetation increases deposition of soil. 
				//vec3 vegetation = localVegetation(oldPos).rgb;
				//float vegDepFac = dot(vegetation, DEPOSITION_COEFFICIENT_PLANT) / DEPOSITION_RESISTANCE_PLANT;
				//float _temp = min(1.0, DEPOSITION_RATE * (1 + vegDepFac));
				//sed = deposit((sedimentTotal - cap) * _temp, sed, oldPos);
				sed = deposit((sedimentTotal - cap) * DEPOSITION_RATE, sed, oldPos);
			}
		}
		velocity = sqrt(max(pow(velocity,2) + hDiff * GRAVITY, 0.0));
		steps++;
		// 7. Soil absorbs some moisture.
		volume = absorbMoisture(volume, hDiff, hOld, oldPos);
		volume = volume * (1 - EVAPORATION_COEFFICIENT);
		if (steps > MAX_STEPS)
		{
			break;
		}
		if (volume <= 0.0)
		{
			break;
		}
		
	}

}