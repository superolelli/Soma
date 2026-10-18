#pragma once
#include <string>
#include <utility>
#include <variant>

namespace soma::gameplay
{
    /// Carries either a successful value or a user-facing validation error without exceptions.
    template <typename T>
    class Result
    {
      public:
        static Result success(T value)
        {
            return Result(std::move(value));
        }

        static Result failure(std::string error)
        {
            return Result(std::move(error));
        }

        [[nodiscard]] bool hasValue() const noexcept
        {
            return std::holds_alternative<T>(storage_);
        }

        [[nodiscard]] const T& value() const
        {
            return std::get<T>(storage_);
        }

        [[nodiscard]] const std::string& error() const
        {
            return std::get<std::string>(storage_);
        }

      private:
        explicit Result(T value)
            : storage_(std::move(value))
        {
        }

        explicit Result(std::string error)
            : storage_(std::move(error))
        {
        }

        std::variant<T, std::string> storage_;
    };
} // namespace soma::gameplay
