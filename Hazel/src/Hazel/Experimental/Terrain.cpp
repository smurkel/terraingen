#include "hzpch.h"
#include "Terrain.h"
#include <glad/glad.h>
#include <random>
#include <fstream>

Hazel::Terrain::Terrain(uint32_t seed, const int N, const float gridsize, const float L, float height, int octaves, float persistence, float lacunarity)
	: p_Seed(seed), p_N(N), p_Gridsize(gridsize), p_L(L), p_H(height), p_Octaves(octaves), p_Persistence(persistence), p_Lacunarity(lacunarity)
{
	// Make initial texture and vertex buffer
	pxd = new float[4 * (uint64_t)N * N];
	pxdErosion = new float[4 * (uint64_t)N * N];
	siv::PerlinNoise _pnoise = siv::PerlinNoise(seed);
	m_Vertices = new glm::vec2[(uint64_t)N * N];
	float _d = L / float(N);
	for (int x = 0; x < N; x++)
	{
		int row = x * N;
		for (int z = 0; z < N; z++)
		{
			float _x = (x - N/2.0) * _d;
			float _z = (z - N/2.0) * _d;
			int baseIdx = 4 * (row + z);
			// height map
			*(pxd + baseIdx + 0) = _x * gridsize / L;
			*(pxd + baseIdx + 1) = height * pNoisePixel(_pnoise, _x, _z, p_Octaves, p_Persistence, p_Lacunarity);
			*(pxd + baseIdx + 2) = _z * gridsize / L;
			*(pxd + baseIdx + 3) = 0.0;
			// erosion map
			*(pxdErosion + baseIdx + 0) = 0.0;
			*(pxdErosion + baseIdx + 1) = 0.0;
			*(pxdErosion + baseIdx + 2) = 0.0;
			*(pxdErosion + baseIdx + 3) = 1.0;
			m_Vertices[row + z] = glm::vec2(x, z);
		}
	}
	m_Heightmap = new csTexture(N, N, pxd);
	m_Erosionmap = new csTexture(N, N, pxdErosion);
	m_RandomTexture = new csTexture(N, N, nullptr);
	delete[] pxd;
	// Make index buffer
	m_Indices = new uint32_t[(N - 1) * (N - 1) * 6];
	int idx = 0;
	for (int x = 0; x < N - 1; x++)
	{
		for (int z = 0; z < N - 1; z++)
		{
			int home = x * N + z;
			m_Indices[idx++] = home + 1;
			m_Indices[idx++] = home;
			m_Indices[idx++] = home + N;
			m_Indices[idx++] = home + N;
			m_Indices[idx++] = home + N + 1;
			m_Indices[idx++] = home + 1;
		}
	}
	// Make VA
	m_VA.reset(Hazel::VertexArray::Create());
	Ref<VertexBuffer> vb;
	vb.reset(VertexBuffer::Create(m_Vertices, sizeof(glm::vec2) * N * N));
	vb->SetLayout({
		{Hazel::ShaderDataType::Float2, "index"},
	});
	Ref<IndexBuffer> ib;
	ib.reset(IndexBuffer::Create(m_Indices, (N - 1) * (N - 1) * 6));
	m_VA->SetPrimitiveType(PrimitiveType::Triangles);
	m_VA->AddVertexBuffer(vb);
	m_VA->SetIndexBuffer(ib);

	m_BlurBuffer = new csTexture(p_N, p_N, nullptr);
}

void Hazel::Terrain::Render(Camera& camera)
{
	m_RenderShader->Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_Heightmap->GetRendererID());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_Erosionmap->GetRendererID());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_HeightMapTexture->GetRendererID());

	std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat("N", (float)p_N);
	std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat("p_Offset", p_Offset);
	std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat("p_TextureOffset", p_TexOffset);
	std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat("p_HeightScale", p_HeightScale);
	std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat("p_ErosionWeight", p_ErosionWeight);
	std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat("p_LushScale", p_LushScale);
	std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat("p_LushOffset", p_LushOffset);
	std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformMat4("u_ViewMatrix", camera.GetViewMatrix());
	std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformMat4("u_ProjectionMatrix", camera.GetProjectionMatrix());
	glm::vec3 viewPosition = camera.GetPositionXYZ();
	std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat3("u_ViewPosition", viewPosition);
	std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat3("u_LightPosition", m_SunPosition);

	m_VA->Bind();
	glDrawElements(m_VA->GetPrimitiveType(), m_VA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

	m_RenderShader->Unbind();
}

