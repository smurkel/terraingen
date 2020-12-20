#type vertex
#version 430

layout(location = 0) in vec2 uv;
layout(binding = 0) uniform sampler2D heightMap;

uniform float p_N;
uniform float p_Cellsize;
uniform float p_HeightOffset;
uniform float waterlevel;
out vec4 Terrain;
out vec2 g_UV;
out float f_Illumination;


void main()
{
	// GENERATE GEOMETRY
	vec2 XZ = (uv - 0.5) * p_Cellsize * (p_N - 1);
	Terrain = texture(heightMap, uv);
	float Y = Terrain.r + Terrain.g + Terrain.b + Terrain.a;
	
	// OUTPUT UV
	g_UV = uv;
	gl_Position = vec4(XZ.r, Y - waterlevel, XZ.g, 1.0);
}

#type geometry
#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;

out float f_Illumination;

in vec4 Terrain[];
out vec4 f_Terrain;

in vec2 g_UV[];
out vec2 f_UV;

float CLIPPING_MARGIN_REFLECTION = -0.0;
float CLIPPING_MARGIN_REFRACTION = 10.0;

void main()
{
	// calculate normal vector
	mat4 vpMat = u_ProjectionMatrix * u_ViewMatrix;
	vec3 V1 = gl_in[0].gl_Position.xyz;
	vec3 V2 = gl_in[1].gl_Position.xyz;
	vec3 V3 = gl_in[2].gl_Position.xyz;
	vec3 Normal = normalize(cross(V2-V1, V3-V1));
	vec3 P = (V1 + V2 + V3) / 3.0;
	vec3 lightDir = normalize(u_LightPosition);
	vec3 viewDir = normalize(P - u_ViewPosition);
	f_Illumination = -dot(Normal, lightDir);

	f_UV = (g_UV[0] + g_UV[1] + g_UV[2]) / 3.0;
	
	// EMIT VERTICES
	f_Terrain = Terrain[0];
	gl_ClipDistance[0] = CLIPPING_MARGIN_REFLECTION + V1.y;
	gl_ClipDistance[1] = CLIPPING_MARGIN_REFRACTION - V1.y;
	gl_Position = vpMat * gl_in[0].gl_Position;
	EmitVertex();
	
	f_Terrain = Terrain[1];
	gl_ClipDistance[0] = CLIPPING_MARGIN_REFLECTION + V2.y;
	gl_ClipDistance[1] = CLIPPING_MARGIN_REFRACTION - V2.y;
	gl_Position = vpMat * gl_in[1].gl_Position;
	EmitVertex();

	f_Terrain = Terrain[2];
	gl_ClipDistance[0] = CLIPPING_MARGIN_REFLECTION + V3.y;
	gl_ClipDistance[1] = CLIPPING_MARGIN_REFRACTION - V3.y;
	gl_Position = vpMat * gl_in[2].gl_Position;
	EmitVertex();
}

#type fragment
#version 430

layout(location = 0) out vec4 fragColor;
layout(binding = 1) uniform sampler2D environmentMap;
layout(binding = 2) uniform sampler2D plantMap;
layout(binding = 3) uniform sampler2D normalMap;

in float f_Illumination;

in vec4 f_Terrain;
in vec2 f_UV;
uniform float waterlevel;

uniform int _Switcher;


void main()
{
	vec4 terrainColor = vec4(0.0);
	float Brightness = f_Illumination;
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
		// PLANTS
		vec4 plant = texture(plantMap, f_UV) * 1.0;
		vec4 plR = vec4(plant.r * 4.0, 0.0, 0.0, 1.0);
		vec4 plG = vec4(0.0, plant.g*4.0, 0.0, 1.0);
		vec4 plB = vec4(0.0, 0.0, plant.b * 4.0, 1.0);
		vec4 plA = vec4(plant.a * 4.0, plant.a * 4.0, 0.0, 1.0);
		terrainColor = plR + plG + plB + plA;
		terrainColor = mix(vec4(0.0, 0.0, 0.0, 1.0), terrainColor, clamp((plant.r + plant.g + plant.b + plant.a) / 0.001, 0.0, 1.0));
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
		float light = texture(environmentMap, f_UV).g;
		terrainColor = vec4(light, 0.0, 1 - light, 1.0);
	}
	else if (_Switcher == 7)
	{
		float temp = (texture(environmentMap, f_UV).b - 25.0) / 15.0;
		float subzero = 0.0;
		if (temp < 0.0)
			subzero = 1.0;
		terrainColor = vec4(temp, temp * 0.2, subzero * (-temp), 1.0);
	}
	fragColor = (Brightness) * terrainColor;
	fragColor.a = 1.0;
}
