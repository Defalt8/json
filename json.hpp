#pragma once
#ifndef JSON_HPP
#define JSON_HPP

#include <cstdint>
#include <utility>
#include <type_traits>
#include <limits>
#include <memory>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <iosfwd>
#include <iomanip>

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
	
	/// use json::get<'json_type'>(base_ptr) to quickly get a reference
	json::base_ptr_t serial_ptr = json::Serializer<int>::serialize(12345);
	int val = int(json::get<json::Integer>(serial_ptr).value());

    /// To set a value at a certain entry you can use set and set_safe
	// NOTE: this overrides any parent types that are not Objects 
	//       thus replacing them with a new Object.

	jobject.set("0001.in_stock", 0);
	jobject.set("0001.vendor", "NYBS");
	jobject.set("0001.reviews"); // set "0001.reviews" to a Null

	jobject.set_safe("0001.vendor.address", "Nothing ave, Utopia"); 
	// would not set because "0001.vendor" is not an Object.
	jobject.set("0001.vendor.address", "Nothing ave, Utopia"); 
	// would override "0001.vendor" with a new Object and insert "address" in it.

	/// To get a pointer to a certain entry you can use get, get<> or get_value
	
	jobject.get("0001"); 
	  // would return a pointer to the base_ptr_t stored with the key "0001"
	  // in the root object
	  // It would return nullptr if the entry is not found.
	
	jobject.get<json::String>("0001.title"); 
	  // would return a pointer to the json::String stored with the key 
	  // "title" in the object stored with the key "0001" in the root object.
	  // It would return nullptr if the entry is not found or it is of a different type.
	
	jobject.get_value<json::Number>("0001.price"); 
	  // would return a pointer to the value of the json::Number stored at "0001"->"price".
	  // It would return nullptr if the entry is not found or it is of a different type.

	/// to remove you can use remove or erase.
	jobject.remove("0001.vendor");
	jobject.erase("0001.reviews");

	/// you can modify these two before printing
	// setting all these to nullptr will print tightly packed in a single line.
	json::value_spacing = nullptr; // default is " "
	json::newline = nullptr; // default is "\n"
	json::padding = nullptr; // default is "   "
	

	/// output to stdout
	std::cout << jobject << std::endl;
	

	/// output to file
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

	/// parse the object from the string stream
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

union uInt
{ 
	int64_t i64 = 0; 
	int32_t i32;     
	int16_t i16;     
	int8_t  i8;      
	uint64_t u64; 
	uint32_t u32; 
	uint16_t u16; 
	uint8_t  u8; 

	constexpr uInt()             = default;
	constexpr uInt(uInt &&)      = default;
	constexpr uInt(uInt const &) = default;

	uInt & operator=(uInt &&)      = default;
	uInt & operator=(uInt const &) = default;

	constexpr uInt(int64_t i64_)  : i64 { i64_ } {}
	constexpr uInt(int32_t i32_)  : i64 { i32_ } {}
	constexpr uInt(int16_t i16_)  : i64 { i16_ } {}
	constexpr uInt(int8_t  i8_)   : i64 { i8_  } {}
	constexpr uInt(uint64_t u64_) : u64 { u64_ } {}
	constexpr uInt(uint32_t u32_) : u64 { u32_ } {}
	constexpr uInt(uint16_t u16_) : u64 { u16_ } {}
	constexpr uInt(uint8_t  u8_)  : u64 { u8_  } {}

	constexpr operator int64_t &  () noexcept { return i64; }
	constexpr operator int32_t &  () noexcept { return i32; }
	constexpr operator int16_t &  () noexcept { return i16; }
	constexpr operator int8_t  &  () noexcept { return i8;  }
	constexpr operator uint64_t & () noexcept { return u64; }
	constexpr operator uint32_t & () noexcept { return u32; }
	constexpr operator uint16_t & () noexcept { return u16; }
	constexpr operator uint8_t  & () noexcept { return u8;  }

	constexpr operator int64_t () const noexcept { return i64; }
	constexpr operator int32_t () const noexcept { return i32; }
	constexpr operator int16_t () const noexcept { return i16; }
	constexpr operator int8_t  () const noexcept { return i8;  }
	constexpr operator uint64_t() const noexcept { return u64; }
	constexpr operator uint32_t() const noexcept { return u32; }
	constexpr operator uint16_t() const noexcept { return u16; }
	constexpr operator uint8_t () const noexcept { return u8;  }

};

enum class IntType
{
	  Int64 = 0
	, Int32
	, Int16
	, Int8
	, UInt64
	, UInt32
	, UInt16
	, UInt8
};

struct Int
{
	uInt    u    {};
	IntType type {};
	
	constexpr Int()            = default;
	constexpr Int(Int &&)      = default;
	constexpr Int(Int const &) = default;

	Int & operator=(Int &&)      = default;
	Int & operator=(Int const &) = default;

	constexpr Int(int64_t i64_)  : u { i64_ }, type { IntType::Int64 }  {}
	constexpr Int(int32_t i32_)  : u { i32_ }, type { IntType::Int32 }  {}
	constexpr Int(int16_t i16_)  : u { i16_ }, type { IntType::Int16 }  {}
	constexpr Int(int8_t  i8_)   : u { i8_  }, type { IntType::Int8  }  {}
	constexpr Int(uint64_t u64_) : u { u64_ }, type { IntType::UInt64 } {}
	constexpr Int(uint32_t u32_) : u { u32_ }, type { IntType::UInt32 } {}
	constexpr Int(uint16_t u16_) : u { u16_ }, type { IntType::UInt16 } {}
	constexpr Int(uint8_t  u8_)  : u { u8_  }, type { IntType::UInt8  } {}

	constexpr int64_t  & i64() noexcept { return u.i64; }
	constexpr int32_t  & i32() noexcept { return u.i32; }
	constexpr int16_t  & i16() noexcept { return u.i16; }
	constexpr int8_t   & i8 () noexcept { return u.i8;  }
	constexpr uint64_t & u64() noexcept { return u.u64; }
	constexpr uint32_t & u32() noexcept { return u.u32; }
	constexpr uint16_t & u16() noexcept { return u.u16; }
	constexpr uint8_t  & u8 () noexcept { return u.u8;  }

