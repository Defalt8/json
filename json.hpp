#pragma once
#ifndef JSON_HPP
#define JSON_HPP

#include <utility>
#include <type_traits>
#include <memory>
#include <string>
#include <list>
#include <unordered_map>
#include <iosfwd>

/**
 * @brief Single-header library for simple JSON parsing, printing and manipulation.
 * 
 * Demo:
 * 
 	// json::Object expects only array of json::Entry's for construction 
 	// the key type is json::string_t 
 	// the value type is automatically deduced and stored as a unique_ptr of json::Base
	// NOTE: Object and Array types must be explicitly stated as there is no way to 
	//       differentiate between the two during construction.
 	json::Object jobject({
 		  { "0001", 
 		  	json::Object({
 			    {"title", "Alice in thunderland"}
 			  , {"author", "Sir Writes Alot"}
 			  , {"price", 59.99f}
 			  , {"in_stock", 5}
 			  , {"available", true}
 			  , {"vendor", json::null}
 			  , {"reviews", json::Array("good", "meh", "I hate it!")}
 		  	})
 		  }
 	});
	

	// you can modify these two before printing
	json::newline = ""; // default is "\n"
	json::padding = ""; // default is "   "
	

	// output to stdout
	std::cout << jobject << std::endl;
	

	// output to file
	std::ofstream ofs("movies.json", std::ios_base::out);
	ofs << jobject;
	

	char const * sample = R"({
		"title": "Finding the end.",
		"author": "brewspear",
		"free_shipping": true,
		"is_not_cool": false,
		"publisher": null,
		"in_stock": 70,
		"price": 59.99,
		"details": 
		{
			"bla": "bla bla",
			"kus": 578
		},
		"array":
		[
			1,2,3,4,5
		]
	})";

	json::Object jobject;
	std::stringstream sst(sample);

	// parse the object from the string stream
	try 
	{
		sst >> jobject; 
		std::cout << jobject << std::endl;
	} 
	catch(std::exception const & ex)
	{
		std::cerr << ex.what() << " -> ";
		std::cerr.write(&sample[int(sst.tellg())-1], 24);
		std::cerr << std::endl;
	}

 */

namespace json {

class Base;
class Null;
class Boolean;
class Integer;
class Number;
class String;
class Array;
class Object;
struct Entry;

using null_t     = struct {};
using boolean_t  = bool;
using integer_t  = int;
using number_t   = float;
using string_t   = std::string;
using base_ptr_t = std::unique_ptr<json::Base>;
using array_element_t   = base_ptr_t;
using array_elements_t = std::list<array_element_t>;
using array_elements_iterator_t = array_elements_t::iterator;
using object_entry_t = std::pair<string_t,base_ptr_t>;
using object_entries_t = std::unordered_map<string_t,base_ptr_t>;
using object_entries_iterator_t = object_entries_t::iterator;
static constexpr null_t null {};
static char const * padding = "   ";
static char const * newline = "\n";

static inline base_ptr_t make_base_ptr(base_ptr_t value_);
static inline base_ptr_t make_base_ptr(null_t value_);
static inline base_ptr_t make_base_ptr(boolean_t value_);
static inline base_ptr_t make_base_ptr(integer_t value_);
static inline base_ptr_t make_base_ptr(number_t value_);
static inline base_ptr_t make_base_ptr(string_t value_);
static inline base_ptr_t make_base_ptr(char const * value_);
static inline base_ptr_t make_base_ptr(Array value_);
static inline base_ptr_t make_base_ptr(Object value_);

static std::ostream & print(std::ostream & ost, json::Null const & jnull);
static std::ostream & print(std::ostream & ost, json::Boolean const & jboolean);
static std::ostream & print(std::ostream & ost, json::Integer const & jinteger);
static std::ostream & print(std::ostream & ost, json::Number const & jnumber);
static std::ostream & print(std::ostream & ost, json::String const & jstring);
static std::ostream & print(std::ostream & ost, json::Array const & jarray, size_t depth_ = 0);
static std::ostream & print(std::ostream & ost, json::Object const & jobject, size_t depth_ = 0);
static std::ostream & print(std::ostream & ost, json::Base const & jbase, size_t depth_ = 0);

static std::istream & parse(std::istream & ist, json::Null & jnull, char first_char, bool first_char_read = false) noexcept(false);
static std::istream & parse(std::istream & ist, json::Boolean & jboolean, char first_char, bool first_char_read = false) noexcept(false);
static std::istream & parse(std::istream & ist, json::Integer & jinteger, char first_char, bool first_char_read = false) noexcept(false);
static std::istream & parse(std::istream & ist, json::Number & jnumber, char first_char, bool first_char_read = false) noexcept(false);
static std::istream & parse(std::istream & ist, json::String & jstring, bool skip_opening_check = false) noexcept(false);
static std::istream & parse(std::istream & ist, json::Entry & jentry, char & ch) noexcept(false);
static std::istream & parse(std::istream & ist, json::array_element_t & jelement, char & ch) noexcept(false);
static std::istream & parse(std::istream & ist, json::Array & jarray, bool skip_opening_check = false) noexcept(false);
static std::istream & parse(std::istream & ist, json::Object & jobject, bool skip_opening_check = false) noexcept(false);

enum class Type
{
	  Base
	, Null
	, Boolean
	, Integer
	, Number
	, String
	, Object
	, Array
};

class Base
{
 protected:
	Base()             = default;
	Base(Base &&)      = default;
	Base(Base const &) = delete;

