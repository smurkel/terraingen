#include "hzpch.h"
#include "Landscape.h"

#include <glad/glad.h>
#include <stb_image.h>
#include <random>

namespace Hazel
{
	Map::Map(int N, int M, float* pxd)
	{
		m_N = N;
		m_M = M;
		glGenTextures(1, &m_RendererID);
		glActiveTexture(GL_TEXTURE0); // Set the current 'texture unit' in the OpenGL pipeline that is in use.
		glBindTexture(GL_TEXTURE_2D, m_RendererID); // This texture is then bound to the above set unit.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, M, 0, GL_RGBA, GL_FLOAT, pxd); // specify the 2d image in this texture object

	}

	Map::~Map()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	Landscape::Landscape()
	{
	}

	void Landscape::InitializeMaps(int N)
	{
		p_N = N;
		buffer = new float[4 * p_N * p_N];
		for (int i = 0; i < p_N * p_N; i++)
		{
			*(buffer + 4 * i + 0) = 0.0;
			*(buffer + 4 * i + 1) = 0.0;
			*(buffer + 4 * i + 2) = 0.0;
			*(buffer + 4 * i + 3) = 1.0;
		}
		vegetationMap = new Map(p_N, p_N, buffer);
		moistureMap = new Map(p_N, p_N, buffer);
		heatMap = new Map(p_N, p_N, buffer);
		massflowMap = new Map(p_N, p_N, buffer);
		bufferMap = new Map(p_N, p_N, buffer);
	}

	void Landscape::Random(int N, float cellsize, float heightscale)
	{
		srand(time(NULL));
		uint32_t seed = rand() % 32767 + 1;
		siv::PerlinNoise noisegenerator = siv::PerlinNoise(seed);
		buffer = new float[4 * (uint64_t)N * N];

		float perlinCellFactor = prand_Scale * cellsize / N;
		for (int x = 0; x < N; x++)
		{
			int row = x * N;
			for (int z = 0; z < N; z++)
			{
				int baseIdx = 4 * (row + z);
				// bedrock
				*(buffer + baseIdx + 0) = 5.0 + heightscale * PerlinNoisePixel(noisegenerator, (seed % 100) + x * perlinCellFactor,z * perlinCellFactor, prand_Octaves, prand_Persistence, prand_Lacunarity);
				// rock
				float rock = 0.0;// heightscale * 1.0 * PerlinNoisePixel(noisegenerator, 2 * x * perlinCellFactor, 2 * z * perlinCellFactor, prand_Octaves, prand_Persistence, prand_Lacunarity);
				if (rock < 0.0)
					rock = 0.0;
				*(buffer + baseIdx + 1) = rock;
				float sand = 0.0;//heightscale * 0.5 * PerlinNoisePixel(noisegenerator, 5 * x * perlinCellFactor, 5 * z * perlinCellFactor, prand_Octaves, prand_Persistence, prand_Lacunarity);
				if (sand < 0.0)
					sand = 0.0;
				*(buffer + baseIdx + 2) = sand;
				*(buffer + baseIdx + 3) = 0.0;
			}
		}

		heightMap = new Map(N, N, buffer);
		
		if (N != p_N)
		{
			p_N = N;
			GenerateVA();
		}
		InitializeMaps(p_N);
		p_Cellsize = cellsize;
		p_Heightscale = heightscale;
	}
	void Landscape::Load(const std::string& path, float cellsize)
	{
		int width, height, channels;
		float* pxd = stbi_loadf(path.c_str(), &width, &height, &channels, 0); // are the images I will import RGB or RGBA?
		
		HZ_CORE_ASSERT(width == height, "Image not square!");
		HZ_CORE_ASSERT(channels == 4, "Image not RGBA!");
		
		heightMap = new Map(width, width, pxd);
		stbi_image_free(pxd);
		if (width != p_N)
		{
			p_N = width;
			GenerateVA();
		}
		InitializeMaps(p_N);
		p_Cellsize = cellsize;
		p_Heightscale = 1.0;
	}

