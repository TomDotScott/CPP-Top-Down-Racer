#pragma once
#include <unordered_map>

namespace globals
{
	constexpr int k_playerAmount = 2;
	constexpr float k_carTrackSpeed = 150.f;
	constexpr float k_carGrassSpeed = 50.f;
	constexpr float k_carStartingRotation = 1.5708f;
	
	constexpr float k_carSpriteWidth = 20.f;
	constexpr float k_carSpriteHeight = 34.f;

	constexpr float k_carOriginX = 10.f;
	constexpr float k_carOriginY = 17.f;

	constexpr int k_totalLaps = 1;
	
	constexpr int k_numCheckPoints = 6;
	constexpr float k_checkPointWidth = 50.f;
	constexpr float k_checkPointHeight = 150.f;
	
	inline const std::string k_reservedServerUsername = "SERVER";

	inline const sf::Vector2f k_checkPointColliderSize { globals::k_checkPointWidth, globals::k_checkPointHeight };
	
	constexpr int k_screenWidth = 1024;
	constexpr int k_screenHeight = 768;

	constexpr inline float sqr_magnitude(sf::Vector2f& v)
	{
		return(v.x * v.x + v.y * v.y);
	}
	
	template<typename T, typename Ty>
	inline bool is_value_in_map(const std::unordered_map<T, Ty>& map, const T& key)
	{
		return map.find(key) != map.end();
	}
	
}
