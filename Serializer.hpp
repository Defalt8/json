#pragma once
#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

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

static inline bool 
is_little_endian() noexcept
{
	static constexpr union U { uint32_t ui32_ = 0xABCDEF12; uint8_t bytes[4]; } u {};
	return u.bytes[0] == 0x12;
}

static constexpr char
hex_char(uint8_t value) 
{
	if(value <= 9)
		return '0' + value;
	else if(value <= 15)
		return 'a' + value - 10;
	else
		return '0';
}

static constexpr uint8_t  
hex_value(char hex_char) 
{
	if(hex_char >= '0' && hex_char <= '9')
		return uint8_t(hex_char - '0');
	else if(hex_char >= 'a' && hex_char <= 'f')
		return 10 + uint8_t(hex_char - 'a');
	else
		return 0;
}

template <typename T>
static json::string_t
data_to_hex_string(T const & data)
{
	constexpr size_t byte_length = 2;
	json::string_t hex_string(sizeof(T) * byte_length, '\0');
	char * buffer = &hex_string[0];
	uint8_t const * data_bytes = reinterpret_cast<uint8_t const *>(&data);
	if(is_little_endian())
	{
		for(size_t i = 0; i < sizeof(T); ++i)
		{
			size_t k = sizeof(T) - i - 1;
			buffer[i * byte_length]     = hex_char(data_bytes[k] / 0x0F);
			buffer[i * byte_length + 1] = hex_char(data_bytes[k] % 0x0F);
		}
	}
	else
	{
		for(size_t i = 0; i < sizeof(T); ++i)
		{
			buffer[i * byte_length]     = hex_char(data_bytes[i] / 0x0F);
			buffer[i * byte_length + 1] = hex_char(data_bytes[i] % 0x0F);
		}
	}
	return std::move(hex_string);
}

template <typename T>
static T
hex_string_to_data(json::string_t const & hex_string)
{
	constexpr size_t byte_length = 2;
	char const * buffer = &hex_string[0];
	T data;
	uint8_t * data_bytes = reinterpret_cast<uint8_t *>(&data);
	if(is_little_endian())
	{
		for(size_t i = 0; i < sizeof(T); ++i)
		{
			size_t k = sizeof(T) - i - 1;
			data_bytes[k] = hex_value(buffer[i * byte_length]) * 0x0F 
				+ hex_value(buffer[i * byte_length + 1]);
		}
	}
	else
	{
		for(size_t i = 0; i < sizeof(T); ++i)
		{
			data_bytes[i] = hex_value(buffer[i * byte_length]) * 0x0F 
				+ hex_value(buffer[i * byte_length + 1]);
		}
	}
	return std::move(data);
}

class SerializerBase
{

 protected:
	SerializerBase() = default;
	SerializerBase(SerializerBase &&) = default;
	
 public:
	virtual ~SerializerBase() = default;
	virtual json::base_ptr_t serialize() const = 0;
	virtual bool deserialize(json::base_ptr_t const &) = 0;
};

template <typename T>
class Serializer final : public SerializerBase
{
 public:
	using data_t = T;
	using serial_t = String;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		return json::make_base_ptr(data_to_hex_string(data_ref));
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::String)
			return false;
		auto const * jstring = dynamic_cast<json::String const *>(base_ptr.get());
		*m_data_ptr = hex_string_to_data<data_t>(jstring->value());
		return true;
	}

};

template <typename T, std::enable_if_t<std::is_constructible<Serializer<T>,T &>::value,bool> = true>
static inline Serializer<T>
make_serializer(T & data_ref) noexcept
{
	return { data_ref };
}

template <typename T>
static inline Serializer<T>
make_serializer(T const & data_ref) noexcept = delete;

template <typename T>
class Serializer<T const> final
{
	Serializer() = delete;
	Serializer(T *) = delete;
};

template <typename T>
class Serializer<T *> final
{
	Serializer() = delete;
	Serializer(T *) = delete;
};

