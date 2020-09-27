#include "hzpch.h"
#include "Model.h"
#include <fstream>

namespace Hazel {

	Model::Model(std::string filepath, std::string texture_filepath)
	{
		bool temp = LoadObj(filepath);
		HZ_CORE_ASSERT(temp, ".obj not loaded");
		temp = LoadTexture(texture_filepath);
		HZ_CORE_ASSERT(temp, "texture not loaded");
	}

	bool Model::LoadObj(std::string filepath)
	{
		// Load the model data: vertices, normals, uvs
		std::vector< glm::vec3 > _vertices;
		std::vector< glm::vec3 > _normals;
		std::vector<glm::vec2> _uvs;
		std::vector<PhysicsProbe> _physics_probes;
		std::vector< FaceIndex > _indices;


		// OPEN THE FILE AND READ IT UNTIL THE END
		char* path = new char[filepath.length() + 1];
		std::strcpy(path, filepath.c_str());
		FILE* file = fopen(path, "r");
		if (file == NULL)
		{
			HZ_CORE_ASSERT(0, "File can not be opened in Model::LoadObj");
			return false;
		}
		
		while (1) 
		{
			// read line
			char lineHeader[128];
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
				break;

			// vertex
			if (strcmp(lineHeader, "v") == 0)
			{
				glm::vec3 vertex;
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				_vertices.push_back(vertex);
			}

			// normal
			if (strcmp(lineHeader, "vn") == 0)
			{
				glm::vec3 normal;
				fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				_normals.push_back(normal);
			}

			// uv
			if (strcmp(lineHeader, "vt") == 0)
			{
				glm::vec2 uv;
				fscanf(file, "%f %f\n", &uv.x, &uv.y);
				_uvs.push_back(uv);
			}
			// floaters / physics probes
			if (strcmp(lineHeader, "fl") == 0)
			{
				PhysicsProbe temp;
				fscanf(file, "%f %f %f %f %f %f %f %f\n", &temp.XYZd.x, &temp.XYZd.y, &temp.XYZd.z, &temp.XYZd.w, &temp.NxNyNzV.x, &temp.NxNyNzV.y, &temp.NxNyNzV.z, &temp.NxNyNzV.w);
				_physics_probes.push_back(temp);
			}
			// face
			else if (strcmp(lineHeader, "f") == 0)
			{
				FaceIndex fi;
				unsigned int I[9];
				int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d \n", &I[0], &I[1], &I[2], &I[3], &I[4], &I[5], &I[6], &I[7], &I[8]);
				if (matches != 9)
				{
					HZ_CORE_ERROR("Number of indices per face does not correspond to format 3 x (v/vn/vt)! \n");
					return false;
				}
				fi.v_i = { I[0],I[3] ,I[6] };
				fi.vt_i = { I[1],I[4] ,I[7] };
				fi.vn_i = { I[2],I[5] ,I[8] };
				_indices.push_back(fi);
			}
		}
		int nFaces = _indices.size();
		int nVertices = 3 * nFaces;
		m_Vertices = new ModelVertex[nVertices];
		m_Indices = new uint32_t[3 * nFaces];
		for (int f = 0;f < nFaces;f++)
		{
			for (int i = 0;i < 3;i++)
			{
				int idx = f * 3 + i;
				m_Vertices[idx].position = _vertices[(_indices[f].v_i[i]) - 1];
				m_Vertices[idx].normal = _normals[(_indices[f].vn_i[i]) - 1];
				m_Vertices[idx].uv = _uvs[(_indices[f].vt_i[i]) - 1];
				m_Indices[idx] = idx;
			}
		}

		m_ProbeCount = _physics_probes.size();
		m_Probes = new PhysicsProbe[m_ProbeCount];
		for (int p = 0;p < m_ProbeCount;p++)
		{
			m_Probes[p] = _physics_probes[p];
		}

		m_VA.reset(Hazel::VertexArray::Create());
		m_VA->SetPrimitiveType(Hazel::PrimitiveType::Triangles);

		m_VB.reset(Hazel::VertexBuffer::Create(&m_Vertices[0], sizeof(ModelVertex) * nVertices));
		m_VB->SetLayout({
			{Hazel::ShaderDataType::Float3, "position"},
			{Hazel::ShaderDataType::Float3, "normal"},
			{Hazel::ShaderDataType::Float2, "uv"},
			});

		m_IB.reset(Hazel::IndexBuffer::Create(&m_Indices[0], nFaces * 3));

		m_VA->AddVertexBuffer(m_VB);
		m_VA->SetIndexBuffer(m_IB);

	}

	bool Model::LoadTexture(std::string filepath)
	{
		m_Texture = Hazel::Texture2D::Create(filepath);
		if (m_Texture)
			return true;
		return false;
	}


	Model::~Model()
	{
		if (m_Indices) delete[] m_Indices;
		if (m_Vertices) delete[] m_Vertices;
	}

}

