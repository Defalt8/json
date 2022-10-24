#pragma once
#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <ctime>

namespace benchmark {

namespace c {
#ifdef _MSC_VER
	using timespec_t = ::timespec;
	static inline timespec_t 
	get_time_since_epoch() noexcept
	{
		timespec_t ts {};
		::timespec_get(&ts, TIME_UTC);
		return std::move(ts);
	}

	static inline int
	get_local_hour_offset() noexcept
	{
		::time_t time_ = 0;
		::tm tm_ {};
		::localtime_s(&tm_, &time_);
		return int(tm_.tm_hour);
	}

	static inline ::tm
	get_local_time() noexcept
	{
		::time_t time_ = 0;
		::time(&time_);
		::tm tm_ {};
		::localtime_s(&tm_, &time_);
		return tm_;
	}

	static inline ::tm
	get_gm_time() noexcept
	{
		::time_t time_ = 0;
		::time(&time_);
		::tm tm_ {};
		::gmtime_s(&tm_, &time_);
		return tm_;
	}
#else
	using timespec_t = ::timespec;
	static inline timespec_t 
	get_time_since_epoch() noexcept
	{ 
		timespec_t ts {};
		::clock_gettime(CLOCK_REALTIME, &ts); 
		return axl::move(ts); 
	}

	static inline int
	get_local_hour_offset() noexcept
	{
		::time_t time_ = 0;
		::tm * tm_ = ::localtime(&time_);
		return int(tm_->tm_hour);
	}

	static inline ::tm
	get_local_time() noexcept
	{
		::time_t time_ = 0;
		::time(&time_);
		::tm * tm_ = ::localtime(&time_);
		return {*tm_};
	}

	static inline ::tm
	get_gm_time() noexcept
	{
		::time_t time_ = 0;
		::time(&time_);
		::tm * tm_ = ::gmtime(&time_);
		return {*tm_};
	}
#endif
}

class Time
{
 public:
	using seconds_t = int64_t;
	using nanos_t   = int64_t;
	struct STime
	{
		seconds_t seconds      = 0;
		nanos_t   nano_seconds = 0;

		STime() = default;
		
		STime(seconds_t seconds_, nanos_t nano_seconds_)
			: seconds      { seconds_ }
			, nano_seconds { nano_seconds_ }
		{}
	
	};
	using time_t = STime;

private:
	time_t m_time {};

public:
	Time() = default;

	Time(seconds_t seconds, nanos_t nano_seconds)
		: m_time { seconds, nano_seconds }
	{}

	time_t const & time() const noexcept { return m_time; }

	template <typename T = seconds_t>
	T
	years() const noexcept
	{ 
		return T((m_time.seconds + m_time.nano_seconds / 1000000000LL) / 31557600LL); 
	}

	template <typename T = seconds_t>
	T
	fyears() const noexcept
	{ 
		return (T(m_time.seconds) + T(m_time.nano_seconds) / T(1000000000LL)) / T(31557600LL); 
	}

	template <typename T>
	T
	years(T const & mod_divisor) const noexcept
	{ 
		return T(math::mod<seconds_t>(((m_time.seconds + m_time.nano_seconds / 1000000000LL) / 31557600LL), mod_divisor)); 
	}

	template <typename T = seconds_t>
	T
	weeks() const noexcept
	{ 
		return T((m_time.seconds + m_time.nano_seconds / 1000000000LL) / 604800LL); 
	}

	template <typename T = seconds_t>
	T
	fweeks() const noexcept
	{ 
		return (T(m_time.seconds) + T(m_time.nano_seconds) / T(1000000000LL)) / T(604800LL); 
	}

	template <typename T>
	T
	weeks(T const & mod_divisor) const noexcept
	{ 
		return T(math::mod<seconds_t>(((m_time.seconds + m_time.nano_seconds / 1000000000LL) / 604800LL), mod_divisor)); 
	}

	template <typename T = seconds_t>
	T
	days() const noexcept
	{ 
		return T((m_time.seconds + m_time.nano_seconds / 1000000000LL) / 86400LL); 
	}

