/**
 * This header defines two main functions, `serialize` & `deserialize`.
 * 
 * - These functions can be overloaded and used recursively.
 * 
 * - `serialize` takes at least one argument which is the object/value
 *     to be serialized. Extra arguments must be supported for containers.
 *     It must return a base_ptr_t of the json serialized object. 
 *     `base_ptr_t serialize(T const & object)`
 *     Note that the single argument version of the function must be
 *       available.
 *     `base_ptr_t serialize(T const (&array)[size_], Args &&... args)`
 *     These args would be forwarded to the element serializer if it supports it.
 *     `base_ptr_t serialize(double const value)` 
 *     `base_ptr_t serialize(double const value, int precision_)`
 * 
 * - `deserialize` exactly takes two arguments which are the object/value 
 *     to be deserialized and a base_ptr_t json serialized object.
 *     `bool deserialize(T & object, base_ptr_t const & serial)`
 *     It must return true on success or false otherwise. 
 * 
 * - Integer conversions are handled carefully with `clamped_integer_convert`.
 *  
 */

#pragma once
#ifndef JSON_SERIALIZER_HPP
#define JSON_SERIALIZER_HPP

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <type_traits>
#include <utility>
#include <array>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include "json.hpp"

namespace json {

namespace detail {

template <typename T, typename F
	, bool = std::is_signed<T>::value
	, bool = std::is_signed<F>::value
	, bool = (sizeof(T) == sizeof(F))
	, bool = (sizeof(T) < sizeof(F))>
struct ClampedIntegerConverter
{
	static constexpr T
	convert(F const & value) noexcept { return T(value); }
};

// T su == T su 
template <typename T, bool is_T_signed>
struct ClampedIntegerConverter<T,T,is_T_signed,is_T_signed,true,false>
{
	static constexpr T const &
	convert(T const & value) noexcept { return value; }
};

// T su == F su 
template <typename T, typename F, bool is_T_signed>
struct ClampedIntegerConverter<T,F,is_T_signed,is_T_signed,true,false>
{
	static constexpr T
	convert(F const & value) noexcept { return T(value); }
};

// T s == F u 
template <typename T, typename F>
struct ClampedIntegerConverter<T,F,true,false,true,false>
{
	static constexpr T
	convert(F const & value) noexcept 
	{
		if(value >= static_cast<F>(std::numeric_limits<T>::max()))
			return std::numeric_limits<T>::max();
		return T(value);
	}
};

// T u == F s 
template <typename T, typename F>
struct ClampedIntegerConverter<T,F,false,true,true,false>
{
	static constexpr T
	convert(F const & value) noexcept 
	{
		if(value <= static_cast<F>(std::numeric_limits<T>::min()))
			return std::numeric_limits<T>::min();
		return T(value);
	}
};

// T s < F u 
template <typename T, typename F>
struct ClampedIntegerConverter<T,F,true,false,false,true>
{
	static constexpr T
	convert(F const & value) noexcept 
	{
		if(value >= static_cast<F>(std::numeric_limits<T>::max()))
			return std::numeric_limits<T>::max();
		return T(value);
	}
};

// T u < F s 
template <typename T, typename F>
struct ClampedIntegerConverter<T,F,false,true,false,true>
{
	static constexpr T
	convert(F const & value) noexcept 
	{
		if(value >= static_cast<F>(std::numeric_limits<T>::max()))
			return std::numeric_limits<T>::max();
		else if(value <= static_cast<F>(std::numeric_limits<T>::min()))
			return std::numeric_limits<T>::min();
		return T(value);
	}
};

// T s < F s 
template <typename T, typename F>
struct ClampedIntegerConverter<T,F,true,true,false,true>
{
	static constexpr T
	convert(F const & value) noexcept 
	{
		if(value >= static_cast<F>(std::numeric_limits<T>::max()))
			return std::numeric_limits<T>::max();
		else if(value <= static_cast<F>(std::numeric_limits<T>::min()))
			return std::numeric_limits<T>::min();
		return T(value);
	}
};

// T s > F u 
template <typename T, typename F>
struct ClampedIntegerConverter<T,F,true,false,false,false>
{
	static constexpr T
	convert(F const & value) noexcept { return T(value); }
};

// T u > F s 
template <typename T, typename F>
struct ClampedIntegerConverter<T,F,false,true,false,false>
{
	static constexpr T
	convert(F const & value) noexcept 
	{
		if(value >= std::numeric_limits<F>::max())
			return static_cast<T>(std::numeric_limits<F>::max());
		else if(value <= static_cast<F>(std::numeric_limits<T>::min()))
			return std::numeric_limits<T>::min();
		return T(value);
	}
};


} // namespace detail

template <typename T, typename F>
static constexpr T
clamped_integer_convert(F const & value) noexcept
{
	return detail::ClampedIntegerConverter<T,F>::convert(value);
}

template <typename T, typename... Args>
static inline base_ptr_t 
serialize_raw(T const & object, Args &&... args) 
{
	return make_base_ptr(data_to_hex_string(object)); 
}

template <typename T, typename... Args>
static bool
deserialize_raw(T & object, base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::String)
		return false;
	object = hex_string_to_data<T>(get<String>(serial).value());
	return true;
}

