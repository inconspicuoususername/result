module;

#include <expected>
#include <functional>
#include <source_location>

export module result;
export import :error;

namespace result {
    export template <class T>
    struct Result;

    // Maps either Result<U> or std::expected<U> both onto Result<U>
    template <class R>
    struct as_result;

    template <class U>
    struct as_result<Result<U> > {
        using type = Result<U>;
    };

    template <class U>
    struct as_result<std::expected<U, Error> > {
        using type = Result<U>;
    };

    template <class R>
    using as_result_t = as_result<std::remove_cvref_t<R> >::type;

    export template <class T>
    struct Result : std::expected<T, Error> {
        using Base = std::expected<T, Error>;
        using Base::Base;

        // Wrap a std::expected back into a Result
        Result(Base b) noexcept : Base(std::move(b)) {
        }


        //context
        template <class Self>
        Result with_ctx(this Self&& self,
                        std::source_location loc =
                            std::source_location::current()) {
            return std::forward<Self>(self).Base::transform_error(
                [&](Error e) {
                    e.context("", loc);
                    return e;
                });
        }


        template <class Self>
        Result with_ctx(this Self&& self, std::string msg,
                        std::source_location loc =
                            std::source_location::current()) {
            return std::forward<Self>(self).Base::transform_error(
                [&](Error e) {
                    e.context(std::move(msg), loc);
                    return e;
                });
        }

        template <class Self, std::invocable F>
        Result with_ctx(this Self&& self, F&& f,
                        std::source_location loc =
                            std::source_location::current()) {
            return std::forward<Self>(self).Base::transform_error(
                [&](Error e) {
                    e.context(std::forward<F>(f)(), loc);
                    return e;
                });
        }

        //replacemente of std expecteds monadic ops that returns Result instead

        template <class Self, class F>
        auto and_then(this Self&& self, F&& f) {
            using R = as_result_t<decltype(apply(std::forward<Self>(self), f))>;
            if (!self.has_value())
                return R(std::unexpect, std::forward<Self>(self).error());
            return R(apply(std::forward<Self>(self), std::forward<F>(f)));
        }

        template <class Self, class F>
        auto or_else(this Self&& self, F&& f) {
            using R = as_result_t<std::invoke_result_t<F, decltype(std::forward<
                Self>(self).error())> >;
            if (self.has_value()) {
                if constexpr (std::is_void_v<T>) return R();
                else return R(std::in_place, *std::forward<Self>(self));
            }
            return R(std::invoke(std::forward<F>(f),
                                 std::forward<Self>(self).error()));
        }

#define EXPR(AAAAAA) std::forward<Self>(self).Base::AAAAAA(std::forward<F>(f))

        template <class Self, class F>
        auto transform(this Self&& self, F&& f) {
            auto out = EXPR(transform);
            return as_result_t<decltype(out)>(std::move(out));
        }

        template <class Self, class F>
        auto transform_error(this Self&& self, F&& f) {
            auto out = EXPR(transform_error);
            return as_result_t<decltype(out)>(std::move(out));
        }

#undef EXPR

    private:
        // invoke f with the contained value (or with nothing, for Result<void>)
        template <class Self, class F>
        static decltype(auto) apply(Self&& self, F&& f) {
            if constexpr (std::is_void_v<T>)
                return std::invoke(
                    std::forward<F>(f));
            else
                return std::invoke(std::forward<F>(f),
                                   *std::forward<Self>(self));
        }
    };

    export [[gnu::cold]] inline auto fail(std::string msg,
                                          std::source_location loc =
                                              std::source_location::current()) {
        return std::unexpected(Error(std::move(msg), loc));
    }
}