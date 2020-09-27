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

		uint32_t GetRendererID() { return m_RendererID; }
	private:
		uint32_t m_RendererID;
	};

	// This one will be computed using compute shaders.
	class Ocean
	{
	public:
		Ocean(); 
		void Generate(const int N, const int L = 1000, const float A = 1.0, const glm::vec2 wind = { 5.0, 5.0 }, const int windexponent = 2);

		void Update(Timestep ts);
		void UpdateSpectrum();
		void ComputeFFT(OceanTexture* input);
		void Render(Camera& camera);
		//void RenderBoat(Camera& camera, Boat& boat);

		void SetColorVec4(int c, glm::vec4 Color);
		glm::vec4 GetColorVec4(int c);

		void SetSunPos(glm::vec3 sunpos) { m_SunPosition = sunpos; }
		glm::vec3 GetSunPos() { return m_SunPosition; }

		glm::vec2 GetWind() { return m_Wind; }
		void SetWind(glm::vec2& wind) { m_Wind = wind; }

		float GetAmplitude() { return m_A; }
		void SetAmplitude(float A) { m_A = A; }

		void SetMurkiness(float murkiness) { m_Murkiness = murkiness; }
		float GetMurkiness() { return m_Murkiness; }

		uint32_t GetN() { return m_N; }
		int GetL() { return m_L; }
		OceanTexture* GetDisplacementMap(int which);
		void BindOceanTexture(uint32_t TextureID, uint32_t Slot);
		void InvDynamic() { m_Live = !m_Live; }

		void SetReflectionTextureID(uint32_t id) { m_ReflectionTextureID = id; }
		void SetRefractionTextureID(uint32_t id) { m_RefractionTextureID = id; }
		void SetRefractionDepthTextureID(uint32_t id) { m_RefractionDepthTextureID = id; }
	private:
		uint32_t BitReverse(uint32_t val, uint32_t N);
		struct OceanVertex
		{
			float x, z;
		};
		// TO DO:
		// set W0 in spectrum_update - frequencies are currently not quantized
		// glDispatchCompute(..., ..., 1) which size is best? hard-coded as 16 16 in multiple places.
		// Rate now hard-coded to time = time * 0.05;

		// Upon construction, all the necessary shaders are compiled. 
		float m_Time = 0;
		uint32_t m_N = 0;
		int m_L = 1000;
		glm::vec2 m_Wind = { 0.0, 0.0 };
		int m_Windexponent = 6;
		float m_A = 1.0;
		bool m_Live = true;
		float m_Murkiness = 10.0;
		uint32_t* m_Noise;
		// Compute shaders and buffers:
		OceanTexture* buffer;
		OceanTexture* h0_a;
		OceanTexture* h0_b;
		OceanTexture* Butterfly;
		OceanTexture* h_dx;
		OceanTexture* h_dy;
		OceanTexture* h_dz;
		Ref<Shader> cs_Generate = Shader::Create("assets/shaders/compute/Spectrum_generate.glsl"); 
		Ref<Shader> cs_Update = Shader::Create("assets/shaders/compute/Spectrum_update.glsl");
		Ref<Shader> cs_Butterfly = Shader::Create("assets/shaders/compute/ButterflyTexture.glsl");
		Ref<Shader> cs_FFT = Shader::Create("assets/shaders/compute/FFT.glsl");
		Ref<Shader> cs_FFT_post = Shader::Create("assets/shaders/compute/FFT_inversionpermutation.glsl");
		// Rendering shader, buffer, colours, etc.
		Ref<Shader> m_Shader = Shader::Create("assets/shaders/OceanShader.glsl");
		Ref<VertexArray> m_VA;
		OceanVertex* vertices;
		uint32_t* v_indices;
		uint32_t m_ReflectionTextureID = 0;
		uint32_t m_RefractionTextureID = 0;
		uint32_t m_RefractionDepthTextureID = 0;
		glm::vec3 m_SunPosition = { 20.0, 10.0, 80.0 };
		glm::vec4 c_Emissive = { 0.0, 0.188, 0.258, 1.0 };
		glm::vec4 c_Ambient = { 0.1, 0.24, 0.25, 1.0 };
		glm::vec4 c_Diffuse = { 0.15, 0.33, 0.36, 1.0 };
		glm::vec4 c_Specular = { 1.34, 0.8, 0.4, 1.0 };
	};

	
}