template <typename T, T... sequence_>
struct IntegerSequence
{};

template <size_t... indices_>
using IndexSequence = IntegerSequence<size_t,indices_...>;

namespace detail {

	/** IntegerSequenceMaker */
	template <typename T, T size_, T start_ = 0, bool = (size_ == T{}), T... indices_>
	struct IntegerSequenceMaker
	{
		using type = IntegerSequence<T,indices_...>;
	};

	template <typename T, T size_, T start_>
	struct IntegerSequenceMaker<T,size_,start_,true>
	{
		using type = IntegerSequence<T>;
	};

	template <typename T, T size_, T start_>
	struct IntegerSequenceMaker<T,size_,start_,false>
	{
		using type = typename IntegerSequenceMaker<T,size_,start_,false,start_+size_-1>::type;
	};

	template <typename T, T size_, T start_, T idx_, T... indices_>
	struct IntegerSequenceMaker<T,size_,start_,false,idx_,indices_...>
	{
		using type = typename IntegerSequenceMaker<T,size_,start_,false,idx_-1,idx_,indices_...>::type;
	};

	template <typename T, T size_, T start_, T... indices_>
	struct IntegerSequenceMaker<T,size_,start_,false,start_,indices_...>
	{
		using type = IntegerSequence<T,start_,indices_...>;
	};

} // namespace detail

template <typename T, T size_, T start_ = T{}>
using make_integer_sequence_t         = typename detail::IntegerSequenceMaker<T,size_,start_>::type;

template <size_t size_, size_t start_ = 0>
using make_index_sequence_t           = typename detail::IntegerSequenceMaker<size_t,size_,start_>::type;

namespace detail {

	template <size_t index_, typename Tuple, typename F, typename... Args>
	static constexpr bool
	func_wrapper(Tuple && tuple, F && func, Args &&... args)
	{
		func(index_, std::get<index_>(tuple), std::forward<Args>(args)...);
		return true;
	}

	template <typename Tuple, typename F, size_t... seq_, typename... Args>
	static constexpr void
	for_each_impl(Tuple && tuple, F && func, json::IndexSequence<seq_...> const & seq, Args &&... args)
	{
		bool _[] { func_wrapper<seq_>(tuple, func, std::forward<Args>(args)...)... };
	}

} // namespace detail

template <typename Tuple, typename F, typename... Args>
static constexpr void
for_each(Tuple && tuple, F && func, Args &&... args)
{
	constexpr size_t size_ = std::tuple_size<std::decay_t<Tuple>>::value; 
	detail::for_each_impl(tuple, func, json::make_index_sequence_t<size_>(), std::forward<Args>(args)...);
}

using descriptor_id_t  = char const *;

template <typename T>
struct Descriptor
{
	using id_t  = descriptor_id_t;
	using ptr_t = T *;

