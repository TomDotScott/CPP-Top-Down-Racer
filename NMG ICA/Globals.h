#pragma once
#include <array>
#include <unordered_map>
#include <SFML/Graphics/Color.hpp>

namespace globals
{
	constexpr int k_playerAmount = 4;
	constexpr float k_carSpeed = 100.f;

	template<typename T, typename Ty>
	inline bool is_value_in_map(const std::unordered_map<T, Ty>& map, const T& key)
	{
		return map.find(key) != map.end();
	}
	
}