	constexpr int64_t  i64() const noexcept { return u.i64; }
	constexpr int32_t  i32() const noexcept { return u.i32; }
	constexpr int16_t  i16() const noexcept { return u.i16; }
	constexpr int8_t   i8 () const noexcept { return u.i8;  }
	constexpr uint64_t u64() const noexcept { return u.u64; }
	constexpr uint32_t u32() const noexcept { return u.u32; }
	constexpr uint16_t u16() const noexcept { return u.u16; }
	constexpr uint8_t  u8 () const noexcept { return u.u8;  }

	template <typename T = int>
	constexpr operator T & () noexcept { return static_cast<T &>(u); }
	
	template <typename T = int>
	constexpr operator T () const noexcept { return static_cast<T const &>(u); }
	
	template <typename T>
	constexpr bool operator==(T const & rhs) const noexcept { return static_cast<T const &>(u) == rhs; }
	template <typename T>
	constexpr bool operator!=(T const & rhs) const noexcept { return static_cast<T const &>(u) != rhs; }
	template <typename T>
	constexpr bool operator<(T const & rhs) const noexcept { return static_cast<T const &>(u) < rhs; }
	template <typename T>
	constexpr bool operator>(T const & rhs) const noexcept { return static_cast<T const &>(u) > rhs; }
	template <typename T>
	constexpr bool operator<=(T const & rhs) const noexcept { return static_cast<T const &>(u) <= rhs; }
	template <typename T>
	constexpr bool operator>=(T const & rhs) const noexcept { return static_cast<T const &>(u) >= rhs; }

};

static std::ostream &
operator<<(std::ostream & ost, Int const & i) 
{
	switch(i.type)
	{
		default:
		case IntType::Int64:   ost << i.i64(); break;
		case IntType::Int32:   ost << i.i32(); break;
		case IntType::Int16:   ost << i.i16(); break;
		case IntType::Int8:    ost << uint16_t(i.i8());  break;
		case IntType::UInt64:  ost << i.u64(); break;
		case IntType::UInt32:  ost << i.u32(); break;
		case IntType::UInt16:  ost << i.u16(); break;
		case IntType::UInt8:   ost << uint16_t(i.u8());  break;
	}
	return ost;
}

using null_t     = struct {};
using boolean_t  = bool;
using integer_t  = Int; 
using number_t   = double;
using string_t   = std::string;
using base_ptr_t = std::unique_ptr<json::Base>;
using array_element_t  = base_ptr_t;
using array_elements_t = std::list<array_element_t>;
using array_elements_iterator_t       = array_elements_t::iterator;
using array_elements_const_iterator_t = array_elements_t::const_iterator;
using object_entry_t = std::pair<string_t,base_ptr_t>;
using object_entries_t = std::unordered_map<string_t,base_ptr_t>;
using object_entries_iterator_t = object_entries_t::iterator;
static constexpr null_t      null {};
static constexpr number_t    nan  = std::numeric_limits<number_t>::quiet_NaN();
static constexpr float       nanf = std::numeric_limits<float>::quiet_NaN();
static constexpr long double nanl = std::numeric_limits<long double>::quiet_NaN();
static constexpr number_t    inf  = std::numeric_limits<number_t>::infinity();
static constexpr float       inff = std::numeric_limits<float>::infinity();
static constexpr long double infl = std::numeric_limits<long double>::infinity();
static char const * value_spacing = " ";
static char const * padding = "   ";
static char const * newline = "\n";

static inline base_ptr_t make_base_ptr();
static inline base_ptr_t make_base_ptr(base_ptr_t value_);
static inline base_ptr_t make_base_ptr(Null value_);
static inline base_ptr_t make_base_ptr(null_t value_);
static inline base_ptr_t make_base_ptr(Boolean value_);
static inline base_ptr_t make_base_ptr(boolean_t value_);
static inline base_ptr_t make_base_ptr(Integer value_);
static inline base_ptr_t make_base_ptr(integer_t value_);
static inline base_ptr_t make_base_ptr(Number value_);
static inline base_ptr_t make_base_ptr(number_t value_);
static inline base_ptr_t make_base_ptr(String value_);
static inline base_ptr_t make_base_ptr(string_t value_);
static inline base_ptr_t make_base_ptr(char const * value_);
static inline base_ptr_t make_base_ptr(Array value_);
static inline base_ptr_t make_base_ptr(Object value_);

static std::ostream & print(std::ostream & ost, json::Null const & jnull);
static std::ostream & print(std::ostream & ost, json::Boolean const & jboolean);
static std::ostream & print(std::ostream & ost, json::Integer const & jinteger);
static std::ostream & print(std::ostream & ost, json::Number const & jnumber);
static std::ostream & print(std::ostream & ost, json::String const & jstring);
static std::ostream & print(std::ostream & ost, json::Array const & jarray, size_t depth_ = 0, bool padding_ = true);
static std::ostream & print(std::ostream & ost, json::Object const & jobject, size_t depth_ = 0);
static std::ostream & print(std::ostream & ost, json::Base const & jbase, size_t depth_ = 0);

static std::istream & parse(std::istream & ist, json::Null & jnull, char first_char = '\0', bool first_char_read = false) noexcept(false);
static std::istream & parse(std::istream & ist, json::Boolean & jboolean, char first_char = '\0', bool first_char_read = false) noexcept(false);
static std::istream & parse(std::istream & ist, json::Integer & jinteger, char first_char = '\0', bool first_char_read = false) noexcept(false);
static std::istream & parse(std::istream & ist, json::Number & jnumber, char first_char = '\0', bool first_char_read = false) noexcept(false);
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

template <class C> struct CType   { static constexpr Type value = Type::Base; };
template <> struct CType<Null>    { static constexpr Type value = Type::Null; };
template <> struct CType<Boolean> { static constexpr Type value = Type::Boolean; };
template <> struct CType<Integer> { static constexpr Type value = Type::Integer; };
template <> struct CType<Number>  { static constexpr Type value = Type::Number; };
template <> struct CType<String>  { static constexpr Type value = Type::String; };
template <> struct CType<Object>  { static constexpr Type value = Type::Object; };
template <> struct CType<Array>   { static constexpr Type value = Type::Array; };

