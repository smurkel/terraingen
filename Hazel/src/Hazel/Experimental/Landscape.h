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
	class Map
	{
	public:
		Map(int N, int M, float* pxd);
		~Map();
		uint32_t GetRendererID() { return m_RendererID; }
	private:
		uint32_t m_RendererID;
		int m_N;
		int m_M;
	};

	class Landscape
	{
	public:
		Landscape();

		// main aux functions
		void Random(int N, float cellsize, float heightscale);
		void Load(const std::string& path, float cellsize);
		void Render(Camera& camera);
		// main simulation functions
		void Erode();
	private:
		float PerlinNoisePixel(siv::PerlinNoise noisegenerator, float x, float z, int octaves, float persistence, float lacunarity);
		void GenerateVA();
		void InitializeMaps(int N);
	public:
		// parameters
		int p_N = 0;
		float p_Cellsize = 10; // units?;
		float p_Heightscale = 1.0;
		Map* heightMap;
		Map* vegetationMap;
		Map* moistureMap;
		Map* massflowMap;
		Map* heatMap;
		Map* bufferMap;
		// random generation settings
		int prand_Octaves = 12;
		float prand_Scale = 0.55;
		float prand_Persistence = 0.3;
		float prand_Lacunarity = 2.5;
		// buffers
		float* buffer;
	private:
		// for rendering
		glm::vec2* m_Vertices;
		uint32_t* m_Indices;
		Ref<VertexArray> m_VA;
		Ref<Shader> m_RenderShader = Shader::Create("assets/shaders/LandscapeShader.glsl");
		Ref<Shader> cs_Erosion = Shader::Create("assets/shaders/LandscapeErosion.glsl");
	};

}