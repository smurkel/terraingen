#pragma once

#include "Library.h"
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Core/Timestep.h"

#include "Hazel/Entity/Model.h"
#include "Hazel/Ocean/WaterPhysicsEngine.h"
namespace Hazel
{

	class Entity
	{
	public:
		virtual void Render() = 0;
		virtual void OnUpdate(Timestep ts, WaterPhysicsEngine& WPE) = 0;

		void SetModel(Ref<Hazel::Model>& model) { m_Model = model; }
		void SetShader(Ref<Hazel::Shader>& shader) { m_Shader = shader; }

		Ref<Model> GetModel() { return m_Model; }
		Ref<Shader> GetShader() { return m_Shader; }
	private:
		Ref<Hazel::Model> m_Model;
		Ref<Hazel::Shader> m_Shader;
	};

	class EntityStack
	{
	public:
		EntityStack();
		~EntityStack();

		void Add(Entity* entity);
		void Delete(Entity* entity);
		std::vector<Entity*>::iterator begin() { return m_Entities.begin(); }
		std::vector<Entity*>::iterator end() { return m_Entities.end(); }
	private:
		std::vector<Entity*> m_Entities;
	};

}