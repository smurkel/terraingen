#include "hzpch.h"
#include "Model.h"

#include <fstream>


namespace Hazel {


	Ref<Model> Model::Create(const std::string& filepath)
	{
		return std::make_shared<Model3D>(filepath);
	}

	Model3D::Model3D(const std::string& filename)
	{
		LoadObjFile(filename);
	}
	void Model3D::Bind() const
	{
		m_VertexArray->Bind();
	}
	bool Model3D::LoadObjFile(const std::string& filepath)
	{
		// Load the model data: vertices, normals, uvs
		std::vector< glm::vec3 > _vertices;
		std::vector< glm::vec3 > _normals;
		std::vector<glm::vec2> _uvs;
		//std::vector<PhysicsProbe> _physics_probes;
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
				int temp = fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				_vertices.push_back(vertex);
			}

			// normal
			if (strcmp(lineHeader, "vn") == 0)
			{
				glm::vec3 normal;
				int temp = fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				_normals.push_back(normal);
			}

			// uv
			if (strcmp(lineHeader, "vt") == 0)
			{
				glm::vec2 uv;
				int temp = fscanf(file, "%f %f\n", &uv.x, &uv.y);
				_uvs.push_back(uv);
			}
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
		ModelVertex* _Vertices = new ModelVertex[nVertices];
		uint32_t* _Indices = new uint32_t[3 * nFaces];
		for (int f = 0; f < nFaces; f++)
		{
			for (int i = 0; i < 3; i++)
			{
				int idx = f * 3 + i;
				_Vertices[idx].position = _vertices[(_indices[f].v_i[i]) - 1];
				_Vertices[idx].normal = _normals[(_indices[f].vn_i[i]) - 1];
				_Vertices[idx].uv = _uvs[(_indices[f].vt_i[i]) - 1];
				_Indices[idx] = idx;
			}
		}

		m_VertexArray.reset(Hazel::VertexArray::Create());
		m_VertexArray->SetPrimitiveType(Hazel::PrimitiveType::Triangles);

		Ref<Hazel::VertexBuffer> VB;
		VB.reset(Hazel::VertexBuffer::Create(&_Vertices[0], sizeof(ModelVertex) * nVertices));
		VB->SetLayout({
			{Hazel::ShaderDataType::Float3, "Position"},
			{Hazel::ShaderDataType::Float3, "Normal"},
			{Hazel::ShaderDataType::Float2, "UV"},
			});

		Ref<Hazel::IndexBuffer> IB;
		IB.reset(Hazel::IndexBuffer::Create(&_Indices[0], nFaces * 3));

		m_VertexArray->AddVertexBuffer(VB);
		m_VertexArray->SetIndexBuffer(IB);
		return true;

	}
}

