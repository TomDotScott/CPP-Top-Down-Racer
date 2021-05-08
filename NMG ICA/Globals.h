#pragma once
#include <unordered_map>

namespace globals
{
	constexpr int k_playerAmount = 2;
	constexpr float k_carTrackSpeed = 150.f;
	constexpr float k_carGrassSpeed = 50.f;

	const std::string k_reservedServerUsername = "SERVER";
	
	constexpr int k_screenWidth = 1024;
	constexpr int k_screenHeight = 768;
	
	template<typename T, typename Ty>
	inline bool is_value_in_map(const std::unordered_map<T, Ty>& map, const T& key)
	{
		return map.find(key) != map.end();
	}
	
}
