#pragma once
#ifndef COMMON_SERIALS_HPP
#define COMMON_SERIALS_HPP

#include "Serializer.hpp"

namespace json {

using common = struct {};

template <>
class Serial<bool,common,false,false> : public SerialBase
{
 public:
	using data_t = bool;

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
		auto const * jboolean = dynamic_cast<json::Boolean const *>(base_ptr.get());
		if(!jboolean)
			return;
		*m_data = data_t(jboolean->value());
	}
};

template <>
class Serial<char,common,false,false> : public SerialBase
{
 public:
	using data_t = char;

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
		char buffer[] = { *m_data, '\0' };
		return json::make_base_ptr(buffer);
	}

	void
	deserialize(json::base_ptr_t const & base_ptr) override
	{
		if(!m_data)
			return;
		auto const * jstring = dynamic_cast<json::String const *>(base_ptr.get());
		if(!jstring)
			return;
		auto const & string = jstring->value();
		*m_data = string.size() > 0 ? string[0] : '\0';
	}
};

template <>
class Serial<int8_t,common,false,false> : public SerialBase
{
 public:
	using data_t = int8_t;

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
		auto const * jinteger = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jinteger)
			return;
		*m_data = jinteger->value().i8();
	}
};

template <>
class Serial<uint8_t,common,false,false> : public SerialBase
{
 public:
	using data_t = uint8_t;

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
		auto const * jinteger = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jinteger)
			return;
		*m_data = jinteger->value().u8();
	}
};

template <>
class Serial<int16_t,common,false,false> : public SerialBase
{
 public:
	using data_t = int16_t;

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
		auto const * jinteger = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jinteger)
			return;
		*m_data = jinteger->value().i16();
	}
};

template <>
class Serial<uint16_t,common,false,false> : public SerialBase
{
 public:
	using data_t = uint16_t;

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
		auto const * jinteger = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jinteger)
			return;
		*m_data = jinteger->value().u16();
	}
};

template <>
class Serial<int32_t,common,false,false> : public SerialBase
{
 public:
	using data_t = int32_t;

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
		auto const * jinteger = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jinteger)
			return;
		*m_data = jinteger->value().i32();
	}
};

template <>
class Serial<uint32_t,common,false,false> : public SerialBase
{
 public:
	using data_t = uint32_t;

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
		auto const * jinteger = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jinteger)
			return;
		*m_data = jinteger->value().u32();
	}
};

template <>
class Serial<int64_t,common,false,false> : public SerialBase
{
 public:
	using data_t = int64_t;

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
		auto const * jinteger = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jinteger)
			return;
		*m_data = jinteger->value().i64();
	}
};

template <>
class Serial<uint64_t,common,false,false> : public SerialBase
{
 public:
	using data_t = uint64_t;

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
		auto const * jinteger = dynamic_cast<json::Integer const *>(base_ptr.get());
		if(!jinteger)
			return;
		*m_data = jinteger->value().u64();
	}
};

template <>
class Serial<float,common,false,false> : public SerialBase
{
 public:
	using data_t = float;

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
		auto const * base = base_ptr.get();
		switch(base->type())
		{
			case Type::Integer:
			{
				auto const * jinteger = static_cast<json::Integer const *>(base);
				*m_data = data_t(jinteger->value().i64());
				break;
			}
			case Type::Number:
			{
				auto const * jnumber = dynamic_cast<json::Number const *>(base);
				*m_data = data_t(jnumber->value());
				break;
			}
			default:
				break;
		}
	}

};

template <>
class Serial<double,common,false,false> : public SerialBase
{
 public:
	using data_t = double;

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
		auto const * base = base_ptr.get();
		switch(base->type())
		{
			case Type::Integer:
			{
				auto const * jinteger = static_cast<json::Integer const *>(base);
				*m_data = data_t(jinteger->value().i64());
				break;
			}
			case Type::Number:
			{
				auto const * jnumber = dynamic_cast<json::Number const *>(base);
				*m_data = data_t(jnumber->value());
				break;
			}
			default:
				break;
		}
	}

};

template <>
class Serial<long double,common,false,false> : public SerialBase
{
 public:
	using data_t = long double;

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
		auto const * base = base_ptr.get();
		switch(base->type())
		{
			case Type::Integer:
			{
				auto const * jinteger = static_cast<json::Integer const *>(base);
				*m_data = data_t(jinteger->value().i64());
				break;
			}
			case Type::Number:
			{
				auto const * jnumber = dynamic_cast<json::Number const *>(base);
				*m_data = data_t(jnumber->value());
				break;
			}
			default:
				break;
		}
	}

};

template <>
class Serial<json::string_t,common,false,false> : public SerialBase
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

} // namespace json

#endif // COMMON_SERIALS_HPP