	id_t  id  = nullptr;
	ptr_t ptr = nullptr;

};

template <typename MPT>
struct MemberDescriptor
{
	using id_t  = descriptor_id_t;
	using ptr_t = MPT;

	id_t  id  = nullptr;
	ptr_t ptr = nullptr;

};

template <typename T>
static constexpr Descriptor<T>
make_descriptor(descriptor_id_t id_, T & object) noexcept { return { id_, &object }; }

template <class C, typename T>
static constexpr MemberDescriptor<T C::*>
make_descriptor(descriptor_id_t id_, T C::* member_ptr) noexcept { return { id_, member_ptr }; }


/**
 * @brief Specialize this struct for variables like;
 * 
 * 	- Store the pointer-to-members with their ids in a tuple like;
 * 
 *  template <>
 *  struct Descriptors<Player>
 *  {
 *  	static constexpr auto value = std::make_tuple(
 *  			  make_descriptor("name",     &Player::name)
 *  			, make_descriptor("position", &Player::position)
 *  			, make_descriptor("velocity", &Player::velocity)
 *  			, make_descriptor("count",     Player::count_)
 *  		);
 *  	static constexpr bool single_line = false;
 *  };
 *  
 * 
 */
template <class C>
struct Descriptors
{
	// static constexpr auto value = std::make_tuple(
	// 			  make_descriptor("name",     &Player::name)
	// 			, make_descriptor("position", &Player::position)
	// 			, make_descriptor("velocity", &Player::velocity)
	// 			, make_descriptor("count",     Player::count_)
	// 		);
	// static constexpr bool single_line = false;
};

namespace detail {

	struct ObjectSerializer
	{
		template <typename T, class C, typename... Args>
		void
		operator()(size_t i, json::Descriptor<T> const & desc, C const & object, json::Object & jobj, Args &&... args) const
		{
			if(!desc.ptr)
				return;
			auto const & id = desc.id;
			auto const & target = *desc.ptr;
			jobj.set(id, serialize(target, std::forward<Args>(args)...));
		}

		template <typename T, class C, typename... Args>
		void
		operator()(size_t i, json::MemberDescriptor<T> const & member_desc, C const & object, json::Object & jobj, Args &&... args) const
		{
			if(!member_desc.ptr)
				return;
			auto const & id = member_desc.id;
			auto const & target = object.*member_desc.ptr;
			jobj.set(id, serialize(target, std::forward<Args>(args)...));
		}
	};

	struct ObjectDeserializer
	{
		template <typename T, class C, typename... Args>
		void
		operator()(size_t i, json::Descriptor<T> const & desc, C & object, json::Object const & jobj, Args &&... args) const
		{
			if(!desc.ptr)
				return;
			auto const & id = desc.id;
			auto const * jserial = jobj.get(id);
			if(jserial)
			{
				auto & target = *desc.ptr;
				deserialize(target, *jserial, std::forward<Args>(args)...);
			}
		}

		template <typename T, class C, typename... Args>
		void
		operator()(size_t i, json::MemberDescriptor<T> const & member_desc, C & object, json::Object const & jobj, Args &&... args) const
		{
			if(!member_desc.ptr)
				return;
			auto const & id = member_desc.id;
			auto const * jserial = jobj.get(id);
			if(jserial)
			{
				auto & target = object.*member_desc.ptr;
				deserialize(target, *jserial, std::forward<Args>(args)...);
			}
		}
	};

} // namespace detail

template <class C, typename = decltype(Descriptors<C>::value), typename... Args>
static base_ptr_t 
serialize(C const & object, Args &&... args)
{
	json::Object serial;
	serial.single_line() = Descriptors<C>::single_line;
	json::for_each(Descriptors<C>::value, detail::ObjectSerializer(), object, serial, std::forward<Args>(args)...);
	return make_base_ptr(std::move(serial));
}

