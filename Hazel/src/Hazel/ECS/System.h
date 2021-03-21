#pragma once

#include "ECSTypes.h"
#include <set>

namespace Hazel
{
	class System
	{
	public:
		std::set<Entity> mEntities;
	};
}