template <typename T> struct JSONType    { using type = Object; };
template <> struct JSONType<null_t>      { using type = Null; };
template <> struct JSONType<boolean_t>   { using type = Boolean; };
template <> struct JSONType<integer_t>   { using type = Integer; };
template <> struct JSONType<int8_t>      { using type = Integer; };
template <> struct JSONType<uint8_t>     { using type = Integer; };
template <> struct JSONType<int16_t>     { using type = Integer; };
template <> struct JSONType<uint16_t>    { using type = Integer; };
template <> struct JSONType<int32_t>     { using type = Integer; };
template <> struct JSONType<uint32_t>    { using type = Integer; };
template <> struct JSONType<int64_t>     { using type = Integer; };
template <> struct JSONType<uint64_t>    { using type = Integer; };
template <> struct JSONType<number_t>    { using type = Number; };
template <> struct JSONType<float>       { using type = Number; };
template <> struct JSONType<long double> { using type = Number; };
template <>             struct JSONType<string_t>     { using type = String; };
template <>             struct JSONType<char>         { using type = String; };
template <>             struct JSONType<char *>       { using type = String; };
template <>             struct JSONType<char const *> { using type = String; };
template <>             struct JSONType<char[]>       { using type = String; };
template <>             struct JSONType<char const[]> { using type = String; };
template <size_t size_> struct JSONType<char[size_]>  { using type = String; };
template <size_t size_> struct JSONType<char const [size_]> { using type = String; };
template <typename E,size_t size_> struct JSONType<E[size_]>            { using type = Array; };
template <typename E,size_t size_> struct JSONType<std::array<E,size_>> { using type = Array; };
template <typename E>              struct JSONType<E[]>                 { using type = Array; };
template <typename E>              struct JSONType<std::vector<E>>      { using type = Array; };
template <typename E>              struct JSONType<std::list<E>>        { using type = Array; };
template <typename V> struct JSONType<std::map<string_t,V>>           { using type = Object; };
template <typename V> struct JSONType<std::unordered_map<string_t,V>> { using type = Object; };

template <typename T> using json_t = typename JSONType<T>::type;

// precision < 0 means max precision but cut out the right-most zeros
// min_sci_value is the minimum value where the output will switch to scientific notation
// max_sci_value is the maximum value where the output will switch to scientific notation
static string_t
double_to_string(double rhs, int precision_ = -1, double min_sci_value = 1e-2, double max_sci_value = 1e+4) noexcept
{
	constexpr int c_min_integer_index  = 1;
	constexpr int c_max_integer_index  = 16;
	constexpr int c_min_fraction_index = 18;
	constexpr int c_max_fraction_index = 33;
	constexpr int c_max_precision      = 16;
	if(precision_ > c_max_precision)
		precision_ = c_max_precision;
	bool negative = rhs < 0;
	double value = negative ? -rhs : rhs;
	bool scientific_notation = false;
	int exponent_ = 0;
	if(value == 0.0)
		return "0.0";
	else if(value != value) // is nan
		return "null";
	else if(value == std::numeric_limits<double>::infinity())
		return negative ? "-9e+999" : "9e+999";
	else if(value >= std::numeric_limits<double>::max())
	{
		constexpr char max_val_str[]     = "1.7976931348623158e+308";
		constexpr char max_neg_val_str[] = "-1.7976931348623158e+308";
		if(precision_ < 0)
			return negative ? max_neg_val_str : max_val_str;
		else if(negative)
		{
			int precision_index_ = precision_ == 0 ? -1 : precision_;
			std::string str_ { &max_neg_val_str[0], &max_neg_val_str[3+precision_index_] };
			str_ += "e+308";
			return std::move(str_);
		}
		else
		{
			int precision_index_ = precision_ == 0 ? -1 : precision_;
			std::string str_ { &max_val_str[0], &max_val_str[2+precision_index_] };
			str_ += "e+308";
			return std::move(str_);
		}
	}
	if(value >= max_sci_value) // use scientific notation
	{
		scientific_notation = true;
		double sig_digits_ = floor(log10(value));
		value /= pow(10.0, sig_digits_);
		exponent_ += int(sig_digits_);
	}
	else if(value < min_sci_value)
	{
		scientific_notation = true;
		double sig_digits_ = floor(-log10(value)+1);
		value *= pow(10.0, sig_digits_);
		exponent_ -= int(sig_digits_);
	}
	double integer = 0.0;
	double fraction = modf(value, &integer);
	char buffer[] {
		  ' '
		, char('0' + uint64_t(integer / 1e+15) % 10)
		, char('0' + uint64_t(integer / 1e+14) % 10)
		, char('0' + uint64_t(integer / 1e+13) % 10)
		, char('0' + uint64_t(integer / 1e+12) % 10)
		, char('0' + uint64_t(integer / 1e+11) % 10)
		, char('0' + uint64_t(integer / 1e+10) % 10)
		, char('0' + uint64_t(integer / 1e+9) % 10)
		, char('0' + uint64_t(integer / 1e+8) % 10)
		, char('0' + uint64_t(integer / 1e+7) % 10)
		, char('0' + uint64_t(integer / 1e+6) % 10)
		, char('0' + uint64_t(integer / 1e+5) % 10)
		, char('0' + uint64_t(integer / 1e+4) % 10)
		, char('0' + uint64_t(integer / 1e+3) % 10)
		, char('0' + uint64_t(integer / 1e+2) % 10)
		, char('0' + uint64_t(integer / 1e+1) % 10)
		, char('0' + uint64_t(integer) % 10)
		, '.'
		, char('0' + uint64_t(fraction * 1e+1) % 10)
		, char('0' + uint64_t(fraction * 1e+2) % 10)
		, char('0' + uint64_t(fraction * 1e+3) % 10)
		, char('0' + uint64_t(fraction * 1e+4) % 10)
		, char('0' + uint64_t(fraction * 1e+5) % 10)
		, char('0' + uint64_t(fraction * 1e+6) % 10)
		, char('0' + uint64_t(fraction * 1e+7) % 10)
		, char('0' + uint64_t(fraction * 1e+8) % 10)
		, char('0' + uint64_t(fraction * 1e+9) % 10)
		, char('0' + uint64_t(fraction * 1e+10) % 10)
		, char('0' + uint64_t(fraction * 1e+12) % 10)
		, char('0' + uint64_t(fraction * 1e+13) % 10)
		, char('0' + uint64_t(fraction * 1e+14) % 10)
		, char('0' + uint64_t(fraction * 1e+15) % 10)
		, char('0' + uint64_t(fraction * 1e+16) % 10)
		, char('0' + uint64_t(fraction * 1e+17) % 10)
		, '\0'
		, '\0'
		, '\0'
		, '\0'
		, '\0'
		, '\0'
	};
	int i = c_min_integer_index;
	int j = c_max_fraction_index;
	for(; i < c_max_integer_index && buffer[i] == '0'; ++i);
	// trim the rightmost zeros
	if(precision_ < 0)
	{
		for(; j > c_min_fraction_index && buffer[j] == '0'; --j);
		// round the rightmost nines
		if(j == c_max_fraction_index)
		{
			int nines_i = j;
			for(nines_i = j; nines_i >= c_min_fraction_index && buffer[nines_i] == '9'; --nines_i);
			if(nines_i < j)
				precision_ = nines_i - c_min_fraction_index + 1;
		}
	}
	// round to precision
	if(precision_ >= 0)
	{
		int const precision_index = c_min_fraction_index + std::max(std::min(precision_, c_max_precision), 0);
		for(; j > precision_index && buffer[j] == '0'; --j);
		// rounding right to zeros
		int carry = 0;
		for(; j >= precision_index; --j)
		{
			char & chr = buffer[j];
			int digit = int(chr - '0') + carry;
			if(digit >= 5)
				carry = 1;
			else
				carry = 0;
			chr = '0';
		}
		// continue rounding fraction
		for(int k = j; carry > 0 && k >= c_min_fraction_index; --k)
		{
			char & chr = buffer[k];
			int digit = int(chr - '0') + carry;
			if(digit > 9)
			{
				carry = 1;
				chr = '0';
			}
			else
			{
				carry = 0;
				chr = '0' + char(digit);
			}
		}
		// continue rounding integer
		if(carry > 0)
		{
			int const min_integer_index = std::min(i - 1, c_min_integer_index);
			int k = c_max_integer_index;
			for(; carry > 0 && k >= min_integer_index; --k)
			{
				char & chr = buffer[k];
				int digit = int(chr - '0') + carry;
				if(digit > 9)
				{
					carry = 1;
					chr = '0';
				}
				else
				{
					carry = 0;
					chr = '0' + char(digit);
				}
			}
			if(k < i - 1)
				i = k + 1;
		}
	}
	// negative sign
	if(negative)
		buffer[--i] = '-';
	if(precision_ == 0) // 0 precision => remove decimal point
		--j;
	if(scientific_notation)
	{
		bool negative_exp = exponent_ < 0;
		if(negative_exp)
			exponent_ = -exponent_;
		char exp_str[] {
			  char('0' + (exponent_ / 100) % 10)
			, char('0' + (exponent_ / 10) % 10)
			, char('0' + (exponent_) % 10)
			, '\0'
		};
		if(j == c_min_fraction_index && buffer[j] == '0')
			j -= 2;
		buffer[++j] = 'e';
		buffer[++j] = negative_exp ? '-' : '+';
		size_t exp_i = exp_str[0] == '0' ? (exp_str[1] == '0' ? 2 : 1) : 0;
		for(size_t i = exp_i; i < 3; ++i)
			buffer[++j] = exp_str[i];
	}
	buffer[++j] = '\0';
	return { &buffer[i], &buffer[j] };
}

