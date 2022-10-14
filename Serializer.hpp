#pragma once
#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include <cstdint>
#include <utility>
#include <array>
#include <vector>
#include "json.hpp"

namespace json {

template <typename group_ = void, typename... Types> class Serializer;

static constexpr bool 
is_little_endian() noexcept
{
	constexpr union U { uint32_t ui32_ = 0xABCDEF12; uint8_t bytes[4]; } u;
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

template <typename F, typename S>
struct Pair
{
	F first  {};
	S second {};

	Pair() = default;
	Pair(Pair &&) = default;
	Pair(Pair const &) = default;
	Pair & operator=(Pair &&) = default;
	Pair & operator=(Pair const &) = default;

	template <typename T, typename U>
	Pair(T && first_, U && second_)
		: first  { static_cast<F>(first_) }
		, second { static_cast<S>(second_) }
	{}
};

class SerialBase
{
 protected:
	SerialBase() = default;
	SerialBase(SerialBase &&) = default;
	SerialBase(SerialBase const &) = default;

 public:
	virtual ~SerialBase() = default;

	virtual json::string_t const & id()  const = 0;
	virtual json::base_ptr_t serialize() const = 0;
	virtual void deserialize(json::base_ptr_t const & base_ptr) = 0;
};

namespace detail {

template <typename T, typename C = decltype(std::declval<T>().serialize())>
static inline bool 
is_serializable(bool);

template <typename T>
static inline void 
is_serializable(...);

} // namespace detail

template <typename T, typename = decltype(detail::is_serializable<T>(false))>
struct IsSerializable : std::bool_constant<true> {};
template <typename T>
struct IsSerializable<T,void> : std::bool_constant<false> {};

template <typename T> struct StripArray { using type = T; };
template <typename T> struct StripArray<T[]> { using type = T; };
template <typename T> struct StripArray<std::vector<T>> { using type = T; };
template <typename T, size_t size_> struct StripArray<T[size_]> { using type = T; };
template <typename T> using strip_array_t = typename StripArray<T>::type;

// Specialize this class to your types using a unique group type
template <typename T, typename group_ = void, bool is_array_ = std::is_array<T>::value
	, bool is_serializer_ = IsSerializable<strip_array_t<T>>::value>
class Serial
{};

template <typename T, typename group_>
class Serial<T,group_,false,false> : public SerialBase
{
 public:
	using data_t = T;

 private:
	json::string_t m_id {};
	data_t       * m_data = nullptr;

 public:
	Serial() = default;
	Serial(Serial &&) = default;
	Serial(Serial const &) = default;
	Serial(json::string_t id_, data_t & val_ref)
		: m_id   { std::move(id_) }
		, m_data { &val_ref }
	{}

	json::string_t const & id()   const override { return m_id; }
	data_t               * data() const noexcept { return m_data; }
	
	json::base_ptr_t
	serialize() const override
	{
		if(!m_data)
			return {};
		return json::make_base_ptr(data_to_hex_string(*m_data));
	}

	void
	deserialize(json::base_ptr_t const & base_ptr) override
	{
		if(!m_data)
			return;
		auto const * jstring = dynamic_cast<json::String const *>(base_ptr.get());
		if(!jstring)
			return;
		*m_data = hex_string_to_data<T>(jstring->value());
	}

};

template <typename T, typename group_>
class Serial<T,group_,false,true> : public SerialBase
{
 public:
	using data_t = T;

 private:
	json::string_t m_id {};
	data_t       * m_data = nullptr;

 public:
	Serial() = default;
	Serial(Serial &&) = default;
	Serial(Serial const &) = default;
	Serial(json::string_t id_, data_t & val_ref)
		: m_id   { std::move(id_) }
		, m_data { &val_ref }
	{}

	json::string_t const & id()   const override { return m_id; }
	data_t               * data() const noexcept { return m_data; }
	
	json::base_ptr_t
	serialize() const override
	{
		if(!m_data)
			return {};
		return json::make_base_ptr(m_data->serialize());
	}

	void
	deserialize(json::base_ptr_t const & base_ptr) override
	{
		if(!m_data)
			return;
		auto const * jobject = dynamic_cast<json::Object const *>(base_ptr.get());
		if(!jobject)
			return;
		m_data->deserialize(*jobject);
	}

};

template <typename group_, typename... Types>
class Serial<Serializer<group_,Types...>,group_,false,false> : public SerialBase
{
 public:
	using data_t = Serializer<group_,Types...>;

 private:
	json::string_t m_id {};
	data_t       * m_data = nullptr;

 public:
	Serial() = default;
	Serial(Serial &&) = default;
	Serial(Serial const &) = default;
	Serial(json::string_t id_, data_t & val_ref)
		: m_id   { std::move(id_) }
		, m_data { &val_ref }
	{}

	json::string_t const & id()   const override { return m_id; }
	data_t               * data() const noexcept { return m_data; }
	
	json::base_ptr_t
	serialize() const override
	{
		if(!m_data)
			return {};
		return json::make_base_ptr(m_data->serialize());
	}

	void
	deserialize(json::base_ptr_t const & base_ptr) override
	{
		if(!m_data)
			return;
		auto const * jobject = dynamic_cast<json::Object const *>(base_ptr.get());
		if(!jobject)
			return;
		m_data->deserialize(*jobject);
	}

};

template <typename group_>
class Serial<json::string_t,group_,false,false> : public SerialBase
{
 public:
	using data_t = json::string_t;

 private:
	json::string_t m_id {};
	data_t       * m_data = nullptr;

 public:
	Serial() = default;
	Serial(Serial &&) = default;
	Serial(Serial const &) = default;
	Serial(json::string_t id_, data_t & val_ref)
		: m_id   { std::move(id_) }
		, m_data { &val_ref }
	{}

	json::string_t const & id()   const override { return m_id; }
	data_t               * data() const noexcept { return m_data; }
	
	json::base_ptr_t
	serialize() const override
	{
		if(!m_data)
			return {};
		return json::make_base_ptr(*m_data);
	}

	void
	deserialize(json::base_ptr_t const & base_ptr) override
	{
		if(!m_data)
			return;
		auto const * jstring = dynamic_cast<json::String const *>(base_ptr.get());
		if(!jstring)
			return;
		*m_data = jstring->value();
	}

};

template <typename group_, typename T, size_t size_>
class Serial<T[size_],group_,true,false> : public SerialBase
{
 public:
	using data_t = T[size_];

 private:
	json::string_t m_id   {};
	T            * m_data = nullptr;

 public:
	Serial() = default;
	Serial(Serial &&) = default;
	Serial(Serial const &) = default;
	Serial(json::string_t id_, T * array_)
		: m_id   { std::move(id_) }
		, m_data { array_ }
	{}

	json::string_t const & id()   const override { return m_id; }
	data_t               * data() const noexcept { return m_data; }
	
	json::base_ptr_t
	serialize() const override
	{
		if(!m_data)
			return {};
		json::Array jarray;
		for(size_t i = 0; i < size_; ++i)
			jarray.elements().push_back(json::make_base_ptr(data_to_hex_string(m_data[i])));
		return json::make_base_ptr(std::move(jarray));
	}

	void
	deserialize(json::base_ptr_t const & base_ptr) override
	{
		if(!m_data)
			return;
		auto const * jarray = dynamic_cast<json::Array const *>(base_ptr.get());
		if(!jarray)
			return;
		auto it = jarray->elements().begin();
		for(size_t i = 0; i < size_; ++i)
			m_data[i] = hex_string_to_data<T>(static_cast<json::String const *>((*it++).get())->value());
	}

};

template <typename T, size_t size_, typename group_>
class Serial<T[size_],group_,true,true> : public SerialBase
{
 public:
	using data_t = T[size_];

 private:
	json::string_t m_id   {};
	T            * m_data = nullptr;

 public:
	Serial() = default;
	Serial(Serial &&) = default;
	Serial(Serial const &) = default;
	Serial(json::string_t id_, T * array_)
		: m_id   { std::move(id_) }
		, m_data { array_ }
	{}

	json::string_t const & id()   const override { return m_id; }
	T                    * data() const noexcept { return m_data; }
	
	json::base_ptr_t
	serialize() const override
	{
		if(!m_data)
			return {};
		json::Array jarray;
		for(size_t i = 0; i < size_; ++i)
			jarray.elements().push_back(json::make_base_ptr(m_data[i].serialize()));
		return json::make_base_ptr(std::move(jarray));
	}

	void
	deserialize(json::base_ptr_t const & base_ptr) override
	{
		if(!m_data)
			return;
		auto const * jarray = dynamic_cast<json::Array const *>(base_ptr.get());
		if(!jarray)
			return;
		auto it = jarray->elements().begin();
		for(size_t i = 0; i < size_; ++i)
			m_data[i].deserialize(*static_cast<json::Object const *>((*it++).get()));
	}

};

template <typename T, typename group_>
class Serial<std::vector<T>,group_,false,true> : public SerialBase
{
 public:
	using data_t = std::vector<T>;

 private:
	json::string_t m_id   {};
	data_t       * m_data = nullptr;

 public:
	Serial() = default;
	Serial(Serial &&) = default;
	Serial(Serial const &) = default;
	Serial(json::string_t id_, data_t & array_)
		: m_id   { std::move(id_) }
		, m_data { &array_ }
	{}

	json::string_t const & id()   const override { return m_id; }
	T                    * data() const noexcept { return m_data; }
	
	json::base_ptr_t
	serialize() const override
	{
		if(!m_data)
			return {};
		json::Array jarray;
		for(auto const & e : *m_data)
			jarray.elements().push_back(json::make_base_ptr(e.serialize()));
		return json::make_base_ptr(std::move(jarray));
	}

	void
	deserialize(json::base_ptr_t const & base_ptr) override
	{
		if(!m_data)
			return;
		auto const * jarray = dynamic_cast<json::Array const *>(base_ptr.get());
		if(!jarray)
			return;
		auto & elements_ = jarray->elements();
		auto it = elements_.begin();
		auto & vector_ = *m_data;
		vector_.resize(elements_.size());
		for(auto & e : vector_)
			e.deserialize(*static_cast<json::Object const *>((*it++).get()));
	}

};

class SerializerBase
{
	SerializerBase(SerializerBase &&) = default;
	SerializerBase(SerializerBase const &) = default;
	SerializerBase & operator=(SerializerBase &&) = default;
	SerializerBase & operator=(SerializerBase const &) = default;
 public:
	SerializerBase() = default;
	virtual ~SerializerBase() = default;
};

template <typename group_, typename... Types>
class Serializer : public SerializerBase
{
	std::array<std::unique_ptr<SerialBase>,sizeof...(Types)> m_values {};

 public:
	Serializer() = default;
	Serializer(Serializer &&) = default;
	Serializer(Serializer const &) = default;
	Serializer & operator=(Serializer &&) = default;
	Serializer & operator=(Serializer const &) = default;
	Serializer(Pair<json::string_t,Types &>... values)
		: m_values { std::unique_ptr<SerialBase>(new Serial<Types,group_>(std::move(values.first), values.second))... }
	{}

	void register_(Pair<json::string_t,Types &>... values)
	{
		m_values = { std::unique_ptr<SerialBase>(new Serial<Types,group_>(std::move(values.first), values.second))... };
	}

	json::Object
	serialize() const
	{
		json::Object jobject;
		for(auto const & v : m_values)
		{
			if(v)
				jobject.set(v->id(), v->serialize());
		}
		return std::move(jobject);
	}

	void
	deserialize(json::Object const & base_ptr)
	{
		for(auto const & v : m_values)
		{
			if(v)
			{
				auto const & id_ = v->id();
				auto const * entry = base_ptr.get(id_);
				if(entry)
					v->deserialize(*entry);
			}
		}
	}

};


} // namespace json

#endif // SERIALIZER_HPP