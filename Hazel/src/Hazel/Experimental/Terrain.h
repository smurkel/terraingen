#pragma once

#include "Platform/OpenGL/OpenGLShader.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/Camera.h"
#include <glm/gtc/type_ptr.hpp>
#include "PerlinNoise.h"
#include "Hazel/Renderer/Texture.h"

namespace Hazel
{
	class csTexture
	{
	public:
		csTexture(int N, int M, float* pxd);
		~csTexture();
		uint32_t GetRendererID() { return m_RendererID; }
	private:
		uint32_t m_RendererID;
	};

	class Terrain
	{
	public:
		Terrain(uint32_t seed = 0, const int N = 512, const float gridsize = 50, const float L = 1, float height = 1.0, int octaves = 8, float persistence = 0.25, float lacunarity = 1.8);
		
		// main functions
		void Render(Camera& camera);
		void Erode(int ITERATIONS = 1, float VELOCITY = 1.0, float VOLUME = 10.0, float INERTIA = 0.6, float CAPACITY = 8.0, float EROSION = 0.2, float GRAVITY = 10.0, float EVAPORATION = 0.01, float DEPOSITION = 0.05, float MINSLOPE = 0.01, int MAXSTEPS = 64, bool INVERTEROSION = false, float WATERLEVEL = -10.0);
		void Blur();
		void Export();
		// small misc functions
		void DeleteCSTextures();
		uint32_t GetHeightMapID() { return m_Heightmap->GetRendererID(); }
		uint32_t GetErosionMapID() { return m_Erosionmap->GetRendererID(); }
		uint32_t GetNoiseMapID() { return m_RandomTexture->GetRendererID(); }
		uint32_t GetBlurMapID() { return m_BlurBuffer->GetRendererID(); }
		void NewSeed() { srand(time(NULL)); p_Seed = rand() % 65535 + 1; }
		uint32_t GetSeed() { return p_Seed; }
		void SetHeightOffset(float offset) { p_Offset = offset; }
		float GetHeightOffset() { return p_Offset; }
		void SetTextureOffset(float offset) { p_TexOffset = offset; }
		float GetTextureOffset() { return p_TexOffset; }
		void SetErosionWeight(float weight) { p_ErosionWeight = weight; }
		float GetErosionWeight(){ return p_ErosionWeight; }
		void SetHeightScalingFactor(float scaling) { p_HeightScale = scaling; }
		float GetHeightScalingFactor() { return p_HeightScale; }
		void SetLushScale(float scale) { p_LushScale = scale; }
		void SetLushOffset(float offs) { p_LushOffset = offs; }
		void SetSunPosition(glm::vec3 sunPos) { m_SunPosition = sunPos; }
		glm::vec3 GetSunPosition() { return m_SunPosition; }
		void UpdateTexture() { m_HeightMapTexture.reset(); m_HeightMapTexture = Hazel::Texture2D::Create(_textureStr); }
	private:
		float pNoisePixel(siv::PerlinNoise pnoise, float x, float z, int octaves, float persistence, float lacunarity);
		// parameters for terrain generation
		uint32_t p_Seed;
		int p_N;
		float p_L;
		float p_Gridsize;
		float p_H;
		int p_Octaves;
		float p_Persistence;
		float p_Lacunarity;
		// NOTE - SETTING: erode(toTake, oldPos) -> erode(-toTake, oldPos); in cs_Erosion shader causes an interesting 'plateauing' effect.
		// data, shaders and some rendering uniforms
		float* pxd = nullptr;
		float* pxdErosion = nullptr;
		csTexture* m_Heightmap;
		csTexture* m_Erosionmap;
		csTexture* m_RandomTexture;
		float* random_pxd = nullptr;
		// 20.0, 100.0, 80.0
		glm::vec3 m_SunPosition = glm::vec3(0.0, 0.0, 0.0);
		glm::vec2* m_Vertices;
		uint32_t* m_Indices;
		Ref<VertexArray> m_VA;
		Ref<Shader> m_RenderShader = Shader::Create("assets/shaders/TerrainShader.glsl");
		Ref<Shader> cs_Erosion = Shader::Create("assets/shaders/compute/Erosion.glsl");
		std::string _textureStr = "assets/textures/hMap2.png";
		Ref<Texture2D> m_HeightMapTexture = Texture2D::Create(_textureStr);
		float p_Offset = 0.0f;
		float p_TexOffset = 0.0f;
		float p_ErosionWeight = 1.0f;
		float p_HeightScale = 1.0f;
		float p_LushScale = 1.0f;
		float p_LushOffset = 1.0f;
		Ref<Shader> cs_GaussianBlur = Shader::Create("assets/shaders/compute/GaussianBlur.glsl");
		csTexture* m_BlurBuffer;
	};

}