#pragma once

#include <unordered_map>
#include <cassert>
#include "Hazel/ECS/ResourceArray.h"
#include "Hazel/Renderer/Model.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Texture.h"

namespace Hazel
{

	class ResourceManager
	{
	public:
		template<typename T>
		void RegisterResource()
		{
			const char* typeName = typeid(T).name();
			assert(mResourceTypes.find(typeName) == mResourceTypes.end(), "Registering resource type more than once.");
			mResourceTypes.insert({ typeName , mNextResourceType });
			mResourceArrays.insert({ typeName, std::make_shared<ResourceArray<T>>() });
			++mNextResourceType;
		}

		template<typename T>
		Ref<T> GetResource(const std::string& filepath)
		{
			return GetResourceArray<T>()->Get(filepath);
		}
	private:
		std::unordered_map<const char*, int> mResourceTypes{};
		std::unordered_map<const char*, std::shared_ptr<IResourceArray>> mResourceArrays{};
		int mNextResourceType{};

		template<typename T>
		std::shared_ptr<ResourceArray<T>> GetResourceArray()
		{
			const char* typeName = typeid(T).name();

			assert(mResourceTypes.find(typeName) != mResourceTypes.end(), "Resource not registered before use.");

			return std::static_pointer_cast<ResourceArray<T>>(mResourceArrays[typeName]);
		}
	};

}