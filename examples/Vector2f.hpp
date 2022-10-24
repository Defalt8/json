#pragma once
#ifndef VECTOR2F_HPP
#define VECTOR2F_HPP

#include <iosfwd>
#include "common.hpp"

struct Vector2f
{
	// float x = 0.0f, y = 0.0f;
	union 
	{
		float values[2] {};
		union { struct { float x, y; }; };
	};

	Vector2f() = default;
	Vector2f(Vector2f &&) = default;
	Vector2f(Vector2f const &) = default;
	Vector2f & operator=(Vector2f &&) = default;
	Vector2f & operator=(Vector2f const &) = default;
	
	Vector2f(float v_)
		: values { v_, v_ }
	{}

	Vector2f(float x_, float y_)
		: values { x_, y_ }
	{}


};

static std::ostream &
operator<<(std::ostream & ost, Vector2f const & vec)
{
	ost <<  "<" << vec.x << ", " << vec.y << ">";
	return ost;
}

namespace json {

template <>
struct Descriptors<Vector2f>
{
	static constexpr auto value = std::make_tuple(
			  make_descriptor("x", &Vector2f::x)
			, make_descriptor("y", &Vector2f::y)
		);
	static constexpr bool single_line = true;
};

}  // namespace json

#endif // VECTOR2F_HPP