#pragma once


#include "Platform/OpenGL/OpenGLShader.h"
#include "Hazel/Core/Timestep.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Camera.h"



namespace Hazel
{
	class OceanTexture
	{
	public:
		OceanTexture(int N, int M);
		~OceanTexture();
		uint32_t GetRendererID() { return m_RendererID; }
	private:
		uint32_t m_RendererID;
	};

	// This one will be computed using compute shaders.
	class OceanGeometry
	{
	public:
		OceanGeometry();
		~OceanGeometry();
		void Generate(const int N, const int L = 1000, const float A = 1.0, const glm::vec2 wind = { 5.0, 5.0 }, const int windexponent = 2, float kmax = 1000000.0);

		void Update(float Time);
		void UpdateSpectrum();
		void LaunchFFTStage1();
		void LaunchFFTStage2();
		
		uint32_t GetXYZNID(int xyzn);

		void SetCSGenerate(Ref<Shader> cs) { cs_Generate = cs; }
		void SetCSUpdate(Ref<Shader> cs) { cs_Update = cs; }
		void SetCSFFT(Ref<Shader> cs) { cs_FFT = cs; }
		void SetCSFFTPost(Ref<Shader> cs) { cs_FFT_post = cs; }

		void BindOceanTexture(uint32_t TextureID, uint32_t Slot);
		void _tic(unsigned int* qID);
		float _toc(unsigned int* qID);

		OceanTexture* GetDisplacementMap(int xyzn)
		{
			switch (xyzn)
			{
			case 0:	return h_dx;
			case 1:	return h_dy;
			case 2: return h_dz;
			case 3: return h_dn;
			}
		}
	private:
		int _FFT_Stage1(OceanTexture* input);
		void _FFT_Stage2(OceanTexture* input, int pingpong);
		int ppy = 0;
		int ppx = 0;
		int ppz = 0;
		int ppn = 0;
		// Upon construction, all the necessary shaders are compiled. 
		uint32_t m_N = 0;
		int m_ComputeSpace[2] = { 16, 16 };
		int m_L = 0;
		glm::vec2 m_Wind = { 0.0, 0.0 };
		int m_Windexponent = 0;
		float m_A = 0.0;
		float m_kmax = 0.0;
		// Compute shaders and buffers:
		uint32_t* m_Noise = nullptr;
		OceanTexture* buffer = nullptr;
		OceanTexture* h0_a = nullptr;
		OceanTexture* h0_b = nullptr;
		OceanTexture* h_dx = nullptr;
		OceanTexture* h_dy = nullptr;
		OceanTexture* h_dz = nullptr;
		OceanTexture* h_dn = nullptr;
		// Rendering shader, buffer, colours, etc.
		Ref<Shader> cs_Generate;
		Ref<Shader> cs_Update;
		Ref<Shader> cs_FFT;
		Ref<Shader> cs_FFT_post;
	};

	class Ocean
	{
	public:
		Ocean();
		~Ocean();
		void Generate(int resolution, float Amplitude, glm::vec2 Wind, int Windexponent, bool lightversion = true);
		void Update(Timestep ts);
		void UpdateSpectrum();

		void Render(Camera& camera);

		void SetReflectionTextureID(uint32_t id) { m_ReflectionTextureID = id; }
		void SetRefractionTextureID(uint32_t id) { m_RefractionTextureID = id; }
		void SetRefractionDepthTextureID(uint32_t id) { m_RefractionDepthTextureID = id; }
		void SetReflectionDepthTextureID(uint32_t id) { m_ReflectionDepthTextureID = id; }

		void SetMurkiness(float murkiness) { m_Murkiness = murkiness; }
		float GetMurkiness() { return m_Murkiness; }
		void SetColorVec4(int c, glm::vec4 Color);
		glm::vec4 GetColorVec4(int c);
		void SetWaterlevel(float level) { m_Waterlevel = level; }
		float GetWaterlevel() { return m_Waterlevel; }
		void SetSunPos(glm::vec3 sunpos) { m_SunPosition = sunpos; }
		glm::vec3 GetSunPos() { return m_SunPosition; }
		glm::vec2 GetWind() { return m_Wind; }
		void SetWind(glm::vec2& wind) { m_Wind = wind; }
		float GetAmplitude() { return m_A; }
		void SetAmplitude(float A) { m_A = A; }
		uint32_t GetN() { return m_N; }
		void SetSimulationSize(float size) { SimulationSize = size; }
		float GetSimulationSize() { return SimulationSize; }

