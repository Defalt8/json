#include <iostream>
#include <fstream>
#include <sstream>
#include "json.hpp"
#include "Serializer.hpp"

#ifndef RESOURCE_PATH
#define RESOURCE_PATH "./"
#endif

struct Vector2f
{
	float values[2] {};

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

	float       & x()       { return values[0]; }
	float       & y()       { return values[1]; }
	float const & x() const { return values[0]; }
	float const & y() const { return values[1]; }

};

struct GameData
{
	json::string_t player_name;
	Vector2f       player_position;
};

static std::ostream &
operator<<(std::ostream & ost, GameData const & game_data)
{
	ost << " Player \"" << game_data.player_name << "\" is at ";
	ost <<  "[ " << game_data.player_position.x() << ", " << game_data.player_position.y() << " ]" << std::endl;
	return ost;
}

static std::istream &
operator>>(std::istream & ist, GameData & game_data)
{
	ist.ignore();
	try {
		json::String jstr;
		ist >> jstr >> game_data.player_position.x() >> game_data.player_position.y();
		game_data.player_name = jstr.string();
	}
	catch(std::runtime_error const & ex)
	{
		std::cerr << "Input error: " << ex.what() << std::endl;
	}
	return ist;
}

namespace json {

template <>
class Serializer<Vector2f> final : public SerializerBase
{
 public:
	using data_t   = Vector2f;
	using serial_t = json::Array;

 private:
	data_t * m_data_ptr  = nullptr;
	int      m_precision = 6;
	
 public:
	Serializer() = default;
	Serializer(Serializer &&) = default;
	Serializer(data_t & data, int precision_ = 6)
		: m_data_ptr  { &data }
		, m_precision { precision_ }
	{}
	
	base_ptr_t
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr, m_precision);
	}
	
	bool
	deserialize(base_ptr_t const & base_ptr) override
	{
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr);
	}

	static base_ptr_t
	serialize(data_t & data, int precision_ = 6)
	{
		return make_base_ptr(json::Array({ 
			  json::Number(data.x(), precision_) 
			, json::Number(data.y(), precision_) 
		}));
	}

	static bool
	deserialize(data_t & data, base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::Array)
			return false;
		return Serializer<float[2]>::deserialize(data.values, base_ptr);
	}

	int       & precision()       { return m_precision; }
	int const & precision() const { return m_precision; }

};

template <>
class Serializer<GameData> final : public SerializerBase
{
 public:
	using data_t   = GameData;
	using serial_t = json::Object;

 private:
	data_t * m_data_ptr = nullptr;
	int      m_precision = 6;
	
 public:
	Serializer() = default;
	Serializer(Serializer &&) = default;
	Serializer(data_t & data, int precision_ = 6)
		: m_data_ptr  { &data }
		, m_precision { precision_ }
	{}
	
	base_ptr_t
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr, m_precision);
	}
	
	bool
	deserialize(base_ptr_t const & base_ptr) override
	{
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr);
	}

	static base_ptr_t
	serialize(data_t & data, int precision_ = 6)
	{
		return make_base_ptr(json::Object({ 
			  {"player_name", Serializer<decltype(data.player_name)>::serialize(data.player_name)}
			, {"player_position", Serializer<decltype(data.player_position)>::serialize(data.player_position, precision_)} 
		}));
	}

	static bool
	deserialize(data_t & data, base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::Object)
			return false;
		auto const & object = get<json::Object>(base_ptr);
		return Serializer<decltype(data.player_name)>::deserialize(data.player_name, object.at("player_name")) 
			&& Serializer<decltype(data.player_position)>::deserialize(data.player_position, object.at("player_position"));
	}

	int       & precision()       { return m_precision; }
	int const & precision() const { return m_precision; }

};

} // namespace json

int main()
{
	GameData game_data { "player", {3,4} };
	auto serializer       = json::make_serializer(game_data, 3);
	auto game_data_serial = serializer.serialize();
	// load game_data
	{ 
		std::ifstream ifs(RESOURCE_PATH"game_data.json");
		if(ifs.is_open())
		{
			ifs >> json::get<json::Object>(game_data_serial);
			serializer.deserialize(game_data_serial);
		}
	}
	// get info from console input
	{
		std::cout << game_data << std::endl;

		char edit_ = 'n';
		std::cout << "Edit? [y/n]: ";
		std::cin >> edit_;
		if(edit_ == 'Y' || edit_ == 'y')
		{
			std::cout << "enter name[\"name\"] and position[x y]: ";
			std::cin >> game_data;
			std::cout << std::endl;
		}
		
		std::cout << game_data << std::endl;
	}
	// save game_data
	{
		std::ofstream ofs(RESOURCE_PATH"game_data.json");
		if(ofs.is_open())
			ofs << json::get<json::Object>(serializer.serialize());
		else
			std::cerr << "failed to open \"" << RESOURCE_PATH"game_data.json" << "\" for writting!" << std::endl;
	}
}