template <class C>
static C &
get(base_ptr_t & base_ptr) noexcept(false)
{
	if(!base_ptr)
		throw std::runtime_error("json::get: accessing null base_ptr");
	if(CType<C>::value != base_ptr->type())
		throw std::runtime_error("json::get: wrong cast, base_ptr");
	return *static_cast<C *>(base_ptr.get());
}

template <class C>
static C const &
get(base_ptr_t const & base_ptr) noexcept(false)
{
	if(!base_ptr)
		throw std::runtime_error("json::get: accessing null base_ptr");
	if(CType<C>::value != base_ptr->type())
		throw std::runtime_error("json::get: wrong cast, base_ptr");
	return *static_cast<C const *>(base_ptr.get());
}

class Base
{
 protected:
	Base()             = default;
	Base(Base &&)      = default;
	Base(Base const &) = delete;

	Base & operator=(Base &&)      = default;
	Base & operator=(Base const &) = delete;

 public:
	virtual ~Base() = default;
	virtual Type type() const noexcept = 0;

};

class Null final : public Base 
{
 public:
	using value_t = null_t;

	Null()             = default;
	~Null()            = default;
	Null(Null &&)      = default;
	Null(Null const &) = delete;
	Null(null_t null_) {}
	
	Null & operator=(Null &&)      = default;
	Null & operator=(Null const &) = delete;

	Type type() const noexcept override  { return Type::Null; }
	
	value_t       null()        noexcept { return json::null; }
	value_t const null()  const noexcept { return json::null; }
	value_t       value()       noexcept { return json::null; }
	value_t const value() const noexcept { return json::null; }
};

class Boolean final : public Base 
{
	boolean_t m_boolean {};

 public:
	using value_t = boolean_t;

	Boolean()                = default;
	~Boolean()               = default;
	Boolean(Boolean &&)      = default;
	Boolean(Boolean const &) = delete;
	Boolean(boolean_t boolean_)
		: m_boolean { boolean_ }
	{}
	
	Boolean & operator=(Boolean &&)      = default;
	Boolean & operator=(Boolean const &) = delete;

	Type type() const noexcept override { return Type::Boolean; }

	boolean_t       & boolean()       noexcept { return m_boolean; }
	boolean_t const & boolean() const noexcept { return m_boolean; }
	value_t         & value()         noexcept { return m_boolean; }
	value_t   const & value()   const noexcept { return m_boolean; }
};

class Integer final : public Base 
{
	integer_t m_integer {};

 public:
	using value_t = integer_t;

	Integer()                = default;
	~Integer()               = default;
	Integer(Integer &&)      = default;
	Integer(Integer const &) = delete;
	Integer(integer_t integer_)
		: m_integer { integer_ }
	{}
	
	Integer & operator=(Integer &&)      = default;
	Integer & operator=(Integer const &) = delete;
	
	Type type() const noexcept override { return Type::Integer; }

	integer_t       & integer()       noexcept { return m_integer; }
	integer_t const & integer() const noexcept { return m_integer; }
	value_t         & value()         noexcept { return m_integer; }
	value_t   const & value()   const noexcept { return m_integer; }
};

class Number final : public Base 
{
	number_t m_number {};

 public:
	using value_t = number_t;

