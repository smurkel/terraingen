#include "hzpch.h"
#include "Ocean.h"
#include <glad/glad.h>

#include <time.h>
#include <glm/gtc/matrix_transform.hpp>
#include "WaterPhysicsEngine.h"

namespace Hazel
{

	Ocean::Ocean()
	{
		
	}

	void Ocean::Generate(int N, int L, float A, glm::vec2 wind, int windexponent)
	{
		m_N = N;
		m_L = L;
		m_Wind = wind;
		m_Windexponent = windexponent;
		m_A = A;
		buffer = new OceanTexture(N, N);
		// 1: generate H0
		h0_a = new OceanTexture(N, N);
		h0_b = new OceanTexture(N, N);

		cs_Generate->Bind(); // equivalent to glUseProgram(cs_stage1);
		// sampler2d in cs_stage1 looks for noise textures in texture units 0, 1, 2, and 3.
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("noise_r0", 0);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("noise_i0", 1);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("noise_r1", 2);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("noise_i1", 3);

		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("N", N);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("L", L);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("windexponent", windexponent);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformFloat("A", A);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformFloat2("_windDirection", wind);
		// generate noise textures
		srand((unsigned)time(NULL));
		m_Noise = new uint32_t[4];
		float* temp = new float[N * N];
		for (int i = 0;i < 4;i++)
		{
			glGenTextures(1, &m_Noise[i]); // name a new texture
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, m_Noise[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // necessary?
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // necessary?
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // necessary?
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // necessary?
			for (int j = 0;j < N * N;j++)
			{
				temp[j] = (float)rand() / RAND_MAX;
			}
			temp[(N - 1) * (N - 1) / 2] = 0;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, N, N, 0, GL_RED, GL_FLOAT, &temp[0]);
			glBindTextureUnit(i, m_Noise[i]);
		}

		// bind textures h0_a, h0_b to image slot 0, 1.
		glBindImageTexture(0, h0_a->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(1, h0_b->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		// Dispatch and wait for completion.
		glDispatchCompute(16, 16, 1); // LOCAL SIZE?
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // all operations that access an image are put before the barrier, any operation issued after this lien is but after the barrier.

		// 2: initialize textures to hold the time depentent fourier spectrum.
		h_dx = new OceanTexture(N, N);
		h_dy = new OceanTexture(N, N);
		h_dz = new OceanTexture(N, N);
		// 3: 
		Butterfly = new OceanTexture((int)std::log2(N), N);
		cs_Butterfly->Bind();

		std::dynamic_pointer_cast<OpenGLShader>(cs_Butterfly)->UploadUniformInt("N", N);
		// generate bit reversed indices 0 > N-1
		uint32_t* indices = new uint32_t[N];
		for (int n = 0;n < N;n++)
		{
			indices[n] = BitReverse(n, N);
		}

		GLuint ssbo;
		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, N * sizeof(uint32_t), &indices[0], GL_STREAM_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);

		glBindImageTexture(0, Butterfly->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glDispatchCompute(16, 16, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // all operations that access an image are put before the barrier, any operation issued after this lien is but after the barrier.

		// Prepare the vertex array
		m_VA.reset(Hazel::VertexArray::Create());
		vertices = new OceanVertex[(uint64_t)m_N * m_N];
		v_indices = new uint32_t[((uint64_t)m_N - 1) * (m_N - 1) * 6];
		int idx = 0;
		for (int i = 0;i < m_N;i++)
		{
			for (int j = 0;j < m_N;j++)
			{
				vertices[i * m_N + j].x = (float)j / m_N;
				vertices[i * m_N + j].z = (float)i / m_N;
				if ((i < (m_N - 1)) && (j < (m_N - 1)))
				{
					int home = i * m_N + j;
					v_indices[idx++] = home + 1;
					v_indices[idx++] = home;
					v_indices[idx++] = home + m_N;
					v_indices[idx++] = home + m_N;
					v_indices[idx++] = home + m_N + 1;
					v_indices[idx++] = home + 1;
				}
			}
		}
		
		Ref<VertexBuffer> vb;
		vb.reset(VertexBuffer::Create(vertices, sizeof(OceanVertex) * m_N * m_N));
		vb->SetLayout({
			{ Hazel::ShaderDataType::Float2, "index" },
		});
		Hazel::Ref<Hazel::IndexBuffer> ib;
		ib.reset(IndexBuffer::Create(v_indices, (m_N - 1) * (m_N - 1) * 6));
		m_VA->SetPrimitiveType(PrimitiveType::Triangles);
		m_VA->AddVertexBuffer(vb);
		m_VA->SetIndexBuffer(ib);
		Update(0.0);

	}




	void Ocean::UpdateSpectrum()
	{
		cs_Generate->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("noise_r0", 0);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("noise_i0", 1);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("noise_r1", 2);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("noise_i1", 3);

		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("N", m_N);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("L", m_L);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformInt("windexponent", m_Windexponent);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformFloat("A", m_A);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformFloat2("_windDirection", m_Wind);
		for (int i = 0;i < 4;i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, m_Noise[i]);
			glBindTextureUnit(i, m_Noise[i]);
		}
		// bind textures h0_a, h0_b to image slot 0, 1.
		glBindImageTexture(0, h0_a->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(1, h0_b->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		// Dispatch and wait for completion.
		glDispatchCompute(16, 16, 1); // LOCAL SIZE?
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // all operations that access an image are put before the barrier, any operation issued after this lien is but after the barrier.
	}
	void Ocean::Update(Timestep ts)
	{
		if (m_Live)
		{
			m_Time += ts * 1;
			cs_Update->Bind();
			std::dynamic_pointer_cast<OpenGLShader>(cs_Update)->UploadUniformInt("N", m_N);
			std::dynamic_pointer_cast<OpenGLShader>(cs_Update)->UploadUniformInt("L", m_L);
			std::dynamic_pointer_cast<OpenGLShader>(cs_Update)->UploadUniformFloat("t", m_Time);
			glBindImageTexture(0, h_dy->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glBindImageTexture(1, h_dx->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glBindImageTexture(2, h_dz->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glBindImageTexture(3, h0_a->GetRendererID(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			glBindImageTexture(4, h0_b->GetRendererID(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

			glDispatchCompute(16, 16, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			
			ComputeFFT(h_dy);
			ComputeFFT(h_dx);
			ComputeFFT(h_dz);
		}
	}

	void Ocean::Render(Camera& camera)
	{
		m_Shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformInt("L", m_L);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, h_dx->GetRendererID());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, h_dy->GetRendererID());
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, h_dz->GetRendererID());
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_ReflectionTextureID);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m_RefractionTextureID);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, m_RefractionDepthTextureID);

		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformFloat4("C_Emissive", c_Emissive);
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformFloat4("C_Ambient", c_Ambient);
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformFloat4("C_Diffuse", c_Diffuse);
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformFloat4("C_Specular", c_Specular);
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformFloat("u_Murkiness", m_Murkiness);
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformMat4("u_ViewMatrix", camera.GetViewMatrix());
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformMat4("u_ProjectionMatrix", camera.GetProjectionMatrix());
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformFloat("z_Near", camera.GetZNear());
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformFloat("z_Far", camera.GetZFar());
		glm::vec3 viewPosition = camera.GetPositionXYZ();
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformFloat3("u_ViewPosition", viewPosition);
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformFloat3("u_LightPosition", m_SunPosition);
		
		m_VA->Bind();
		glDrawElements(m_VA->GetPrimitiveType(), m_VA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

		m_Shader->Unbind();

	}

	OceanTexture* Ocean::GetDisplacementMap(int which)
	{
		switch (which)
		{
		case 0: return h_dx; break;
		case 1: return h_dy; break;
		case 2: return h_dz; break;
		}
	}

	void Ocean::BindOceanTexture(uint32_t textureID, uint32_t slot)
	{
		glBindTextureUnit(slot, textureID);
	}

	uint32_t Ocean::BitReverse(uint32_t val, uint32_t N)
	{
		int n = (int)std::log2(N);
		// convert val to N-bit:
		int reverse = 0;
		for (int i = 0;i < n; i++)
		{
			if ((val % 2) == 1)
				reverse += std::pow(2, n - i - 1);
			val = val / 2;
		}
		return reverse;
	}

	void Ocean::ComputeFFT(OceanTexture* input)
	{
		cs_FFT->Bind();
		glBindImageTexture(0, Butterfly->GetRendererID(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

		std::dynamic_pointer_cast<OpenGLShader>(cs_FFT)->UploadUniformInt("direction", 0); // horizontal (0) first
		glBindImageTexture(1, input->GetRendererID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(2, buffer->GetRendererID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		int pingpong = 0;
		for (int s = 0;s < log2(m_N); s++)
		{
			std::dynamic_pointer_cast<OpenGLShader>(cs_FFT)->UploadUniformInt("pingpong", pingpong % 2); 
			std::dynamic_pointer_cast<OpenGLShader>(cs_FFT)->UploadUniformInt("stage", s); // 	
			glDispatchCompute(16, 16, 1);
			pingpong++;
		}
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		std::dynamic_pointer_cast<OpenGLShader>(cs_FFT)->UploadUniformInt("direction", 1); // vertical (1) second
		pingpong = 0;
		for (int s = 0;s < log2(m_N); s++)
		{
			std::dynamic_pointer_cast<OpenGLShader>(cs_FFT)->UploadUniformInt("pingpong", pingpong % 2); 
			std::dynamic_pointer_cast<OpenGLShader>(cs_FFT)->UploadUniformInt("stage", s); // 	
			glDispatchCompute(16, 16, 1);
			pingpong++;
		}
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		cs_FFT_post->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(cs_FFT_post)->UploadUniformInt("pingpong", (pingpong) % 2);
		std::dynamic_pointer_cast<OpenGLShader>(cs_FFT_post)->UploadUniformInt("N", m_N);

		// Inversion and permutation
		glBindImageTexture(0, input->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(1, input->GetRendererID(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(2, buffer->GetRendererID(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glDispatchCompute(16, 16, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void Ocean::SetColorVec4(int c, glm::vec4 Color)
	{
		// c controls which color is changed: 0 - emissive, 1 - ambient, 2 - diffuse, 3 - specular
		switch (c)
		{
		case 0: c_Emissive = Color;
		case 1: c_Ambient = Color;
		case 2: c_Diffuse = Color;
		case 3: c_Specular = Color;
		}
	}

	glm::vec4 Ocean::GetColorVec4(int c)
	{
		switch (c)
		{
		case 0: return c_Emissive;
		case 1: return c_Ambient;
		case 2: return c_Diffuse;
		case 3: return c_Specular;
		default: return glm::vec4(0.0, 0.0, 0.0, 0.0);
		}
	}

	OceanTexture::OceanTexture(int N, int M)
	{
		glGenTextures(1, &m_RendererID);
		glActiveTexture(GL_TEXTURE0); // Set the current 'texture unit' in the OpenGL pipeline that is in use.
		glBindTexture(GL_TEXTURE_2D, m_RendererID); // This texture is then bound to the above set unit.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // necessary?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // necessary?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // necessary?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // necessary?
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, M, 0, GL_RGBA, GL_FLOAT, NULL); // specify the 2d image in this texture object
	}

}