	template <typename T = seconds_t>
	T
	fdays() const noexcept
	{ 
		return (T(m_time.seconds) + T(m_time.nano_seconds) / T(1000000000LL)) / T(86400LL); 
	}

	template <typename T>
	T
	days(T const & mod_divisor) const noexcept
	{ 
		return T(math::mod<seconds_t>(((m_time.seconds + m_time.nano_seconds / 1000000000LL) / 86400LL), mod_divisor)); 
	}

	template <typename T = seconds_t>
	T
	hours() const noexcept
	{ 
		return T((m_time.seconds + m_time.nano_seconds / 1000000000LL) / 3600LL); 
	}

	template <typename T = seconds_t>
	T
	fhours() const noexcept
	{ 
		return (T(m_time.seconds) + T(m_time.nano_seconds) / T(1000000000LL)) / T(3600LL); 
	}

	template <typename T>
	T
	hours(T const & mod_divisor) const noexcept
	{ 
		return T(math::mod<seconds_t>(((m_time.seconds + m_time.nano_seconds / 1000000000LL) / 3600LL), mod_divisor)); 
	}

	template <typename T, typename U>
	T
	hours(T const & mod_divisor, U const & offset) const noexcept
	{ 
		return T(math::mod<seconds_t>(((m_time.seconds + m_time.nano_seconds / 1000000000LL) / 3600LL) + offset, mod_divisor)); 
	}

	template <typename T = seconds_t>
	T
	minutes() const noexcept
	{ 
		return T((m_time.seconds + m_time.nano_seconds / 1000000000LL) / 60LL); 
	}

	template <typename T = seconds_t>
	T
	fminutes() const noexcept
	{ 
		return (T(m_time.seconds) + T(m_time.nano_seconds) / T(1000000000LL)) / T(60LL); 
	}

	template <typename T>
	T
	minutes(T const & mod_divisor) const noexcept
	{ 
		return T(math::mod<seconds_t>(((m_time.seconds + m_time.nano_seconds / 1000000000LL) / 60LL), mod_divisor)); 
	}

	template <typename T = seconds_t>
	T
	seconds() const noexcept
	{ 
		return T(m_time.seconds + m_time.nano_seconds / 1000000000LL); 
	}

	template <typename T = seconds_t>
	T
	fseconds() const noexcept
	{ 
		return T(m_time.seconds) + T(m_time.nano_seconds) / T(1000000000LL); 
	}

	template <typename T>
	T
	seconds(T const & mod_divisor) const noexcept
	{ 
		return T(math::mod<seconds_t>((m_time.seconds + m_time.nano_seconds / 1000000000LL), mod_divisor)); 
	}

	template <typename T = seconds_t>
	T
	millis() const noexcept
	{ 
		return T(m_time.seconds * 1000LL + m_time.nano_seconds / 1000000LL); 
	}

	template <typename T = seconds_t>
	T
	fmillis() const noexcept
	{ 
		return T(m_time.seconds) * T(1000LL) + T(m_time.nano_seconds) / (1000000LL); 
	}

	template <typename T>
	T
	millis(T const & mod_divisor) const noexcept
	{ 
		return T(math::mod<seconds_t>((m_time.seconds * 1000LL + m_time.nano_seconds / 1000000LL), mod_divisor)); 
	}

	template <typename T = seconds_t>
	T
	micros() const noexcept
	{ 
		return T(m_time.seconds * 1000000LL + m_time.nano_seconds / 1000LL); 
	}

	template <typename T = seconds_t>
	T
	fmicros() const noexcept
	{ 
		return T(m_time.seconds) * T(1000000LL) + T(m_time.nano_seconds) / T(1000LL); 
	}

	template <typename T>
	T
	micros(T const & mod_divisor) const noexcept
	{ 
		return T(math::mod<seconds_t>((m_time.seconds * 1000000LL + m_time.nano_seconds / 1000LL), mod_divisor)); 
	}

	template <typename T = seconds_t>
	T
	nanos() const noexcept
	{ 
		return T(m_time.seconds * 1000000000LL + m_time.nano_seconds); 
	}

