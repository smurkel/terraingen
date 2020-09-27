#include "hzpch.h"
#include "Entity.h"

namespace Hazel
{
	EntityStack::EntityStack()
	{
	}

	EntityStack::~EntityStack()
	{
		for (Entity* entity : m_Entities)
			delete entity;
	}

	void EntityStack::Add(Entity* entity)
	{
		m_Entities.emplace_back(entity);
	}

	void EntityStack::Delete(Entity* entity)
	{
		auto it = std::find(m_Entities.begin(), m_Entities.end(), entity);
		if (it != m_Entities.end())
		{
			m_Entities.erase(it);
		}
	}
}