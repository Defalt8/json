#include "common.hpp"
#include "Vector2f.hpp"

int main()
{
	using namespace json;
	// json::newline = nullptr;
	// json::indentation = nullptr;
	// if(0)
	{
		std::vector<Vector2f> posv { {3.5f,-4.4f}, {0.4f,-.5f}, {1.2f,4.f} };
		
		auto serial = serialize(posv, 2);
		print(*serial);
		print_array(posv);

		get<Number>(get<Object>(get<Array>(serial)[0])["x"]).value() = -2.5;
		get<Array>(serial).elements().pop_back();
		
		deserialize(posv, serial);
		print(*serial);
		print_array(posv);
		print_line_break();
	}
}
