#pragma once
#include <unordered_map>

namespace globals
{
	namespace cars
	{
		constexpr float k_carTrackSpeed = 300.f;
		constexpr float k_carGrassSpeed = 50.f;
		constexpr float k_carStartingRotation = 1.5708f;
		constexpr float k_carSpriteWidth = 20.f;
		constexpr float k_carSpriteHeight = 34.f;

		constexpr float k_carOriginX = 10.f;
		constexpr float k_carOriginY = 17.f;

	} // namespace cars

	namespace game
	{
		constexpr int k_playerAmount = 4;
		
		constexpr int k_numCheckPoints = 11;
		constexpr float k_checkPointWidth = 50.f;
		constexpr float k_checkPointHeight = 150.f;

		constexpr float k_aiDistanceThreshold = 100.f;

		inline const sf::Vector2f k_checkPointColliderSize{ globals::game::k_checkPointWidth, globals::game::k_checkPointHeight };

		constexpr int k_screenWidth = 1024;
		constexpr int k_screenHeight = 768;
		
		constexpr int k_totalLaps = 3;
	}


	inline const std::string k_reservedServerUsername = "SERVER";


	/**
	 * \brief Calculates the square magnitude of a given vector
	 * \param v The vector to calculate
	 * \return The Square Mag of the vector
	 */
	constexpr inline float sqr_magnitude(sf::Vector2f& v)
	{
		return(v.x * v.x + v.y * v.y);
	}

	/**
	 * \brief Calculates the angle in radians between two vectors
	 * \param a The first vector
	 * \param b The target vector
	 * \return The angle in radians between the vectors
	 */
	inline float get_angle_rad(const sf::Vector2f& a, const sf::Vector2f& b)
	{
		return atan2f(b.y - a.y, b.x - a.x);
	}

	/**
	 * \brief Finds whether a given key is in a std::unordered_map<>
	 * \tparam T The key type
	 * \tparam Ty The value type
	 * \param map The std::unordered_map<> to search
	 * \param key The key to look for
	 * \return True if the value is in the map
	 */
	template<typename T, typename Ty>
	inline bool is_key_in_map(const std::unordered_map<T, Ty>& map, const T& key)
	{
		return map.find(key) != map.end();
	}

} // namespace globals