		// INTERACTIVE FUNCTIONS
		void TogglePause() { m_Live = !m_Live; }
		uint32_t X0 = 0, X1 = 0, X2 = 0, Y0 = 0, Y1 = 0, Y2 = 0, Z0 = 0, Z1 = 0, Z2 = 0;
		uint32_t N0 = 0, N1 = 0, N2 = 0;
		int GetSuboceanScale(int select)
		{
			switch (select)
			{
				case 0: return m_L0;
				case 1: return m_L1;
				case 2: return m_L2;
			}
		}
		float GetHeightScaling() { return ((float)RenderSize / ((float)SimulationSize)); }
		// debug
		int _GetBFTextureID() { return Butterfly->GetRendererID(); }
	private:
		bool m_LightVersion = false;
		uint32_t BitReverse(uint32_t val, uint32_t N);
		struct OceanVertex
		{
			float x, z;
		};
		
		glm::vec2 m_Wind = { 0.0, 0.0 };
		int m_Windexponent = 4;
		float m_A = 1.0;
		int m_L0 = 700;
		int m_L1 = 43;
		int m_L2 = 25;
		int m_N = 0;
		float m_Murkiness = 2.0; // rename: transparency
		float m_Waterlevel = 0.0;
		float m_Time = 0.0;
		OceanGeometry* m_SubOcean_0;
		OceanGeometry* m_SubOcean_1;
		OceanGeometry* m_SubOcean_2;

		OceanTexture* Butterfly;
		Ref<Shader> cs_Generate = Shader::Create("assets/shaders/compute/Spectrum_generate.glsl");
		Ref<Shader> cs_Update = Shader::Create("assets/shaders/compute/Spectrum_update.glsl");
		Ref<Shader> cs_Butterfly = Shader::Create("assets/shaders/compute/ButterflyTexture.glsl");
		Ref<Shader> cs_FFT = Shader::Create("assets/shaders/compute/FFT.glsl");
		Ref<Shader> cs_FFT_post = Shader::Create("assets/shaders/compute/FFT_inversionpermutation.glsl");

		int N_Gridpoints = 128; // Resolution of the mesh onto which the ocean is rendered.
		float SimulationSize = 2500; // Size of the seascape that is contained within the render plane (larger = more repetitions of the underlying geometry textures)
		float RenderSize = 128; // On-screen size of the plane in which water is rendered
		bool m_Live = true;
		Ref<Shader> m_ShaderDefault = Shader::Create("assets/shaders/OceanShaderSmooth.glsl");
		Ref<Shader> m_ShaderLight = Shader::Create("assets/shaders/OceanShaderSmoothLight.glsl");
		Ref<Shader> m_Shader;
		Ref<VertexArray> m_VA;
		OceanVertex* vertices;
		uint32_t* v_indices;

		uint32_t m_ReflectionTextureID = 0;
		uint32_t m_RefractionTextureID = 0;
		uint32_t m_RefractionDepthTextureID = 0;
		uint32_t m_ReflectionDepthTextureID = 0;

		glm::vec3 m_SunPosition = { 1000.0, 2000.0, 0.0 };
		glm::vec4 c_Emissive = { 255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0} ;
		glm::vec4 c_Ambient = { 1.0 / 255.0, 18.0 / 255.0, 33.0 / 255.0, 255.0 / 255.0 };
		glm::vec4 c_Diffuse = { 30.0 / 255.0, 58.0 / 255.0, 78.0 / 255.0, 255.0 / 255.0 };
		glm::vec4 c_Specular = { 103.0 / 255.0, 103.0 / 255.0, 103.0 / 255.0, 255.0 / 255.0 };
	};

	
}
