#pragma once
#include <array>
#include <unordered_map>
#include <SFML/Graphics/Color.hpp>

namespace globals
{
	constexpr int k_playerAmount = 2;
	constexpr float k_carSpeed = 100.f;

	constexpr int k_screenWidth = 800;
	constexpr int k_screenHeight = 600;
	
	template<typename T, typename Ty>
	inline bool is_value_in_map(const std::unordered_map<T, Ty>& map, const T& key)
	{
		return map.find(key) != map.end();
	}
	
}