template <typename T>
class Serializer<T const *> final
{
	Serializer() = delete;
	Serializer(T const &) = delete;
};

// null_t
template <>
class Serializer<null_t> final : public SerializerBase
{ 
 public:
	using data_t = null_t;
	using serial_t = Null;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::Null)
			return false;
		data_ref = json::get<json::Null>(base_ptr).value();
		return true;
	}

};

// boolean_t | bool
template <>
class Serializer<boolean_t> final : public SerializerBase
{ 
 public:
	using data_t = boolean_t;
	using serial_t = Boolean;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::Boolean)
			return false;
		data_ref = json::get<json::Boolean>(base_ptr).value();
		return true;
	}
	
	static bool 
	deserialize(std::vector<bool>::reference & data_ref, json::base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::Boolean)
			return false;
		data_ref = json::get<json::Boolean>(base_ptr).value();
		return true;
	}

	static inline bool 
	deserialize(std::vector<bool>::reference && data_ref, json::base_ptr_t const & base_ptr)
	{
		return deserialize(static_cast<std::vector<bool>::reference &>(data_ref), base_ptr);
	}
	
};

// char
template <>
class Serializer<char> final : public SerializerBase
{ 
 public:
	using data_t = char;
	using serial_t = String;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		char buffer[] { data_ref, '\0' };
		return json::make_base_ptr(json::String(buffer));
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::String)
			return false;
		auto str = json::get<json::String>(base_ptr).value();
		data_ref = data_t(str.size() > 0 ? str[0] : '\0');
		return true;
	}

};

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
	return json::detail::ClampedIntegerConverter<T,F>::convert(value);
}

template <typename data_t>
static inline bool
deserialize_integer(data_t & data_ref, json::base_ptr_t const & base_ptr)
{
	if(!base_ptr)
		return false;
	switch(base_ptr->type())
	{
		case json::Type::Integer:
		{
			if(std::is_signed<data_t>::value)
				data_ref = clamped_integer_convert<data_t>(json::get<json::Integer>(base_ptr).value().i64());
			else
				data_ref = clamped_integer_convert<data_t>(json::get<json::Integer>(base_ptr).value().u64());
			return true;
		}
		case json::Type::Number:
		{
			data_ref = data_t(round(json::get<json::Number>(base_ptr).value()));
			return true;
		}
		case json::Type::Boolean:
		{
			data_ref = data_t(json::get<json::Boolean>(base_ptr).value());
			return true;
		}
		case json::Type::Null:
		{
			data_ref = {};
			return true;
		}
	}
	return false;
}

// int8_t
template <>
class Serializer<int8_t> final : public SerializerBase
{ 
 public:
	using data_t = int8_t;
	using serial_t = Integer;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		return json::make_base_ptr(data_ref);
	}

	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		return deserialize_integer(data_ref, base_ptr);
	}

};

// uint8_t
template <>
class Serializer<uint8_t> final : public SerializerBase
{ 
 public:
	using data_t = uint8_t;
	using serial_t = Integer;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		return deserialize_integer(data_ref, base_ptr);
	}

};

// int16_t
template <>
class Serializer<int16_t> final : public SerializerBase
{ 
 public:
	using data_t = int16_t;
	using serial_t = Integer;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		return deserialize_integer(data_ref, base_ptr);
	}

};

// uint16_t
template <>
class Serializer<uint16_t> final : public SerializerBase
{ 
 public:
	using data_t = uint16_t;
	using serial_t = Integer;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		return deserialize_integer(data_ref, base_ptr);
	}
	
};

// int32_t
template <>
class Serializer<int32_t> final : public SerializerBase
{ 
 public:
	using data_t = int32_t;
	using serial_t = Integer;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		return deserialize_integer(data_ref, base_ptr);
	}
	
};

// uint32_t
template <>
class Serializer<uint32_t> final : public SerializerBase
{ 
 public:
	using data_t = uint32_t;
	using serial_t = Integer;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		return deserialize_integer(data_ref, base_ptr);
	}

};