	template <typename T = seconds_t>
	T
	fnanos() const noexcept
	{ 
		return T(m_time.seconds) * T(1000000000LL) + T(m_time.nano_seconds); 
	}

	template <typename T>
	T
	nanos(T const & mod_divisor) const noexcept
	{ 
		return T(math::mod<seconds_t>((m_time.seconds * 1000000000LL + m_time.nano_seconds), mod_divisor)); 
	}

	Time 
	operator-(Time const & rhs) const noexcept
	{
		return (rhs.m_time.nano_seconds <= m_time.nano_seconds)
			? Time { m_time.seconds - rhs.m_time.seconds, m_time.nano_seconds - rhs.m_time.nano_seconds }
			: Time { m_time.seconds - rhs.m_time.seconds - 1, (m_time.nano_seconds + 1000000000L - rhs.m_time.nano_seconds) };
	}

	Time 
	operator+(Time const & rhs) const noexcept
	{
		auto nano_sec = m_time.nano_seconds + rhs.m_time.nano_seconds;
		auto carry = nano_sec / 1000000000LL;
		return Time { m_time.seconds + rhs.m_time.seconds + carry, nano_sec % 1000000000L };
	}

	static Time
	now() noexcept
	{
		auto etime_ = c::get_time_since_epoch();
		return { seconds_t(etime_.tv_sec) + 62168472000LL, nanos_t(etime_.tv_nsec) }; 
	}

	static Time
	since_epoch() noexcept
	{
		auto etime_ = c::get_time_since_epoch();
		return { seconds_t(etime_.tv_sec), nanos_t(etime_.tv_nsec) }; 
	}

};

static void 
delay(double secs)
{
	auto checkpoint_ = Time::since_epoch();
	while((Time::since_epoch() - checkpoint_).template fseconds<double>() < secs);
}

template <typename F>
static double
single_test(F && function, bool return_nan_on_exception = true) noexcept(false)
{
	Time start_, end_;
	try {
		start_ = Time::since_epoch();
		function();
		end_ = Time::since_epoch();
	} 
	catch(std::exception const & ex)
	{
		fprintf(stderr, "Exception: %s\n", ex.what());
		if(return_nan_on_exception)
			return nanf("");
		else 
			throw -1;
	}
	catch(char const * ex)
	{
		fprintf(stderr, "Exception: %s\n", ex);
		if(return_nan_on_exception)
			return nanf("");
		else 
			throw -1;
	}
	catch(int ex)
	{
		fprintf(stderr, "Exception: %d\n", ex);
		if(return_nan_on_exception)
			return nanf("");
		else 
			throw -1;
	}
	catch(...)
	{
		fprintf(stderr, "Unhandled exception!\n");
		if(return_nan_on_exception)
			return nanf("");
		else 
			throw -1;
	}
	return (end_ - start_).template fseconds<double>();
}

template <typename F>
static double
test(F && function, size_t count_ = 1, size_t warmup_count_ = 0)
{
	try {
		for(size_t i = 0; i < warmup_count_; ++i)
			single_test(static_cast<F&&>(function), false);
		double time_ = 0.0;
		for(size_t i = 0; i < count_; ++i)
			time_ += single_test(static_cast<F&&>(function), false);
		return time_ / count_;
	}
	catch(...)
	{
		return nan("");
	}
}

template <typename F>
static void
rep_test(char const * label_, F && function, size_t count_ = 1, size_t warmup_count_ = 0)
{
	double time_ = test(static_cast<F&&>(function), count_, warmup_count_);
	if(time_ != time_)
	{
		printf("%-24s: %lf -- %lf:%lf:%lf:%lf\n", label_, time_, time_, time_, time_, time_);
		return;
	}
	else
	{
		double isec_, itrash_;
		double imil_  = modf(time_, &isec_);
		double imic_  = modf(time_ * 1e3, &itrash_);
		double inano_ = modf(time_ * 1e6, &itrash_);
		printf("%-24s: %.9lf -- %4d:%03d:%03d:%03d\n", label_, time_, int(isec_), int(imil_*1e3f), int(imic_*1e3f), int(inano_*1e3f));
	}
}

} // namespace benchmark

#endif // BENCHMARK_HPP