#include "hzpch.h"
#include "Ocean.h"
#include <glad/glad.h>

//#include <time.h>
#include <glm/gtc/matrix_transform.hpp>
//#include "WaterPhysicsEngine.h"

namespace Hazel
{

	OceanGeometry::OceanGeometry()
	{
		
	}

	OceanGeometry::~OceanGeometry()
	{
		delete[] buffer;
		delete[] h0_a;
		delete[] h0_b;
		delete[] h_dx;
		delete[] h_dy;
		delete[] h_dz;
		//delete[] h_dn;

	}

	void OceanGeometry::Generate(int N, int L, float A, glm::vec2 wind, int windexponent, float kmax)
	{
		if (N == 128)
		{
			m_ComputeSpace[0] = 8;
			m_ComputeSpace[1] = 8;
		}
		m_kmax = kmax;
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
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformFloat("kmax", m_kmax);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformFloat2("_windDirection", wind);
		// generate noise textures
		srand((unsigned)time(NULL));
		m_Noise = new uint32_t[4];
		float* temp = new float[N * N];
		for (int i = 0;i < 4;i++)
		{
			glGenTextures(1, &m_Noise[i]);
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, m_Noise[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
		glDispatchCompute(m_ComputeSpace[0], m_ComputeSpace[1], 1); // LOCAL SIZE?
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // all operations that access an image are put before the barrier, any operation issued after this lien is but after the barrier.

		// 2: initialize textures to hold the time depentent fourier spectrum.
		h_dx = new OceanTexture(N, N);
		h_dy = new OceanTexture(N, N);
		h_dz = new OceanTexture(N, N);
		//h_dn = new OceanTexture(N, N);
		
		

	}


	void OceanGeometry::UpdateSpectrum()
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
		std::dynamic_pointer_cast<OpenGLShader>(cs_Generate)->UploadUniformFloat("kmin", m_kmax);
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
		glDispatchCompute(m_ComputeSpace[0], m_ComputeSpace[1], 1); // LOCAL SIZE?
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // all operations that access an image are put before the barrier, any operation issued after this lien is but after the barrier.
	}
	void OceanGeometry::Update(float Time)
	{
		std::dynamic_pointer_cast<OpenGLShader>(cs_Update)->UploadUniformInt("N", m_N);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Update)->UploadUniformInt("L", m_L);
		std::dynamic_pointer_cast<OpenGLShader>(cs_Update)->UploadUniformFloat("t", Time);
		glBindImageTexture(0, h_dy->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(1, h_dx->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(2, h_dz->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(3, h0_a->GetRendererID(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(4, h0_b->GetRendererID(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		//glBindImageTexture(5, h_dn->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glDispatchCompute(m_ComputeSpace[0], m_ComputeSpace[1], 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void OceanGeometry::BindOceanTexture(uint32_t textureID, uint32_t slot)
	{
		glBindTextureUnit(slot, textureID);
	}

	void OceanGeometry::LaunchFFTStage1()
	{
		ppy = _FFT_Stage1(h_dy);
		ppx = _FFT_Stage1(h_dx);
		ppz = _FFT_Stage1(h_dz);
		//ppn = _FFT_Stage1(h_dn);
	}

	void OceanGeometry::LaunchFFTStage2()
	{
		_FFT_Stage2(h_dy, ppy);
		_FFT_Stage2(h_dx, ppx);
		_FFT_Stage2(h_dz, ppz);
		//_FFT_Stage2(h_dn, ppn);
	}

	int OceanGeometry::_FFT_Stage1(OceanTexture* input)
	{

		std::dynamic_pointer_cast<OpenGLShader>(cs_FFT)->UploadUniformInt("direction", 0); // horizontal (0) first
		glBindImageTexture(1, input->GetRendererID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(2, buffer->GetRendererID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		int pingpong = 0;
		for (int s = 0; s < log2(m_N); s++)
		{
			std::dynamic_pointer_cast<OpenGLShader>(cs_FFT)->UploadUniformInt("pingpong", pingpong % 2);
			std::dynamic_pointer_cast<OpenGLShader>(cs_FFT)->UploadUniformInt("stage", s); // 	
			glDispatchCompute(m_ComputeSpace[0], m_ComputeSpace[1], 1);
			pingpong++;
		}
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		std::dynamic_pointer_cast<OpenGLShader>(cs_FFT)->UploadUniformInt("direction", 1); // vertical (1) second
		pingpong = 0;
		for (int s = 0; s < log2(m_N); s++)
		{
			std::dynamic_pointer_cast<OpenGLShader>(cs_FFT)->UploadUniformInt("pingpong", pingpong % 2);
			std::dynamic_pointer_cast<OpenGLShader>(cs_FFT)->UploadUniformInt("stage", s); // 	
			glDispatchCompute(m_ComputeSpace[0], m_ComputeSpace[1], 1);
			pingpong++;
		}
		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		return pingpong;
	}

	void OceanGeometry::_FFT_Stage2(OceanTexture* input, int pingpong)
	{
		std::dynamic_pointer_cast<OpenGLShader>(cs_FFT_post)->UploadUniformInt("pingpong", (pingpong) % 2);
		std::dynamic_pointer_cast<OpenGLShader>(cs_FFT_post)->UploadUniformFloat("N", (float)m_N);
		// Inversion and permutation
		glBindImageTexture(0, input->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(1, input->GetRendererID(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(2, buffer->GetRendererID(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glDispatchCompute(m_ComputeSpace[0], m_ComputeSpace[1], 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}


	uint32_t OceanGeometry::GetXYZNID(int xyzn)
	{
		if (xyzn == 0)
			return h_dx->GetRendererID();
		else if (xyzn == 1)
			return h_dy->GetRendererID();
		else if (xyzn == 2)
			return h_dz->GetRendererID();
		else
			return 9999999;
			//return h_dn->GetRendererID();
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // necessary?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // necessary?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // necessary?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // necessary?
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, M, 0, GL_RGBA, GL_FLOAT, NULL); // specify the 2d image in this texture object
	}

	OceanTexture::~OceanTexture()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	// OCEAN HR

	Ocean::Ocean()
	{
		m_SubOcean_0 = new OceanGeometry();
		m_SubOcean_1 = new OceanGeometry();
		m_SubOcean_2 = new OceanGeometry();
	}

	Ocean::~Ocean()
	{
		delete[] m_SubOcean_0;
		delete[] m_SubOcean_1;
		delete[] m_SubOcean_2;
	}


	void OceanGeometry::_tic(unsigned int* qID)
	{
		glGenQueries(2, qID);
		glQueryCounter(qID[0], GL_TIMESTAMP);
	}

	float OceanGeometry::_toc(unsigned int* qID)
	{
		glQueryCounter(qID[1], GL_TIMESTAMP);
		GLint qTimer = 0;
		while (!qTimer)
			glGetQueryObjectiv(qID[1], GL_QUERY_RESULT_AVAILABLE, &qTimer);
		uint64_t t_start, t_stop;
		glGetQueryObjectui64v(qID[0], GL_QUERY_RESULT, &t_start);
		glGetQueryObjectui64v(qID[1], GL_QUERY_RESULT, &t_stop);
		return (t_stop - t_start) / 1000000.0;
	}

	uint32_t Ocean::BitReverse(uint32_t val, uint32_t N)
	{
		int n = (int)std::log2(N);
		// convert val to N-bit:
		int reverse = 0;
		for (int i = 0; i < n; i++)
		{
			if ((val % 2) == 1)
				reverse += std::pow(2, n - i - 1);
			val = val / 2;
		}
		return reverse;
	}

	void Ocean::Generate(float Amplitude, glm::vec2 Wind, int Windexponent, bool lightversion)
	{
		m_LightVersion = lightversion;
		if (m_LightVersion)
			m_Shader = m_ShaderLight;
		else
			m_Shader = m_ShaderDefault;
		// forward the shaders
		m_SubOcean_0->SetCSGenerate(cs_Generate);
		m_SubOcean_1->SetCSGenerate(cs_Generate);
		m_SubOcean_0->SetCSUpdate(cs_Update);
		m_SubOcean_1->SetCSUpdate(cs_Update);
		m_SubOcean_0->SetCSFFT(cs_FFT);
		m_SubOcean_1->SetCSFFT(cs_FFT);
		m_SubOcean_0->SetCSFFTPost(cs_FFT_post);
		m_SubOcean_1->SetCSFFTPost(cs_FFT_post);
		
		// start generating
		m_A = Amplitude;
		m_Wind = Wind;
		m_Windexponent = Windexponent;

		float kmax1 = 100000.0 + (m_LightVersion) * (M_PI * m_N / m_L2);
		m_SubOcean_0->Generate(m_N, m_L0, m_A, m_Wind, m_Windexponent, M_PI * m_N / m_L1);
		m_SubOcean_1->Generate(m_N, m_L1, m_A, m_Wind, m_Windexponent, kmax1);

		if (!m_LightVersion)
		{
			m_SubOcean_2->SetCSFFTPost(cs_FFT_post);
			m_SubOcean_2->SetCSUpdate(cs_Update);
			m_SubOcean_2->SetCSGenerate(cs_Generate);
			m_SubOcean_2->SetCSFFT(cs_FFT);
			m_SubOcean_2->Generate(m_N, m_L2, m_A, m_Wind, m_Windexponent);
		}

		// Generate butterfly texture:
		Butterfly = new OceanTexture((int)std::log2(m_N), m_N);
		cs_Butterfly->Bind();

		std::dynamic_pointer_cast<OpenGLShader>(cs_Butterfly)->UploadUniformInt("N", m_N);
		
		uint32_t* indices = new uint32_t[m_N];
		for (int n = 0; n < m_N; n++)
		{
			indices[n] = BitReverse(n, m_N);
		}

		GLuint ssbo;
		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_N * sizeof(uint32_t), &indices[0], GL_STREAM_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);

		glBindImageTexture(0, Butterfly->GetRendererID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		if (m_N == 128)
			glDispatchCompute(8, 8, 1);
		else
			glDispatchCompute(16, 16, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // all operations that access an image are put before the barrier, any operation issued after this lien is but after the barrier.

		// Prepare the vertex array
		m_VA.reset(Hazel::VertexArray::Create());
		vertices = new OceanVertex[(uint64_t)N_Gridpoints * N_Gridpoints];
		v_indices = new uint32_t[((uint64_t)N_Gridpoints - 1) * (N_Gridpoints - 1) * 6];
		int idx = 0;
		for (int i = 0; i < N_Gridpoints; i++)
		{
			for (int j = 0; j < N_Gridpoints; j++)
			{
				vertices[i * N_Gridpoints + j].x = ((float)j / N_Gridpoints - 0.5);
				vertices[i * N_Gridpoints + j].z = ((float)i / N_Gridpoints - 0.5);
				if ((i < (N_Gridpoints - 1)) && (j < (N_Gridpoints - 1)))
				{
					int home = i * N_Gridpoints + j;
					v_indices[idx++] = home + 1;
					v_indices[idx++] = home;
					v_indices[idx++] = home + N_Gridpoints;
					v_indices[idx++] = home + N_Gridpoints;
					v_indices[idx++] = home + N_Gridpoints + 1;
					v_indices[idx++] = home + 1;
				}
			}
		}

		Ref<VertexBuffer> vb;
		vb.reset(VertexBuffer::Create(vertices, sizeof(OceanVertex) * N_Gridpoints * N_Gridpoints));
		vb->SetLayout({
			{ Hazel::ShaderDataType::Float2, "index" },
			});
		Hazel::Ref<Hazel::IndexBuffer> ib;
		ib.reset(IndexBuffer::Create(v_indices, (N_Gridpoints - 1) * (N_Gridpoints - 1) * 6));
		m_VA->SetPrimitiveType(PrimitiveType::Triangles);
		m_VA->AddVertexBuffer(vb);
		m_VA->SetIndexBuffer(ib);

		// Get texture IDs
		X0 = m_SubOcean_0->GetXYZNID(0);
		Y0 = m_SubOcean_0->GetXYZNID(1);
		Z0 = m_SubOcean_0->GetXYZNID(2);
		//N0 = m_SubOcean_0->GetXYZNID(3);
		X1 = m_SubOcean_1->GetXYZNID(0);
		Y1 = m_SubOcean_1->GetXYZNID(1);
		Z1 = m_SubOcean_1->GetXYZNID(2);
		//N1 = m_SubOcean_1->GetXYZNID(3);
		if (!m_LightVersion)
		{
			X2 = m_SubOcean_2->GetXYZNID(0);
			Y2 = m_SubOcean_2->GetXYZNID(1);
			Z2 = m_SubOcean_2->GetXYZNID(2);
			//N2 = m_SubOcean_2->GetXYZNID(3);
		}
	}

	void Ocean::Update(Timestep ts)
	{
		if (!m_Live)
			return;
		m_Time += ts;
		// new update function
		// Update the spectra to the new timepoint
		cs_Update->Bind();
		m_SubOcean_0->Update(m_Time);
		m_SubOcean_1->Update(m_Time);
		if (!m_LightVersion)
			m_SubOcean_2->Update(m_Time);
		cs_Update->Unbind();

		// Launch the FFTs
		// FFT Stage 1
		cs_FFT->Bind();
		glBindImageTexture(0, Butterfly->GetRendererID(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		m_SubOcean_0->LaunchFFTStage1();
		m_SubOcean_1->LaunchFFTStage1();
		if (!m_LightVersion)
			m_SubOcean_2->LaunchFFTStage1();
		cs_FFT->Unbind();

		// FFT Stage 2
		cs_FFT_post->Bind();
		m_SubOcean_0->LaunchFFTStage2();
		m_SubOcean_1->LaunchFFTStage2();
		if (!m_LightVersion)
			m_SubOcean_2->LaunchFFTStage2();
		cs_FFT_post->Unbind();

	}

	void Ocean::UpdateSpectrum()
	{
		m_SubOcean_0->UpdateSpectrum();
		m_SubOcean_1->UpdateSpectrum();
		if (!m_LightVersion)
			m_SubOcean_2->UpdateSpectrum();
	}

	void Ocean::Render(Camera& camera)
	{


		m_Shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformInt("Ls", SimulationSize);
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformFloat("Lr", RenderSize);
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformInt("L0", m_L0);
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformInt("L1", m_L1);
		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, X0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Y0);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, Z0);
		//glActiveTexture(GL_TEXTURE9);
		//glBindTexture(GL_TEXTURE_2D, N0);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, X1);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, Y1);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, Z1);
		//glActiveTexture(GL_TEXTURE10);
		//glBindTexture(GL_TEXTURE_2D, N1);
		if (!m_LightVersion)
		{
			std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformInt("L2", m_L2);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, X2);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, Y2);
			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, Z2);
			//glActiveTexture(GL_TEXTURE11);
			//glBindTexture(GL_TEXTURE_2D, N2);
		}
		
		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, m_ReflectionTextureID);
		glActiveTexture(GL_TEXTURE13);
		glBindTexture(GL_TEXTURE_2D, m_RefractionTextureID);
		glActiveTexture(GL_TEXTURE14);
		glBindTexture(GL_TEXTURE_2D, m_RefractionDepthTextureID);
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(GL_TEXTURE_2D, m_ReflectionDepthTextureID);
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
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformFloat("u_Waterlevel", m_Waterlevel);
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformInt("NORMAL", NORMAL);
		m_VA->Bind();
		glDrawElements(m_VA->GetPrimitiveType(), m_VA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

		m_Shader->Unbind();
		
	}

}