template <class C, typename = decltype(Descriptors<C>::value), typename... Args>
static bool 
deserialize(C & object, base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::Object)
		return false;
	json::for_each(Descriptors<C>::value, detail::ObjectDeserializer(), object, get<Object>(serial), std::forward<Args>(args)...);
	return true;
}

// null_t

template <typename... Args>
static inline base_ptr_t serialize(null_t const & null_, Args &&... args) { return make_base_ptr<Null>(); }

template <typename... Args>
static bool 
deserialize(null_t & null_, base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::Null)
		return false;
	return true;
}

// boolean_t

template <typename... Args>
static base_ptr_t serialize(boolean_t const value_, Args &&... args) { return make_base_ptr(value_); }

template <typename... Args>
static bool 
deserialize(boolean_t & value_, base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::Boolean)
		return false;
	value_ = get<Boolean>(serial).value();
	return true;
}

template <typename... Args>
static bool 
deserialize(std::vector<boolean_t>::reference & value_, base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::Boolean)
		return false;
	value_ = get<Boolean>(serial).value();
	return true;
}

template <typename... Args>
static inline bool 
deserialize(std::vector<boolean_t>::reference && value_, base_ptr_t const & serial, Args &&... args)
{
	return deserialize(static_cast<std::vector<bool>::reference &>(value_), serial);
}

// char

template <typename... Args>
static base_ptr_t 
serialize(char const value_, Args &&... args)
{
	char buffer[] { value_, '\0' };
	return make_base_ptr(String(buffer));
}

template <typename... Args>
static bool 
deserialize(char & value_, base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::String)
		return false;
	auto const & str = get<String>(serial).value();
	value_ = char(str.size() > 0 ? str[0] : '\0');
	return true;
}

// integer_t

template <typename T>
static inline bool
deserialize_integer(T & value_, base_ptr_t const & serial)
{
	if(!serial)
		return false;
	switch(serial->type())
	{
		case Type::Integer:
		{
			if(std::is_signed<T>::value)
				value_ = clamped_integer_convert<T>(get<Integer>(serial).value().i64());
			else
				value_ = clamped_integer_convert<T>(get<Integer>(serial).value().u64());
			return true;
		}
		case Type::Number:
		{
			value_ = T(round(get<Number>(serial).value()));
			return true;
		}
		case Type::Boolean:
		{
			value_ = T(get<Boolean>(serial).value());
			return true;
		}
		case Type::Null:
		{
			value_ = {};
			return true;
		}
	}
	return false;
}

template <typename... Args>
static base_ptr_t serialize(int8_t const value_, Args &&... args)   { return make_base_ptr(value_); }
template <typename... Args>
static base_ptr_t serialize(uint8_t const value_, Args &&... args)  { return make_base_ptr(value_); }
template <typename... Args>
static base_ptr_t serialize(int16_t const value_, Args &&... args)  { return make_base_ptr(value_); }
template <typename... Args>
static base_ptr_t serialize(uint16_t const value_, Args &&... args) { return make_base_ptr(value_); }
template <typename... Args>
static base_ptr_t serialize(int32_t const value_, Args &&... args)  { return make_base_ptr(value_); }
template <typename... Args>
static base_ptr_t serialize(uint32_t const value_, Args &&... args) { return make_base_ptr(value_); }
template <typename... Args>
static base_ptr_t serialize(int64_t const value_, Args &&... args)  { return make_base_ptr(value_); }
template <typename... Args>
static base_ptr_t serialize(uint64_t const value_, Args &&... args) { return make_base_ptr(value_); }

