#pragma once
#include "Platform/OpenGL/OpenGLShader.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/Camera.h"
#include <glm/gtc/type_ptr.hpp>
#include "PerlinNoise.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/Framebuffer.h"
namespace Hazel
{	
	class IsleTexture
	{
	public: 
		IsleTexture(int N, float* pxd);
		~IsleTexture();
		uint32_t GetRendererID() { return m_RendererID; }
	private:
		uint32_t m_RendererID;
		int m_N;
	};

	class Isle
	{
	public:
		Isle();
		~Isle();

		
		void GenerateRandom(int N = 256);
		void Render(Camera& camera);

		void Erode();
		void LoadHeightmap(const std::string& path, float cellsize);
		void Blur(int map = 0);
		void Dry() { _soil_dry(); }
		void Grow() { _growth(); }
		void Export();
		void Sunlight() { _compute_lightmap(); _compute_temperature(); }
		void Slide() { _slide(); }
		void ThermalErosion() { _thermal_erosion(); }
		void Erupt() { _erupt(); }
		void Coral() { _growth_coral(); }
		void ResolutionDouble();
		void ResolutionHalve();

		// technical
		Ref<Shader> GetShader() { return m_RenderShader; }
		// debug
		//uint32_t GetLightMapID() { return m_FramebufferA->GetColorAttachmentRendererID(); }
		//uint32_t GetShadowMapID() { return m_FramebufferB->GetColorAttachmentRendererID(); }
		uint32_t GetEnvironmentMapID() { return m_Environmentmap->GetRendererID(); }
		void SwitchUp() { _Switcher = (_Switcher + 1) % _nSwitches; HZ_CORE_INFO("{0}", _Switcher); }
		void SwitchDown() { _Switcher = (_Switcher == 0) * (_nSwitches - 1) + (1 - (_Switcher == 0)) * (_Switcher - 1); HZ_CORE_INFO("{0}", _Switcher); }
	private:
		// technical
		void GPUUpload();
		void GenerateVA();
		void InitialiseMaps();
		float* _resolution_double(float* source, int source_resolution);
		float* _resolution_halve(float* source, int source_resolution);
		glm::vec4 _blerp(float* mat, int mat_N, glm::vec2 pos); // bilinear interpolation
		float _blerpf(float* mat, int mat_N, glm::vec2 pos); // bilinear interpolation
		// simulation
		float PixelTilt(glm::ivec2 pos);
		//		erosion
		void ErodeDroplet(glm::vec2 pos);
		glm::vec4 LocalHeight(glm::vec2 P);
		glm::vec2 Isle::LocalGradient(glm::vec2 P);
		glm::vec4 _deposit(float vol, glm::vec4 sed, glm::vec2 pold);
		glm::vec4 _sediment(glm::vec4 sed, glm::vec2 pos);
		glm::vec4 _erode(float damage, glm::vec2 pold);
		glm::vec4 _erode_pixel(glm::ivec2 pos, float damage);
		void _deposit_underwater(glm::vec4 sed, glm::vec2 pos);
		//		water
		float LocalEnvironmentmapValue(glm::vec2 P, int channel);
		float _soil_absorb(float volume, glm::vec2 pos);
		void _soil_dry();
		//		vegetation
		void _growth();
		void _growth_pixel(glm::ivec2 px);
		glm::vec4 LocalVegetation(glm::vec2 P);
		//		illumination
		void _compute_lightmap();
		float* _sun_illuminate(glm::vec3 sun_position);
		glm::vec3 sunPosition(float LOC_LATITUDE, float LOC_TIME, float DECLINATION = 0.0);
		void ComputeNormals();
		//		rock slides
		void _slide();
		void _do_slide(glm::vec2 pos, float fraction = 1.0);
		void _remove_material(glm::vec4 S, glm::vec2 pos);
		//		thermal erosion
		void _thermal_erosion();
		void _thermal_erosion_pixel(glm::ivec2 P);
		//		temperature
		void _compute_temperature();
		//		volcano
		void _erupt();
		void _lava_flow(glm::vec2 pos, float vol);
		//		coral
		void _growth_coral();
		void _growth_pixel_coral(glm::ivec2 P);
		// rendering
		void _upload_colours();
	public:
		// parameters:
		int p_N = 256;
		float p_Scale = 1.0;
		float p_Height = 23;
		float p_Cellsize = 0.5;

