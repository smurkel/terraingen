#pragma once

#include "ECSTypes.h"
#include <array>
#include <cassert>
#include <queue>

namespace Hazel
{
	class EntityManager
	{
		// This class is in charge of distributing entity IDs and keeping record of which IDs are in use and which are not.
		// Std::queue provides a useful first-in, first-out way of doing so.
	public:
		EntityManager()
		{
			for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
			{
				mAvailableEntities.push(entity);
			}
		}
		Entity CreateEntity()
		{
			assert(mLivingEntityCount < MAX_ENTITIES, "Maximum number of existing entities reached.");
			Entity id = mAvailableEntities.front();
			mAvailableEntities.pop();
			++mLivingEntityCount;
			return id;
		}
		void DestroyEntity(Entity entity)
		{
			assert(entity < MAX_ENTITIES, "Entity out of range.");
			mSignatures[entity].reset();
			mAvailableEntities.push(entity);
			--mLivingEntityCount;
		}
		void SetSignature(Entity entity, Signature signature)
		{
			assert(entity < MAX_ENTITIES, "Entity out of range.");
			mSignatures[entity] = signature;
		}
		Signature GetSignature(Entity entity)
		{
			assert(entity < MAX_ENTITIES, "Entity out of range.");
			return mSignatures[entity];
		}
	private:
		std::queue<Entity> mAvailableEntities{}; // the {} is a way of initializing this variable. Explained here: https://en.cppreference.com/w/cpp/language/list_initialization#copy-list-initialization
		std::array<Signature, MAX_ENTITIES> mSignatures{};
		uint32_t mLivingEntityCount{};
	};
}