	Number()               = default;
	~Number()              = default;
	Number(Number &&)      = default;
	Number(Number const &) = delete;
	Number(number_t number_)
		: m_number { number_ }
	{}
	
	Number & operator=(Number &&)      = default;
	Number & operator=(Number const &) = delete;
	
	Type type() const noexcept override { return Type::Number; }

	number_t       & number()       noexcept { return m_number; }
	number_t const & number() const noexcept { return m_number; }
	value_t        & value()        noexcept { return m_number; }
	value_t  const & value()  const noexcept { return m_number; }
};

class String final : public Base 
{
	string_t m_string {};

 public:
	using value_t = string_t;

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
	
	String & operator=(String &&)      = default;
	String & operator=(String const &) = delete;
	
	Type type() const noexcept override { return Type::String; }

	string_t       & string()       noexcept { return m_string; }
	string_t const & string() const noexcept { return m_string; }
	value_t        & value()        noexcept { return m_string; }
	value_t  const & value()  const noexcept { return m_string; }

};

class Array final : public Base
{
	array_elements_t m_elements {};

 public:
	using value_t = array_elements_t;

	Array()  = default;
	~Array() = default;
	Array(Array &&)      = default;
	Array(Array const &) = delete;

	Array & operator=(Array &&)      = default;
	Array & operator=(Array const &) = delete;

	template <size_t size_>
	Array(base_ptr_t (&& base_ptrs_)[size_])
	{
		for(auto & e : base_ptrs_)
			m_elements.push_back(std::move(e));
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

	Type type() const noexcept override { return Type::Array; }

	// T must match the json type. Like int types for Integer
	template <typename T, class C = json_t<T>>
	C *
	get(T const & value_)
	{
		auto it = m_elements.begin();
		for(; it != m_elements.end(); ++it)
		{
			auto & cit = get<C>(*it);
			if(cit.value() == value_)
				return &cit;
		}
		return nullptr;
	}

	// T must match the json type. Like int types for Integer
	template <typename T, class C = json_t<T>>
	C const *
	get(T const & value_) const
	{
		auto it = m_elements.begin();
		for(; it != m_elements.end(); ++it)
		{
			auto const & cit = get<C>(*it);
			if(cit.value() == value_)
				return &cit;
		}
		return nullptr;
	}

	// T must match the json type. Like int types for Integer
	template <typename T, typename U, class C = json_t<T>>
	C *
	set(T const & old_value_, U const & new_value_)
	{
		auto it = m_elements.begin();
		for(; it != m_elements.end(); ++it)
		{
			auto & cit = get<C>(*it);
			auto & value = cit.value();
			if(value == old_value_)
			{
				value = new_value_;
				return &cit;
			}
		}
		return nullptr;
	}

	// CAUTION: O(n) time complexity
	array_element_t &
	at(size_t index)
	{
		auto it = m_elements.begin();
		for(size_t i = 0; i < index && it != m_elements.end(); ++i)
			++it;
		if(it == m_elements.end())
			throw std::runtime_error("json::Array::at index out of bounds");
		return *it;
	}

	// CAUTION: O(n) time complexity
	array_element_t const &
	at(size_t index) const
	{
		auto it = m_elements.begin();
		for(size_t i = 0; i < index && it != m_elements.end(); ++i)
			++it;
		if(it == m_elements.end())
			throw std::runtime_error("json::Array::at index out of bounds");
		return *it;
	}

	// T must match the json type. Like int types for Integer
	template <typename T, class C = json_t<T>>
	C *
	insert(T value_)
	{
		auto it = m_elements.insert(m_elements.end(), make_base_ptr(std::move(value_)));
		return &get<C>(*it);
	}

	// T must match the json type. Like int types for Integer
	template <typename T, class C = json_t<T>>
	C *
	rinsert(T const & value_)
	{
		auto it = m_elements.insert(m_elements.begin(), make_base_ptr(value_));
		return &get<C>(*it);
	}

	// T must match the json type. Like int types for Integer
	template <typename T>
	bool
	remove(T const & value_, int skip_ = 0, int count_ = 1)
	{
		if(count_ <= 0)
			return false;
		using C = json_t<T>;
		for(auto it = m_elements.begin(); it != m_elements.end();)
		{
			auto const & e = get<C>(*it);
			if(e.value() == value_ && skip_-- <= 0)
			{
				auto cur_it = it;
				++it;
				m_elements.erase(cur_it);
				if(--count_ <= 0)
					return true;
				continue;
			}
			++it;
		}
		return false;
	}

	template <typename C>
	void
	erase(C * element_ptr)
	{
		if(!element_ptr)
			return;
		for(auto it = m_elements.begin(); it != m_elements.end(); ++it)
		{
			auto const & e = get<C>(*it);
			if(&e == element_ptr)
			{
				auto cur_it = it;
				++it;
				m_elements.erase(cur_it);
				return;
			}
		}
	}

	void
	erase(array_elements_const_iterator_t const & pos_)
	{
		m_elements.erase(pos_);
	}

	template <typename T>
	array_elements_iterator_t
	find(T const & value_)
	{
		using C = json_t<T>;
		for(auto it = m_elements.begin(); it != m_elements.end(); ++it)
		{
			auto const & e = get<C>(*it);
			if(e.value() == value_)
				return it;
		}
		return m_elements.end();
	}

	template <typename T>
	array_elements_const_iterator_t
	find(T const & value_) const
	{
		using C = typename json_t<T>;
		for(auto it = m_elements.cbegin(); it != m_elements.cend(); ++it)
		{
			auto const & e = get<C>(*it);
			if(e.value() == value_)
				return it;
		}
		return m_elements.cend();
	}

	array_elements_t       & elements()       noexcept { return m_elements; }
	array_elements_t const & elements() const noexcept { return m_elements; }
	value_t                & value()          noexcept { return m_elements; }
	value_t          const & value()    const noexcept { return m_elements; }

};

class Object final : public Base
{
	object_entries_t m_entries {};