template <typename... Args>
static bool deserialize(int8_t & value_, base_ptr_t const & serial, Args &&... args)   { return deserialize_integer(value_, serial); }
template <typename... Args>
static bool deserialize(uint8_t & value_, base_ptr_t const & serial, Args &&... args)  { return deserialize_integer(value_, serial); }
template <typename... Args>
static bool deserialize(int16_t & value_, base_ptr_t const & serial, Args &&... args)  { return deserialize_integer(value_, serial); }
template <typename... Args>
static bool deserialize(uint16_t & value_, base_ptr_t const & serial, Args &&... args) { return deserialize_integer(value_, serial); }
template <typename... Args>
static bool deserialize(int32_t & value_, base_ptr_t const & serial, Args &&... args)  { return deserialize_integer(value_, serial); }
template <typename... Args>
static bool deserialize(uint32_t & value_, base_ptr_t const & serial, Args &&... args) { return deserialize_integer(value_, serial); }
template <typename... Args>
static bool deserialize(int64_t & value_, base_ptr_t const & serial, Args &&... args)  { return deserialize_integer(value_, serial); }
template <typename... Args>
static bool deserialize(uint64_t & value_, base_ptr_t const & serial, Args &&... args) { return deserialize_integer(value_, serial); }

// number_t

template <typename T>
static inline bool
deserialize_number(T & value_, base_ptr_t const & serial)
{
	if(!serial)
		return false;
	switch(serial->type())
	{
		case Type::Number:
		{
			value_ = T(get<Number>(serial).value());
			return true;
		}
		case Type::Integer:
		{
			value_ = T(get<Integer>(serial).value().i64());
			return true;
		}
		case Type::Null:
		{
			value_ = std::numeric_limits<T>::quiet_NaN();
			return true;
		}
	}
	return false;
}

template <typename... Args>
static base_ptr_t serialize(float const value_, int precision_ = -1, Args &&... args)       { return value_ != value_ ? make_base_ptr(null) : make_base_ptr(value_, precision_); }
template <typename... Args>
static base_ptr_t serialize(double const value_, int precision_ = -1, Args &&... args)      { return value_ != value_ ? make_base_ptr(null) : make_base_ptr(value_, precision_); }
template <typename... Args>
static base_ptr_t serialize(long double const value_, int precision_ = -1, Args &&... args) { return value_ != value_ ? make_base_ptr(null) : make_base_ptr(value_, precision_); }

template <typename... Args>
static bool deserialize(float & value_, base_ptr_t const & serial, Args &&... args)       { return deserialize_number(value_, serial); }
template <typename... Args>
static bool deserialize(double & value_, base_ptr_t const & serial, Args &&... args)      { return deserialize_number(value_, serial); }
template <typename... Args>
static bool deserialize(long double & value_, base_ptr_t const & serial, Args &&... args) { return deserialize_number(value_, serial); }

// string_t

template <typename... Args>
static base_ptr_t serialize(string_t const & string_, Args &&... args) { return make_base_ptr(string_); } 

template <typename... Args>
static bool 
deserialize(string_t & string_, base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::String)
		return false;
	string_ = get<String>(serial).value();
	return true;
}

// char[size_]

template <size_t size_, typename... Args>
static base_ptr_t serialize(char const (& string_)[size_], Args &&... args) { return make_base_ptr(string_); }

template <size_t size_, typename... Args>
static bool 
deserialize(char (& string_)[size_], base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::String)
		return false;
	auto const & str  = get<String>(serial).value();
	size_t min_size   = std::min(size_, str.size() + 1);
	size_t min_size_1 = min_size = 0 ? 0 : (min_size - 1);
	#ifdef _MSC_VER
	strncpy_s(string_, str.c_str(), min_size_1);
	#else
	strncpy(string_, str.c_str(), min_size);
	#endif
	string_[min_size_1] = '\0';
	return true;
}


// T[size_]

template <typename T, size_t size_, typename... Args>
static base_ptr_t 
serialize(T const (& array_)[size_], Args &&... args)
{
	Array jarr;
	for(auto const & e : array_)
		jarr.elements().push_back(serialize(e, std::forward<Args>(args)...));
	return make_base_ptr(std::move(jarr));
}

template <typename T, size_t size_, typename... Args>
static bool 
deserialize(T (& array_)[size_], base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::Array)
		return false;
	auto const & arr = get<Array>(serial).elements();
	auto it  = arr.begin();
	for(auto & e : array_)
	{
		if(it == arr.end())
			break;
		if(!deserialize(e, *it, std::forward<Args>(args)...))
			return false;
		++it;
	}
	return true;
}

