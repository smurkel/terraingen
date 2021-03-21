#pragma once

#include "System.h"
#include "ECSTypes.h"
#include <cassert>
#include <memory>
#include <unordered_map>

namespace Hazel
{
	class SystemManager
	{
	public:
		template<typename T>
		std::shared_ptr<T> RegisterSystem()
		{
			const char* typeName = typeid(T).name();
			assert(mSystems.find(typeName) == mSystems.end(), "Registering systems more than once");
			auto system = std::make_shared<T>();
			mSystems.insert({ typeName, system });
			return system;
		}

		template<typename T>
		void SetSignature(Signature signature)
		{
			const char* typeName = typeid(T).name();
			assert(mSystems.find(typeName) != mSystems.end(), "System used before registered");
			mSignatures.insert({ typeName, signature });
		}

		void EntityDestroyed(Entity entity)
		{
			for (auto const& pair : mSystems)
			{
				auto const& system = pair.second;
				system->mEntities.erase(entity);
			}
		}

		void EntitySignatureChanged(Entity entity, Signature entitySignature)
		{
			// Notify each system that an entity's signature changed
			for (auto const& pair : mSystems)
			{
				auto const& type = pair.first;
				auto const& system = pair.second;
				auto const& systemSignature = mSignatures[type];

				// Entity signature matches system signature - insert into set
				if ((entitySignature & systemSignature) == systemSignature) // '(a == b) == b': all 1's in a set to 0 if b is 0 there as well. If b a subset of a, 'a == b' returns b.
				{
					system->mEntities.insert(entity);
				}
				else
				{
					system->mEntities.erase(entity);
				}
			}
		}
	private:
		std::unordered_map<const char*, Signature> mSignatures{};
		std::unordered_map<const char*, std::shared_ptr<System>> mSystems{};
	};
}