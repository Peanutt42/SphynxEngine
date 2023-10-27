#pragma once

#include "std.hpp"
#include "Logging/Logging.hpp"
#include "Debug/CrashHandler.hpp"

namespace Sphynx {
    template<typename T, typename E = std::string_view>
    struct Result {
        using type = std::remove_const_t<T>;

		constexpr Result() = default;
		constexpr Result(const Result&) = default;
		Result& operator=(const Result&) = default;
		constexpr Result(Result&&) noexcept = default;
		Result& operator=(Result&&) noexcept = default;
		constexpr ~Result() = default;

		constexpr Result(const T& value) : m_Value(value) {}
		Result& operator=(const T& value) {
			m_Value = value;
			return *this;
		}
		constexpr Result(T&& value) noexcept : m_Value(std::forward<T>(value)) {}
		Result& operator=(T&& value) {
			m_Value = std::forward<T>(value);
			return *this;
		}

		bool is_ok() const { return std::holds_alternative<T>(m_Value); }
		bool is_error() const { return std::holds_alternative<E>(m_Value); }

		operator bool() const { return is_ok(); }

		template<typename E2, typename = std::enable_if_t<!std::is_same_v<E, E2>>>
		operator Result<T, E2>() const {
			if (is_ok())
				return Result<T, std::string>(std::get<T>(m_Value));
			else {
				Result<T, E2> result;
				result.set_error(E2(std::get<E>(m_Value).data()));
				return result;
			}
		}

		template<typename... Args>
		const T& expect(fmt::format_string<Args...> msg, Args&&... args) const {
			SE_ASSERT(is_ok(), "{}: {}", fmt::format(msg, std::forward<Args>(args)...), std::get<E>(m_Value));
			return std::get<T>(m_Value);
		}
		template<typename... Args>
		T& expect(fmt::format_string<Args...> msg, Args&&... args) {
			SE_ASSERT(is_ok(), "{}: {}", fmt::format(msg, std::forward<Args>(args)...), std::get<E>(m_Value));
			return std::get<T>(m_Value);
		}

		const T& unwrap() const {
			SE_ASSERT(is_ok(), "Tried to unwrap a result with error: {}", std::get<E>(m_Value));
			return std::get<T>(m_Value);
		}
		const T& operator*() const { return unwrap(); }
		T& unwrap() {
			SE_ASSERT(is_ok(), "Tried to unwrap a result with error: {}", std::get<E>(m_Value));
			return std::get<T>(m_Value);
		}
		T& operator*() { return unwrap(); }

		const E& get_error() const {
			if (is_error())
				return std::get<E>(m_Value);
			throw std::bad_variant_access();
		}

		void set_error(const E& error) {
			m_Value = error;
		}

    private:
		std::variant<T, E> m_Value;
    };


	template<typename T>
	constexpr inline static Result<T> Error(std::string_view error) {
		Result<T> result;
		result.set_error(error);
		return result;
	}

	template<typename T, typename Arg1, typename... ArgRest>
	[[nodiscard]] constexpr inline static Result<T, std::string> Error(fmt::format_string<Arg1, ArgRest...> msg, Arg1&& arg1, ArgRest&&... argRest) {
		Result<T, std::string> result;
		result.set_error(fmt::format(msg, std::forward<Arg1>(arg1), std::forward<ArgRest>(argRest)...));
		return result;
	}
}