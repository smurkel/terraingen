#pragma once

#include <bitset>

namespace Hazel
{
	//	ENTITY
	using Entity = std::uint32_t;
	const Entity MAX_ENTITIES = 1024;

	// COMPONENTS
	using ComponentType = std::uint8_t;
	const ComponentType MAX_COMPONENTS = 32;

	using Signature = std::bitset<MAX_COMPONENTS>; // used to store which components an entity has, and which components a system needs.

}