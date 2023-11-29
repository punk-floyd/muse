/**
 * @file    compare_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Three-way comparison operator support
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_compare__included
#define sys_compare__included

#include <_core_.h>

_SYS_BEGIN_NS

class strong_ordering;
class weak_ordering;

namespace imp {
    using cmp_val_t = signed char;
}

class partial_ordering
{
    imp::cmp_val_t _value;

    // Constructed via the compiler
    constexpr explicit partial_ordering(imp::cmp_val_t value) noexcept
        : _value(value) {}

    /// Returns our value which will be one of -1, 0, or 1
    constexpr imp::cmp_val_t value() const noexcept
        { return _value; }

    friend class strong_ordering;
    friend class weak_ordering;

public:
    static const partial_ordering less;
    static const partial_ordering equivalent;
    static const partial_ordering greater;
    static const partial_ordering unordered;

    friend constexpr bool operator==(partial_ordering v, [[maybe_unused]] int u) noexcept
        { return v.value() == 0; }
    friend constexpr bool operator==(partial_ordering v, partial_ordering u) noexcept
        { return v.value() == u.value(); }

    friend constexpr bool operator<(partial_ordering v, [[maybe_unused]] int u) noexcept
        { return v.value() == -1; }
    friend constexpr bool operator<([[maybe_unused]] int u, partial_ordering v) noexcept
        { return v.value() ==  1; }

    friend constexpr bool operator<=(partial_ordering v, [[maybe_unused]] int u) noexcept
        { return (v.value() == -1) || (v.value() == 0); }
    friend constexpr bool operator<=([[maybe_unused]] int u, partial_ordering v) noexcept
        { return (v.value() ==  1) || (v.value() == 0); }

    friend constexpr bool operator>(partial_ordering v, [[maybe_unused]] int u) noexcept
        { return v.value() ==  1; }
    friend constexpr bool operator>([[maybe_unused]] int u, partial_ordering v) noexcept
        { return v.value() == -1; }

    friend constexpr bool operator>=(partial_ordering v, [[maybe_unused]] int u) noexcept
        { return (v.value() ==  1) || (v.value() == 0); }
    friend constexpr bool operator>=([[maybe_unused]] int u, partial_ordering v) noexcept
        { return (v.value() == -1) || (v.value() == 0); }

    friend constexpr partial_ordering operator<=>(partial_ordering v, [[maybe_unused]] int u) noexcept
        { return v; }
    friend constexpr partial_ordering operator<=>([[maybe_unused]] int u, partial_ordering v) noexcept
    {
        switch (v.value()) {
        case -1: return partial_ordering(1);
        case  1: return partial_ordering(-1);
        default: return v;
        }
    }
};

inline constexpr partial_ordering partial_ordering::less{-1};
inline constexpr partial_ordering partial_ordering::equivalent{0};
inline constexpr partial_ordering partial_ordering::greater{1};
inline constexpr partial_ordering partial_ordering::unordered{2};

struct weak_ordering
{
    imp::cmp_val_t _value;

    /// Constructed via the compiler
    constexpr explicit weak_ordering(imp::cmp_val_t value) noexcept
        : _value(value) {}

    /// Returns our value which will be one of -1, 0, or 1
    constexpr imp::cmp_val_t value() const noexcept
        { return _value; }

    friend class strong_ordering;

public:
    static const weak_ordering less;
    static const weak_ordering equivalent;
    static const weak_ordering greater;

    friend constexpr bool operator==(weak_ordering v, [[maybe_unused]] int u) noexcept
        { return v.value() == 0; }
    friend constexpr bool operator==(weak_ordering v, weak_ordering u) noexcept
        { return v.value() == u.value(); }

    friend constexpr bool operator<(weak_ordering v, [[maybe_unused]] int u) noexcept
        { return v.value() == -1; }
    friend constexpr bool operator<([[maybe_unused]] int u, weak_ordering v) noexcept
        { return v.value() ==  1; }

    friend constexpr bool operator<=(weak_ordering v, [[maybe_unused]] int u) noexcept
        { return v.value() !=  1; }
    friend constexpr bool operator<=([[maybe_unused]] int u, weak_ordering v) noexcept
        { return v.value() != -1; }

    friend constexpr bool operator>(weak_ordering v, [[maybe_unused]] int u) noexcept
        { return v.value() ==  1; }
    friend constexpr bool operator>([[maybe_unused]] int u, weak_ordering v) noexcept
        { return v.value() == -1; }

    friend constexpr bool operator>=(weak_ordering v, [[maybe_unused]] int u) noexcept
        { return v.value() != -1; }
    friend constexpr bool operator>=([[maybe_unused]] int u, weak_ordering v) noexcept
        { return v.value() !=  1; }

    friend constexpr weak_ordering operator<=>(weak_ordering v, [[maybe_unused]] int u) noexcept
        { return v; }
    friend constexpr weak_ordering operator<=>([[maybe_unused]] int u, weak_ordering v) noexcept
        { return weak_ordering(-v.value()); }

    constexpr operator partial_ordering() const noexcept
        { return partial_ordering(value()); }
};

inline constexpr weak_ordering weak_ordering::less{-1};
inline constexpr weak_ordering weak_ordering::equivalent{0};
inline constexpr weak_ordering weak_ordering::greater{1};

struct strong_ordering
{
    imp::cmp_val_t _value;

    /// Constructed via the compiler
    constexpr explicit strong_ordering(imp::cmp_val_t value) noexcept
        : _value(value) {}

    /// Returns our value which will be one of -1, 0, or 1
    constexpr imp::cmp_val_t value() const noexcept
        { return _value; }

public:
    static const strong_ordering less;
    static const strong_ordering equal;
    static const strong_ordering equivalent;
    static const strong_ordering greater;

    friend constexpr bool operator==(strong_ordering v, [[maybe_unused]] int u) noexcept
        { return v.value() == 0; }
    friend constexpr bool operator==(strong_ordering v, strong_ordering u) noexcept
        { return v.value() == u.value(); }

    friend constexpr bool operator<(strong_ordering v, [[maybe_unused]] int u) noexcept
        { return v.value() == -1; }
    friend constexpr bool operator<([[maybe_unused]] int u, strong_ordering v) noexcept
        { return v.value() ==  1; }

    friend constexpr bool operator<=(strong_ordering v, [[maybe_unused]] int u) noexcept
        { return v.value() !=  1; }
    friend constexpr bool operator<=([[maybe_unused]] int u, strong_ordering v) noexcept
        { return v.value() != -1; }

    friend constexpr bool operator>(strong_ordering v, [[maybe_unused]] int u) noexcept
        { return v.value() ==  1; }
    friend constexpr bool operator>([[maybe_unused]] int u, strong_ordering v) noexcept
        { return v.value() == -1; }

    friend constexpr bool operator>=(strong_ordering v, [[maybe_unused]] int u) noexcept
        { return v.value() != -1; }
    friend constexpr bool operator>=([[maybe_unused]] int u, strong_ordering v) noexcept
        { return v.value() !=  1; }

    friend constexpr strong_ordering operator<=>(strong_ordering v, [[maybe_unused]] int u) noexcept
        { return v; }
    friend constexpr strong_ordering operator<=>([[maybe_unused]] int u, strong_ordering v) noexcept
        { return strong_ordering(-v.value()); }

    constexpr operator partial_ordering() const noexcept
        { return partial_ordering(value()); }
    constexpr operator weak_ordering() const noexcept
        { return weak_ordering(value()); }
};

inline constexpr strong_ordering strong_ordering::less{-1};
inline constexpr strong_ordering strong_ordering::equal{0};
inline constexpr strong_ordering strong_ordering::equivalent{0};
inline constexpr strong_ordering strong_ordering::greater{1};

constexpr bool is_eq(sys::partial_ordering cmp) noexcept
    { return cmp == 0; }
constexpr bool is_neq(sys::partial_ordering cmp) noexcept
    { return cmp != 0; }
constexpr bool is_lt(sys::partial_ordering cmp) noexcept
    { return cmp < 0; }
constexpr bool is_lte(sys::partial_ordering cmp) noexcept
    { return cmp <= 0; }
constexpr bool is_gt(sys::partial_ordering cmp) noexcept
    { return cmp > 0; }
constexpr bool is_gte(sys::partial_ordering cmp) noexcept
    { return cmp >= 0; }

template <class T, class U = T>
struct compare_three_way_result{};

template <class T, class U>
    requires requires { declval<const remove_reference_t<T>&>() <=> declval<const remove_reference_t<U>&>(); }
struct compare_three_way_result<T,U> {
    using type = decltype(
        declval<const remove_reference_t<T>&>() <=> declval<const remove_reference_t<U>&>());
};

template <class T, class U = T>
using compare_three_way_result_t = compare_three_way_result<T, U>::type;

namespace imp {
    // Implementation for common_comparison_category_t. Do an accounting of
    // all strong, weak, and partial types. Anything else is invalid.
    template <class... Types>
    constexpr size_t count_strong  = ((is_same_v<Types, strong_ordering>  ? 1 : 0) + ...);
    template <class... Types>
    constexpr size_t count_partial = ((is_same_v<Types, partial_ordering> ? 1 : 0) + ...);
    template <class... Types>
    constexpr size_t count_weak    = ((is_same_v<Types, weak_ordering>    ? 1 : 0) + ...);
    template <class... Types>
    constexpr size_t count_void =
        sizeof...(Types) - count_strong<Types...> - count_partial<Types...> - count_weak<Types...>;

    template <class... Types>
    struct common_comparison_category {
        using type = conditional_t<count_void<Types...>, void,
            conditional_t<count_partial<Types...>, partial_ordering,
                conditional_t<count_weak<Types...>, weak_ordering,
                    strong_ordering>>>;
    };
    template <>
    struct common_comparison_category<> {
        using type = strong_ordering;
    };
}

template <class... Types>
using common_comparison_category_t = imp::common_comparison_category<Types...>::type;

namespace imp {
    template <class T, class Cat>
    concept three_way_compares_as = same_as<common_comparison_category_t<T, Cat>, Cat>;
}

template <class T, class Cat = partial_ordering>
concept three_way_comparable =
    equality_comparable<T, T> &&
    partially_ordered_with<T, T> &&
    requires (const remove_reference_t<T>& a, const remove_reference_t<T>& b) {
        { a <=> b } -> imp::three_way_compares_as<Cat>;
    };

namespace imp {
    template <class T, class U,
        class C = common_reference_t<const T&, const U&>>
    concept ComparisonCommonTypeWithImpl =
        same_as<common_reference_t<const T&, const U&>,
                common_reference_t<const T&&, const U&&>> &&
        requires {
            requires convertible_to<const T&, const C&> ||
                     convertible_to<T,        const T&>;
            requires convertible_to<const U&, const C&> ||
                     convertible_to<U,        const C&>;
        };

    template <class T, class U>
    concept ComparisonCommonTypeWith =
        ComparisonCommonTypeWithImpl<remove_cvref_t<T>, remove_cvref_t<U>>;
}

template <class T, class U, class Cat = partial_ordering>
concept three_way_comparable_with =
    three_way_comparable<T, Cat> &&
    three_way_comparable<U, Cat> &&
    imp::ComparisonCommonTypeWith<T, U> &&
    three_way_comparable<common_reference_t<
        const remove_reference_t<T>&, const remove_reference_t<U>&>, Cat> &&
    equality_comparable<T, U> &&
    partially_ordered_with<T, U> &&
    requires (const remove_reference_t<T>& t, const remove_reference_t<U>& u) {
        { t <=> u } -> imp::three_way_compares_as<Cat>;
        { u <=> t } -> imp::three_way_compares_as<Cat>;
    };

_SYS_END_NS

// The compiler is assuming that we have std::strong_ordering and
// std::partial_ordering. We will just sidle in with our own version.
namespace std {
    using strong_ordering  = sys::strong_ordering;
    using partial_ordering = sys::partial_ordering;
    using weak_ordering    = sys::weak_ordering;
}

#endif // ifndef sys_compare__included
