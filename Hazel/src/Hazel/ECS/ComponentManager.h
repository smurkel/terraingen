#pragma once

#include "ComponentArray.h"
#include "ECSTypes.h"
#include <any>
#include <cassert>
#include <memory>
#include <unordered_map>

namespace Hazel
{

	class ComponentManager
	{
	public:
		template<typename T>
		void RegisterComponent()
		{

			const char* typeName = typeid(T).name(); // name of the Component is derived from the name of that component's class (i.e. Transform)

			assert(mComponentTypes.find(typeName) == mComponentTypes.end(), "Registering component type more than once");

			// Add this component type to the component type map

			mComponentTypes.insert({ typeName, mNextComponentType });
			mComponentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });

			++mNextComponentType;
		}
		template<typename T>
		ComponentType GetComponentType()
		{
			const char* typeName = typeid(T).name();
			assert(mComponentTypes.find(typeName) != mComponentTypes.end(), "Component not registered before use.");
			return mComponentTypes[typeName];
		}
		template<typename T>
		void AddComponent(Entity entity, T component)
		{
			// Add an entities component to the array
			GetComponentArray<T>()->InsertData(entity, component);
		}

		template<typename T>
		void RemoveComponent(Entity entity)
		{
			GetComponentArray<T>()->RemoveData(entity);
		}
		template<typename T>
		T& GetComponent(Entity entity)
		{
			return GetComponentArray<T>()->GetData(entity);
		}
		void EntityDestroyed(Entity entity)
		{
			for (auto const& pair : mComponentArrays)
			{
				auto const& component = pair.second;
				component->EntityDestroyed(entity);
			}
		}
	private:
		std::unordered_map<const char*, ComponentType> mComponentTypes{};
		std::unordered_map<const char*, std::shared_ptr<IComponentArray>> mComponentArrays{};

		// The component type to be assigned to the next registered component - starting at 0.
		ComponentType mNextComponentType{};

		// convenience function to get the statically casted pointer to the ComponentArray of type T.
		template<typename T>
		std::shared_ptr<ComponentArray<T>> GetComponentArray()
		{
			const char* typeName = typeid(T).name();
			assert(mComponentTypes.find(typeName) != mComponentTypes.end(), "Component not registered before use.");
			return std::static_pointer_cast<ComponentArray<T>>(mComponentArrays[typeName]);
		}
	};
}