		// erosion parameters
		glm::vec4 SOIL_HEALTH = glm::vec4(1.8, 1.0, 0.5, 0.6);
		glm::vec4 SEDIMENTATION_RATE = glm::vec4(0.05, 0.15, 0.05, 0.05);
		int MAX_STEPS = 64;
		int RADIUS = 5;
		float MINSLOPE = 0.01;
		float INERTIA = 0.3;
		float EROSION_RATE = 0.5;
		float DEPOSITION_RATE = 0.3;
		float GRAVITY = 10.0;
		float EVAPORATION_COEFFICIENT = 0.05;
		float INITIAL_VELOCITY = 0.0;
		float INITIAL_VOLUME = 0.5;
		float INITIAL_CAPACITY = 0.0;
		float BLUR_BLEND_WEIGHT = 0.2;
		float SOIL_DEGRADATION_RG = 0.6;
		float SOIL_DEGRADATION_RB = 0.3;
		float SOIL_DEGRADATION_GB = 0.6;
		float SEDIMENT_DECAY = 0.0;
		float EROSION_WASTAGE = 0.0; // how much (0 to 1) of the eroded material does not end up as sediment
		glm::vec4 SOIL_ERODE_MAX_LAYER_FRAC = glm::vec4(1.0, 0.2, 0.8, 0.2);
		// water parameters
		glm::vec4 SOIL_MOISTURE_CAPACITY = glm::vec4(0.00, 0.02, 0.01, 0.03);
		float SOIL_MOISTURE_CAPACITY_BASE = 0.0;
		float SOIL_MOISTURE_ABSORPTION_RATE = 0.7;
		float SOIL_MOISTURE_ABSORPTION_STEEPNESS_MAXIMUM = 70.0;
		float SOIL_MOISTURE_ABSORPTION_STEEPNESS_INFLUENCE = 0.5;
		float SOIL_CONTAINED_DRYING_RATE = 0.1;
		float SOIL_OPEN_DRYING_RATE = 0.4;
		// land slides
		glm::vec4 GRAV_FRICTION_ANGLE = { 30.0f, 40.0f, 12.0f, 25.0f }; 
		float GRAV_FRICTION_ANGLE_UNDERWATER_MULTIPLIER = 1.6;
		float GRAV_FRAC_MIN = 0.7;
		float GRAV_FRAC_MAX = 0.9;
		// thermal erosion
		float THERMAL_EROSION_TEMP_SCALE = 1 / 20.0; // units: 1/Celsius
		float THERMAL_EROSION_VEGETATION_PROTECT_SCALE = 1.0; // units: 1/PlantCover
		float THERMAL_EROSION_FRACTURE_VOLUME = 0.5;
		// plant parameters. Types are: 1. tree, 2. shrub, 3. grass, 4. humus-seeder
		// TREE prefers to grow on SOIL_G, zero to medium steepness, much humus, low to medium temp.
		// SHRUB prefers to grow on SOIL_B, zero to high steepness, little humus, high temp.
		// GRASS prefers to grow on any soil, zero to medium steepness, medium humus and temp.
		// SEEDER grows at a low rate anywhere where the humidity is medium
		glm::vec4 PL_MAX_POP = glm::vec4(0.6, 0.4, 0.7, 0.2);
		float PL_MAX_POP_TOTAL = 1.0;
		glm::vec4 PL_MAX_AGE = glm::vec4(6.0, 2.0, 1.0, 0.5);
		glm::vec4 PL_HUMUS_PROD = glm::vec4(0.1, 0.2, 0.1, 0.1);
		glm::vec4 PL_HUMIDITY_IDEAL = glm::vec4(0.015, 0.006, 0.008, 0.01);
		glm::vec4 PL_HUMIDITY_RANGE = glm::vec4(0.011, 0.003, 0.0075, 0.004);
		float PL_MIN_HUMID_PROBE_DEPTH = 0.1;
		glm::vec4 PL_TEMP_IDEAL = glm::vec4(20.0, 20.0, 20.0, 20.0);
		glm::vec4 PL_TEMP_RANGE = glm::vec4(10.0, 10.0, 10.0, 10.0);
		glm::vec4 PL_HUMUS_MIN = glm::vec4(0.002, 0.00025, 0.0001, 0.0);
		glm::vec4 PL_HUMUS_GROWTH_SPONSOR = glm::vec4(4.0, 1.5, 3.0, 0.0);
		glm::vec4 PL_TYPE_GROWTH_RATE = glm::vec4(1.8, 1.5, 2.0, 0.3);
		glm::vec4 PL_GRADIENT_IDEAL = glm::vec4(30.0, 55.0, 40.0, 80.0); // ideal flatness of plant type in degrees
		glm::vec4 PL_GRADIENT_RANGE = glm::vec4(45.0, 20.0, 40.0, 15.0); // range of surface tilt that the plant is ok with
		float PL_DEATH_RATE = 0.3;
		float PL_GROWTH_RATE = 0.25;
		float PL_AGE_RATE = 0.3;
		float PL_OVERGROWTH_ATTRITION = 0.3;
		float HUMUS_DECAY_RATE = 0.12;
		glm::vec4 HUMUS_UNDERWATER_PETRIFY_SOIL = glm::vec4(0.0, 0.0, 0.0, 0.0);
		// combined PLANT and EROSION parameters
		glm::vec4 PL_SOIL_PROTECT_FAC = glm::vec4(4.0, 1.0, 0.5, 0.0); // is population of plant i is at PL_MAX_POP.i, erosion damage will be reduced by PL_SOIL_PROTECT_FAC.i
		glm::vec4 PL_SOIL_DEPOSIT_FAC = glm::vec4(4.0, 0.6, 0.5, 0.0);
		glm::vec4 PL_INITIAL_MOISTURE_ABSORB = glm::vec4(0.3, 0.4, 0.2, 0.0);
		glm::vec4 PL_THERMAL_EROSION_PROTECT_FAC = glm::vec4(2.0, 0.0, 1.0, 1.0);
		// illumination
		float LATITUDE = -0.1;
		float LOC_TIME = 0.5;
		float DECLINATION = 10.0;
		glm::vec3 SUN_POS = sunPosition(LATITUDE, LOC_TIME, DECLINATION);
		float LIGHT_T_MIN = 0.25;
		float LIGHT_T_MAX = 0.75;
		float LIGHT_DT = 0.55;
		// temperature
		float TEMP_LIGHT_EXPOSURE_FAC = 10.0; // if exposed to direct (perpendicular) sunlight for the whole day, the temperature of that spot will be this value.
		float TEMP_HEIGHT_FAC = 1.0; // change in temperature for every 1.0 distance from sea level.
		float TEMP_SEALEVEL = 25.0; // temperature at sea level.
		float TEMP_HEIGHT_FAC_UNDERWATER = 2.0;
		// sea
		float WATERLEVEL = 0.0f;
		float DEPOSIT_UNDERWATER = 0.1;
		// volcano
		glm::vec2 VOLCANO_POS = glm::vec2(0.5, 0.5);
		float VOLCANO_SIZE = 0.073; // 0 to 
		float VOLCANO_POS_OFFSET = 0.1;
		float VOLCANO_POS_NOISE = 0.3;
		int VOLCANO_VOLUMES_PER_ERUPTION = 200;
		float LAVA_VOL_MEAN = 1.2;
		float LAVA_VOL_RANGE = 0.4;
		float LAVA_SOLIDIFY_UNDERWATER = 0.05;
		glm::vec4 LAVA_SOILTYPE = glm::vec4(0.0, 0.0, 0.0, 0.4);
		// coral
		
