#include "hzpch.h";
#include "Skybox.h"

#include <glad/glad.h>
#include "stb_image.h"

namespace Hazel {

	Cubemap::Cubemap()
	{
		glGenTextures(1, &m_RendererID);
	}

	void Cubemap::LoadTexture(std::vector<std::string> facepaths)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		unsigned char* pxd;
		for (int i = 0; i < facepaths.size(); i++)
		{
			pxd = stbi_load(facepaths[i].c_str(), &m_Width, &m_Height, &m_Channels, 0);
			HZ_CORE_INFO(facepaths[i].c_str());
			// order of face indices (0->5): +x, -x, +y, -y, +z, -z
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pxd);
			stbi_image_free(pxd);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	Cubemap::~Cubemap()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	Skybox::Skybox()
	{
		float m_Vertices[24] = {
			1.0, -1.0, 1.0,
			1.0, -1.0, -1.0,
			1.0, 1.0, -1.0,
			1.0, 1.0, 1.0,
			-1.0, -1.0, 1.0,
			-1.0, -1.0, -1.0,
			-1.0, 1.0, -1.0,
			-1.0, 1.0, 1.0,
		};
		uint32_t m_Indices[36] = {
			4, 0, 3,
			4, 3, 7,
			0, 1, 2,
			0, 2, 3,
			1, 5, 6,
			1, 6, 2,
			5, 4, 7,
			5, 7, 6,
			7, 3, 2,
			7, 2, 6,
			0, 5, 1,
			0, 4, 5,

		};

		Hazel::Ref<Hazel::VertexBuffer> m_VB;
		Hazel::Ref<Hazel::IndexBuffer> m_IB;

		m_VA.reset(Hazel::VertexArray::Create());
		m_VA->SetPrimitiveType(Hazel::PrimitiveType::Triangles);

		m_VB.reset(Hazel::VertexBuffer::Create(&m_Vertices[0], sizeof(float) * 24));
		m_VB->SetLayout({
			{Hazel::ShaderDataType::Float3, "position"},
			});

		m_IB.reset(Hazel::IndexBuffer::Create(&m_Indices[0], 36));

		m_VA->AddVertexBuffer(m_VB);
		m_VA->SetIndexBuffer(m_IB);

	}

	Skybox::~Skybox()
	{
	}

	void Skybox::SetCubemap(int preset_index)
	{
		std::vector<std::string> arg;
		switch (preset_index)
		{
		case 0:
			arg.push_back("assets/textures/skybox/preset0/right.jpg");
			arg.push_back("assets/textures/skybox/preset0/left.jpg");
			arg.push_back("assets/textures/skybox/preset0/top.jpg");
			arg.push_back("assets/textures/skybox/preset0/bottom.jpg");
			arg.push_back("assets/textures/skybox/preset0/front.jpg");
			arg.push_back("assets/textures/skybox/preset0/back.jpg");
			break;
		case 1:
			arg.push_back("assets/textures/skybox/preset1/+x.png");
			arg.push_back("assets/textures/skybox/preset1/-x.png");
			arg.push_back("assets/textures/skybox/preset1/+y.png");
			arg.push_back("assets/textures/skybox/preset1/-y.png");
			arg.push_back("assets/textures/skybox/preset1/+z.png");
			arg.push_back("assets/textures/skybox/preset1/-z.png");
			break;
		default:
			HZ_CORE_ERROR("Skybox preset index {0} is not an available cubemap", preset_index);
		}
		
		m_Cubemap.LoadTexture(arg);
	}

	void Skybox::Render(Camera& camera)
	{
		glDepthFunc(GL_LEQUAL);
		m_Shader->Bind();
		glm::mat4 PMat = camera.GetProjectionMatrix();
		glm::mat4 VMat = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		glm::mat4 VPMat = PMat * VMat;
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformMat4("u_ViewProjectionMatrix", VPMat);
		m_VA->Bind();
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_Cubemap.GetRendererID());
		glDrawElements(GL_TRIANGLES, m_VA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
		m_Shader->Unbind();
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
	}

	

}