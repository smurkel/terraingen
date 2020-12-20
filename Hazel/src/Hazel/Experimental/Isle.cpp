#include "hzpch.h"
#include "Isle.h"

#include <glad/glad.h>
#include <stb_image.h>
#include <random>
#include <ctime>

#include <stb_image_write.h>


namespace Hazel
{
	Isle::Isle()
	{
		glGetIntegerv(GL_VIEWPORT, _viewport);
		srand(time(NULL));
		LoadHeightmap("C:/Users/Mart/Desktop/stuff/VUE/Pitcairn.png", 0.5);
		//GenerateRandom(256);
		std::vector<int> _idx;
		std::vector<float> _weight;
		float _normfac = 0.0;
		for (int i = 1 - RADIUS; i < RADIUS; i++)
		{
			for (int j = 1 - RADIUS; j < RADIUS; j++)
			{
				if ((i * i + j * j) < (RADIUS * RADIUS))
				{
					lut_N += 1;
					_idx.push_back(i);
					_idx.push_back(j);
					float _temp = RADIUS - std::sqrt(i * i + j * j);
					_weight.push_back(_temp);
					_normfac += _temp;
				}
			}
		}
		lut_idx = new int[lut_N * 2];
		lut_weight = new float[lut_N];
		for (int i = 0; i < lut_N; i++)
		{
			lut_idx[2 * i] = _idx[2 * i];
			lut_idx[2 * i + 1] = _idx[2 * i + 1];
			lut_weight[i] = _weight[i] / _normfac;
		}

		InitialiseMaps();
		m_Heightmap = new IsleTexture(8, nullptr);
		m_Environmentmap = new IsleTexture(8, nullptr);
		m_Vegetationmap = new IsleTexture(8, nullptr);
		m_Normalmap = new IsleTexture(8, nullptr);
		_isOnGPU = false;
		GPUUpload();
	}

	Isle::~Isle()
	{
		delete m_Heightmap;
	}

	void Isle::GPUUpload()
	{
		if (!_isOnGPU)
		{
			ComputeNormals();
			delete m_Heightmap;
			delete m_Environmentmap;
			delete m_Vegetationmap;
			delete m_Normalmap;
			m_Heightmap = new IsleTexture(p_N, heightmap);
			HZ_CORE_INFO("Height map ID: {0}", m_Heightmap->GetRendererID());
			m_Environmentmap = new IsleTexture(p_N, environmentmap);
			m_Vegetationmap = new IsleTexture(p_N, vegetationmap);
			m_Normalmap = new IsleTexture(p_N, normalmap);
			_isOnGPU = true;
		}
		
	}


	void Isle::GenerateVA()
	{
		_isOnGPU = false;
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

		// reset the blur buffer
		m_BlurBuffer = new IsleTexture(p_N, nullptr);
	}

	void Isle::InitialiseMaps()
	{
		environmentmap = new float[4 * (uint64_t)p_N * p_N];
		vegetationmap = new float[4 * (uint64_t)p_N * p_N];
		vegetationlifespanmap = new float[4 * (uint64_t)p_N * p_N];
		normalmap = new float[4 * (uint64_t)p_N * p_N];
		for (int x = 0; x < p_N; x++)
		{
			int row = p_N * x;
			for (int z = 0; z < p_N; z++)
			{
				int baseidx = 4 * (row + z);
				*(environmentmap + baseidx + 0) = 0.0;
				*(environmentmap + baseidx + 1) = 0.0;
				*(environmentmap + baseidx + 2) = 0.0;
				*(environmentmap + baseidx + 3) = 0.0;
				*(vegetationmap + baseidx + 0) = 0.0;
				*(vegetationmap + baseidx + 1) = 0.0;
				*(vegetationmap + baseidx + 2) = 0.0;
				*(vegetationmap + baseidx + 3) = 0.0;
				*(vegetationlifespanmap + baseidx + 0) = 0.0;
				*(vegetationlifespanmap + baseidx + 1) = 0.0;
				*(vegetationlifespanmap + baseidx + 2) = 0.0;
				*(vegetationlifespanmap + baseidx + 3) = 0.0;
				*(normalmap + baseidx + 0) = 0.0;
				*(normalmap + baseidx + 1) = 0.0;
				*(normalmap + baseidx + 2) = 0.0;
				*(normalmap + baseidx + 3) = 1.0;
			}
		}
	}

	void Isle::GenerateRandom(int N)
	{
		uint32_t seed = rand() % 32767 + 1;
		siv::PerlinNoise noisegenerator = siv::PerlinNoise(seed);
		heightmap = new float[4 * (uint64_t)N * N];
		WATERLEVEL = 0.0f;// p_Height;
		for (int x = 0; x < N; x++)
		{
			int row = N * x;
			for (int z = 0; z < N; z++)
			{
				int baseidx = 4 * (row + z);
				*(heightmap + baseidx + 0) = p_Height + p_Height * PerlinNoisePixel(noisegenerator, p_Scale * x / N, p_Scale * z / N, 12, 0.3, 2.2);
				if (*(heightmap + baseidx + 0) < 0.0)
					*(heightmap + baseidx + 0) = 0.0;
				*(heightmap + baseidx + 1) = 0.0;
				*(heightmap + baseidx + 2) = 0.0;// p_Height + p_Height * PerlinNoisePixel(noisegenerator, 1.5 * p_Scale * x / N, 1.5 * p_Scale * z / N, 12, 0.3, 2.2);
				*(heightmap + baseidx + 3) = 0.0;

			}
		}

		InitialiseMaps();
		p_N = N;
		GenerateVA();
		_isOnGPU = false;
	}

