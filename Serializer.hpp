#pragma once
#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include <cstdint>
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
	virtual void deserialize(json::base_ptr_t const &) = 0;
};

template <typename T>
class Serializer : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		return json::make_base_ptr(data_to_hex_string(data_ref));
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jstring = dynamic_cast<json::String const *>(base_ptr.get());
		if(!jstring)
			return;
		*m_data_ptr = hex_string_to_data<data_t>(jstring->value());
	}

};

template <typename T>
static inline Serializer<T>
make_serializer(T & data_ref) noexcept
{
	return { data_ref };
}

// null_t
template <>
class Serializer<null_t> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jnull = dynamic_cast<json::Null const *>(base_ptr.get());
		if(!jnull)
			return;
		data_ref = jnull->value();
	}

};

// boolean_t | bool
template <>
class Serializer<boolean_t> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jbool = dynamic_cast<json::Boolean const *>(base_ptr.get());
		if(!jbool)
			return;
		data_ref = jbool->value();
	}
	
	static void 
	deserialize(std::vector<bool>::reference & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jbool = dynamic_cast<json::Boolean const *>(base_ptr.get());
		if(!jbool)
			return;
		data_ref = jbool->value();
	}

	static inline void 
	deserialize(std::vector<bool>::reference && data_ref, json::base_ptr_t const & base_ptr)
	{
		deserialize(static_cast<std::vector<bool>::reference &>(data_ref), base_ptr);
	}
	
};

// char
template <>
class Serializer<char> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		char buffer[] { data_ref, '\0' };
		return json::make_base_ptr(json::String(buffer));
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jstr = dynamic_cast<json::String const *>(base_ptr.get());
		if(!jstr)
			return;
		auto const & str = jstr->value();
		data_ref = data_t(str.size() > 0 ? str[0] : '\0');
	}

};

// int8_t
template <>
class Serializer<int8_t> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jval = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jval)
			return;
		data_ref = data_t(jval->value());
	}

};

// uint8_t
template <>
class Serializer<uint8_t> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jval = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jval)
			return;
		data_ref = data_t(jval->value());
	}

};

// int16_t
template <>
class Serializer<int16_t> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jval = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jval)
			return;
		data_ref = data_t(jval->value());
	}

};

// uint16_t
template <>
class Serializer<uint16_t> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jval = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jval)
			return;
		data_ref = data_t(jval->value());
	}

};

// int32_t
template <>
class Serializer<int32_t> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jval = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jval)
			return;
		data_ref = data_t(jval->value());
	}

};

// uint32_t
template <>
class Serializer<uint32_t> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jval = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jval)
			return;
		data_ref = data_t(jval->value());
	}


};

// int64_t
template <>
class Serializer<int64_t> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jval = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jval)
			return;
		data_ref = data_t(jval->value());
	}


};

// uint64_t
template <>
class Serializer<uint64_t> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jval = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jval)
			return;

		data_ref = data_t(jval->value());
	}
	
};

// float
template <>
class Serializer<float> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jval = dynamic_cast<json::Number const *>(base_ptr.get());
		if(!jval)
			return;
		data_ref = data_t(jval->value());
	}

};

// double
template <>
class Serializer<double> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jval = dynamic_cast<json::Number const *>(base_ptr.get());
		if(!jval)
			return;
		data_ref = data_t(jval->value());
	}

};

// long double
template <>
class Serializer<long double> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jval = dynamic_cast<json::Number const *>(base_ptr.get());
		if(!jval)
			return;
		data_ref = data_t(jval->value());
	}

};

// string_t
template <>
class Serializer<string_t> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jval = dynamic_cast<json::String const *>(base_ptr.get());
		if(!jval)
			return;
		data_ref = data_t(jval->value());
	}

};

