

#type vertex
#version 430

layout(location = 0) in vec2 uv;
layout(binding = 0) uniform sampler2D heightMap;
layout(binding = 3) uniform sampler2D normalMap;

uniform float p_N;
uniform float p_Cellsize;
uniform float p_HeightOffset;
uniform vec3 u_LightPosition;
uniform float waterlevel;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;

out vec4 f_Terrain;
out vec2 f_UV;
out vec3 f_Normal;
out float f_Illumination;

float CLIPPING_MARGIN_REFLECTION = -0.0;
float CLIPPING_MARGIN_REFRACTION = 10.0;

void main()
{
	// GENERATE GEOMETRY
	vec2 XZ = (uv - 0.5) * p_Cellsize * (p_N - 1);
	f_Terrain = texture(heightMap, uv);
	float Y = f_Terrain.r + f_Terrain.g + f_Terrain.b + f_Terrain.a - waterlevel;
	// OUTPUT LIGHT
	vec3 f_Normal = (texture(normalMap, uv).rgb - 0.5) * 2.0;
	vec3 lightDir = normalize(u_LightPosition);
	f_Illumination = -dot(f_Normal, lightDir);
	// OUTPUT UV
	f_UV = uv;
	gl_Position = u_ProjectionMatrix * u_ViewMatrix * vec4(XZ.r, Y, XZ.g, 1.0);
	gl_ClipDistance[0] = CLIPPING_MARGIN_REFLECTION + Y;
	gl_ClipDistance[1] = CLIPPING_MARGIN_REFRACTION - Y;
}

#type fragment
#version 430

layout(location = 0) out vec4 fragColor;
layout(binding = 1) uniform sampler2D environmentMap;
layout(binding = 2) uniform sampler2D plantMap;
layout(binding = 3) uniform sampler2D normalMap;

in float f_Illumination;
in vec4 f_Terrain;
in vec3 f_Normal;
in vec2 f_UV;
uniform float waterlevel;

uniform int _Switcher;


void main()
{
	vec4 terrainColor = vec4(0.0);
	float Brightness = f_Illumination + 1.0;
	float moisture = texture(environmentMap, f_UV).r;
	if (_Switcher == 0)
	{
		// TERRAIN W PLANT
		vec4 terrainColorG = vec4(153.0 / 255.0, 143.0 / 255.0, 112.0 / 255.0, 1.0);
		vec4 terrainColorR = vec4(161.0 / 255.0, 157.0 / 255.0, 145.0 / 255.0, 1.0);
		vec4 terrainColorB = vec4(232.0 / 255.0, 231.0 / 255.0, 195.0 / 255.0, 1.0);
		vec4 terrainColorA = vec4(92.0 / 255.0,  58.0 / 255.0,  4.0/255.0, 1.0);
		terrainColor = mix(terrainColorR, terrainColorG, clamp(f_Terrain.g / 0.05, 0.0, 1.0));
		terrainColor = mix(terrainColor, terrainColorB, clamp(f_Terrain.b / 0.1, 0.0, 1.0));
		terrainColor = mix(terrainColor, terrainColorA, clamp(f_Terrain.a / 0.5, 0.0, 1.0));


		vec4 plant = texture(plantMap, f_UV);
		vec4 plantColorR = vec4(27.0 / 255.0, 92.0 / 255.0, 4.0 / 255.0, 1.0);
		vec4 plantColorG = vec4(60.0 / 255.0, 147.0 / 255.0, 52.0 / 255.0, 1.0);
		vec4 plantColorB = vec4(110.0 / 255.0, 168.0 / 255.0, 71.0 / 255.0, 1.0);
		vec4 plantColorA = vec4(90.0 / 255.0, 112.0 / 255.0, 90.0 / 255.0, 1.0);
		vec4 plantColor = mix(plantColorA, plantColorG, clamp(plant.g / 0.1, 0.0, 1.0));
		plantColor = mix(plantColor, plantColorB, clamp(plant.b / 0.7, 0.0, 1.0));
		plantColor = mix(plantColor, plantColorR, clamp(plant.r / 0.1, 0.0, 1.0));

		float _planttotal = plant.r + plant.g + plant.b + plant.a;

		terrainColor = mix(terrainColor, plantColor, clamp(_planttotal / 0.7, 0.0, 1.0));

		// HUMIDITY
		float humidlayerdepth = f_Terrain.g + f_Terrain.b + f_Terrain.a;
		float humidity = 0.0;
		if (humidlayerdepth > 0.0)
			humidity = moisture / max(0.2, humidlayerdepth);
		else
			humidity = moisture / 1.0;

		Brightness -= humidity * 8.0;
		Brightness *= 1.0;
		Brightness = max(Brightness, 0.2);

	}
	else if (_Switcher == 1)
	{
		//// TERRAIN WITH NO PLANTS
		float Rock = f_Terrain.g / 0.05;
		float Sand = f_Terrain.b / 0.05;
		float Humm = f_Terrain.a / 0.05;
		terrainColor = vec4(Rock, Sand, Humm, 1.0);
		Brightness -= 0.2;
	}
	else if (_Switcher == 2)
	{
		// HUMIDITY
		float humidlayerdepth = f_Terrain.g + f_Terrain.b + f_Terrain.a;
		float humidity = 0.0;
		if (humidlayerdepth > 0.0)
			humidity = moisture / max(0.2, humidlayerdepth);
		else
			humidity = moisture / 1.0;
		terrainColor = vec4(humidity / 0.03, humidity / 0.01, humidity / 0.001, 1.0);
		Brightness = 1.0;
	}
	else if (_Switcher == 3)
	{
		// PLANTS BUT NOT MOSS
		vec4 plant = texture(plantMap, f_UV) * 1.0;
		terrainColor = vec4(plant.r * 2.0, plant.g * 2.0, plant.b * 2.0, 1.0);
		terrainColor = mix(vec4(0.0, 0.0, 0.0, 1.0), terrainColor, clamp((plant.r + plant.g + plant.b) / 0.001, 0.0, 1.0));
	}
	else if (_Switcher == 4)
	{
		// MOSS and HUMUS
		vec4 plant = texture(plantMap, f_UV);
		terrainColor = vec4(1.0, 1.0, 1.0, 1.0);
		terrainColor = mix(terrainColor, vec4(0.6, 0.4, 0.0, 1.0), clamp(f_Terrain.a / 0.001, 0.0, 1.0));
		terrainColor = mix(terrainColor, vec4(0.2, 0.7, 0.2, 1.0), clamp(plant.a / 0.001, 0.0, 1.0));
	}
	else if (_Switcher == 5)
	{
		vec3 normal = texture(normalMap, f_UV).rgb;
		terrainColor = vec4(normal.r, normal.b, normal.g, 1.0);
		Brightness = 1.0;
	}
	else if (_Switcher == 6)
	{
		vec4 plant = texture(plantMap, f_UV);
		terrainColor = plant;
		Brightness = 1.0;
	}
	fragColor = (Brightness) * terrainColor;
	fragColor.a = 1.0;
}