	void Isle::Render(Camera& camera)
	{
		GPUUpload();
		m_RenderShader->Bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_Heightmap->GetRendererID());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_Environmentmap->GetRendererID());
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_Vegetationmap->GetRendererID());
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_Normalmap->GetRendererID());
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformMat4("u_ViewMatrix", camera.GetViewMatrix());
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformMat4("u_ProjectionMatrix", camera.GetProjectionMatrix());
		glm::vec3 viewPosition = camera.GetPositionXYZ();
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat3("u_ViewPosition", viewPosition);
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat3("u_LightPosition", SUN_POS);
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat("p_N", (float)p_N);
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat("p_HeightOffset", p_Height);
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat("p_Cellsize", p_Cellsize);
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformFloat("waterlevel", WATERLEVEL);
		std::dynamic_pointer_cast<OpenGLShader>(m_RenderShader)->UploadUniformInt("_Switcher", _Switcher);

		m_VA->Bind();
		glDrawElements(m_VA->GetPrimitiveType(), m_VA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

		m_RenderShader->Unbind();
	}

	void Isle::Erode()
	{
		for (int i = 0; i < (p_N * p_N / 250) ; i++)
		{
			float r1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (p_N - 2);
			float r2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (p_N - 2);
			glm::vec2 pos = glm::vec2(r1, r2);
			//HZ_CORE_INFO("\nDroplet at: ({0}, {1})", r1, r2);
			ErodeDroplet(pos);
		}
		_isOnGPU = false;
	}

	void Isle::LoadHeightmap(const std::string& path, float cellsize)
	{
		int width, height, channels;
		float* pxd = stbi_loadf(path.c_str(), &width, &height, &channels, 0); // are the images I will import RGB or RGBA?

		HZ_CORE_ASSERT(width == height, "Image not square!");
		HZ_CORE_ASSERT(channels == 3, "Image not RGB!");

		p_N = width;
		GenerateVA();

		heightmap = new float[4 * (uint64_t)p_N * p_N];
		for (int i = 0; i < p_N * p_N; i++)
		{
			*(heightmap + 4 * i) = *(pxd + 3 * i) * 10.0;
			*(heightmap + 4 * i + 1) = 0.0;
			*(heightmap + 4 * i + 2) = 0.0;
			*(heightmap + 4 * i + 3) = 0.0;
		}

		InitialiseMaps();
		stbi_image_free(pxd);
		
		p_Cellsize = cellsize;
	}

	void Isle::Export()
	{
		time_t rawtime;
		struct tm* timeinfo;
		char buffer[80];
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer, 80, "%g%m%d_%H%M%S_", timeinfo);

		std::string filename = "C:/Users/Mart/Desktop/stuff/VUE/";
		filename.append(buffer);
		filename.append("_heightmap.png");

		uint8_t* imgcopy = new uint8_t[p_N * p_N];
		float min = 1000.0;
		float max = 0.0;
		for (int i = 0; i < p_N * p_N; i++)
		{
			float* idx = heightmap + 4 * i;
			float h = (*(idx + 0) + *(idx + 1) + *(idx + 2) + *(idx + 3));
			min = std::fmin(h, min);
			max = std::fmax(h, max);
		}
		HZ_CORE_INFO("Min: {0}, max: {1}", min, max);
		for (int i = 0; i < p_N * p_N; i++)
		{
			float* idx = heightmap + 4 * i;
			*(imgcopy + i) = (uint8_t)(255.99f * (((*(idx + 0) + *(idx + 1) + *(idx + 2) + *(idx + 3)) - min) / (max - min)));
			*(imgcopy + i) = (*(imgcopy + i) > 0.0) * (*(imgcopy + i));
		}
		int status = stbi_write_png(filename.c_str(), p_N, p_N, 1, imgcopy, p_N);
		if (status == 0)
		{
			HZ_CORE_ASSERT(false, "Export not succesful.");
		}
		filename = "C:/Users/Mart/Desktop/stuff/VUE/";
		filename.append(buffer);
		filename.append("_soilmap.png");
		imgcopy = new uint8_t[3 * p_N * p_N];
		min = 1000.0;
		max = 0.0;
		for (int i = 0; i < p_N * p_N; i++)
		{
			float* idx = (heightmap + 4 * i);
			float sg = *(idx + 1);
			float sb = *(idx + 2);
			float sa = *(idx + 3);
			min = std::fmin(min, sg);
			min = std::fmin(min, sb);
			min = std::fmin(min, sa);
			max = std::fmax(max, sg);
			max = std::fmax(max, sb);
			max = std::fmax(max, sa);
		}
		for (int i = 0; i < p_N * p_N; i++)
		{
			*(imgcopy + 3 * i + 0) = (uint8_t)(255.99f * (*(heightmap + 4 * i + 1) - min) / (max - min));
			*(imgcopy + 3 * i + 1) = (uint8_t)(255.99f * (*(heightmap + 4 * i + 2) - min) / (max - min));
			*(imgcopy + 3 * i + 2) = (uint8_t)(255.99f * (*(heightmap + 4 * i + 3) - min) / (max - min));
		}
		status = stbi_write_png(filename.c_str(), p_N, p_N, 3, imgcopy, 3 * p_N);
		filename = "C:/Users/Mart/Desktop/stuff/VUE/";
		filename.append(buffer);
		filename.append("_plantmap.png");
		imgcopy = new uint8_t[4 * p_N * p_N];
		min = 0.0;
		max = 1.0;
		for (int i = 0; i < 4 * p_N * p_N; i++)
		{
			float* idx = vegetationmap + i;
			*(imgcopy + i) = (uint8_t)(255.99f * ((*(idx) - min) / (max - min)));
		}
		status = stbi_write_png(filename.c_str(), p_N, p_N, 4, imgcopy, 4 * p_N);

	}

	void Isle::ResolutionDouble()
	{
		_isOnGPU = false;
		heightmap = _resolution_double(heightmap, p_N);
		vegetationmap = _resolution_double(vegetationmap, p_N);
		vegetationlifespanmap = _resolution_double(vegetationlifespanmap, p_N);
		environmentmap = _resolution_double(environmentmap, p_N);
		normalmap = _resolution_double(normalmap, p_N);
		float map_size_i = (p_N - 1) * p_Cellsize;
		p_Cellsize = p_Cellsize * (p_N - 1) / (2 * p_N - 1);
		p_N *= 2;
		GenerateVA();
		GPUUpload();
	}

	float* Isle::_resolution_double(float* source, int source_resolution)
	{
		int N = 2 * source_resolution;

		float* retval = new float[4 * N * N];

		float step = (source_resolution - 1.0f) / (N - 1.0f) - 0.0000001f;
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				glm::vec4 temp = _blerp(source, source_resolution, glm::vec2(i * step, j * step));
				*(retval + 4 * (N * i + j) + 0) = temp.r;
				*(retval + 4 * (N * i + j) + 1) = temp.g;
				*(retval + 4 * (N * i + j) + 2) = temp.b;
				*(retval + 4 * (N * i + j) + 3) = temp.a;
			}
		}
		return retval;
	}

	void Isle::ResolutionHalve()
	{
		HZ_CORE_ASSERT((p_N % 2) == 0, "Terrain resolution is not a power of two!");
		_isOnGPU = false;
		heightmap = _resolution_halve(heightmap, p_N);
		vegetationmap = _resolution_halve(vegetationmap, p_N);
		vegetationlifespanmap = _resolution_halve(vegetationlifespanmap, p_N);
		environmentmap = _resolution_halve(environmentmap, p_N);
		normalmap = _resolution_halve(normalmap, p_N);
		p_Cellsize = p_Cellsize * (p_N - 1) / (p_N / 2 - 1);
		p_N /= 2;
		GenerateVA();
		GPUUpload();
	}

	float* Isle::_resolution_halve(float* source, int source_resolution)
	{
		HZ_CORE_ASSERT(source_resolution != 2, "Resolution can not be made any smaller!");
		int Ni = source_resolution;
		float* copy = new float[4 * Ni * Ni];
		for (int i = 0; i < 4 * Ni * Ni; i++)
		{
			*(copy + i) = *(source + i);
		}

		int Nf = Ni / 2;
		delete[] source;
		source = new float[4 * Nf * Nf];

		for (int i = 0; i < Nf; i++)
		{
			for (int j = 0; j < Nf; j++)
			{
				int x = 2 * i;
				int z = 2 * j;
				float* idx = copy + 4 * (x * Ni + z);
				for (int k = 0; k < 4; k++)
				{
					float tl = *(idx + k);
					*(source + 4 * (Nf * i + j) + k) = tl;
				}
			}
		}
		return source;
	}

	glm::vec4 Isle::_blerp(float* mat, int mat_N, glm::vec2 pos)
	{
		int x = (int)std::floor(pos.r);
		int z = (int)std::floor(pos.g);
		float u = pos.r - x;
		float v = pos.g - z;

		float* idx = mat + 4 * (mat_N * x + z);
		glm::vec4 tl = glm::vec4(*(idx + 0), *(idx + 1), *(idx + 2), *(idx + 3));
		idx = mat + 4 * (mat_N * (x + 1) + z);
		glm::vec4 tr = glm::vec4(*(idx + 0), *(idx + 1), *(idx + 2), *(idx + 3));
		idx = mat + 4 * (mat_N * x + z + 1);
		glm::vec4 bl = glm::vec4(*(idx + 0), *(idx + 1), *(idx + 2), *(idx + 3));
		idx = mat + 4 * (mat_N * (x + 1) + z + 1);
		glm::vec4 br = glm::vec4(*(idx + 0), *(idx + 1), *(idx + 2), *(idx + 3));

		glm::vec4 retval = (1 - u) * (1 - v) * tl + u * (1 - v) * tr + (1 - u) * v * bl + u * v * br;
		return retval;
	}

	float Isle::_blerpf(float* mat, int mat_N, glm::vec2 pos)
	{
		int x = (int)std::floor(pos.r);
		int z = (int)std::floor(pos.g);
		float u = pos.r - x;
		float v = pos.g - z;

		float* idx = mat + 4 * (mat_N * x + z);
		float tl = *idx;
		idx = mat + 4 * (mat_N * (x + 1) + z);
		float tr = *idx;
		idx = mat + 4 * (mat_N * x + z + 1);
		float bl = *idx;
		idx = mat + 4 * (mat_N * (x + 1) + z + 1);
		float br = *idx;

		float retval = (1 - u) * (1 - v) * tl + u * (1 - v) * tr + (1 - u) * v * bl + u * v * br;
		return retval;
	}

	void Isle::Blur(int map)
	{
		GPUUpload();
		// Make the blur textures
		cs_GaussianBlur->Bind();
		glActiveTexture(GL_TEXTURE0);
		if (map == 0)
			glBindTexture(GL_TEXTURE_2D, m_Heightmap->GetRendererID());
		else if (map == 1)
			glBindTexture(GL_TEXTURE_2D, m_Environmentmap->GetRendererID());
		else if (map == 2)
			glBindTexture(GL_TEXTURE_2D, m_Vegetationmap->GetRendererID());
		glBindImageTexture(0, m_BlurBuffer->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		std::dynamic_pointer_cast<OpenGLShader>(cs_GaussianBlur)->UploadUniformInt("IN", 0);
		std::dynamic_pointer_cast<OpenGLShader>(cs_GaussianBlur)->UploadUniformFloat("N", (float)p_N);
		std::dynamic_pointer_cast<OpenGLShader>(cs_GaussianBlur)->UploadUniformInt("direction", 0); // 0 is horizontal, 1 is vertical
		glDispatchCompute(p_N, p_N, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_BlurBuffer->GetRendererID());
		if (map == 0)
			glBindImageTexture(0, m_Heightmap->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		else if (map == 1)
			glBindImageTexture(0, m_Environmentmap->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		else if (map == 2)
			glBindImageTexture(0, m_Vegetationmap->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		
		std::dynamic_pointer_cast<OpenGLShader>(cs_GaussianBlur)->UploadUniformInt("IN", 0);
		std::dynamic_pointer_cast<OpenGLShader>(cs_GaussianBlur)->UploadUniformFloat("N", (float)p_N);
		std::dynamic_pointer_cast<OpenGLShader>(cs_GaussianBlur)->UploadUniformInt("direction", 1); // 0 is horizontal, 1 is vertical
		glDispatchCompute(p_N, p_N, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		cs_GaussianBlur->Unbind();
		if (map == 0)
			glBindTexture(GL_TEXTURE_2D, m_Heightmap->GetRendererID());
		else if (map == 1)
			glBindTexture(GL_TEXTURE_2D, m_Environmentmap->GetRendererID());
		else if (map == 2)
			glBindTexture(GL_TEXTURE_2D, m_Vegetationmap->GetRendererID());
		float* _temp = new float[p_N * p_N * 4];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, _temp);
		float* _ptr = heightmap;
		if (map == 1)
			_ptr = environmentmap;
		if (map == 2)
			_ptr = vegetationmap;
		for (int i = 0; i < p_N * p_N * 4; i++)
		{
			*(_ptr + i) = *(_ptr + i) * (1 - BLUR_BLEND_WEIGHT) + *(_temp + i) * (BLUR_BLEND_WEIGHT);
		}

		_isOnGPU = false;
	}

	float Isle::PixelTilt(glm::ivec2 pos)
	{
		int x = pos.r;
		int z = pos.g;
		glm::vec3 n;
		n.x = (*(normalmap + 4 * (p_N * x + z) + 0) - 0.5) * 2.0;
		n.y = (*(normalmap + 4 * (p_N * x + z) + 1) - 0.5) * 2.0;
		n.z = (*(normalmap + 4 * (p_N * x + z) + 2) - 0.5) * 2.0;
		glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
		float dot = glm::dot(n, up);
		float tilt = glm::abs(glm::acos(dot) / M_PI * 180.0f);
		//HZ_CORE_INFO("{0}", tilt);
		return tilt;
	}

	void Isle::ErodeDroplet(glm::vec2 pos)
	{
		glm::vec2 P = pos;							// position
		float V = INITIAL_VOLUME;					// volume
		float v = INITIAL_VELOCITY;					// velocity
		float C = INITIAL_CAPACITY;					// capacity
		glm::vec2 D = glm::vec2(0.0);				// direction
		glm::vec4 S = glm::vec4(0.0);				// sediment
		glm::vec4 H = LocalHeight(P);

		V = V * (1.0 - std::fmin(glm::dot(PL_INITIAL_MOISTURE_ABSORB, LocalVegetation(P)), 1.0f));
		int steps = 0;
		while (steps < MAX_STEPS)
		{
			S = S * (1 - SEDIMENT_DECAY);
			D = D * INERTIA - LocalGradient(P) * (1 - INERTIA);
			if ((D.r == 0.0) && (D.g == 0.0))
			{
				float theta = 2 * 3.14159 * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				D.r = std::sin(theta);
				D.g = std::cos(theta);
			}
			else
			{
				D = D / std::sqrt(D.r * D.r + D.g * D.g);
			}
			glm::vec2 Pold = glm::vec2(P.r, P.g);
			P += D;
			if ((P.r < 3) || (P.g < 3) || (P.r >= (p_N-3)) || (P.g >= (p_N-3)))
			{
				break;
			}
			S = _sediment(S, P);
			float Stotal = S.r + S.g + S.b + S.a;
			glm::vec4 Hold = H;
			H = LocalHeight(P);
			glm::vec4 _Hdif = H - Hold;
			float Hdif = _Hdif.r + _Hdif.g + _Hdif.b + _Hdif.a;
			
			_soil_absorb(V, Pold);
			if (Hdif > 0.0)
			{
				S = _deposit(std::fmin(Hdif, Stotal), S, Pold);
			}
			else
			{
				C = std::fmax(-Hdif, MINSLOPE) * v * V;
				if (C > Stotal)
				{
					float damage = v * V * EROSION_RATE;
					// local vegetation protects from erosion
					float veg_protect = std::fmin(glm::dot(LocalVegetation(Pold), PL_SOIL_PROTECT_FAC), 1.0f);
					S += _erode(damage * (1.0 - veg_protect), Pold);	
				}
				else
				{
					// local vegetation promotes deposition
					float veg_deposit = std::fmin(glm::dot(LocalVegetation(Pold), PL_SOIL_DEPOSIT_FAC), 1.0f);
					float deprate = std::fmin(DEPOSITION_RATE * (1.0 + veg_deposit), 1.0);
					S = _deposit((Stotal - C) * deprate, S, Pold);
				}
			}
			if ((H.r + H.g + H.b + H.a) <= WATERLEVEL)
			{
				_deposit_underwater(S, P);
				break;
			}
			float _v = (v * v - Hdif * GRAVITY);
			v = std::sqrt((_v > 0.0) * _v);
			V = V * (1 - EVAPORATION_COEFFICIENT);
			steps++;
			//HZ_CORE_INFO("\tparameters:\n P = ({0}, {1})\nV = {2}\nv = {3}\nC = {4}\nD = ({5}, {6})\nS = ({7}, {8}, {9}, {10})\nH = ({11}, {12}, {13}, {14})", P.r, P.g, V, v, C, D.r, D.g, S.r, S.g, S.b, S.a, H.r, H.g, H.b, H.a);
		}
	}

	glm::vec4 Isle::LocalHeight(glm::vec2 P)
	{
		int x = std::floor(P.r);
		int z = std:: floor(P.g);
		float u = P.r - x;
		float v = P.g - z;

		float* idx = (heightmap + 4 * (x * p_N + z));
		glm::vec4 tl = glm::vec4(*(idx + 0), *(idx + 1), *(idx + 2), *(idx + 3));
		idx = (heightmap + 4 * ((x + 1) * p_N + z));
		glm::vec4 tr = glm::vec4(*(idx + 0), *(idx + 1), *(idx + 2), *(idx + 3));
		idx = (heightmap + 4 * (x * p_N + z + 1));
		glm::vec4 bl = glm::vec4(*(idx + 0), *(idx + 1), *(idx + 2), *(idx + 3));
		idx = (heightmap + 4 * ((x + 1) * p_N + z + 1));
		glm::vec4 br = glm::vec4(*(idx + 0), *(idx + 1), *(idx + 2), *(idx + 3));
		
		glm::vec4 lh = (1 - u) * (1 - v) * tl + u * (1 - v) * tr + (1 - u) * v * bl + u * v * br;
		return lh;
	}

	glm::vec4 Isle::LocalVegetation(glm::vec2 P)
	{
		int x = std::floor(P.r);
		int z = std::floor(P.g);
		float u = P.r - x;
		float v = P.g - z;

		float* idx = (vegetationmap + 4 * (x * p_N + z));
		glm::vec4 tl = glm::vec4(*(idx + 0), *(idx + 1), *(idx + 2), *(idx + 3));
		idx = (vegetationmap + 4 * ((x + 1) * p_N + z));
		glm::vec4 tr = glm::vec4(*(idx + 0), *(idx + 1), *(idx + 2), *(idx + 3));
		idx = (vegetationmap + 4 * (x * p_N + z + 1));
		glm::vec4 bl = glm::vec4(*(idx + 0), *(idx + 1), *(idx + 2), *(idx + 3));
		idx = (vegetationmap + 4 * ((x + 1) * p_N + z + 1));
		glm::vec4 br = glm::vec4(*(idx + 0), *(idx + 1), *(idx + 2), *(idx + 3));

		glm::vec4 lh = (1 - u) * (1 - v) * tl + u * (1 - v) * tr + (1 - u) * v * bl + u * v * br;
		return lh;
	}

	void Isle::_compute_lightmap()
	{
		SUN_POS = sunPosition(LATITUDE, LOC_TIME, DECLINATION);
		for (int i = 0; i < p_N * p_N; i++)
		{
			*(environmentmap + 4 * i + 1) = 0.0;
		}
		float* buffer = new float[p_N * p_N];



		float scaleExposure = ((LIGHT_T_MAX - LIGHT_T_MIN) / LIGHT_DT);
		for (float t = LIGHT_T_MIN; t <= LIGHT_T_MAX; t+= LIGHT_DT)
		{
			glm::vec3 sunPos = sunPosition(LATITUDE, t);
			buffer = _sun_illuminate(sunPos);
			for (int i = 0; i < p_N * p_N; i++)
			{
				*(environmentmap + 4 * i + 1) += buffer[i] / scaleExposure;
			}
		}
		
		_isOnGPU = false;
	}

	float* Isle::_sun_illuminate(glm::vec3 sun_position)
	{
		float* H = new float[p_N * p_N];
		bool* checked = new bool[p_N * p_N];
		bool* light = new bool [p_N * p_N];
		float* exposure = new float[p_N * p_N];
		for (int i = 0; i < p_N * p_N; i++)
		{
			*(checked + i) = false;
			*(light + i) = false;
			*(exposure + i) = 0.0;
			*(H + i) = 0.0f;
			for (int k = 0; k < 4; k++)
			{
				*(H + i) += *(heightmap + 4 * i + k);
			}
		}

		glm::vec3 sunPos = sun_position;
		glm::vec3 step = sunPos / std::sqrt(sunPos.r * sunPos.r + sunPos.b * sunPos.b);

		for (int i = 2; i < p_N - 2; i++)
		{
			for (int j = 2; j < p_N - 2; j++)
			{
				if (!(*(checked + p_N * i + j)));
				{
					bool inRange = true;
					bool inLight = true;
					glm::vec3 pos = glm::vec3(i, *(H + p_N * i + j), j);
					std::vector<glm::ivec2> children;
					children.push_back(glm::ivec2(i, j));
					if (pos.y < WATERLEVEL)
					{
						*(checked + p_N + i * j) = true;
						continue;
					}
					while (inRange & inLight)
					{
						pos += step;
						glm::ivec2 pixel = glm::ivec2((int)std::round(pos.x), (int)std::round(pos.z));
						if ((pixel.r < 2) | (pixel.g < 2) | (pixel.r >= p_N - 2) | (pixel.g >= p_N - 2))
						{
							inRange = false;
							break;
						}
						float Y = *(H + p_N * pixel.r + pixel.g);
						children.push_back(pixel);
						if (Y > pos.y)
						{
							inLight = false;
							break;
						}
						else
						{
							if (*(checked + pixel.r * p_N + pixel.g))
							{
								break;
							}
						}
					}
					for (int k = 0; k < children.size(); k++)
					{
						glm::ivec2 px = children[k];
						int pxidx = px.r * p_N + px.g;
						glm::vec3 pxnorm = glm::vec3(*(normalmap + 4 * pxidx), *(normalmap + 4 * pxidx + 1), *(normalmap + 4 * pxidx + 2));
						*(checked + pxidx) = true;
						*(light + pxidx) = inLight;
						float dotp = glm::dot(sunPos, pxnorm);
						*(exposure + pxidx) += std::fmax(0.0, dotp*dotp);
					}
				}
			}
		}
		delete[] H;
		delete[] checked;
		return exposure;
	}

	glm::vec3 Isle::sunPosition(float LOC_LATITUDE, float LOC_TIME, float DECLINATION)
	{
		float time = LOC_TIME - std::floor(LOC_TIME);
		float hour_time = 360.0f * (time - 0.5);

		float zenith = -glm::acos(glm::sin(glm::radians(LOC_LATITUDE)) * glm::sin(glm::radians(DECLINATION)) + glm::cos(glm::radians(LOC_LATITUDE)) * glm::cos(glm::radians(DECLINATION)) * glm::cos(glm::radians(hour_time)));
		float azimuth = glm::asin(-glm::sin(glm::radians(hour_time)) * glm::cos(glm::radians(DECLINATION)) / glm::sin(zenith));

		glm::vec3 position = glm::vec3(
			glm::cos(azimuth) * glm::sin(zenith),
			glm::cos(zenith),
			glm::sin(azimuth) * glm::sin(zenith)
		);
		return position;
	}

	void Isle::ComputeNormals()
	{
		float* H = new float[p_N * p_N];
		for (int h = 0; h < p_N * p_N; h++)
		{
			*(H + h) = 0.0;
			for (int k = 0; k < 4; k++)
				*(H + h) += *(heightmap + 4 * h + k);
		}

		int K[9] = { -p_N - 1, -p_N, -p_N + 1, -1, 0, 1, p_N - 1, p_N, p_N + 1 };

		for (int x = 1; x < p_N - 1; x++)
		{
			for (int z = 1; z < p_N - 1; z++)
			{
				float* idx = H + p_N * x + z;
				glm::vec3 n;
				n.x = -(*(idx + K[2]) + 2 * *(idx + K[5]) + *(idx + K[8]) - *(idx + K[0]) - 2 * *(idx + K[3]) - *(idx + K[6]));
				n.z = -(*(idx + K[0]) + 2 * *(idx + K[1]) + *(idx + K[2]) - *(idx + K[6]) - 2 * *(idx + K[7]) - *(idx + K[8]));
				n.y = p_Cellsize;
				n = normalize(n);
				*(normalmap + 4 * (p_N * x + z)) = 0.5 + n.x / 2.0;
				*(normalmap + 4 * (p_N * x + z) + 1) = 0.5 + n.y / 2.0;
				*(normalmap + 4 * (p_N * x + z) + 2) = 0.5 + n.z / 2.0;

			}
		}

	}

	void Isle::_slide()
	{
		_isOnGPU = false;
		for (int i = 0; i < 2000; i++)
		{
			float r1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (p_N - 2);
			float r2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (p_N - 2);

			_do_slide(glm::vec2(r1, r2));
			_do_slide(glm::vec2(r1, r2));
			_do_slide(glm::vec2(r1, r2));
		}
		
	}

	void Isle::_do_slide(glm::vec2 pos, float fraction)
	{
		
		float GRAV_FRAC_MIN = 0.6;
		glm::vec4 GRAV_MAX_HDIF = glm::tan(GRAV_FRICTION_ANGLE * 2.0f * 3.14159f / 360.0f) * p_Cellsize;
		

		glm::vec4 S = { 0.0, 0.0, 0.0, 0.0 };
		glm::vec2 P0;
		float H0;
		glm::vec4 _H0;
		glm::vec2 g;
		glm::vec2 P1 = pos;
		glm::vec4 _H1 = LocalHeight(P1);
		float H1 = _H1.r + _H1.g + _H1.b + _H1.a;

		while (true)
		{

			P0 = P1;
			g = LocalGradient(P0);
			if ((g.r + g.g) == 0.0)
				break;
			P1 = P0 - glm::normalize(g);
			if ((P1.r < 3) || (P1.g < 3) || (P1.r >= (p_N - 3)) || (P1.g >= (p_N - 3)))
				break;
			_H0 = _H1;
			H0 = H1;
			_H1 = LocalHeight(P1);
			H1 = _H1.r + _H1.g + _H1.b + _H1.a;

			// calculate what fraction is above the friction angle
			//HZ_CORE_INFO("H0 = {0} + {1} + {2} + {3} = {4}", _H0.r, _H0.g, _H0.b, _H0.a, H0);
			//HZ_CORE_INFO("H1 = {0} + {1} + {2} + {3} = {4}", _H1.r, _H1.g, _H1.b, _H1.a, H1);
			S.r = std::fmax(_H0.r - (H1 + GRAV_MAX_HDIF.r), 0.0f);
			S.r = std::fmin(S.r, _H1.r);
			S.g = std::fmax((_H0.r + _H0.g) - (H1 + GRAV_MAX_HDIF.g), 0.0f);
			S.g = std::fmin(S.g, _H0.g);
			//S.g = std::clamp((_H0.r + _H0.g) - (H1 + GRAV_MAX_HDIF.g), 0.0f, _H0.g);
			//HZ_CORE_INFO("S.g = std::clamp(({0} + {1}) - ({2} + {3}), 0.0f, {4})", _H0.r, _H0.g, H1, GRAV_MAX_HDIF.g, _H0.g);
			//S.a = std::clamp((_H0.r + _H0.g + _H0.a) - (H1 + GRAV_MAX_HDIF.a), 0.0f, _H0.a);
			S.a = std::fmax((_H0.r + _H0.g + _H0.a) - (H1 + GRAV_MAX_HDIF.a), 0.0f);
			S.a = std::fmin(S.a, _H0.a);
			//S.b = std::clamp((_H0.r + _H0.g + _H0.b + _H0.a) - (H1 + GRAV_MAX_HDIF.b), 0.0f, _H0.b);
			S.b = std::fmax((_H0.r + _H0.g + _H0.b + _H0.a) - (H1 + GRAV_MAX_HDIF.b), 0.0f);
			S.b = std::fmin(S.b, _H0.b);
			//HZ_CORE_INFO("S.b = std::clamp((_H0.r + _H0.g + _H0.b + _")
			//HZ_CORE_INFO("Sliding material: rock {0} sand {1} huuum {2}", S.g, S.b, S.a);
			if ((S.r + S.g + S.b + S.a) == 0)
				break;
			// select a random fraction of that volume
			float frac = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (GRAV_FRAC_MAX - GRAV_FRAC_MIN) + GRAV_FRAC_MIN;
			frac *= fraction;
			S = S * frac;
			// drop this fraction in P1
			_remove_material(S, P0);
			_deposit(S.r + S.g + S.a + S.b, S, P1);
			// controls and next iteration:
		}
		

	}
	void Isle::_remove_material(glm::vec4 sed, glm::vec2 pos)
	{
		int x = std::floor(pos.r);
		int z = std::floor(pos.g);
		float u = pos.r - x;
		float v = pos.g - z;

		float* idx = (heightmap + 4 * (x * p_N + z));
		float f = (1 - u) * (1 - v);
		float _floatmin = 0.00000001;
		for (int i = 0; i < 4; i++)
		{
			*(idx + i) -= f * sed[i];
			*(idx + i) *= (*(idx + i) > _floatmin);
		}
		idx = (heightmap + 4 * ((x + 1) * p_N + z));
		f = u * (1 - v);
		for (int i = 0; i < 4; i++)
		{
			*(idx + i) -= f * sed[i];
			*(idx + i) *= (*(idx + i) > _floatmin);
		}
		idx = (heightmap + 4 * (x * p_N + z + 1));
		f = (1 - u) * v;
		for (int i = 0; i < 4; i++)
		{
			*(idx + i) -= f * sed[i];
			*(idx + i) *= (*(idx + i) > _floatmin);
		}
		idx = (heightmap + 4 * ((x + 1) * p_N + z + 1));
		f = u * v;
		for (int i = 0; i < 4; i++)
		{
			*(idx + i) -= f * sed[i];
			*(idx + i) *= (*(idx + i) > _floatmin);
		}
	}

	void Isle::_thermal_erosion()
	{
		for (int i = 0; i < 1000; i++)
		{
			int r1 = (int)(static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (p_N - 1));
			int r2 = (int)(static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (p_N - 1));
			_thermal_erosion_pixel(glm::ivec2(r1, r2));
		}
		_isOnGPU = false;
	}

	void Isle::_thermal_erosion_pixel(glm::ivec2 P)
	{
		int idx = 4 * (P.r * p_N + P.g);
		float H = *(heightmap + idx + 0) + *(heightmap + idx + 1) + *(heightmap + idx + 2) + *(heightmap + idx + 3);
		float dT = std::fabs(*(environmentmap + idx + 1));
		HZ_CORE_INFO("H = {0}, dT = {1}", H, dT);
		float* _veg = vegetationmap + idx;
		glm::vec4 veg = glm::vec4(*(_veg), *(_veg + 1), *(_veg + 2), *(_veg + 3));
		float vegetationProtectFactor = glm::dot(veg, PL_THERMAL_EROSION_PROTECT_FAC);
		float slope = PixelTilt(P) / 90.0;

		
		float p = THERMAL_EROSION_TEMP_SCALE * dT * slope / (1 + THERMAL_EROSION_VEGETATION_PROTECT_SCALE * vegetationProtectFactor);
		HZ_CORE_INFO("p = {0}", p);

		float r1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		if (r1 < p)
		{
			HZ_CORE_INFO("Thermal erosion done");
			*(heightmap + idx) -= THERMAL_EROSION_FRACTURE_VOLUME;
			*(heightmap + idx + 1) += THERMAL_EROSION_FRACTURE_VOLUME;
			_do_slide(glm::vec2((float)P.r, (float)P.g));
		}
	}

	void Isle::_compute_temperature()
	{
		_isOnGPU = false;
		_compute_lightmap();
		for (int i = 0; i < p_N; i++)
		{
			for (int j = 0; j < p_N; j++)
			{
				float* h = (heightmap + 4 * (p_N * i + j));
				float height = *(h + 0) + *(h + 1) + *(h + 2) + *(h + 3);
				float temperature;
				float HeightAboveSealevel = height - WATERLEVEL;
				if (height < WATERLEVEL)
					temperature = TEMP_SEALEVEL - HeightAboveSealevel * TEMP_HEIGHT_FAC_UNDERWATER;
				else
				{
					
					float light = *(environmentmap + 4 * (p_N * i + j) + 1);

					temperature = TEMP_SEALEVEL - HeightAboveSealevel * TEMP_HEIGHT_FAC + light * TEMP_LIGHT_EXPOSURE_FAC;
				}
				*(environmentmap + 4 * (p_N * i + j) + 2) = temperature;
			}
			HZ_CORE_INFO("T = {0}", *(environmentmap + 4 * i * p_N + 2));
		}
	}

	void Isle::_erupt()
	{
		for (int i = 0; i < VOLCANO_VOLUMES_PER_ERUPTION; i++)
		{
			// spawn a lava flow at a random position
			float theta = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2 * 3.141592;
			float r = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * VOLCANO_POS_NOISE * VOLCANO_SIZE + (1.0 - VOLCANO_POS_NOISE) * VOLCANO_SIZE;
			float r1 = ((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) - 0.5) * VOLCANO_POS_OFFSET * VOLCANO_SIZE;
			float r2 = ((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) - 0.5) * VOLCANO_POS_OFFSET * VOLCANO_SIZE;
			glm::vec2 vpos = VOLCANO_POS + glm::vec2(r1 + glm::cos(theta) * r, r2 + glm::sin(theta) * r);
			vpos *= (float)p_N;
			// random volume of lava
			//HZ_CORE_INFO("Volcano pos: {0} {1}", vpos.r, vpos.g);
			float lava = LAVA_VOL_MEAN + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2 * LAVA_VOL_RANGE - LAVA_VOL_RANGE;
			_lava_flow(vpos, lava);
		}
		_isOnGPU = false;
	}

	void Isle::_lava_flow(glm::vec2 pos, float vol)
	{
		glm::vec4 H1 = LocalHeight(pos);
		glm::vec4 H0;
		float h1 = H1.r + H1.g + H1.b + H1.a;
		float h0;

		float Lava = 1.0f * vol;
		while (Lava > vol / 20.0f)
		{
			glm::vec2 g = -LocalGradient(pos);
			float theta = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2 * 3.141592;
			glm::vec2 r = glm::vec2(glm::cos(theta), glm::sin(theta));
			if ((g.r == 0.0) & (g.g == 0.0))
			{
				theta = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2 * 3.141592;
				g = glm::vec2(glm::cos(theta), glm::sin(theta));
			}
			g = glm::normalize(g);
			

			pos = pos + r;//g + r;
			//H0 = H1;
			//h0 = h1;
			//H1 = LocalHeight(pos);
			//h1 = H1.r + H1.g + H1.b + H1.a;

			// determine how much lava to lose:
			float toDrop = LAVA_SOLIDIFY_UNDERWATER * Lava;
			Lava -= toDrop;
			glm::vec4 sed = LAVA_SOILTYPE * toDrop;
			_deposit(toDrop, sed, pos);
		}
	}

	void Isle::_growth_coral()
	{
		for (int i = 2; i < p_N - 2; i++)
		{
			for (int j = 2; j < p_N - 2; j++)
			{
				_growth_pixel_coral(glm::ivec2(i, j));
			}
		}
		_isOnGPU = false;
	}

	void Isle::_growth_pixel_coral(glm::ivec2 P)
	{
		// where does coral grow:
		int idx = 4 * (P.r * p_N + P.g);
		float* _h = heightmap + 4 * (P.r * p_N + P.g);
		float px_humus = *(_h + 3);
		float px_tilt = PixelTilt(P);
		glm::vec4 H = glm::vec4(*(_h + 0), *(_h + 1), *(_h + 2), px_humus);
		float h = H.r + H.g + H.b + H.a;

		
		float depth = WATERLEVEL - h - CORAL_DEPTH_MIN;
		if (depth > 0.0)
		{
			float fac_tilt = std::fmax(0.0, (1.0 - std::fabs(CORAL_GRADIENT_IDEAL - px_tilt) / CORAL_GRADIENT_RANGE));
			float fac_depth = (depth < (CORAL_DEPTH_MAX)) * std::fmax(0.0, (1.0 - (std::abs(CORAL_DEPTH_IDEAL - depth) / CORAL_DEPTH_MAX)));
			float coral_growth = CORAL_BASE_GROWTH * fac_depth * fac_depth / std::sqrt(px_tilt + 1.0);
			coral_growth = std::fmin(coral_growth, CORAL_GROWTH_MAX);
			//HZ_CORE_INFO("Tilt {0}", fac_tilt);
			//HZ_CORE_INFO("Depth {0}, fac_depth {1}", depth, fac_depth);
			glm::vec4 coral_vol = coral_growth * CORAL_SOILTYPE;
			glm::vec2 g = LocalGradient(P);
			float r1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 3.0;

			g = -g * r1 + glm::vec2(P.r, P.g);
			if ((g.r > 2.0) & (g.g > 2.0) & (g.r < (p_N - 2.0)) & (g.g < (p_N - 2.0)))
				_deposit(coral_vol.r + coral_vol.g + coral_vol.b + coral_vol.a, coral_vol, g);
		}
		
	}

	glm::vec2 Isle::LocalGradient(glm::vec2 P)
	{
		int x = std::floor(P.r);
		int z = std::floor(P.g);
		float u = P.r - x;
		float v = P.g - z;

		float* idx = (heightmap + 4 * (x * p_N + z));
		float tl = *(idx + 0) + *(idx + 1) + *(idx + 2) + *(idx + 3);
		idx = (heightmap + 4 * ((x + 1) * p_N + z));
		float tr = *(idx + 0) + *(idx + 1) + *(idx + 2) + *(idx + 3);
		idx = (heightmap + 4 * (x * p_N + z + 1));
		float bl = *(idx + 0) + *(idx + 1) + *(idx + 2) + *(idx + 3);
		idx = (heightmap + 4 * ((x + 1) * p_N + z + 1));
		float br = *(idx + 0) + *(idx + 1) + *(idx + 2) + *(idx + 3);

		glm::vec2 lg = glm::vec2((1 - v) * (tr - tl) + v * (br - bl), (1 - u) * (bl - tl) + u * (br - tr));
		return lg;
	}

	glm::vec4 Isle::_deposit(float vol, glm::vec4 sed, glm::vec2 pold)
	{
		glm::vec4 S = glm::vec4(sed);
		glm::vec4 toDeposit = glm::vec4(0.0);

		// deposit heavy material first: r -> g -> a -> b
		float _volRemaining = vol;
		toDeposit.r += std::fmin(_volRemaining, S.r);
		S.r -= toDeposit.r;
		_volRemaining -= toDeposit.r;
		toDeposit.g += std::fmin(_volRemaining, S.g);
		S.g -= toDeposit.g;
		_volRemaining -= toDeposit.g;
		toDeposit.a += std::fmin(_volRemaining, S.a);
		S.a -= toDeposit.a;
		_volRemaining -= toDeposit.a;
		toDeposit.b += std::fmin(_volRemaining, S.b);
		S.b -= toDeposit.b;
		_volRemaining -= toDeposit.b;

		int x = std::floor(pold.r);
		int z = std::floor(pold.g);
		float u = pold.r - x;
		float v = pold.g - z;

		float* idx = (heightmap + 4 * (x * p_N + z));
		float _fac = (1 - u) * (1 - v);
		*(idx + 0) += _fac * toDeposit.r;
		*(idx + 1) += _fac * toDeposit.g;
		*(idx + 2) += _fac * toDeposit.b;
		*(idx + 3) += _fac * toDeposit.a;

		
		idx = (heightmap + 4 * ((x + 1) * p_N + z));
		_fac = u * (1 - v);
		*(idx + 0) += _fac * toDeposit.r;
		*(idx + 1) += _fac * toDeposit.g;
		*(idx + 2) += _fac * toDeposit.b;
		*(idx + 3) += _fac * toDeposit.a;
		
		idx = (heightmap + 4 * (x * p_N + z + 1));
		_fac = (1 - u) * v;
		*(idx + 0) += _fac * toDeposit.r;
		*(idx + 1) += _fac * toDeposit.g;
		*(idx + 2) += _fac * toDeposit.b;
		*(idx + 3) += _fac * toDeposit.a;
		
		idx = (heightmap + 4 * ((x + 1) * p_N + z + 1));
		_fac = u * v;
		*(idx + 0) += _fac * toDeposit.r;
		*(idx + 1) += _fac * toDeposit.g;
		*(idx + 2) += _fac * toDeposit.b;
		*(idx + 3) += _fac * toDeposit.a;
		return S;
	}

	glm::vec4 Isle::_sediment(glm::vec4 sed, glm::vec2 pos)
	{
		glm::vec4 toDeposit = sed * SEDIMENTATION_RATE;

		int x = std::floor(pos.r);
		int z = std::floor(pos.g);
		float u = pos.r - x;
		float v = pos.g - z;

		float* idx = (heightmap + 4 * (x * p_N + z));
		float _fac = (1 - u) * (1 - v);
		*(idx + 0) += _fac * toDeposit.r;
		*(idx + 1) += _fac * toDeposit.g;
		*(idx + 2) += _fac * toDeposit.b;
		*(idx + 3) += _fac * toDeposit.a;


		idx = (heightmap + 4 * ((x + 1) * p_N + z));
		_fac = u * (1 - v);
		*(idx + 0) += _fac * toDeposit.r;
		*(idx + 1) += _fac * toDeposit.g;
		*(idx + 2) += _fac * toDeposit.b;
		*(idx + 3) += _fac * toDeposit.a;

		idx = (heightmap + 4 * (x * p_N + z + 1));
		_fac = (1 - u) * v;
		*(idx + 0) += _fac * toDeposit.r;
		*(idx + 1) += _fac * toDeposit.g;
		*(idx + 2) += _fac * toDeposit.b;
		*(idx + 3) += _fac * toDeposit.a;

		idx = (heightmap + 4 * ((x + 1) * p_N + z + 1));
		_fac = u * v;
		*(idx + 0) += _fac * toDeposit.r;
		*(idx + 1) += _fac * toDeposit.g;
		*(idx + 2) += _fac * toDeposit.b;
		*(idx + 3) += _fac * toDeposit.a;

		return sed - toDeposit;
	}
	glm::vec4 Isle::_erode(float damage, glm::vec2 pold)
	{
		glm::vec4 Snew = glm::vec4(0.0);
		int x = std::round(pold.r);
		int z = std::round(pold.g);

		for (int i = 0; i < lut_N; i++)
		{
			int _x = x + lut_idx[2 * i];
			int _z = z + lut_idx[2 * i + 1];

			if ((_x) > 0 && (_z) > 0 && (_x < p_N) && (_z < p_N))
			{
				float w = lut_weight[i];
				Snew += _erode_pixel(glm::vec2(_x, _z), damage * w) * (1 - EROSION_WASTAGE);
			}
		}
		glm::vec4 Snew_degraded = glm::vec4(Snew.r * (1 - SOIL_DEGRADATION_RG - SOIL_DEGRADATION_RB), Snew.r * SOIL_DEGRADATION_RG + Snew.g * (1 - SOIL_DEGRADATION_GB), Snew.r * SOIL_DEGRADATION_RB + Snew.g * SOIL_DEGRADATION_GB + Snew.b, Snew.a);
		return Snew_degraded;
	}

	glm::vec4 Isle::_erode_pixel(glm::ivec2 pos, float damage)
	{
		float* idx = (heightmap + 4 * (pos.r * p_N + pos.g));
		glm::vec4 soil = glm::vec4(*(idx), *(idx + 1), *(idx + 2), *(idx + 3));
		glm::vec4 health = soil * SOIL_HEALTH;
		glm::vec4 Snew = glm::vec4(0.0);
		Snew.b += std::fmin(damage, health.b * SOIL_ERODE_MAX_LAYER_FRAC.b);
		health.b -= Snew.b;
		damage -= Snew.b;

		Snew.a += std::fmin(damage, health.a * SOIL_ERODE_MAX_LAYER_FRAC.a);
		health.a -= Snew.a;
		damage -= Snew.a;

		Snew.g += std::fmin(damage, health.g * SOIL_ERODE_MAX_LAYER_FRAC.g);
		health.g -= Snew.g;
		damage -= Snew.g;

		Snew.r += std::fmin(damage, health.r * SOIL_ERODE_MAX_LAYER_FRAC.r);
		health.r -= Snew.r;
		damage -= Snew.r;

		glm::vec4 Snew_real = Snew / SOIL_HEALTH;
		*(idx + 0) -= Snew_real.r;
		*(idx + 1) -= Snew_real.g;
		*(idx + 2) -= Snew_real.b;
		*(idx + 3) -= Snew_real.a;
		return Snew_real;
	}

	void Isle::_deposit_underwater(glm::vec4 sed, glm::vec2 pos)
	{
		int x = std::round(pos.r);
		int z = std::round(pos.g);

		for (int i = 0; i < lut_N; i++)
		{
			int _x = x + lut_idx[2 * i];
			int _z = z + lut_idx[2 * i + 1];

			if ((_x) > 2 && (_z) > 2 && (_x < p_N - 2) && (_z < p_N - 2))
			{
				glm::vec4 H = LocalHeight(glm::vec2(_x, _z));
				if ((H.r + H.g + H.b + H.a) <= WATERLEVEL)
				{
					float w = lut_weight[i];
					*(heightmap + 4 * (p_N * _x + _z) + 0) += w * sed.r;
					*(heightmap + 4 * (p_N * _x + _z) + 1) += w * sed.g;
					*(heightmap + 4 * (p_N * _x + _z) + 2) += w * sed.b;
					*(heightmap + 4 * (p_N * _x + _z) + 3) += w * sed.a;
				}
			}
		}
	}

	float Isle::LocalEnvironmentmapValue(glm::vec2 P, int channel)
	{
		int x = std::floor(P.r);
		int z = std::floor(P.g);
		float u = P.r - x;
		float v = P.g - z;

		float* idx = (environmentmap + 4 * (x * p_N + z));
		float tl = *(idx + channel);
		idx = (environmentmap + 4 * ((x + 1) * p_N + z));
		float tr = *(idx + channel);
		idx = (environmentmap + 4 * (x * p_N + z + 1));
		float bl = *(idx + channel);
		idx = (environmentmap + 4 * ((x + 1) * p_N + z + 1));
		float br = *(idx + channel);

		float localval = (1 - u) * (1 - v) * tl + u * (1 - v) * tr + (1 - u) * v * bl + u * v * br;
		return localval;
	}

	float Isle::_soil_absorb(float volume, glm::vec2 pos)
	{
		int x = std::floor(pos.r);
		int z = std::floor(pos.g);
		float u = pos.r - x;
		float v = pos.g - z;

		float localWater = LocalEnvironmentmapValue(pos, 0);
		glm::vec4 localHeight = LocalHeight(pos);


		float _tl = PixelTilt(glm::ivec2(x, z));
		float _tr = PixelTilt(glm::ivec2(x + 1, z));
		float _bl = PixelTilt(glm::ivec2(x, z + 1));
		float _br = PixelTilt(glm::ivec2(x + 1, z + 1));
		float tilt = (1 - u) * (1 - v) * _tl + u * (1 - v) * _tr + (1 - u) * v * _bl + u * v * _br;
		// soil absorbs a fraction of the volume depending on 
		float capacity = glm::dot(localHeight, SOIL_MOISTURE_CAPACITY) + SOIL_MOISTURE_CAPACITY_BASE;
		float flatnessFactor = (1 - SOIL_MOISTURE_ABSORPTION_STEEPNESS_INFLUENCE) + SOIL_MOISTURE_ABSORPTION_STEEPNESS_INFLUENCE * (1.0f - std::fmin(tilt / SOIL_MOISTURE_ABSORPTION_STEEPNESS_MAXIMUM, 1.0f));
		float toAbsorb = (capacity - localWater) * SOIL_MOISTURE_ABSORPTION_RATE * volume * flatnessFactor;
		toAbsorb = std::fmin(toAbsorb, volume);
		toAbsorb = std::fmax(0.0, toAbsorb);
		

		float* idx = (environmentmap + 4 * (x * p_N + z));
		*(idx) += (1 - u) * (1 - v) * toAbsorb;
		idx = (environmentmap + 4 * ((x + 1) * p_N + z));
		*(idx) += u * (1 - v) * toAbsorb;
		idx = (environmentmap + 4 * (x * p_N + z + 1));
		*(idx) += (1 - u) * v * toAbsorb;
		idx = (environmentmap + 4 * ((x + 1) * p_N + z + 1));
		*(idx) += u * v * toAbsorb;

		return (volume - toAbsorb);
	}

	void Isle::_soil_dry()
	{
		HZ_CORE_INFO("drying");
		for (int i = 0; i < p_N * p_N; i++)
		{	
			float* idx = heightmap + 4 * i;
			glm::vec4 soil = glm::vec4(*(idx + 0), *(idx + 1), *(idx + 2), *(idx + 3));
			float localCapacity = glm::dot(soil, SOIL_MOISTURE_CAPACITY) + SOIL_MOISTURE_CAPACITY_BASE;
			float localMoisture = *(environmentmap + 4 * i);
			float newMoisture = std::fmin(localCapacity, localMoisture) * (1 - SOIL_CONTAINED_DRYING_RATE) + std::fmax(localMoisture - localCapacity, 0.0f) * (1 - SOIL_OPEN_DRYING_RATE);
			*(environmentmap + 4 * i) = newMoisture;
		}
		_isOnGPU = false;
	}

	void Isle::_growth()
	{
		if (!_isOnGPU) GPUUpload();
			
		for (int i = 2;i<p_N - 2;i++)
		{
			for (int j = 2; j < p_N - 2; j++)
			{
				_growth_pixel(glm::ivec2(i, j));
			}
		}
		_isOnGPU = false;
	}

	void Isle::_growth_pixel(glm::ivec2 px)
	{
		// Per plant, compute the optimal population.
		int x = px.r;
		int z = px.g;
		int idx = 4 * (x * p_N + z);
		float px_moisture = *(environmentmap + idx);
		float px_soil_g = *(heightmap + idx + 1);
		float px_soil_b = *(heightmap + idx + 2);
		float px_soil_r = *(heightmap + idx + 0);
		float px_humus = *(heightmap + idx + 3);
		float px_humus_decay = 0.0;
		float px_humus_fall = 0.0;
		if ((px_soil_r + px_soil_g + px_soil_b + px_humus) > WATERLEVEL)
		{
			float _humidsoillayers = (px_soil_g + px_soil_b + px_humus);
			float px_humidity = 0.0;
			if (_humidsoillayers > 0.0)
				px_humidity = px_moisture / (px_soil_g + px_soil_b + px_humus);
			else
				px_humidity = px_moisture / PL_MIN_HUMID_PROBE_DEPTH;
			px_humidity = std::fmax(0.0f, px_humidity);
			float px_temperature = 20.0; // will be in environmentmap later.
			// calculate steepness of the face
			float px_tilt = PixelTilt(glm::ivec2(x, z));
			//
			float px_humus_decay = -px_humus * (HUMUS_DECAY_RATE);

			glm::vec4 _pop = glm::vec4(*(vegetationmap + idx + 0), *(vegetationmap + idx + 1), *(vegetationmap + idx + 2), *(vegetationmap + idx + 3));
			float _totalpop = glm::dot(_pop, glm::vec4(1.0));
			float _base_growth_all = (PL_MAX_POP_TOTAL - _totalpop) / PL_MAX_POP_TOTAL * PL_GROWTH_RATE;
			for (int type = 0; type < 4; type++)
			{
				float Population = _pop[type];
				float Age = *(vegetationlifespanmap + idx + type);
				// death by old age
				float _tooOld = (1.0f - (PL_MAX_AGE[type] - Age) / PL_MAX_AGE[type]) * PL_DEATH_RATE * Population;
				Population -= _tooOld;
				px_humus_fall += PL_HUMUS_PROD[type] * _tooOld;
				Age = Age * (1 - _tooOld) + PL_AGE_RATE * Population;

				float _fac_tilt = std::fmax(0.0, (1.0 - (std::abs(PL_GRADIENT_IDEAL[type] - px_tilt) / PL_GRADIENT_RANGE[type])));
				float _max_pop = PL_MAX_POP[type] * (_fac_tilt + 0.2);
				if ((Population > _max_pop) && (_max_pop > 0.0)) // death by overgrowth
				{
					float _overgrowth = PL_OVERGROWTH_ATTRITION * (Population - _max_pop);
					Age *= (_overgrowth / Population);
					Population -= _overgrowth;
					px_humus_fall += PL_HUMUS_PROD[type] * _overgrowth;
					//HZ_CORE_INFO("overgrowth death type: {0}", type);
				}
				else // new growth
				{
					float _base_growth = ((_max_pop - Population) / _max_pop) * PL_TYPE_GROWTH_RATE[type] * _base_growth_all; // product of per_type_base_growth and total_base_growth, both 0 to 1
					float _fac_humidity = std::fmax(0.0f, (1.0f - std::abs((PL_HUMIDITY_IDEAL[type] - px_humidity)) / PL_HUMIDITY_RANGE[type])); // 0 to 1
					float _fac_temp = std::fmax(0.0f, (1.0f - std::abs((PL_TEMP_IDEAL[type] - px_temperature)) / PL_TEMP_RANGE[type])); // 0 to 1
					float _fac_humus = (px_humus >= PL_HUMUS_MIN[type]) - PL_HUMUS_MIN[type] * PL_HUMUS_GROWTH_SPONSOR[type];
					_fac_humus = 0.0f + (_fac_humus > 0.0f) * (1 + _fac_humus * px_humus); // 0, or 1+x where x depends on plant type and humus availability.
					Population += _fac_tilt * _base_growth * _fac_humidity * _fac_temp * _fac_humus; // was also * _fac_tilt

				}
				*(vegetationmap + idx + type) = Population;
				*(vegetationlifespanmap + idx + type) = Age;
			}
		}
		else
		{
			px_humus_decay = -px_humus * HUMUS_DECAY_RATE;
			for (int k = 0; k < 4; k++)
			{
				*(vegetationmap + idx + k) *= (1 - PL_OVERGROWTH_ATTRITION);
			}
			*(heightmap + idx + 0) -= px_humus_decay * HUMUS_UNDERWATER_PETRIFY_SOIL.r;
			*(heightmap + idx + 1) -= px_humus_decay * HUMUS_UNDERWATER_PETRIFY_SOIL.g;
			*(heightmap + idx + 2) -= px_humus_decay * HUMUS_UNDERWATER_PETRIFY_SOIL.b;

		}
		
		*(heightmap + idx + 3) += px_humus_decay + 0.6 * px_humus_fall;
		*(heightmap + idx + 3 + 4) += 0.1 * px_humus_fall;
		*(heightmap + idx + 3 - 4) += 0.1 * px_humus_fall;
		*(heightmap + idx + 3 + 4 * p_N) += 0.1 * px_humus_fall;
		*(heightmap + idx + 3 - 4 * p_N) += 0.1 * px_humus_fall;
	}


	IsleTexture::IsleTexture(int N, float* pxd)
	{
		m_N = N;
		glGenTextures(1, &m_RendererID);
		glActiveTexture(GL_TEXTURE0); // Set the current 'texture unit' in the OpenGL pipeline that is in use.
		glBindTexture(GL_TEXTURE_2D, m_RendererID); // This texture is then bound to the above set unit.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, N, 0, GL_RGBA, GL_FLOAT, pxd); // specify the 2d image in this texture object
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}
	IsleTexture::~IsleTexture()
	{
		glDeleteTextures(1, &m_RendererID);
	}


	float Isle::PerlinNoisePixel(siv::PerlinNoise pnoise, float x, float z, int octaves, float persistence, float lacunarity)
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