// int64_t
template <>
class Serializer<int64_t> final : public SerializerBase
{ 
 public:
	using data_t = int64_t;
	using serial_t = Integer;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		return deserialize_integer(data_ref, base_ptr);
	}
	
};

// uint64_t
template <>
class Serializer<uint64_t> final : public SerializerBase
{ 
 public:
	using data_t = uint64_t;
	using serial_t = Integer;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		return deserialize_integer(data_ref, base_ptr);
	}
	
};

template <typename data_t>
static inline bool
deserialize_number(data_t & data_ref, json::base_ptr_t const & base_ptr)
{
	if(!base_ptr)
		return false;
	switch(base_ptr->type())
	{
		case json::Type::Number:
		{
			data_ref = data_t(json::get<json::Number>(base_ptr).value());
			return true;
		}
		case json::Type::Integer:
		{
			data_ref = data_t(json::get<json::Integer>(base_ptr).value().i64());
			return true;
		}
		case json::Type::Null:
		{
			data_ref = std::numeric_limits<data_t>::quiet_NaN();
			return true;
		}
	}
	return false;
}

// float
template <>
class Serializer<float> final : public SerializerBase
{ 
 public:
	using data_t = float;
	using serial_t = Number;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		if(data_ref != data_ref) // is nan
			return json::make_base_ptr(json::null);
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		return deserialize_number(data_ref, base_ptr);
	}

};

// double
template <>
class Serializer<double> final : public SerializerBase
{ 
 public:
	using data_t = double;
	using serial_t = Number;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		if(data_ref != data_ref) // is nan
			return json::make_base_ptr(json::null);
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		return deserialize_number(data_ref, base_ptr);
	}

};

// long double
template <>
class Serializer<long double> final : public SerializerBase
{ 
 public:
	using data_t = long double;
	using serial_t = Number;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		if(data_ref != data_ref) // is nan
			return json::make_base_ptr(json::null);
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		return deserialize_number(data_ref, base_ptr);
	}

};

// string_t
template <>
class Serializer<string_t> final : public SerializerBase
{ 
 public:
	using data_t = string_t;
	using serial_t = String;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::String)
			return false;
		data_ref = json::get<json::String>(base_ptr).value();
		return true;
	}

};

// char[size_]
template <size_t size_>
class Serializer<char[size_]> final : public SerializerBase
{ 
 public:
	using data_t = char[size_];
	using serial_t = String;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::String)
			return false;
		auto const & str  = json::get<json::String>(base_ptr).value();
		size_t min_size   = std::min(size_, str.size() + 1);
		size_t min_size_1 = min_size = 0 ? 0 : (min_size - 1);
	  #ifdef _MSC_VER
		strncpy_s(data_ref, str.c_str(), min_size_1);
	  #else
		strncpy(data_ref, str.c_str(), min_size);
	  #endif
		data_ref[min_size_1] = '\0';
		return true;
	}

};

// T[size_]
template <typename T, size_t size_>
class Serializer<T[size_]> final : public SerializerBase
{ 
 public:
	using data_t = T[size_];
	using serial_t = Array;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		json::Array jarr;
		for(auto const & e : data_ref)
			jarr.elements().push_back(Serializer<T>::serialize(e));
		return json::make_base_ptr(std::move(jarr));
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::Array)
			return false;
		auto const & arr = json::get<json::Array>(base_ptr).elements();
		auto it  = arr.begin();
		for(auto & e : data_ref)
		{
			if(it == arr.end())
				break;
			if(!Serializer<T>::deserialize(e, *it))
				return false;
			++it;
		}
		return true;
	}

};