// char[]
template <size_t size_>
class Serializer<char[size_]> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		return json::make_base_ptr(data_ref);
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jval = dynamic_cast<json::String const *>(base_ptr.get());
		if(!jval)
			return;
		auto const & str = jval->value();
		size_t min_size = std::min(size_, str.size() + 1);
	  #ifdef _MSC_VER
		strncpy_s(data_ref, str.c_str(), min_size);
	  #else
		strncpy(data_ref, str.c_str(), min_size);
	  #endif
		data_ref[min_size = 0 ? 0 : (min_size - 1)] = '\0';
	}

};

// std::array
template <typename T, size_t size_>
class Serializer<std::array<T,size_>> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		json::Array jarr;
		for(auto const & e : data_ref)
			jarr.elements().push_back(Serializer<T>::serialize(e));
		return json::make_base_ptr(std::move(jarr));
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jarr = dynamic_cast<json::Array const *>(base_ptr.get());
		if(!jarr)
			return;
		auto it  = jarr->elements().begin();
		for(auto & e : data_ref)
		{
			Serializer<T>::deserialize(e, *it);
			++it;
		}
	}

};

// std::vector
template <typename T>
class Serializer<std::vector<T>> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		json::Array jarr;
		for(auto const & e : data_ref)
			jarr.elements().push_back(Serializer<T>::serialize(e));
		return json::make_base_ptr(std::move(jarr));
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jarr = dynamic_cast<json::Array const *>(base_ptr.get());
		if(!jarr)
			return;
		auto const & arr = jarr->elements();
		auto it  = arr.begin();
		data_ref = data_t(arr.size());
		for(auto dit = data_ref.begin(); dit != data_ref.end(); ++dit)
		{
			Serializer<T>::deserialize(*dit, *it);
			++it;
		}
	}

};

// std::list
template <typename T>
class Serializer<std::list<T>> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		json::Array jarr;
		for(auto const & e : data_ref)
			jarr.elements().push_back(Serializer<T>::serialize(e));
		return json::make_base_ptr(std::move(jarr));
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jarr = dynamic_cast<json::Array const *>(base_ptr.get());
		if(!jarr)
			return;
		data_ref.clear();
		auto const & arr = jarr->elements();
		for(auto & e : arr)
		{
			T new_val;
			Serializer<T>::deserialize(new_val, e);
			data_ref.push_back(std::move(new_val));
		}
	}

};

// std::map
template <typename V>
class Serializer<std::map<string_t,V>> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		json::Object jobj;
		for(auto const & e : data_ref)
			jobj.entries().insert({e.first, Serializer<V>::serialize(e.second)});
		return json::make_base_ptr(std::move(jobj));
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jobj = dynamic_cast<json::Object const *>(base_ptr.get());
		if(!jobj)
			return;
		data_ref.clear();
		auto const & obj = jobj->entries();
		for(auto & e : obj)
		{
			V new_val;
			Serializer<V>::deserialize(new_val, e.second);
			data_ref.insert({e.first, std::move(new_val)});
		}
	}

};

// std::unordered_map
template <typename V>
class Serializer<std::unordered_map<string_t,V>> : public SerializerBase
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

	void 
	deserialize(json::base_ptr_t const & base_ptr) override 
	{ 
		if(!m_data_ptr)
			return;
		deserialize(*m_data_ptr, base_ptr); 
	};

	static json::base_ptr_t 
	serialize(data_t const & data_ref)
	{
		json::Object jobj;
		for(auto const & e : data_ref)
			jobj.entries().insert({e.first, Serializer<V>::serialize(e.second)});
		return json::make_base_ptr(std::move(jobj));
	}
	
	static void 
	deserialize(data_t & data_ref, json::base_ptr_t const & base_ptr)
	{
		auto const * jobj = dynamic_cast<json::Object const *>(base_ptr.get());
		if(!jobj)
			return;
		data_ref.clear();
		auto const & obj = jobj->entries();
		for(auto & e : obj)
		{
			V new_val;
			Serializer<V>::deserialize(new_val, e.second);
			data_ref.insert({e.first, std::move(new_val)});
		}
	}

};


} // namespace json

#endif // SERIALIZER_HPP