// std::array

template <typename T, size_t size_, typename... Args>
static base_ptr_t 
serialize(std::array<T,size_> const & array_, Args &&... args)
{
	Array jarr;
	for(auto const & e : array_)
		jarr.elements().push_back(serialize(e, std::forward<Args>(args)...));
	return make_base_ptr(std::move(jarr));
}

template <typename T, size_t size_, typename... Args>
static bool 
deserialize(std::array<T,size_> & array_, base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::Array)
		return false;
	auto const & arr = get<Array>(serial).elements();
	auto it  = arr.begin();
	for(auto & e : array_)
	{
		if(it == arr.end())	
			break;
		if(!deserialize(e, *it, std::forward<Args>(args)...))
			return false;
		++it;
	}
	return true;
}

// std::vector

template <typename T, typename... Args>
static base_ptr_t 
serialize(std::vector<T> const & array_, Args &&... args)
{
	Array jarr;
	for(auto const & e : array_)
		jarr.elements().push_back(serialize(e, std::forward<Args>(args)...));
	return make_base_ptr(std::move(jarr));
}

template <typename T, typename... Args>
static bool 
deserialize(std::vector<T> & array_, base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::Array)
		return false;
	auto const & arr = get<Array>(serial).elements();
	auto it  = arr.begin();
	array_ = std::vector<T>(arr.size());
	for(auto dit = array_.begin(); dit != array_.end(); ++dit, ++it)
	{
		if(!deserialize(*dit, *it, std::forward<Args>(args)...))
			return false;
	}
	return true;
}

// std::list

template <typename T, typename... Args>
static base_ptr_t 
serialize(std::list<T> const & list_, Args &&... args)
{
	Array jarr;
	for(auto const & e : list_)
		jarr.elements().push_back(serialize(e, std::forward<Args>(args)...));
	return make_base_ptr(std::move(jarr));
}

template <typename T, typename... Args>
static bool 
deserialize(std::list<T> & list_, base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::Array)
		return false;
	list_.clear();
	auto const & arr = get<Array>(serial).elements();
	for(auto & e : arr)
	{
		T new_val;
		if(!deserialize(new_val, e, std::forward<Args>(args)...))
			return false;
		list_.push_back(std::move(new_val));
	}
	return true;
}

// std::map

template <typename V, typename... Args>
static base_ptr_t 
serialize(std::map<string_t,V> const & map_, Args &&... args)
{
	Object jobj;
	for(auto const & e : map_)
		jobj.entries().insert({e.first, serialize(e.second, std::forward<Args>(args)...)});
	return make_base_ptr(std::move(jobj));
}

template <typename V, typename... Args>
static bool 
deserialize(std::map<string_t,V> & map_, base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::Object)
		return false;
	map_.clear();
	auto const & obj = get<Object>(serial).entries();
	for(auto & e : obj)
	{
		V new_val;
		if(!deserialize(new_val, e.second, std::forward<Args>(args)...))
			return false;
		map_.insert({e.first, std::move(new_val)});
	}
	return true;
}

// std::unordered_map

template <typename V, typename... Args>
static base_ptr_t 
serialize(std::unordered_map<string_t,V> const & map_, Args &&... args)
{
	Object jobj;
	for(auto const & e : map_)
		jobj.entries().insert({e.first, serialize(e.second)});
	return make_base_ptr(std::move(jobj));
}

template <typename V, typename... Args>
static bool 
deserialize(std::unordered_map<string_t,V> & map_, base_ptr_t const & serial, Args &&... args)
{
	if(!serial || serial->type() != Type::Object)
		return false;
	map_.clear();
	auto const & obj = get<Object>(serial).entries();
	for(auto & e : obj)
	{
		V new_val;
		if(!deserialize(new_val, e.second, std::forward<Args>(args)...))
			return false;
		map_.insert({e.first, std::move(new_val)});
	}
	return true;
}

} // namespace json

#endif // JSON_SERIALIZER_HPP