// std::array
template <typename T, size_t size_>
class Serializer<std::array<T,size_>> final : public SerializerBase
{ 
 public:
	using data_t = std::array<T,size_>;
	using serial_t = Array;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		json::Array jarr;
		for(auto const & e : data_ref)
			jarr.elements().push_back(Serializer<T>::serialize(e));
		return json::make_base_ptr(std::move(jarr));
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::Array)
			return false;
		auto const & arr = json::get<json::Array>(base_ptr).elements();
		auto it  = arr.begin();
		for(auto & e : data_ref)
		{
			if(it == arr.end())	
				break;
			if(!Serializer<T>::deserialize(e, *it))
				return false;
			++it;
		}
		return true;
	}

};

// std::vector
template <typename T>
class Serializer<std::vector<T>> final : public SerializerBase
{ 
 public:
	using data_t = std::vector<T>;
	using serial_t = Array;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		json::Array jarr;
		for(auto const & e : data_ref)
			jarr.elements().push_back(Serializer<T>::serialize(e));
		return json::make_base_ptr(std::move(jarr));
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::Array)
			return false;
		auto const & arr = json::get<json::Array>(base_ptr).elements();
		auto it  = arr.begin();
		data_ref = data_t(arr.size());
		for(auto dit = data_ref.begin(); dit != data_ref.end(); ++dit, ++it)
		{
			if(!Serializer<T>::deserialize(*dit, *it))
				return false;
		}
		return true;
	}

};

// std::list
template <typename T>
class Serializer<std::list<T>> final : public SerializerBase
{ 
 public:
	using data_t = std::list<T>;
	using serial_t = Array;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		json::Array jarr;
		for(auto const & e : data_ref)
			jarr.elements().push_back(Serializer<T>::serialize(e));
		return json::make_base_ptr(std::move(jarr));
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::Array)
			return false;
		data_ref.clear();
		auto const & arr = json::get<json::Array>(base_ptr).elements();
		for(auto & e : arr)
		{
			T new_val;
			if(!Serializer<T>::deserialize(new_val, e))
				return false;
			data_ref.push_back(std::move(new_val));
		}
		return true;
	}

};

// std::map
template <typename V>
class Serializer<std::map<string_t,V>> final : public SerializerBase
{ 
 public:
	using data_t = std::map<string_t,V>;
	using serial_t = Object;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		json::Object jobj;
		for(auto const & e : data_ref)
			jobj.entries().insert({e.first, Serializer<V>::serialize(e.second)});
		return json::make_base_ptr(std::move(jobj));
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::Object)
			return false;
		data_ref.clear();
		auto const & obj = json::get<json::Object>(base_ptr).entries();
		for(auto & e : obj)
		{
			V new_val;
			if(!Serializer<V>::deserialize(new_val, e.second))
				return false;
			data_ref.insert({e.first, std::move(new_val)});
		}
		return true;
	}

};

// std::unordered_map
template <typename V>
class Serializer<std::unordered_map<string_t,V>> final : public SerializerBase
{ 
 public:
	using data_t = std::unordered_map<string_t,V>;
	using serial_t = Object;

 private:
	data_t * m_data_ptr = nullptr;

 public:
	Serializer(Serializer &&) = default;
	Serializer(data_t & data_ref)
		: m_data_ptr { &data_ref }
	{}

	data_t * data_ptr() const noexcept { return m_data_ptr; }

	json::base_ptr_t 
	serialize() const override
	{
		if(!m_data_ptr)
			return {};
		return serialize(*m_data_ptr);
	}

	bool 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return false;
		return deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		json::Object jobj;
		for(auto const & e : data_ref)
			jobj.entries().insert({e.first, Serializer<V>::serialize(e.second)});
		return json::make_base_ptr(std::move(jobj));
	}
	
	static bool 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		if(!base_ptr || base_ptr->type() != json::Type::Object)
			return false;
		data_ref.clear();
		auto const & obj = json::get<json::Object>(base_ptr).entries();
		for(auto & e : obj)
		{
			V new_val;
			if(!Serializer<V>::deserialize(new_val, e.second))
				return false;
			data_ref.insert({e.first, std::move(new_val)});
		}
		return true;
	}

};


} // namespace json

#endif // SERIALIZER_HPP