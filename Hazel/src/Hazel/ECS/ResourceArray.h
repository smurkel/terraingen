#pragma once

#include "Hazel/Renderer/Model.h"
#include "Hazel/Renderer/Shader.h"
#include <unordered_map>
#include "Hazel/Log.h"

namespace Hazel
{
	class IResourceArray
	{
	public:
		virtual ~IResourceArray() = default;
	};

	template<typename T>
	class ResourceArray : public IResourceArray
	{
	public:
		inline bool Exists(std::string filepath) { return mResourceArray.find(filepath) != mResourceArray.end(); }
		Ref<T> Add(std::string filepath)
		{
			HZ_CORE_INFO("Loading resource: {0}", filepath);
			Ref<T> rsc = T::Create(filepath);
			std::pair<std::string, Ref<T>> newResource(filepath, rsc);
			mResourceArray.insert(newResource);
			return rsc;
		}
		Ref<T> Get(std::string filepath) 
		{ 
			if (Exists(filepath))
				return mResourceArray[filepath];
			else
				return Add(filepath);
		}
	private:
		std::unordered_map<std::string, Ref<T>> mResourceArray{};
	};
}