 public:
	virtual ~Base() = default;
	virtual Type type() const = 0;

};

class Null final : public Base 
{
 public:
	Null()             = default;
	~Null()            = default;
	Null(Null &&)      = default;
	Null(Null const &) = delete;
	Null(null_t null_) {}
	Type type() const override { return Type::Null; }
};

class Boolean final : public Base 
{
	boolean_t m_boolean {};

 public:
	Boolean()                = default;
	~Boolean()               = default;
	Boolean(Boolean &&)      = default;
	Boolean(Boolean const &) = delete;
	Boolean(boolean_t boolean_)
		: m_boolean { boolean_ }
	{}
	Type type() const override { return Type::Boolean; }

	boolean_t const & boolean() const { return m_boolean; }
	boolean_t       & boolean()       { return m_boolean; }
};

class Integer final : public Base 
{
	integer_t m_integer {};

 public:
	Integer()                = default;
	~Integer()               = default;
	Integer(Integer &&)      = default;
	Integer(Integer const &) = delete;
	Integer(integer_t integer_)
		: m_integer { integer_ }
	{}
	Type type() const override { return Type::Integer; }

	integer_t const & integer() const { return m_integer; }
	integer_t       & integer()       { return m_integer; }
};

class Number final : public Base 
{
	number_t m_number {};

 public:
	Number()               = default;
	~Number()              = default;
	Number(Number &&)      = default;
	Number(Number const &) = delete;
	Number(number_t number_)
		: m_number { number_ }
	{}
	Type type() const override { return Type::Number; }

	number_t const & number() const { return m_number; }
	number_t       & number()       { return m_number; }
};

class String final : public Base 
{
	string_t m_string {};

 public:
	String()               = default;
	~String()              = default;
	String(String &&)      = default;
	String(String const &) = delete;
	String(string_t string_)
		: m_string { std::move(string_) }
	{}
	String(char const * string_)
		: m_string { string_ }
	{}
	Type type() const override { return Type::String; }

	string_t const & string() const { return m_string; }
	string_t       & string()       { return m_string; }

};

class Array final : public Base
{
	array_elements_t m_elements {};

 public:
	Array()  = default;
	~Array() = default;
	Array(Array &&)      = default;
	Array(Array const &) = delete;

	template <size_t size_>
	Array(array_element_t (&& elements_)[size_])
	{
		for(auto & element_ : elements_)
			m_elements.push_back(std::move(element_));
	}