	void Landscape::Render(Camera& camera)
	{
		m_RenderShader->Bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightMap->GetRendererID());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, moistureMap->GetRendererID());
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformMat4("u_ViewMatrix", camera.GetViewMatrix());
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformMat4("u_ProjectionMatrix", camera.GetProjectionMatrix());
		glm::vec3 viewPosition = camera.GetPositionXYZ();
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat3("u_ViewPosition", viewPosition);
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat3("u_LightPosition", glm::vec3(0.0, -1.0, 0.0));
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat("p_N", (float)p_N);
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat("p_Cellsize", p_Cellsize);

		m_VA->Bind();
		glDrawElements(m_VA->GetPrimitiveType(), m_VA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

		m_RenderShader->Unbind();
	}

	void Landscape::Erode()
	{
		std::default_random_engine rng;
		srand(time(NULL));
		rng.seed(rand());
		std::uniform_real_distribution<float> distribution(0, 1);
		buffer = new float[4 * p_N * p_N];
		for (int i = 0; i < p_N * p_N; i++)
		{
			*(buffer + i * 4 + 0) = distribution(rng);
			*(buffer + i * 4 + 1) = distribution(rng);
			*(buffer + i * 4 + 2) = distribution(rng);
			*(buffer + i * 4 + 3) = 0.0;
		}
		bufferMap = new Map(p_N, p_N, buffer);

		cs_Erosion->Bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bufferMap->GetRendererID());
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformInt("random", 0);
		glBindImageTexture(0, heightMap->GetRendererID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(1, moistureMap->GetRendererID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(2, vegetationMap->GetRendererID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(3, heatMap->GetRendererID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		// all uniform variables:
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformInt("HEIGHT_MAP_RESOLUTION", p_N); 
		HZ_CORE_INFO("N: {0}", p_N);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformInt("MAX_STEPS", 256);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("INITIAL_VOLUME", 100.0); // initial volume of an erosion droplet. This relates to the amount of rainfall per time unit. 
		// VEGETATION COVER SHOULD BE NORMALIZED: 0 TO 1 - per category: tree, shrub, grass.
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat3("ABSORPTION_COEFFICIENT_PLANT", glm::vec3(5.0, 2.0, 1.0));
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat3("SHIELDING_COEFFICIENT_PLANT", glm::vec3(7.0, 4.0, 3.0));
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat3("DEPOSITION_COEFFICIENT_PLANT", glm::vec3(1.0, 2.0, 1.0));
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("SHIELDING_RESISTANCE_PLANT", 10.0);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("DEPOSITION_RESISTANCE_PLANT", 10.0);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("ABSORPTION_RESISTANCE_PLANT", 2000.0); // absorbed fraction will be: vec3 localVegetation *dot* vec3 ABSORPTION_COEFFICIENT / ABSORPTION_RESISTANCE.
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat4("ABSORPTION_COEFFICIENT_SOIL", glm::vec4(0.001, 0.1, 1.0, 0.0)); // water capacity per volume - for each soil type
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("ABSORPTION_SOIL_FLATNESS_THRESHOLD", 0.1);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("ABSORPTION_RESISTANCE_SOIL", 100.0);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("GRAVITY", 9.8);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("INERTIA", 0.3); // 0 to 1
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("CAPACITY", 0.1); // 0 to 1
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("DEPOSITION_RATE", 0.0); 
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("EROSION_RATE", 0.4); 
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("EVAPORATION_RATE", 0.1);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat4("DURABILITY_COEFFICIENT_SOIL", glm::vec4(10.0, 2.0, 1.0, 1.0));

		glDispatchCompute(p_N / 16, p_N / 16, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		cs_Erosion->Unbind();
	}

	void Landscape::GenerateVA()
	{
		m_Vertices = new glm::vec2[(uint64_t)p_N * p_N];
		// vertices get the texture coordinates
		float pxsize = 1.0f / (float)p_N;
		for (int i = 0; i < p_N; i++)
		{
			int row = i * p_N;
			for (int j = 0; j < p_N; j++)
			{
				m_Vertices[row + j] = glm::vec2((float)i * pxsize, (float)j * pxsize);
			}
		}


		m_Indices = new uint32_t[(p_N - 1) * (p_N - 1) * 6];
		int idx = 0;
		for (int i = 0; i < p_N - 1; i++)
		{
			int row = i * p_N;
			for (int j = 0; j < p_N - 1; j++)
			{
				int base = row + j;
				m_Indices[idx++] = base + 1;
				m_Indices[idx++] = base;
				m_Indices[idx++] = base + p_N;
				m_Indices[idx++] = base + p_N;
				m_Indices[idx++] = base + p_N + 1;
				m_Indices[idx++] = base + 1;
			}
		}


		m_VA.reset(Hazel::VertexArray::Create());
		Ref<VertexBuffer> vb;
		vb.reset(VertexBuffer::Create(m_Vertices, sizeof(glm::vec2) * p_N * p_N));
		vb->SetLayout({
			{Hazel::ShaderDataType::Float2, "uv"},
			});
		Ref<IndexBuffer> ib;
		ib.reset(IndexBuffer::Create(m_Indices, (p_N - 1) * (p_N - 1) * 6));
		m_VA->SetPrimitiveType(PrimitiveType::Triangles);
		m_VA->AddVertexBuffer(vb);
		m_VA->SetIndexBuffer(ib);
	}

	float Landscape::PerlinNoisePixel(siv::PerlinNoise pnoise, float x, float z, int octaves, float persistence, float lacunarity)
	{
		float rval = 0.0;
		float amplitude = 1;
		float frequency = 1;
		for (int i = 0; i < octaves; i++)
		{
			rval += pnoise.noise2D(frequency * x, frequency * z) * amplitude;
			frequency *= lacunarity;
			amplitude *= persistence;
		}
		return rval;
	}
}
