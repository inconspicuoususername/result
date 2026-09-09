#pragma once

#include <expected>
#include <source_location>
#include <utility>

namespace result::detail {
    template <class R>
    decltype(auto) try_unwrap_(R& r) {
        if constexpr (std::is_void_v<typename R::value_type>) return;
        else return std::move(*r);
    }
}

// Unwrap a Result<T>, or return std::unexpected from the enclosing
// Result-returning function on failure.
//   TRY(expr)        - tag with this callsite's source_location only
//   TRY(expr, ctx)   - also attach `ctx` as a message on the frame
//
#define TRY_IMPL(expr, ctx)                                            \
    __extension__ ({                                                   \
        auto&& _r = (expr);                                            \
        if (!_r) [[unlikely]] {                                        \
            auto _e = std::move(_r.error());                           \
            _e.context(ctx, std::source_location::current());          \
            return std::unexpected(std::move(_e));                     \
        }                                                              \
        ::result::detail::try_unwrap_(_r);                             \
    })
#define TRY_1(expr)      TRY_IMPL(expr, "")
#define TRY_2(expr, ctx) TRY_IMPL(expr, ctx)
#define TRY_PICK(_1, _2, NAME, ...) NAME
#define TRY(...) TRY_PICK(__VA_ARGS__, TRY_2, TRY_1)(__VA_ARGS__)