void Hazel::Terrain::Erode(int ITERATIONS, float VELOCITY, float VOLUME, float INERTIA, float CAPACITY, float EROSION, float GRAVITY, float EVAPORATION, float DEPOSITION, float MINSLOPE, int MAXSTEPS, bool INVERTEROSION, float WATERLEVEL)
{
	for (int i = 0; i < ITERATIONS; i++)
	{
		// fill random texture
		std::default_random_engine generator;
		srand(time(NULL));
		int seed = rand() % 65535 + 1;
		generator.seed(seed);
		std::uniform_real_distribution<float> distribution(0, 1);
		random_pxd = new float[4 * p_N * p_N];
		for (int i = 0; i < p_N; i++)
		{
			int row = i * p_N;
			for (int j = 0; j < p_N; j++)
			{
				*(random_pxd + 4 * (row + j)) = distribution(generator);
				*(random_pxd + 4 * (row + j) + 1) = distribution(generator);
				*(random_pxd + 4 * (row + j) + 2) = distribution(generator);
				*(random_pxd + 4 * (row + j) + 3) = distribution(generator);
			}
		}
		m_RandomTexture = new csTexture(p_N, p_N, random_pxd);

		cs_Erosion->Bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_Heightmap->GetRendererID());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_RandomTexture->GetRendererID());
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformInt("height", 0);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformInt("xzNoise", 1);
		glBindImageTexture(0, m_Erosionmap->GetRendererID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("N", (float)p_N);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("VELOCITY", VELOCITY);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("VOLUME", VOLUME);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("INERTIA", INERTIA);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("CAPACITY", CAPACITY);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("EROSION", EROSION);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("GRAVITY", GRAVITY);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("EVAPORATION", EVAPORATION);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("DEPOSITION", DEPOSITION);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("WATERLEVEL", WATERLEVEL);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformFloat("MINSLOPE", MINSLOPE);
		int intINVERTEROSION = INVERTEROSION;
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformInt("INVERTEROSION", intINVERTEROSION);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Erosion)->UploadUniformInt("MAXSTEPS", MAXSTEPS);

		glDispatchCompute(16, 16, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		cs_Erosion->Unbind();
	}
}

void Hazel::Terrain::Blur()
{
	// Make the blur textures
	cs_GaussianBlur->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_Erosionmap->GetRendererID());
	glBindImageTexture(0, m_BlurBuffer->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	std::dynamic_pointer_cast<OpenGLShader>(cs_GaussianBlur)->UploadUniformInt("IN", 0);
	std::dynamic_pointer_cast<OpenGLShader>(cs_GaussianBlur)->UploadUniformFloat("N", (float)p_N);
	std::dynamic_pointer_cast<OpenGLShader>(cs_GaussianBlur)->UploadUniformInt("direction", 0); // 0 is horizontal, 1 is vertical
	glDispatchCompute(p_N, p_N, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_BlurBuffer->GetRendererID());
	glBindImageTexture(0, m_Erosionmap->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	std::dynamic_pointer_cast<OpenGLShader>(cs_GaussianBlur)->UploadUniformInt("IN", 0);
	std::dynamic_pointer_cast<OpenGLShader>(cs_GaussianBlur)->UploadUniformFloat("N", (float)p_N);
	std::dynamic_pointer_cast<OpenGLShader>(cs_GaussianBlur)->UploadUniformInt("direction", 1); // 0 is horizontal, 1 is vertical
	glDispatchCompute(p_N, p_N, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	cs_GaussianBlur->Unbind();
}

void Hazel::Terrain::Export()
{
	glBindTexture(GL_TEXTURE_2D, m_Heightmap->GetRendererID());
	float* heightMap;
	heightMap = new float[p_N * p_N * 4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA32F, GL_FLOAT, heightMap);
	glBindTexture(GL_TEXTURE_2D, m_Erosionmap->GetRendererID());
	float* erosionMap;
	erosionMap = new float[p_N * p_N * 4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA32F, GL_FLOAT, erosionMap);
	std::ofstream objFile("C:/Users/Mart/Desktop/dev/Hazel/Sandbox/assets/_terrain_gens/test.obj");
	for (int x = 0; x < p_N; x++)
	{
		for (int z = 0; z < p_N; z++)
		{
			uint32_t idx = (x * p_N + z) * 4 + 1; // textures are rgba, info is in green channel.

		}
	}
	// erosion weight
	// height scale
	// height offset


}

void Hazel::Terrain::DeleteCSTextures()
{
	delete m_Erosionmap;
	delete m_Heightmap;
	delete m_RandomTexture;
}
float Hazel::Terrain::pNoisePixel(siv::PerlinNoise pnoise, float x, float z, int octaves, float persistence, float lacunarity)
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

Hazel::csTexture::csTexture(int N, int M, float* pxd)
{
	glGenTextures(1, &m_RendererID);
	glActiveTexture(GL_TEXTURE0); // Set the current 'texture unit' in the OpenGL pipeline that is in use.
	glBindTexture(GL_TEXTURE_2D, m_RendererID); // This texture is then bound to the above set unit.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, M, 0, GL_RGBA, GL_FLOAT, pxd); // specify the 2d image in this texture object
}

Hazel::csTexture::~csTexture()
{
	glDeleteTextures(1, &m_RendererID);
}