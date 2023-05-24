/**
 * @file    variant_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Declares and defines variant
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_variant__included
#define sys_variant__included

#include <_core_.h>
#include <initializer_list_.h>
#include <type_traits_.h>
#include <type_list_.h>
#include <concepts_.h>
#include <utility_.h>
#include <limits_.h>
#include <new_.h>

_SYS_BEGIN_NS

namespace imp {
    // Is type T okay for a variant?
    template <class T>
    constexpr bool is_variant_safe = !is_reference_v<T> && !is_array_v<T> && !is_void_v<T>;
    // Are all types okay for a variant?
    template <class... Types>
    constexpr bool are_variant_safe = (is_variant_safe<Types> && ...);
}

/// Unit type intended for use as a well-behaved empty alternative in variant
struct monostate {
    constexpr auto operator<=>(const monostate&) const noexcept
        { return strong_ordering::equal; }
};

/// Variants may not hold references, arrays, or type void
template <class... Types>
concept variant_safe_types = imp::are_variant_safe<Types...>;

/// Obtains the size of the variant's list of alternatives at compile time
template <class... Types>
inline constexpr size_t variant_size_v = tl::size<type_list<Types...>>;

/// Obtains the type of the alternative specified by its index, at compile time
template <size_t N, class... Types>
using variant_alternative_t = tl::type_at<type_list<Types...>, N>;

/// Return value for variant<...>::get_index when variant has no value
inline constexpr size_t variant_npos = size_t(-1);

template <class... Types>
struct variant_traits {
    // Variant is default constructible if all its types are default constructible, etc...
    static constexpr bool is_def_constructible = (is_default_constructible_v<Types> && ...);
    static constexpr bool is_cpy_constructible = (is_copy_constructible_v<Types>    && ...);
    static constexpr bool is_mov_constructible = (is_move_constructible_v<Types>    && ...);
    static constexpr bool is_nothrow_mov_constructible = (is_nothrow_move_constructible_v<Types> && ...);
    static constexpr bool is_cpy_assignable    = (is_copy_assignable_v<Types>       && ...);
    static constexpr bool is_mov_assignable    = (is_move_assignable_v<Types>       && ...);
    static constexpr bool is_swappable         = (is_swappable<Types>               && ...);
    static constexpr bool is_nothrow_swappable = (is_nothrow_swappable<Types>       && ...);
};

// Variant storage; uses recursive union
namespace imp {

    /// var_store_work: Recursive storage for formatter objects
    template <class... Alts>
    union var_store_work;      // Primary template

    template <class Alt, class... Alts>
    union var_store_work<Alt, Alts...> {
        Alt                         alt;
        var_store_work<Alts...>     store;
    };
    template <class Alt>
    union var_store_work<Alt> {
        Alt                         alt;
    };

    template <class... Alts>
    union var_store
    {
        constexpr var_store()  { this->store.alt = decltype(store.alt){}; }
        constexpr ~var_store() {}

        var_store_work<Alts...> store;
    };

    /// Get reference to the alternate with index Idx
    template <size_t Idx, size_t IdxAt, class Alt, class... Alts>
    constexpr inline decltype(auto) get_alt(var_store_work<Alt, Alts...>& vs) noexcept
    {
        // Note the() around the return value here. This ensures that we
        // return a reference and not a copy.

        if constexpr (Idx == IdxAt)
            return (vs.alt);
        else if constexpr (sizeof...(Alts))
            return (get_alt<Idx, IdxAt+1, Alts...>(vs.store));
        else
            static_assert(sys::dependent_false_v<Alt>, "Bad index");
    }

    /// Get reference to the alternate with index Idx
    template <size_t Idx, size_t IdxAt, class Alt, class... Alts>
    constexpr inline decltype(auto) alt_clvref(const var_store_work<Alt, Alts...>& vs) noexcept
    {
        // Note the() around the return value here. This ensures that we
        // return a reference and not a copy.

        if constexpr (Idx == IdxAt)
            return (vs.alt);
        else if constexpr (sizeof...(Alts))
            return (alt_clvref<Idx, IdxAt+1, Alts...>(vs.store));
        else
            static_assert(sys::dependent_false_v<Alt>, "Bad index");
    }

} // End namespace imp

template <class... Types>
    requires non_empty_pack<Types...> && variant_safe_types<Types...>
class variant
{
public:

    using my_types    = type_list<Types...>;
    using my_traits   = variant_traits<Types...>;
    using first_type  = tl::type_at<type_list<Types...>, 0>;
    using my_store    = imp::var_store<Types...>;

    // -- Construction

    /// Default construction: value initialize first type
    constexpr variant() noexcept(is_nothrow_default_constructible_v<first_type>)
        requires is_default_constructible_v<first_type>
    {
        construct_idx<0>();
    }

    /// Copy constructor
    constexpr variant(const variant& other)
        requires variant_traits<Types...>::is_cpy_constructible
    {
        if (!other.is_valueless())
            copy_cx_work(other);
    }

    /// Move constructor
    constexpr variant(variant&& other) noexcept(my_traits::is_nothrow_mov_constructible)
        requires variant_traits<Types...>::is_mov_constructible
    {
        if (!other.is_valueless())
            move_cx_work(move(other));
    }

    /// Construct with specified alternative type
    template <class T, class... Args>
        requires (tl::unique<my_types, T> && is_constructible_v<T, Args...>)
    constexpr explicit variant(in_place_type_t<T>, Args&&... args)
    {
        inplace_type_cx_work<0, T, Args...>(forward<Args>(args)...);
    }

    /// Construct with specified alternative index
    template <size_t Idx, class... Args>
        requires ((Idx < tl::size<my_types>) && is_constructible_v<tl::type_at<my_types, Idx>, Args...>)
    constexpr explicit variant(in_place_index_t<Idx>, Args&&... args)
    {
        construct_idx<Idx>(forward<Args>(args)...);
    }

    /// Construct with specified alternative type and an initializer list
    template <class T, class U, class... Args>
        requires (tl::unique<my_types, T> && is_constructible_v<T, initializer_list<U>&, Args...>)
    constexpr explicit variant(in_place_type_t<T>, initializer_list<U> il, Args&&... args)
    {
        inplace_type_il_cx_work<0, T, U, Args...>(il, forward<Args>(args)...);
    }

    /// Construct with specified alternative index and an initializer list
    template <size_t Idx, class U, class... Args>
        requires ((Idx < tl::size<my_types>) && is_constructible_v<tl::type_at<my_types, Idx>, initializer_list<U>&, Args...>)
    constexpr explicit variant(in_place_index_t<Idx>, initializer_list<U> il, Args&&... args)
    {
        construct_idx<Idx>(il, forward<Args>(args)...);
    }

    // /// Converting constructor : TODO
    // template <class T>
    // variant(T&& t) //noexcept(is_nothrow_constructible_v<TheMatchedType, T>)
    // { }

    constexpr ~variant()
    {
        release();
    }

    // -- State

    /// Returns true if object currently holds the specified type T
    template <class T> requires (tl::unique<my_types, T>)
    constexpr bool holds_alternative() const noexcept
    {
        return get_index() == tl::find_first<my_types, T>;
    }

    /// Returns the zero-based index of the alternative held by the variant
    constexpr size_t get_index() const noexcept
        { return _idx; }

    /// Checks if the variant is in the invalid state
    constexpr bool is_valueless() const noexcept
        { return get_index() == variant_npos; }

    // -- Accessors

    /// Access variant value by index; throws error_variant_access on error
    template <size_t I> requires (I < tl::size<my_types>)
    constexpr variant_alternative_t<I, Types...>& get()
    {
        return (get_index() == I) ?
            get_alt<I>(_val) : throw error_variant_access();
    }
    /// Access variant value by index; throws error_variant_access on error
    template <size_t I> requires (I < tl::size<my_types>)
    constexpr const variant_alternative_t<I, Types...>& get() const
    {
        return (get_index() == I) ?
            get_alt<I>(_val) : throw error_variant_access();
    }

    /// Access variant value; throws error_variant_access on error
    template <class T> requires (tl::unique<my_types, T>)
    constexpr T& get()
    {
        return get<tl::find_first<my_types, T>>();
    }
    /// Access variant value; throws error_variant_access on error
    template <class T> requires (tl::unique<my_types, T>)
    constexpr const T& get() const
    {
        return get<tl::find_first<my_types, T>>();
    }

    /// Access variant value by index; returns nullptr on error
    template <size_t I> requires (I < tl::size<my_types>)
    constexpr add_pointer_t<tl::type_at<my_types, I>> get_if() noexcept
    {
        return (get_index() == I) ? &get_alt<I>(_val) : nullptr;
    }
    /// Access const variant value by index; returns nullptr on error
    template <size_t I> requires (I < tl::size<my_types>)
    constexpr add_pointer_t<const tl::type_at<my_types, I>> get_if() const noexcept
    {
        return (get_index() == I) ? &get_alt<I>(_val) : nullptr;
    }

    /// Access variant by type; returns nullptr on error
    template <class T> requires (tl::unique<my_types, T>)
    constexpr add_pointer_t<T> get_if() noexcept
    {
        constexpr auto Idx = tl::find_first<my_types, T>;
        return (get_index() == Idx) ? &get_alt<Idx>(_val) : nullptr;
    }
    /// Access const variant by type; returns nullptr on error
    template <class T> requires (tl::unique<my_types, T>)
    constexpr add_pointer_t<const T> get_if() const noexcept
    {
        constexpr auto Idx = tl::find_first<my_types, T>;
        return (get_index() == Idx) ? &get_alt<Idx>(_val) : nullptr;
    }

    // -- Assignment

    /// Copy assignment from another variant
    constexpr variant& operator=(const variant& rhs)
        requires (my_traits::is_cpy_constructible && my_traits::is_cpy_assignable)
    {
        if (rhs.is_valueless()) {
            if (!is_valueless())
                release();
            return *this;
        }

        if (rhs.get_index() == this->get_index())
            homogeneous_copy_assign_imp(rhs);
        else
            heterogeneous_copy_assign_imp(rhs);

        return *this;
    }

    /// Move assignment from another variant
    constexpr variant& operator=(variant&& rhs)
        requires (my_traits::is_mov_constructible && my_traits::is_mov_assignable)
    {
        if (rhs.is_valueless()) {
            if (!is_valueless())
                release();
            return *this;
        }

        if (rhs.get_index() == this->get_index())
            homogeneous_move_assign_imp(move(rhs));
        else
            heterogeneous_move_assign_imp(move(rhs));

        return *this;
    }

    template <size_t Idx = 0>
    constexpr void heterogeneous_move_assign_imp(variant&& rhs)
    {
        if constexpr (Idx < tl::size<my_types>) {
            if (Idx == rhs.get_index()) {
                using rhs_type = tl::type_at<my_types, Idx>;
                emplace<Idx>(move(rhs.get<Idx>()));
                return;
            }

            heterogeneous_move_assign_imp<Idx + 1>(move(rhs));
        }
    }

    template <size_t Idx = 0>
    constexpr void homogeneous_move_assign_imp(variant&& rhs)
    {
        if constexpr (Idx < tl::size<my_types>) {
            if (Idx == rhs.get_index()) {
                get_alt<Idx>(_val) = move(rhs.get<Idx>());
                return;
            }

            homogeneous_move_assign_imp<Idx + 1>(move(rhs));
        }
    }

    // -- Emplacement

    template <size_t Idx, class... Args>
        requires (Idx < tl::size<my_types>) && is_constructible_v<variant_alternative_t<Idx, Types...>, Args...>
    constexpr variant_alternative_t<Idx, Types...>& emplace(Args&&... args)
    {
        release();
        construct_idx<Idx>(forward<Args>(args)...);
        return get_alt<Idx>(_val);
    }

    template <class T, class... Args>
        requires tl::unique<my_types, T> && is_constructible_v<T, Args...>
    constexpr T& emplace(Args&&... args)
    {
        return emplace<tl::find_first<my_types, T>, Args...>(forward<Args>(args)...);
    }

    template <size_t Idx, class U, class... Args>
        requires (Idx < tl::size<my_types>) && is_constructible_v<variant_alternative_t<Idx, Types...>, initializer_list<U>&, Args...>
    constexpr variant_alternative_t<Idx, Types...>& emplace(initializer_list<U> il, Args&&... args)
    {
        release();
        construct_idx<Idx>(il, forward<Args>(args)...);
    }

    template <class T, class U, class... Args>
        requires tl::unique<my_types, T> && is_constructible_v<T, initializer_list<U>&, Args...>
    constexpr T& emplace(initializer_list<U> il, Args&&... args)
    {
        return emplace<tl::find_first<my_types, T>, U, Args...>(il, forward<Args>(args)...);
    }

    // Swap the contents of two variants
    constexpr void swap(variant& rhs) noexcept(my_traits::is_nothrow_swappable)
        requires my_traits::is_swappable
    {
        if (!is_valueless() && !rhs.is_valueless()) {
            if (get_index() == rhs.get_index())
                homogeneous_swap(rhs);
            else {
                variant tmp(sys::move(*this));
                *this = sys::move(rhs);
                rhs   = sys::move(tmp);
            }
        }
    }

    template <size_t Idx = 0>
    constexpr void homogeneous_swap(variant& rhs) noexcept(my_traits::is_nothrow_swappable)
    {
        if constexpr (Idx < tl::size<my_types>) {
            if (Idx == get_index())
                sys::swap(get_alt<Idx>(_val), rhs.get_alt<Idx>(rhs._val));
            else
                homogeneous_swap<Idx + 1>(rhs);
        }
    }

private:

    template <size_t Idx, class... Args>
    [[nodiscard]] constexpr inline
        decltype(auto) get_alt(imp::var_store<Args...>& vs) noexcept
    {
        return (imp::get_alt<Idx, 0, Args...>(vs.store));
    }

    template <size_t Idx, class... Args>
    [[nodiscard]] constexpr inline
        decltype(auto) get_alt(const imp::var_store<Args...>& vs) const noexcept
    {
        return (imp::alt_clvref<Idx, 0, Args...>(vs.store));
    }

    template <size_t Idx, class... Args>
    constexpr void construct_idx(Args&&... args)
    {
        construct_at(&get_alt<Idx>(_val), sys::forward<Args>(args)...);
        _idx = Idx;
    }

    /// Main implementation of non-valueless copy constructor
    template <size_t Idx = 0>
    constexpr inline void copy_cx_work(const variant& other)
    {
        if constexpr (Idx < tl::size<my_types>) {
            if (Idx == other.get_index())
                construct_idx<Idx>(other.get<Idx>());
            else
                copy_cx_work<Idx + 1>(other);
        }
    }

    /// Main implementation of non-valueless move constructor
    template <size_t Idx = 0>
    constexpr inline void move_cx_work(variant&& other) noexcept(my_traits::is_nothrow_mov_constructible)
    {
        if constexpr (Idx < tl::size<my_types>) {
            if (Idx == other.get_index())
                construct_idx<Idx>(move(other.get<Idx>()));
            else
                move_cx_work<Idx + 1>(move(other));
        }
    }

    /// Main implementation of inplace construction of alternative with type T
    template <size_t Idx, class T, class... Args>
    constexpr inline void inplace_type_cx_work(Args&&... args)
    {
        if constexpr (Idx < tl::size<my_types>) {
            if constexpr (is_same_v<tl::type_at<my_types, Idx>, T>)
                construct_idx<Idx>(forward<Args>(args)...);
            else
                inplace_type_cx_work<Idx + 1, T, Args...>(forward<Args>(args)...);
        }
    }

    /// Main implementation of inplace construction of alt with type T and init list
    template <size_t Idx, class T, class U, class... Args>
    constexpr inline void inplace_type_il_cx_work(initializer_list<U>& il, Args&&... args)
    {
        if constexpr (Idx < tl::size<my_types>) {
            if constexpr (is_same_v<tl::type_at<my_types, Idx>, T>)
                construct_idx<Idx>(il, forward<Args>(args)...);
            else
                inplace_type_il_cx_work<Idx + 1, T, U, Args...>(il, forward<Args>(args)...);
        }
    }

    // Assignment from another variant with a different type
    template <size_t Idx = 0>
    constexpr inline void heterogeneous_copy_assign_imp(const variant& rhs)
    {
        if constexpr (Idx < tl::size<my_types>) {
            if (Idx == rhs.get_index()) {
                using rhs_type = tl::type_at<my_types, Idx>;
                if constexpr (
                     is_nothrow_copy_constructible_v<rhs_type> ||
                    !is_nothrow_move_constructible_v<rhs_type>) {
                        emplace<Idx>(rhs.get<Idx>());
                }
                else {
                    release();
                    get_alt<Idx>(_val) = rhs.get_alt<Idx>(rhs._val);
                    _idx = Idx;
                }

                return;
            }

            heterogeneous_copy_assign_imp<Idx + 1>(rhs);
        }
    }


    // Assignment from another variant with the same type
    template <size_t Idx = 0>
    constexpr inline void homogeneous_copy_assign_imp(const variant& rhs)
    {
        if constexpr (Idx < tl::size<my_types>) {
            if (Idx == get_index())
                get_alt<Idx>(_val) = rhs.get_alt<Idx>(rhs._val);
            else
                homogeneous_copy_assign_imp<Idx + 1>(rhs);
        }
    }

    // Destroys our value
    constexpr void release() noexcept
    {
        if (!is_valueless()) {
            release_work();
            _idx = variant_npos;
        }
    }
    template <size_t Idx = 0>
    constexpr inline void release_work() noexcept
    {
        if constexpr (Idx < tl::size<my_types>) {
            if (Idx == get_index())
                destruct_at(&get_alt<Idx>(_val));
            else
                release_work<Idx + 1>();
        }
    }

    /// Recursive union storage
    my_store        _val;
    /// Index of our variant or variant_npos if valueless
    size_t          _idx{variant_npos};
};

namespace imp::var {

    template <size_t Idx, size_t TypeCount, class... Types>
    constexpr common_comparison_category_t<compare_three_way_result_t<Types>...>
        homogeneous_threeway_comp(const variant<Types...>& lhs, const variant<Types...>& rhs) noexcept
            requires (three_way_comparable<Types> && ...)
    {
        if constexpr (Idx < TypeCount) {
            if (Idx == rhs.get_index())
                return lhs.template get<Idx>() <=> rhs.template get<Idx>();
            return homogeneous_threeway_comp<Idx + 1, TypeCount>(lhs, rhs);
        }
        else throw error_variant_access();
    }

    template <size_t Idx, size_t Count, class... Types>
    constexpr bool alt_eq(const variant<Types...>& a, const variant<Types...>& b)
    {
        if constexpr (Idx < Count) {
            if (Idx == a.get_index())
                return a.template get<Idx>() == b.template get<Idx>();
            return alt_eq<Idx + 1, Count>(a, b);
        }
        else throw error_variant_access();
    }

    template <size_t Idx, size_t Count, class... Types>
    constexpr bool alt_neq(const variant<Types...>& a, const variant<Types...>& b)
    {
        if constexpr (Idx < Count) {
            if (Idx == a.get_index())
                return a.template get<Idx>() != b.template get<Idx>();
            return alt_neq<Idx + 1, Count>(a, b);
        }
        else throw error_variant_access();
    }

    template <size_t Idx, size_t Count, class... Types>
    constexpr bool alt_lt(const variant<Types...>& a, const variant<Types...>& b)
    {
        if constexpr (Idx < Count) {
            if (Idx == a.get_index())
                return a.template get<Idx>() < b.template get<Idx>();
            return alt_lt<Idx + 1, Count>(a, b);
        }
        else throw error_variant_access();
    }

    template <size_t Idx, size_t Count, class... Types>
    constexpr bool alt_lte(const variant<Types...>& a, const variant<Types...>& b)
    {
        if constexpr (Idx < Count) {
            if (Idx == a.get_index())
                return a.template get<Idx>() <= b.template get<Idx>();
            return alt_lte<Idx + 1, Count>(a, b);
        }
        else throw error_variant_access();
    }

    template <size_t Idx, size_t Count, class... Types>
    constexpr bool alt_gt(const variant<Types...>& a, const variant<Types...>& b)
    {
        if constexpr (Idx < Count) {
            if (Idx == a.get_index())
                return a.template get<Idx>() > b.template get<Idx>();
            return alt_gt<Idx + 1, Count>(a, b);
        }
        else throw error_variant_access();
    }

    template <size_t Idx, size_t Count, class... Types>
    constexpr bool alt_gte(const variant<Types...>& a, const variant<Types...>& b)
    {
        if constexpr (Idx < Count) {
            if (Idx == a.get_index())
                return a.template get<Idx>() >= b.template get<Idx>();
            return alt_gte<Idx + 1, Count>(a, b);
        }
        else throw error_variant_access();
    }
}

template <class... Types>
constexpr common_comparison_category_t<compare_three_way_result_t<Types>...>
    operator<=>(const variant<Types...>& lhs, const variant<Types...>& rhs) noexcept
        requires (three_way_comparable<Types> && ...)
{
    if (lhs.is_valueless() && rhs.is_valueless())
        return strong_ordering::equal;
    if (lhs.is_valueless())
        return strong_ordering::less;
    if (rhs.is_valueless())
        return strong_ordering::greater;
    if (lhs.get_index() != rhs.get_index())
        return lhs.get_index() <=> rhs.get_index();

    return imp::var::homogeneous_threeway_comp<0, sizeof...(Types)>(lhs, rhs);
}

// - Not all variants support <=>, so implement fallbacks

template <class... Types>
constexpr bool operator==(const variant<Types...>& a, const variant<Types...>& b)
{
    if (a.get_index() != b.get_index()) return false;
    if (a.is_valueless())               return true;
    return imp::var::alt_eq<0, sizeof...(Types)>(a, b);
}

template <class... Types>
constexpr bool operator!=(const variant<Types...>& a, const variant<Types...>& b)
{
    if (a.get_index() != b.get_index()) return true;
    if (a.is_valueless())               return false;
    return imp::var::alt_neq<0, sizeof...(Types)>(a, b);
}

template <class... Types>
constexpr bool operator<(const variant<Types...>& a, const variant<Types...>& b)
{
    if (b.is_valueless())              return false;
    if (a.is_valueless())              return true;
    if (a.get_index() < b.get_index()) return true;
    if (a.get_index() > b.get_index()) return false;
    return imp::var::alt_lt<0, sizeof...(Types)>(a, b);
}

template <class... Types>
constexpr bool operator>(const variant<Types...>& a, const variant<Types...>& b)
{
    if (a.is_valueless())              return false;
    if (b.is_valueless())              return true;
    if (a.get_index() > b.get_index()) return true;
    if (a.get_index() < b.get_index()) return false;
    return imp::var::alt_gt<0, sizeof...(Types)>(a, b);
}

template <class... Types>
constexpr bool operator<=(const variant<Types...>& a, const variant<Types...>& b)
{
    if (a.is_valueless())              return true;
    if (b.is_valueless())              return false;
    if (a.get_index() < b.get_index()) return true;
    if (a.get_index() > b.get_index()) return false;
    return imp::var::alt_lte<0, sizeof...(Types)>(a, b);
}

template <class... Types>
constexpr bool operator>=(const variant<Types...>& a, const variant<Types...>& b)
{
    if (b.is_valueless())              return true;
    if (a.is_valueless())              return false;
    if (a.get_index() > b.get_index()) return true;
    if (a.get_index() < b.get_index()) return false;
    return imp::var::alt_gte<0, sizeof...(Types)>(a, b);
}

_SYS_END_NS

#endif // ifndef sys_variant__included