	template <typename... Args>
	base_ptr_t *
	_set(bool override_, string_t id, Args &&... args) noexcept
	{
		size_t index_0   = 0;
		size_t index_1   = std::min(id.find('.', 0), id.size());
		string_t root_id = id.substr(index_0, index_1 - index_0);
		index_0 = index_1 + 1;
		auto it = m_entries.find(root_id);
		if(it == m_entries.end())
		{
			if(index_0 < id.size())
				it = m_entries.insert(object_entry_t(root_id, make_base_ptr(json::Object()))).first;
			else
			{
				it = m_entries.insert(object_entry_t(root_id, make_base_ptr(std::forward<Args>(args)...))).first;
				return &(it->second);
			}
			if(it == m_entries.end())
				return nullptr;
		}
		json::object_entries_t * cur_object_entries = &m_entries;
		while(index_0 < id.size())
		{
			index_1 = std::min(id.find('.', index_0), id.size());
			string_t cur_id = id.substr(index_0, index_1 - index_0);
			index_0 = index_1 + 1;
			if(it->second->type() != Type::Object)
			{
				if(override_)
					it->second = make_base_ptr(json::Object());
				else
					return nullptr;
			}
			cur_object_entries = &static_cast<json::Object *>(it->second.get())->entries();
			it = cur_object_entries->find(cur_id);
			if(it == cur_object_entries->end())
			{
				if(index_0 < id.size())
					it = cur_object_entries->insert(object_entry_t(cur_id, make_base_ptr(json::Object()))).first;
				else
				{
					it = cur_object_entries->insert(object_entry_t(cur_id, make_base_ptr(std::forward<Args>(args)...))).first;
					return &(it->second);
				}
				if(it == cur_object_entries->end())
					return nullptr;
			}
		}
		it->second = make_base_ptr(std::forward<Args>(args)...);
		return &(it->second);
	}

 public:
	using value_t = object_entries_t;

	Object()  = default;
	~Object() = default;
	Object(Object &&)      = default;
	Object(Object const &) = delete;

	Object & operator=(Object &&) = default;
	Object & operator=(Object const &) = delete;

	template <size_t size_>
	Object(Entry (&& entries_)[size_])
	{
		for(auto & entry_ : entries_)
			m_entries.insert(object_entry_t(entry_.key, std::move(entry_.value)));
	}

	Type type() const noexcept override { return Type::Object; }

	// id must be an aggregate of object keys separated by '.'
	// get("user.name") would look for the object entry with the key 'name' 
	//  in the root object entry with the key 'user' 
	base_ptr_t *
	get(string_t const & id) noexcept
	{
		size_t index_0   = 0;
		size_t index_1   = std::min(id.find('.', 0), id.size());
		string_t root_id = id.substr(index_0, index_1 - index_0);
		index_0 = index_1 + 1;
		auto it = m_entries.find(root_id);
		if(it == m_entries.end())
			return nullptr;
		// consequently locate the entries with the sub ids
		while(index_0 < id.size())
		{
			index_1 = std::min(id.find('.', index_0), id.size());
			string_t cur_id = id.substr(index_0, index_1 - index_0);
			index_0 = index_1 + 1;
			if(it->second->type() != Type::Object)
				return nullptr;
			auto & cur_object_entries = static_cast<json::Object *>(it->second.get())->entries();
			it = cur_object_entries.find(cur_id);
			if(it == cur_object_entries.end())
				return nullptr;
		} 
		return &(it->second);
	}

	base_ptr_t const *
	get(string_t const & id) const noexcept
	{
		size_t index_0   = 0;
		size_t index_1   = std::min(id.find('.', 0), id.size());
		string_t root_id = id.substr(index_0, index_1 - index_0);
		index_0 = index_1 + 1;
		auto it = m_entries.find(root_id);
		if(it == m_entries.end())
			return nullptr;
		while(index_0 < id.size())
		{
			index_1 = std::min(id.find('.', index_0), id.size());
			string_t cur_id = id.substr(index_0, index_1 - index_0);
			index_0 = index_1 + 1;
			if(it->second->type() != Type::Object)
				return nullptr;
			auto & cur_object_entries = static_cast<json::Object const *>(it->second.get())->entries();
			it = cur_object_entries.find(cur_id);
			if(it == cur_object_entries.end())
				return nullptr;
		} 
		return &(it->second);
	}
	
	base_ptr_t &
	operator[](string_t const & id) noexcept(false)
	{
		auto * base_ptr_ = this->get(id);
		if(!base_ptr_)
			throw std::runtime_error("json::Object::operator[]: entry not found");
		return *base_ptr_;
	}
	
	base_ptr_t const &
	operator[](string_t const & id) const noexcept(false)
	{
		auto * base_ptr_ = this->get(id);
		if(!base_ptr_)
			throw std::runtime_error("json::Object::operator[]: entry not found");
		return *base_ptr_;
	}

	// C must be a valid json type such as Null, Boolean or String
	template <class C>
	C *
	get(string_t id) noexcept
	{
		auto * base_ptr = get(id);
		if(!base_ptr || base_ptr->get()->type() != CType<C>::value)
			return nullptr;
		return static_cast<C *>(base_ptr->get());
	}

	template <class C>
	C const *
	get(string_t id) const noexcept
	{
		auto const * base_ptr = get(id);
		if(!base_ptr || base_ptr->get()->type() != CType<C>::value)
			return nullptr;
		return static_cast<C const *>(base_ptr->get());
	}

	// C must be a valid json type such as Null, Boolean or String
	template <class C, typename V = typename C::value_t>
	V *
	get_value(string_t id) noexcept
	{
		auto * base_ptr = get(id);
		if(!base_ptr || base_ptr->get()->type() != CType<C>::value)
			return nullptr;
		return &static_cast<C *>(base_ptr->get())->value();
	}

	template <class C, typename V = typename C::value_t>
	V const *
	get_value(string_t id) const noexcept
	{
		auto const * base_ptr = get(id);
		if(!base_ptr || base_ptr->get()->type() != CType<C>::value)
			return nullptr;
		return &static_cast<C const *>(base_ptr->get())->value();
	}

	// Caution: overrides parents that are not of type Object.
	template <typename... Args>
	inline base_ptr_t *
	set(string_t id, Args &&... args) noexcept
	{
		return _set(true, std::move(id), std::forward<Args>(args)...);
	}

	// no overriding parents that are not of type Object.
	template <typename... Args>
	inline base_ptr_t *
	set_safe(string_t id, Args &&... args) noexcept
	{
		return _set(false, std::move(id), std::forward<Args>(args)...);
	}