		float CORAL_GRADIENT_IDEAL = 30.0;
		float CORAL_GRADIENT_RANGE = 90.0;
		float CORAL_DEPTH_IDEAL = 1.0;
		float CORAL_DEPTH_MAX = 7.0;
		float CORAL_DEPTH_MIN = 0.5;
		float CORAL_BASE_GROWTH = 1.0;
		float CORAL_GROWTH_MAX = 0.1;
		
		glm::vec4 CORAL_SOILTYPE = glm::vec4(0.0, 0.6, 0.4, 0.0);
		// maps:
		float* heightmap;
		float* environmentmap; // r: water, g: light exposure, b: temperature, a: coral
		float* vegetationmap;
		float* vegetationlifespanmap;
		float* normalmap;
		bool _isOnGPU = false;
		IsleTexture* m_BlurBuffer;

		// colours
		glm::vec4 terrainColorG = glm::vec4(153.0 / 255.0, 143.0 / 255.0, 112.0 / 255.0, 1.0);
		glm::vec4 terrainColorR = glm::vec4(161.0 / 255.0, 157.0 / 255.0, 145.0 / 255.0, 1.0);
		glm::vec4 terrainColorB = glm::vec4(232.0 / 255.0, 231.0 / 255.0, 195.0 / 255.0, 1.0);
		glm::vec4 terrainColorA = glm::vec4(92.0 / 255.0, 58.0 / 255.0, 4.0 / 255.0, 1.0);
		glm::vec4 plantColorR = glm::vec4(27.0 / 255.0, 92.0 / 255.0, 4.0 / 255.0, 1.0);
		glm::vec4 plantColorG = glm::vec4(60.0 / 255.0, 147.0 / 255.0, 52.0 / 255.0, 1.0);
		glm::vec4 plantColorB = glm::vec4(110.0 / 255.0, 168.0 / 255.0, 71.0 / 255.0, 1.0);
		glm::vec4 plantColorA = glm::vec4(90.0 / 255.0, 112.0 / 255.0, 90.0 / 255.0, 1.0);
	private:
		// for rendering:
		glm::vec2* m_Vertices;
		uint32_t* m_Indices;
		Ref<VertexArray> m_VA;
		int _viewport[4];
		Ref<Shader> m_RenderShader = Shader::Create("assets/shaders/IsleShader.glsl"); // Shader::Create("assets/shaders/IsleShader.glsl");
		IsleTexture* m_Heightmap;
		IsleTexture* m_Environmentmap;
		IsleTexture* m_Vegetationmap;
		IsleTexture* m_Normalmap;
		int _Switcher = 0;
		int _nSwitches = 8;
		// blur shader
		Ref<Shader> cs_GaussianBlur = Shader::Create("assets/shaders/compute/GaussianBlur.glsl");

		// noise
		float PerlinNoisePixel(siv::PerlinNoise noisegenerator, float x, float z, int octaves, float persistence, float lacunarity);
		// LUT erosion neighbours
		int lut_N = 0;
		int* lut_idx;
		float* lut_weight;
	};


}
