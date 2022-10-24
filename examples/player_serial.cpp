#include <tuple>
#include <typeinfo>
#include "common.hpp"
#include "Vector2f.hpp"


struct Player
{
	json::string_t name;
	Vector2f       position;
	Vector2f       velocity;
	struct Monitor { Monitor() { ++count; } } monitor {};

	static size_t count;
};

size_t Player::count = 0;

static std::ostream & operator<<(std::ostream & ost, Player const & player);
static std::istream & operator>>(std::istream & ist, Player & player);

namespace json {


template <>
struct Descriptors<Player>
{
	static constexpr auto value = std::make_tuple(
			  make_descriptor("name",     &Player::name)
			, make_descriptor("position", &Player::position)
			, make_descriptor("velocity", &Player::velocity)
			, make_descriptor("count",     Player::count)
		);
	static constexpr bool single_line = false;
};

} // namespace json


int main()
{
	// if(0)
	{
		// open players.json and try adding player data
		std::vector<Player> players = {{ "Bob", {3,4}, {0.0f,-2.f} }, { "Martha", {5,0}, {1,1} }};
		constexpr int precision_ = 2;
		std::ofstream ofs(RESOURCE_PATH"players.json");
		if(ofs.is_open())
		{
			ofs << *json::serialize(players, precision_);
			print("Saved!");
		}
		else
			std::cerr << "failed to open \"" << RESOURCE_PATH"player.json" << "\" for writting!" << std::endl;
	}
	// if(0)
	{
		Player player { "Bob", {3,4}, {0.0f,-2.f} };
		constexpr int precision_ = 2;
		auto player_serial = json::serialize(player, precision_);
		// load player
		{ 
			std::ifstream ifs(RESOURCE_PATH"player.json");
			if(ifs.is_open())
			{
				ifs >> *player_serial;
				json::deserialize(player, player_serial);
			}
		}
		// ------
		{
			print(player);
			print(*player_serial);
			print_line_break();

			char edit_ = 'n';
			std::cout << "Edit? [y/n]: ";
			std::cin >> edit_;
			if(edit_ == 'Y' || edit_ == 'y')
			{
				std::cout << "enter name, position and velocity: \"name\" px py vx vy: ";
				std::cin >> player;
				std::cout << std::endl;
			}
			print_line_break();
			
			print(player);
			print_line_break();
		}
		// save player
		{
			std::ofstream ofs(RESOURCE_PATH"player.json");
			if(ofs.is_open())
			{
				ofs << *json::serialize(player, precision_);
				print("Saved!");
			}
			else
				std::cerr << "failed to open \"" << RESOURCE_PATH"player.json" << "\" for writting!" << std::endl;
		}
	}
}


static std::ostream &
operator<<(std::ostream & ost, Player const & player)
{
	ost << "Player \"" << player.name << "\" is at ";
	ost << player.position << ", moving with a velocity of " << player.velocity;
	return ost;
}

static std::istream &
operator>>(std::istream & ist, Player & player)
{
	ist.ignore();
	try {
		json::String jstr;
		ist >> jstr;
		ist >> player.position.x >> player.position.y;
		ist >> player.velocity.x >> player.velocity.y;
		player.name = jstr.string();
	}
	catch(std::runtime_error const & ex)
	{
		std::cerr << "Input error: " << ex.what() << std::endl;
	}
	return ist;
}