	bool
	remove(string_t id) noexcept
	{
		size_t index_0   = 0;
		size_t index_1   = std::min(id.find('.', 0), id.size());
		string_t root_id = id.substr(index_0, index_1 - index_0);
		index_0 = index_1 + 1;
		auto it = m_entries.find(root_id);
		if(it == m_entries.end())
			return false;
		object_entries_t * cur_object_entries = &m_entries;
		string_t cur_id = root_id;
		while(index_0 < id.size())
		{
			index_1 = std::min(id.find('.', index_0), id.size());
			cur_id = id.substr(index_0, index_1 - index_0);
			index_0 = index_1 + 1;
			if(it->second->type() != Type::Object)
				return false;
			cur_object_entries = &static_cast<json::Object *>(it->second.get())->entries();
			it = cur_object_entries->find(cur_id);
			if(it == cur_object_entries->end())
				return false;
		} 
		cur_object_entries->erase(it);
		return true;
	}

	// alias for remove
	inline bool erase(string_t id) noexcept { return remove(std::move(id)); }

	object_entries_t       & entries()       noexcept { return m_entries; }
	object_entries_t const & entries() const noexcept { return m_entries; }
	value_t                & value()         noexcept { return m_entries; }
	value_t          const & value()   const noexcept { return m_entries; }

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

static inline base_ptr_t make_base_ptr()                     { return base_ptr_t(new Null()); }
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

static inline void 
print_padding(std::ostream & ost, size_t depth_)
{
	if(padding)
		for(size_t k = 0; k < depth_; ++k)
			ost << padding;
}

static inline void 
print_newline(std::ostream & ost)
{
	if(newline)
		ost << newline;
}

static std::ostream &
print(std::ostream & ost, json::null_t const & jnull)
{
	return ost << "null";
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
	return ost << double_to_string(jnumber.number());
}

static std::ostream &
print(std::ostream & ost, json::String const & jstring)
{
	return ost << '"' << jstring.string() << '"';
}

static std::ostream &
print(std::ostream & ost, json::Array const & jarray, size_t depth_, bool padding_)
{
	if(padding_)
		print_padding(ost, depth_);
	ost << '[';
	auto const & array_elements = jarray.elements();
	size_t i = 0;
	size_t size_ = array_elements.size();
	size_t size_l = size_ == 0 ? 0 : (size_ - 1);
	bool new_line_ = false;
	for(auto const & element : array_elements)
	{
		if(!element)
			continue;
		switch(element->type())
		{
			case Type::Array:
			case Type::Object:
				new_line_ = true;
				print_newline(ost);
				break;
			case Type::String:
				new_line_ = true;
				print_newline(ost);
				print_padding(ost, depth_ + 1);
				break;
			default:
				break;
		}
		json::print(ost, *element, depth_ + 1);
		if(i < size_l)
			ost << ", ";
		++i;
	}
	if(new_line_ && padding_)
	{
		if(i > 0)
			print_newline(ost);
		if(size_ > 0)
			print_padding(ost, depth_);
	}
	ost << ']';
	return ost;
}

static std::ostream &
print(std::ostream & ost, json::Object const & jobject, size_t depth_)
{
	auto const & jobject_entries = jobject.entries();
	size_t i = 0;
	size_t size_ = jobject_entries.size();
	if(size_ > 0)
		print_padding(ost, depth_);
	ost << '{';
	for(auto const & entry : jobject_entries)
	{
		print_newline(ost);
		print_padding(ost, depth_ + 1);
		ost << '"' << entry.first << "\":";
		if(value_spacing)
			ost << value_spacing;
		auto const & value = entry.second;
		switch(value->type())
		{
			case Type::Array:
			{
				auto const & jarray = get<Array>(value);
				auto const & elements_ = jarray.elements();
				Type first_type = Type::Base;
				if(elements_.size() >= 1)
					first_type = (*elements_.begin())->type();
				bool single_line_ = elements_.size() == 0 
					|| (elements_.size() >= 1 && !(first_type == Type::Array || first_type == Type::Object || first_type == Type::String));
				if(!single_line_)
					print_newline(ost);
				json::print(ost, jarray, depth_ + 1, !single_line_);
				break;
			}
			case Type::Object:
			{
				auto const & jobject = get<Object>(value);
				auto const & entries_ = jobject.entries();
				if(entries_.size() > 0)
					print_newline(ost);
				json::print(ost, jobject, depth_ + 1);
				break;
			}
			default:
				json::print(ost, *value, depth_ + 1);
				break;
		}
		if(i < size_ - 1)
			ost << ",";
		++i;
	}
	if(i > 0)
		print_newline(ost);
	if(size_ > 0)
		print_padding(ost, depth_);
	ost << '}';
	return ost;
}

static std::ostream &
print(std::ostream & ost, json::Base const & jbase, size_t depth_)
{
	switch(jbase.type())
	{
		case json::Type::Null:    return json::print(ost, static_cast<json::Null const &>(jbase));
		case json::Type::Boolean: return json::print(ost, static_cast<json::Boolean const &>(jbase));
		case json::Type::Integer: return json::print(ost, static_cast<json::Integer const &>(jbase));
		case json::Type::Number:  return json::print(ost, static_cast<json::Number const &>(jbase));
		case json::Type::String:  return json::print(ost, static_cast<json::String const &>(jbase));
		case json::Type::Object:  return json::print(ost, static_cast<json::Object const &>(jbase), depth_);
		case json::Type::Array:   return json::print(ost, static_cast<json::Array const &>(jbase), depth_);
		default: return ost;
	}
}

static inline std::ostream &
operator<<(std::ostream & ost, json::Base const & jbase) { return json::print(ost, jbase); }
static inline std::ostream &
operator<<(std::ostream & ost, json::null_t const & jnull) { return json::print(ost, jnull); }
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
	if((first_char_read && std::isspace(first_char)) || !first_char_read)
	{
		do ist.read(&first_char, 1); 
		while(std::isspace(first_char));
		first_char_read = true;
	}
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
	if((first_char_read && std::isspace(first_char)) || !first_char_read)
	{
		do ist.read(&first_char, 1); 
		while(std::isspace(first_char));
		first_char_read = true;
	}
	char buffer[6] { first_char, '\0', '\0', '\0', '\0', '\0' };
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
	if((first_char_read && std::isspace(first_char)) || !first_char_read)
	{
		do ist.read(&first_char, 1); 
		while(std::isspace(first_char));
		first_char_read = true;
	}
	char buffer[16] { '\0' };
	char * it    = &buffer[0];
	char * last_ = &buffer[std::size(buffer) - 1];
	bool reading_first_char = true;
	bool sign_              = true;
	bool continue_          = true;
	for(; continue_ && it < last_; ++it)
	{
		char ch;
		if(first_char_read)
		{
			ch = first_char;
			first_char_read = false;
		}
		else
		{
			ist.read(&ch, 1);
			if(ist.eof())
			{
				continue_ = false;
				break;
			}
		}
		switch(ch)
		{
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
				*it = ch;
				if(reading_first_char)
					reading_first_char = false;
				break;
			case '+':
			case '-':
				if(reading_first_char)
				{
					*it = ch;
					sign_ = true;
					reading_first_char = false;
					break;
				}
			default:
				continue_ = false;
				break;
		}
	}
	*it = '\0';
	jinteger.integer() = integer_t(std::atoll(buffer));
	return ist;
}

static std::istream &
parse(std::istream & ist, json::Number & jnumber, char first_char, bool first_char_read) noexcept(false)
{
	if((first_char_read && std::isspace(first_char)) || !first_char_read)
	{
		do ist.read(&first_char, 1); 
		while(std::isspace(first_char));
		first_char_read = true;
	}
	char buffer[40] { '\0' };
	char * it    = &buffer[0];
	char * last_ = &buffer[std::size(buffer) - 1];
	char * sign_it     = nullptr;
	char * exponent_it = nullptr;
	bool reading_first_char = true;
	bool sign_              = true;
	bool exponent_          = false;
	bool exponent_sign_     = false;
	bool decimal_point_     = false;
	bool continue_          = true;
	for(; it < last_; ++it)
	{
		char ch;
		if(first_char_read)
		{
			ch = first_char;
			first_char_read = false;
		}
		else
		{
			ist.read(&ch, 1);
			if(ist.eof())
			{
				continue_ = false;
				break;
			}
		}
		switch(ch)
		{
			case '.':
				if(!decimal_point_)
					decimal_point_ = true;
				else
				{
					continue_ = false;
					break;
				}
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
				*it = ch;
				if(reading_first_char)
					reading_first_char = false;
				break;
			case 'n':
				if(reading_first_char)
				{
					reading_first_char = true;
					char null_buffer[5] { ch, '\0', '\0', '\0', '\0' };
					ist.read(&null_buffer[1], 3);
					if(0 == strcmp(null_buffer, "null"))
					{
						jnumber.number() = number_t(std::numeric_limits<double>::quiet_NaN());
						return ist;
					}
					continue_ = false;
					break;
				}
			case 'E':
			case 'e':
				if(!exponent_)
				{
					*it = ch;
					exponent_it = it;
					exponent_ = true;
				}
				else
					continue_ = false;
				break;
			case '+':
			case '-':
				if(reading_first_char)
				{
					*it = ch;
					sign_it = it;
					sign_ = true;
					reading_first_char = false;
				}
				else if(exponent_ && !exponent_sign_)
				{
					*it = ch;
					exponent_sign_ = true;
				}
				else
					continue_ = false;
				break;
			default:
				continue_ = false;
				break;
		}
		if(!continue_)
			break;
	}
	*it = '\0';
	if(exponent_it)
	{
		int exponent_ = atoi(&exponent_it[1]);
		int sign_     = (sign_it && *sign_it == '-') ? -1 : 1;
		if(exponent_ == 999) // inf
			jnumber.number() = sign_ * std::numeric_limits<double>::infinity();
	}
	jnumber.number() = number_t(std::atof(buffer));
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
	bool escape_ = false;
	while((escape_ ? true : ch != '"'))
	{
		if(ist.eof())
			throw std::runtime_error("json::parse: end of stream.");
		if(ch == '\\' && !escape_)
			escape_ = true;
		else if(escape_)
			escape_ = false;
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
			auto digits = 0;
			bool scientific_ = false;
			bool scientific_sign_ = false;
			bool continue_ = true;
			while(continue_)
			{
				if(ist.eof())
					throw std::runtime_error("json::parse: end of stream.");
				ist.read(&ch, 1);
				if(ch == 'e')
				{
					if(digits == 0)
						throw std::runtime_error("json::parse: At least one digit required before 'e'.");
					else if(!scientific_)
					{
						buffer += ch;
						scientific_ = true;
						continue;
					}
					else
						break;
				}
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
						++digits;
						buffer += ch;
						break;
					case '+': 
						if(scientific_ && !scientific_sign_)
						{
							scientific_sign_ = true;
							buffer += ch;
						}
						else
							throw std::runtime_error("json::parse: Invalid positive sign in numeric value.");
						break;
					case '-':
						if(scientific_ && !scientific_sign_)
						{
							scientific_sign_ = true;
							buffer += ch;
						}
						else
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
			if(decimal_points > 0 || scientific_)
				jentry.value = make_base_ptr(std::atof(buffer.c_str()));
			else
				jentry.value = make_base_ptr(std::atoll(buffer.c_str()));
		}
		else if(ch != '}')
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
				jelement = make_base_ptr(std::atoll(buffer.c_str()));
		}
		else if(ch != ']')
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
	while(!ist.eof())
	{
		array_element_t jelement;
		parse(ist, jelement, ch);
		if(jelement)
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
	if(!_skip_spaces(ist, ch))
		throw std::runtime_error("json::parse: end of stream.");
	if(!skip_opening_check)
	{
		if(ch != '{')
			throw std::runtime_error("json::parse: missing opening brace for object.");
	}
	if(ch == '}')
		return ist;
	else if(ch == '"')
		ist.putback('"');
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
operator>>(std::istream & ist, json::Null & jnull) { return json::parse(ist, jnull, '\0'); }

static inline std::istream &
operator>>(std::istream & ist, json::Boolean & jboolean) { return json::parse(ist, jboolean, '\0'); }

static inline std::istream &
operator>>(std::istream & ist, json::Integer & jinteger) { return json::parse(ist, jinteger, '\0'); }

static inline std::istream &
operator>>(std::istream & ist, json::Number & jnumber) { return json::parse(ist, jnumber, '\0'); }

static inline std::istream &
operator>>(std::istream & ist, json::Array & jarray) { return json::parse(ist, jarray); }

static inline std::istream &
operator>>(std::istream & ist, json::Object & jobject) { return json::parse(ist, jobject); }

} // namespace json

#endif // JSON_HPP