	template <typename T, size_t size_>
	Array(T (&& elements_)[size_])
	{
		for(auto & element_ : elements_)
			m_elements.push_back(make_base_ptr(std::move(element_)));
	}

	template <typename V0, typename... Args>
	Array(V0 && v0, Args &&... args)
		: Array({ make_base_ptr(std::forward<V0>(v0)), make_base_ptr(std::forward<Args>(args))... })
	{}

	Type type() const override { return Type::Array; }

	array_elements_t       & elements()       { return m_elements; }
	array_elements_t const & elements() const { return m_elements; }

};

class Object final : public Base
{
	object_entries_t m_entries {};

 public:
	Object()  = default;
	~Object() = default;
	Object(Object &&)      = default;
	Object(Object const &) = delete;

	template <size_t size_>
	Object(Entry (&& entries_)[size_])
	{
		for(auto & entry_ : entries_)
			m_entries.insert(object_entry_t(entry_.key, std::move(entry_.value)));
	}

	Type type() const override { return Type::Object; }

	object_entries_t       & entries()       { return m_entries; }
	object_entries_t const & entries() const { return m_entries; }

};

struct Entry
{
	string_t   key   {};
	base_ptr_t value {};

	Entry()              = default;
	Entry(Entry &&)      = default;
	Entry(Entry const &) = delete;
	Entry(string_t key_, base_ptr_t value_)
		: key   { std::move(key_) }
		, value { std::move(value_) }
	{}
	template <typename T>
	Entry(string_t key_, T value_)
		: key   { std::move(key_) }
		, value { make_base_ptr(std::move(value_)) }
	{}

};

static inline base_ptr_t make_base_ptr(base_ptr_t value_)    { return std::move(value_); }
static inline base_ptr_t make_base_ptr(Null value_)          { return base_ptr_t(new Null(std::move(value_))); }
static inline base_ptr_t make_base_ptr(null_t value_)        { return base_ptr_t(new Null()); }
static inline base_ptr_t make_base_ptr(Boolean value_)       { return base_ptr_t(new Boolean(std::move(value_))); }
static inline base_ptr_t make_base_ptr(boolean_t value_)     { return base_ptr_t(new Boolean(value_)); }
static inline base_ptr_t make_base_ptr(Integer value_)       { return base_ptr_t(new Integer(std::move(value_))); }
static inline base_ptr_t make_base_ptr(integer_t value_)     { return base_ptr_t(new Integer(value_)); }
template <typename T, std::enable_if_t<std::is_integral<T>::value,bool> = true>
static inline base_ptr_t make_base_ptr(T value_)             { return base_ptr_t(new Integer(integer_t(value_))); }
static inline base_ptr_t make_base_ptr(Number value_)        { return base_ptr_t(new Number(std::move(value_))); }
static inline base_ptr_t make_base_ptr(number_t value_)      { return base_ptr_t(new Number(value_)); }
template <typename T, std::enable_if_t<std::is_floating_point<T>::value,bool> = true>
static inline base_ptr_t make_base_ptr(T value_)             { return base_ptr_t(new Number(number_t(value_))); }
static inline base_ptr_t make_base_ptr(String value_)        { return base_ptr_t(new String(std::move(value_))); }
static inline base_ptr_t make_base_ptr(string_t value_)      { return base_ptr_t(new String(std::move(value_))); }
static inline base_ptr_t make_base_ptr(char const * value_)  { return base_ptr_t(new String(std::move(value_))); }
static inline base_ptr_t make_base_ptr(char * value_)        { return base_ptr_t(new String(std::move(value_))); }
static inline base_ptr_t make_base_ptr(Array value_)         { return base_ptr_t(new Array(std::move(value_))); }
static inline base_ptr_t make_base_ptr(Object value_)        { return base_ptr_t(new Object(std::move(value_))); }

static void 
print_padding(std::ostream & ost, size_t depth_)
{
	for(size_t k = 0; k < depth_; ++k)
		ost << padding;
}

static std::ostream &
print(std::ostream & ost, json::Null const & jnull)
{
	return ost << "null";
}

static std::ostream &
print(std::ostream & ost, json::Boolean const & jboolean)
{
	return ost << (jboolean.boolean() ? "true" : "false");
}

static std::ostream &
print(std::ostream & ost, json::Integer const & jinteger)
{
	return ost << jinteger.integer();
}

static std::ostream &
print(std::ostream & ost, json::Number const & jnumber)
{
	return ost << jnumber.number();
}

static std::ostream &
print(std::ostream & ost, json::String const & jstring)
{
	return ost << '"' << jstring.string() << '"';
}

static std::ostream &
print(std::ostream & ost, json::Array const & jarray, size_t depth_)
{
	print_padding(ost, depth_);
	ost << '[';
	auto const & array_elements = jarray.elements();
	size_t i = 0;
	size_t size_ = array_elements.size();
	for(auto const & element : array_elements)
	{
		ost << newline;
		switch(element->type())
		{
			case Type::Array:
			case Type::Object:
				break;
			default:
				print_padding(ost, depth_ + 1);
				break;
		}
		json::print(ost, *element, depth_);
		if(i < size_ - 1)
			ost << ",";
		++i;
	}
	if(i > 0)
		ost << newline;
	print_padding(ost, depth_);
	ost << ']';
	return ost;
}

static std::ostream &
print(std::ostream & ost, json::Object const & jobject, size_t depth_)
{
	print_padding(ost, depth_);
	ost << '{';
	auto const & jobject_entries = jobject.entries();
	size_t i = 0;
	size_t size_ = jobject_entries.size();
	for(auto const & entry : jobject_entries)
	{
		ost << newline;
		print_padding(ost, depth_ + 1);
		ost << '"' << entry.first << "\": ";
		auto const & value = entry.second;
		switch(value->type())
		{
			case Type::Array:
			case Type::Object:
				ost << newline;
				break;
			default:
				break;
		}
		json::print(ost, *value, depth_);
		if(i < size_ - 1)
			ost << ",";
		++i;
	}
	if(i > 0)
		ost << newline;
	print_padding(ost, depth_);
	ost << '}';
	return ost;
}

static std::ostream &
print(std::ostream & ost, json::Base const & jbase, size_t depth_)
{
	switch(jbase.type())
	{
		case json::Type::Null:
			json::print(ost, static_cast<json::Null const &>(jbase));
			break;
		case json::Type::Boolean:
			json::print(ost, static_cast<json::Boolean const &>(jbase));
			break;
		case json::Type::Integer:
			json::print(ost, static_cast<json::Integer const &>(jbase));
			break;
		case json::Type::Number:
			json::print(ost, static_cast<json::Number const &>(jbase));
			break;
		case json::Type::String:
			json::print(ost, static_cast<json::String const &>(jbase));
			break;
		case json::Type::Object:
			json::print(ost, static_cast<json::Object const &>(jbase), depth_ + 1);
			break;
		case json::Type::Array:
			json::print(ost, static_cast<json::Array const &>(jbase), depth_ + 1);
			break;
		default: 
			break;
	}
	return ost;
}

static inline std::ostream &
operator<<(std::ostream & ost, json::Null const & jnull) { return json::print(ost, jnull); }
static inline std::ostream &
operator<<(std::ostream & ost, json::Boolean const & jbool) { return json::print(ost, jbool); }
static inline std::ostream &
operator<<(std::ostream & ost, json::Integer const & jinteger) { return json::print(ost, jinteger); }
static inline std::ostream &
operator<<(std::ostream & ost, json::Number const & jnumber) { return json::print(ost, jnumber); }
static inline std::ostream &
operator<<(std::ostream & ost, json::String const & jstring) { return json::print(ost, jstring); }
static inline std::ostream &
operator<<(std::ostream & ost, json::Array const & jarray) { return json::print(ost, jarray); }
static inline std::ostream &
operator<<(std::ostream & ost, json::Object const & jobject) { return json::print(ost, jobject); }

static bool
_skip_spaces(std::istream & ist, char & ch, bool check_current_char = false)
{
	if(!check_current_char)
	{
		if(ist.eof())
			return false;
		ist.read(&ch, 1);
	}
	while(std::isspace(ch))
	{
		if(ist.eof())
			return false;
		ist.read(&ch, 1);
	}
	return true;
}

static std::istream &
parse(std::istream & ist, json::Null & jnull, char first_char, bool first_char_read) noexcept(false)
{
	char buffer[5] { first_char, '\0', '\0', '\0', '\0' };
	if(first_char_read)
		ist.read(&buffer[1], 3);
	else
		ist.read(&buffer[0], 4);
	if(0 != strcmp(buffer, "null"))
		throw std::runtime_error("json::parse: invalid null value.");
	return ist;
}

static std::istream &
parse(std::istream & ist, json::Boolean & jboolean, char first_char, bool first_char_read) noexcept(false)
{
	char buffer[6] { first_char, '\0', '\0', '\0', '\0', '\0' };
	if(!first_char_read)
	{
		ist.read(&buffer[0], 1);
		first_char = buffer[0];
	}
	if(first_char == 't')
	{
		ist.read(&buffer[1], 3);
		if(0 != strcmp(buffer, "true"))
			throw std::runtime_error("json::parse: invalid boolean value.");
		jboolean.boolean() = true;
	}
	else if(first_char == 'f')
	{
		ist.read(&buffer[1], 4);
		if(0 != strcmp(buffer, "false"))
			throw std::runtime_error("json::parse: invalid boolean value.");
		jboolean.boolean() = false;
	}
	else
		throw std::runtime_error("json::parse: invalid boolean value.");
	return ist;
}

static std::istream &
parse(std::istream & ist, json::Integer & jinteger, char first_char, bool first_char_read) noexcept(false)
{
	return ist;
}

static std::istream &
parse(std::istream & ist, json::Number & jnumber, char first_char, bool first_char_read) noexcept(false)
{
	return ist;
}

static std::istream &
parse(std::istream & ist, json::String & jstring, bool skip_opening_check) noexcept(false)
{
	char ch;
	if(!skip_opening_check)
	{
		if(!_skip_spaces(ist, ch))
			throw std::runtime_error("json::parse: end of stream.");
		if(ch != '"')
			throw std::runtime_error("json::parse: missing opening quote for string.");
	}
	// read string until end quotes are reached
	std::string buffer;
	ist.read(&ch, 1);
	while(ch != '"')
	{
		if(ist.eof())
			throw std::runtime_error("json::parse: end of stream.");
		buffer += ch;
		ist.read(&ch, 1);
	}
	jstring.string() = std::move(buffer);
	return ist;
}

static std::istream &
parse(std::istream & ist, json::Entry & jentry, char & ch) noexcept(false)
{
	// read "key": value pairs
	if(!ist.eof())
	{
		json::String key;
		parse(ist, key);
		// skip white-spaces until ':' is reached
		if(!_skip_spaces(ist, ch))
			throw std::runtime_error("json::parse: end of stream.");
		if(ch != ':')
			throw std::runtime_error("json::parse: missing key-value separator ':' for object entry.");
		// identify the value type based on the first character
		if(!_skip_spaces(ist, ch))
			throw std::runtime_error("json::parse: end of stream.");
		if(ch == '"') // string
		{
			json::String value;
			parse(ist, value, true);
			jentry.key = std::move(key.string());
			jentry.value = make_base_ptr(std::move(value));
			ist.read(&ch, 1);
		}
		else if(ch == '{') // object
		{
			json::Object value;
			parse(ist, value, true);
			jentry.key = std::move(key.string());
			jentry.value = make_base_ptr(std::move(value));
			ist.read(&ch, 1);
		}
		else if(ch == '[') // array
		{
			json::Array value;
			parse(ist, value, true);
			jentry.key = std::move(key.string());
			jentry.value = make_base_ptr(std::move(value));
			ist.read(&ch, 1);
		}
		else if(ch == 'n') // null
		{
			json::Null value;
			parse(ist, value, ch, true);
			jentry.key = std::move(key.string());
			jentry.value = make_base_ptr(std::move(value));
			ist.read(&ch, 1);
		}
		else if(ch == 't' || ch == 'f') // boolean
		{
			json::Boolean value;
			parse(ist, value, ch, true);
			jentry.key = std::move(key.string());
			jentry.value = make_base_ptr(std::move(value));
			ist.read(&ch, 1);
		}
		else if(std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.') // integer or floating point number
		{
			size_t decimal_points = ch == '.' ? 1 : 0;
			size_t positive_signs = ch == '+' ? 1 : 0;
			size_t negative_signs = ch == '-' ? 1 : 0;
			string_t buffer;
			buffer += ch;
			bool continue_ = true;
			while(continue_)
			{
				if(ist.eof())
					throw std::runtime_error("json::parse: end of stream.");
				ist.read(&ch, 1);
				switch(ch)
				{
					case '.':
						if(decimal_points > 0)
							throw std::runtime_error("json::parse: Multiple decimal points in numeric value.");
						++decimal_points; 
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						buffer += ch;
						break;
					case '+': 
						throw std::runtime_error("json::parse: Invalid positive sign in numeric value.");
						break;
					case '-':
						throw std::runtime_error("json::parse: Invalid negative sign in numeric value.");
						break;
					case ',':
					case '}':
						continue_ = false;
						break;
					default:
						continue_ = false;
						if(!std::isspace(ch))
							throw std::runtime_error("json::parse: invalid numeric token.");
						break;
				}
			}
			jentry.key = std::move(key.string());
			if(decimal_points > 1)
				throw std::runtime_error("json::parse: invalid numeric value. More than one decimal points.");
			if(positive_signs > 1)
				throw std::runtime_error("json::parse: invalid numeric value. More than one positive signs.");
			if(negative_signs > 1)
				throw std::runtime_error("json::parse: invalid numeric value. More than one negative signs.");
			if(positive_signs >= 1 && negative_signs >= 1)
				throw std::runtime_error("json::parse: invalid numeric value. mixed positive and negative signs.");
			if(decimal_points > 0)
				jentry.value = make_base_ptr(std::atof(buffer.c_str()));
			else
				jentry.value = make_base_ptr(std::atoi(buffer.c_str()));
		}
		else
			throw std::runtime_error("json::parse: invalid object entry value token.");
	}
	return ist;
}

static std::istream &
parse(std::istream & ist, json::array_element_t & jelement, char & ch) noexcept(false)
{
	// read "key": value pairs
	if(!ist.eof())
	{
		// identify the value type based on the first character
		if(!_skip_spaces(ist, ch))
			throw std::runtime_error("json::parse: end of stream.");
		if(ch == '"') // string
		{
			json::String value;
			parse(ist, value, true);
			jelement = make_base_ptr(std::move(value));
			ist.read(&ch, 1);
		}
		else if(ch == '{') // object
		{
			json::Object value;
			parse(ist, value, true);
			jelement = make_base_ptr(std::move(value));
			ist.read(&ch, 1);
		}
		else if(ch == '[') // array
		{
			json::Array value;
			parse(ist, value, true);
			jelement = make_base_ptr(std::move(value));
			ist.read(&ch, 1);
		}
		else if(ch == 'n') // null
		{
			json::Null value;
			parse(ist, value, ch, true);
			jelement = make_base_ptr(std::move(value));
			ist.read(&ch, 1);
		}
		else if(ch == 't' || ch == 'f') // boolean
		{
			json::Boolean value;
			parse(ist, value, ch, true);
			jelement = make_base_ptr(std::move(value));
			ist.read(&ch, 1);
		}
		else if(std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.') // integer or floating point number
		{
			size_t decimal_points = ch == '.' ? 1 : 0;
			size_t positive_signs = ch == '+' ? 1 : 0;
			size_t negative_signs = ch == '-' ? 1 : 0;
			string_t buffer;
			buffer += ch;
			bool continue_ = true;
			while(continue_)
			{
				if(ist.eof())
					throw std::runtime_error("json::parse: end of stream.");
				ist.read(&ch, 1);
				switch(ch)
				{
					case '.':
						if(decimal_points > 0)
							throw std::runtime_error("json::parse: Multiple decimal points in numeric value.");
						++decimal_points; 
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						buffer += ch;
						break;
					case '+': 
						throw std::runtime_error("json::parse: Invalid positive sign in numeric value.");
						break;
					case '-':
						throw std::runtime_error("json::parse: Invalid negative sign in numeric value.");
						break;
					case ',':
					case '}':
						continue_ = false;
						break;
					default:
						continue_ = false;
						if(!std::isspace(ch))
							throw std::runtime_error("json::parse: invalid numeric token.");
						break;
				}
			}
			if(decimal_points > 1)
				throw std::runtime_error("json::parse: invalid numeric value. More than one decimal points.");
			if(positive_signs > 1)
				throw std::runtime_error("json::parse: invalid numeric value. More than one positive signs.");
			if(negative_signs > 1)
				throw std::runtime_error("json::parse: invalid numeric value. More than one negative signs.");
			if(positive_signs >= 1 && negative_signs >= 1)
				throw std::runtime_error("json::parse: invalid numeric value. mixed positive and negative signs.");
			if(decimal_points > 0)
				jelement = make_base_ptr(std::atof(buffer.c_str()));
			else
				jelement = make_base_ptr(std::atoi(buffer.c_str()));
		}
		else
			throw std::runtime_error("json::parse: invalid array element value token.");
	}
	return ist;
}

static std::istream &
parse(std::istream & ist, json::Array & jarray, bool skip_opening_check) noexcept(false)
{
	char ch;
	// skip white-spaces until '[' is reached
	if(!skip_opening_check)
	{
		if(!_skip_spaces(ist, ch))
			throw std::runtime_error("json::parse: end of stream.");
		if(ch != '[')
			throw std::runtime_error("json::parse: missing opening square bracket for array.");
	}
	// read "key":value pairs
	while(!ist.eof())
	{
		array_element_t jelement;
		parse(ist, jelement, ch);
		jarray.elements().push_back(std::move(jelement));
		if(!_skip_spaces(ist, ch, true))
			throw std::runtime_error("json::parse: end of stream.");
		if(ch == ',')
			continue;
		else if(ch == ']')
			break;
		else 
			throw std::runtime_error("json::parse: invalid token. ',' or ']' expected.");
	}
	return ist;
}

static std::istream &
parse(std::istream & ist, json::Object & jobject, bool skip_opening_check) noexcept(false)
{
	char ch;
	// skip white-spaces until '{' is reached
	if(!skip_opening_check)
	{
		if(!_skip_spaces(ist, ch))
			throw std::runtime_error("json::parse: end of stream.");
		if(ch != '{')
			throw std::runtime_error("json::parse: missing opening brace for object.");
	}
	// read "key":value pairs
	while(!ist.eof())
	{
		json::Entry jentry;
		parse(ist, jentry, ch);
		jobject.entries().insert(object_entry_t(jentry.key, std::move(jentry.value)));
		if(!_skip_spaces(ist, ch, true))
			throw std::runtime_error("json::parse: end of stream.");
		if(ch == ',')
			continue;
		else if(ch == '}')
			break;
		else 
			throw std::runtime_error("json::parse: invalid token. ',' or '}' expected.");
	}
	return ist;
}

static inline std::istream &
operator>>(std::istream & ist, json::Object & jobject) { return json::parse(ist, jobject); }

} // namespace json

#endif // JSON_HPP