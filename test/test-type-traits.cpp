#include <app.h>
#include <io_ostream_.h>
#include <types_.h>
#include <type_traits_.h>
#include <array_.h>

// Note: If this file compiles okay then the test passes; no need to execute

// Just a function
static int a_function() { return 0; }

// A struct with only static data is empty
struct StructWithStatic { // See is_empty; struct cannot be local
    static int x;
};
int StructWithStatic::x = 0;

// Perfect forwarding constructor makes this class convertible from all
class C_perfect_fwd_cx { // See sys::is_convertible
public: template<class T> C_perfect_fwd_cx(T&&) {}
};

class TestTypeTraits : public sys::app
{
public:

    int Run() override
    {
        // sys::is_void
        {
            static_assert(sys::is_void_v<void>);
            static_assert(sys::is_void_v<const volatile void>);
            static_assert(!sys::is_void_v<void*>);
            static_assert(!sys::is_void_v<int&>);
            static_assert(!sys::is_void_v<int&&>);
            static_assert(!sys::is_void_v<const int&>);
        }

        // sys::is_null_pointer
        {
            static_assert(sys::is_nullptr_v<sys::nullptr_t>);
            static_assert(sys::is_nullptr_v<volatile const sys::nullptr_t>);
            static_assert(sys::is_nullptr_v<decltype(nullptr)>);
        }

        // sys::is_integral
        {
            static_assert(sys::is_integral_v<bool>);
            static_assert(sys::is_integral_v<char>);
            static_assert(sys::is_integral_v<char8_t>);
            static_assert(sys::is_integral_v<char16_t>);
            static_assert(sys::is_integral_v<char32_t>);
            static_assert(sys::is_integral_v<wchar_t>);
            static_assert(sys::is_integral_v<short>);
            static_assert(sys::is_integral_v<int>);
            static_assert(sys::is_integral_v<long>);
            static_assert(sys::is_integral_v<long long>);
            static_assert(sys::is_integral_v<signed char>);
            static_assert(sys::is_integral_v<unsigned char>);
            static_assert(sys::is_integral_v<signed short>);
            static_assert(sys::is_integral_v<signed int>);
            static_assert(sys::is_integral_v<signed long>);
            static_assert(sys::is_integral_v<signed long long>);
            static_assert(sys::is_integral_v<unsigned short>);
            static_assert(sys::is_integral_v<unsigned int>);
            static_assert(sys::is_integral_v<unsigned long>);
            static_assert(sys::is_integral_v<unsigned long long>);
            static_assert(sys::is_integral_v<sys::uint32_t>);
            static_assert(sys::is_integral_v<sys::sint32_t>);
            static_assert(sys::is_integral_v<sys::uint16_t>);
            static_assert(sys::is_integral_v<sys::sint16_t>);
            static_assert(sys::is_integral_v<sys::uint8_t>);
            static_assert(sys::is_integral_v<sys::sint8_t>);
            static_assert(sys::is_integral_v<sys::uint128_t>);
            static_assert(sys::is_integral_v<sys::sint128_t>);
            static_assert(sys::is_integral_v<sys::uintptr_t>);
            static_assert(sys::is_integral_v<sys::sintptr_t>);
            static_assert(sys::is_integral_v<sys::uintmax_t>);
            static_assert(sys::is_integral_v<sys::sintmax_t>);
            static_assert(sys::is_integral_v<sys::time_t>);
            static_assert(sys::is_integral_v<sys::size_t>);
            static_assert(sys::is_integral_v<const volatile int>);
            static_assert(!sys::is_integral_v<float>);
            static_assert(!sys::is_integral_v<double>);
            static_assert(!sys::is_integral_v<long double>);
            static_assert(!sys::is_integral_v<char*>);
            static_assert(!sys::is_integral_v<int*>);
            static_assert(!sys::is_integral_v<int&>);
            static_assert(!sys::is_integral_v<int&&>);
        }

        // sys::is_floating_point
        {
            static_assert(sys::is_floating_point_v<double>);
            static_assert(sys::is_floating_point_v<float>);
            static_assert(sys::is_floating_point_v<long double>);
            static_assert(sys::is_floating_point_v<const volatile double>);
            static_assert(!sys::is_floating_point_v<int>);
            static_assert(!sys::is_floating_point_v<double*>);
            static_assert(!sys::is_floating_point_v<double&>);
            static_assert(!sys::is_floating_point_v<double&&>);
        }

        // sys::is_array
        {
            static_assert(sys::is_array_v<int[]>);
            static_assert(sys::is_array_v<int[4]>);
            static_assert(sys::is_array_v<int[4][3]>);
            static_assert(sys::is_array_v<int[][4]>);
            static_assert(!sys::is_array_v<sys::array<int, 4>>);
            static_assert(!sys::is_array_v<int*>);
        }

        // sys::is_enum
        {
            enum class EnumClassDef {};
            enum class EnumClassInt : int {};
            enum       EnumOldSchool {};
            static_assert(sys::is_enum_v<EnumClassDef>);
            static_assert(sys::is_enum_v<EnumClassInt>);
            static_assert(sys::is_enum_v<EnumOldSchool>);
            static_assert(sys::is_enum_v<const volatile EnumClassDef>);
            static_assert(!sys::is_enum_v<EnumClassDef*>);
            static_assert(!sys::is_enum_v<EnumClassDef&>);
            static_assert(!sys::is_enum_v<EnumClassDef&&>);
        }

        // sys::is_union
        {
            union  UnionA {};
            struct StructA { UnionA ua; };
            static_assert(sys::is_union_v<UnionA>);
            static_assert(sys::is_union_v<const volatile UnionA>);
            static_assert(sys::is_union_v<union IncompleteU>);
            static_assert(sys::is_union_v<decltype(StructA::ua)>);
            static_assert(!sys::is_union_v<StructA>);
            static_assert(!sys::is_union_v<UnionA*>);
            static_assert(!sys::is_union_v<UnionA&>);
            static_assert(!sys::is_union_v<UnionA&&>);
        }

        // sys::is_class
        {
            class C;
            struct S;
            union U;
            static_assert(sys::is_class_v<C>);
            static_assert(sys::is_class_v<S>);
            static_assert(sys::is_class_v<const volatile C>);
            static_assert(sys::is_class_v<class IncompleteC>);
            static_assert(sys::is_class_v<struct IncompleteS>);
            static_assert(!sys::is_class_v<U>);
            static_assert(!sys::is_class_v<C*>);
            static_assert(!sys::is_class_v<C&>);
            static_assert(!sys::is_class_v<C&&>);
            static_assert(!sys::is_class_v<int>);
        }

        // sys::is_function
        {
            struct C1 { void mem_fun(); };
            struct C2 { void operator()(){} };
            auto func_ptr = &a_function;
            auto lambda = [](){};
            static_assert(sys::is_function_v<void()>);
            static_assert(sys::is_function_v<int(char, int, double)>);
            static_assert(sys::is_function_v<decltype(a_function)>);
            static_assert(!sys::is_function_v<decltype(&C1::mem_fun)>);
            static_assert(!sys::is_function_v<C2>); // Overloads ()
            static_assert(!sys::is_function_v<decltype(func_ptr)>);
            static_assert(!sys::is_function_v<decltype(lambda)>);
        }

        // sys::is_pointer
        {
            struct S { int dat; void fun(); };
            static_assert(sys::is_pointer_v<S*>);
            static_assert(sys::is_pointer_v<void*>);
            static_assert(sys::is_pointer_v<const volatile void*>);
            static_assert(sys::is_pointer_v<int**>);
            static_assert(!sys::is_pointer_v<int[]>);
            static_assert(!sys::is_pointer_v<int&>);
            static_assert(!sys::is_pointer_v<int>);
            static_assert(!sys::is_pointer_v<sys::uintptr_t>);
            static_assert(!sys::is_pointer_v<sys::nullptr_t>);
            static_assert(!sys::is_pointer_v<decltype(&S::dat)>);
            static_assert(!sys::is_pointer_v<decltype(&S::fun)>);
        }

        // sys::is_lvalue_reference
        {
            struct S {};
            static_assert(sys::is_lvalue_reference_v<S&>);
            static_assert(sys::is_lvalue_reference_v<int&>);
            static_assert(sys::is_lvalue_reference_v<const volatile int&>);
            static_assert(!sys::is_lvalue_reference_v<int&&>);
            static_assert(!sys::is_lvalue_reference_v<int*>);
            static_assert(!sys::is_lvalue_reference_v<int>);
        }

        // sys::is_rvalue_reference
        {
            struct S {};
            static_assert(sys::is_rvalue_reference_v<S&&>);
            static_assert(sys::is_rvalue_reference_v<int&&>);
            static_assert(sys::is_rvalue_reference_v<const volatile int&&>);
            static_assert(!sys::is_rvalue_reference_v<int&>);
            static_assert(!sys::is_rvalue_reference_v<int*>);
            static_assert(!sys::is_rvalue_reference_v<int>);
        }

        // sys::is_member_object_pointer
        {
            class C { public: static void not_mem_obj(); };
            static_assert(sys::is_member_object_pointer_v<int(C::*)>);
            static_assert(!sys::is_member_object_pointer_v<int(C::*)()>);
            static_assert(!sys::is_member_object_pointer_v<decltype(a_function)>);
            static_assert(!sys::is_member_object_pointer_v<decltype(&C::not_mem_obj)>);
        }

        // sys::is_member_function_pointer
        {
            class C { public: void mem_func(); static void not_mem_func(); };
            static_assert(sys::is_member_function_pointer_v<decltype(&C::mem_func)>);
            static_assert(!sys::is_member_function_pointer_v<decltype(&C::not_mem_func)>);
        }

        // sys::is_member_pointer
        {
            class C { public: int x; void mem_func(); static void not_mem_func(); };
            static_assert(sys::is_member_pointer_v<int(C::*)>);
            static_assert(sys::is_member_pointer_v<int(C::*)()>);
            static_assert(sys::is_member_pointer_v<decltype(&C::x)>);
            static_assert(sys::is_member_pointer_v<decltype(&C::mem_func)>);
            static_assert(!sys::is_member_pointer_v<decltype(&C::not_mem_func)>);
        }

        // sys::is_fundamental
        {
            class C {};
            static_assert(sys::is_fundamental_v<int>);              // Arithmetic
            static_assert(sys::is_fundamental_v<double>);           // Arithmetic
            static_assert(sys::is_fundamental_v<void>);             // void
            static_assert(sys::is_fundamental_v<sys::nullptr_t>);   // nullptr
            static_assert(sys::is_fundamental_v<const volatile int>);
            static_assert(!sys::is_fundamental_v<int*>);
            static_assert(!sys::is_fundamental_v<int&>);
            static_assert(!sys::is_fundamental_v<int&&>);
            static_assert(!sys::is_fundamental_v<class C>);
            static_assert(!sys::is_fundamental_v<class IncompleteC>);
            static_assert(!sys::is_fundamental_v<struct IncompleteS>);
        }

        // sys::is_arithmetic
        {
            class C { public: int x; };
            static_assert(sys::is_arithmetic_v<int>);
            static_assert(sys::is_arithmetic_v<bool>);
            static_assert(sys::is_arithmetic_v<double>);
            static_assert(sys::is_arithmetic_v<const volatile int>);
            static_assert(sys::is_arithmetic_v<decltype(C::x)>);
            static_assert(!sys::is_arithmetic_v<int*>);
            static_assert(!sys::is_arithmetic_v<int&>);
            static_assert(!sys::is_arithmetic_v<int&&>);
            static_assert(!sys::is_arithmetic_v<C>);
        }

        // sys::is_scalar
        {
            enum class Ec { moop };
            class C { public: int x; };
            static_assert(sys::is_scalar_v<int>);
            static_assert(sys::is_scalar_v<double>);
            static_assert(sys::is_scalar_v<Ec>);
            static_assert(sys::is_scalar_v<decltype(Ec::moop)>);
            static_assert(sys::is_scalar_v<sys::nullptr_t>);
            static_assert(sys::is_scalar_v<int*>);
            static_assert(sys::is_scalar_v<decltype(&C::x)>);
            static_assert(sys::is_scalar_v<const volatile int>);
            static_assert(!sys::is_scalar_v<int&>);
            static_assert(!sys::is_scalar_v<int&&>);
            static_assert(!sys::is_scalar_v<C>);
            static_assert(!sys::is_scalar_v<union IncompleteU>);
            static_assert(!sys::is_scalar_v<struct IncompleteS>);
        }

        // sys::is_object - anything but a function, reference, or void
        {
            class C {};
            static_assert(sys::is_object_v<int>);
            static_assert(sys::is_object_v<C>);
            static_assert(sys::is_object_v<int*>);
            static_assert(sys::is_object_v<const volatile int>);
            static_assert(!sys::is_object_v<decltype(a_function)>);
            static_assert(!sys::is_object_v<void>);
            static_assert(!sys::is_object_v<int&>);
            static_assert(!sys::is_object_v<int&&>);
        }

        // sys::is_compound - array, function, object pointer, function pointer
        //  member object pointer, member function pointer, reference, class,
        //  union, or enumeration (all cv qualified variants)
        {
            class C { public: int x; void mem_func(); };
            enum class Ec {};
            static_assert(sys::is_compound_v<int[10]>);
            static_assert(sys::is_compound_v<decltype(a_function)>);
            static_assert(sys::is_compound_v<C*>);
            static_assert(sys::is_compound_v<decltype(&a_function)>);
            static_assert(sys::is_compound_v<decltype(&C::x)>);
            static_assert(sys::is_compound_v<decltype(&C::mem_func)>);
            static_assert(sys::is_compound_v<C&>);
            static_assert(sys::is_compound_v<C&&>);
            static_assert(sys::is_compound_v<C>);
            static_assert(sys::is_compound_v<union IncompleteU>);
            static_assert(sys::is_compound_v<Ec>);
            static_assert(sys::is_compound_v<const volatile C>);
            static_assert(!sys::is_compound_v<int>);
        }

        // sys::is_reference
        {
            class C {};
            static_assert(sys::is_reference_v<int&>);
            static_assert(sys::is_reference_v<int&&>);
            static_assert(sys::is_reference_v<int*&>);
            static_assert(sys::is_reference_v<int*&&>);
            static_assert(sys::is_reference_v<C&>);
            static_assert(sys::is_reference_v<C&&>);
            static_assert(sys::is_reference_v<C*&>);
            static_assert(sys::is_reference_v<C*&&>);
            static_assert(!sys::is_reference_v<int*>);
            static_assert(!sys::is_reference_v<sys::nullptr_t>);
        }

        // sys::is_const
        {
            class C {};
            static_assert(sys::is_const_v<const int>);
            static_assert(sys::is_const_v<const C>);
            static_assert(sys::is_const_v<C* const>);       // pointer cannot change
            static_assert(sys::is_const_v<sys::remove_reference_t<const int&>>);
            static_assert(!sys::is_const_v<int>);
            static_assert(!sys::is_const_v<const int*>);    // pointer can change
            static_assert(!sys::is_const_v<const int&>);
        }

        // sys::is_volatile
        {
            class C {};
            static_assert(sys::is_volatile_v<volatile int>);
            static_assert(sys::is_volatile_v<const volatile int>);
            static_assert(sys::is_volatile_v<volatile C>);
            static_assert(!sys::is_volatile_v<int>);
        }

        // sys::is_trivial
        {
            class T  { int x; };
            class NT { NT() {} };
            static_assert(sys::is_trivial_v<int>);
            static_assert(sys::is_trivial_v<int[]>);
            static_assert(sys::is_trivial_v<const volatile int>);
            static_assert(sys::is_trivial_v<T>);
            static_assert(sys::is_trivial_v<T[]>);
            static_assert(sys::is_trivial_v<const volatile T>);
            static_assert(!sys::is_trivial_v<NT>);
        }

        // sys::is_trivially_copyable
        {
            struct A { int m; };                // Trivially copyable
            struct B { B(const B&) {} };        // Non-trivial
            struct C { virtual void moop(); };  // Non-trivial (v-table)
            struct D {
                int m;
                D (const D&) = default;         // Trivially copyable
                D (int x) : m(x) {}
            };
            static_assert(sys::is_trivially_copyable_v<A>);
            static_assert(sys::is_trivially_copyable_v<D>);
            static_assert(!sys::is_trivially_copyable_v<B>);
            static_assert(!sys::is_trivially_copyable_v<C>);
        }

        // sys::is_standard_layout
        {
            struct a { int m; };
            class  b { int a; public: int b; };
            class  c { virtual void moop(); };
            static_assert(sys::is_standard_layout_v<a>);
            static_assert(!sys::is_standard_layout_v<b>);
            static_assert(!sys::is_standard_layout_v<c>);
        }

        // sys::is_pod
        {
            struct a { int m; };
            class  b { int a; public: int b; };
            class  c { virtual void moop(); };
            static_assert(sys::is_pod_v<a>);
            static_assert(!sys::is_pod_v<b>);
            static_assert(!sys::is_pod_v<c>);
        }

        // sys::is_empty
        {
            struct A {};
            struct B { int x; };
            struct C { virtual ~C(); };
            union  D {};
            struct E { [[no_unique_address]] D d; };
            struct F { int : 0; };
            static_assert(sys::is_empty_v<A>);
            static_assert(sys::is_empty_v<StructWithStatic>);
            static_assert(sys::is_empty_v<E>);
            static_assert(sys::is_empty_v<F>);
            static_assert(!sys::is_empty_v<D>);
            static_assert(!sys::is_empty_v<B>);
            static_assert(!sys::is_empty_v<C>);
        }

        // sys::is_polymorphic
        {
            struct A {};
            struct B { virtual void moop(); };
            struct C : B {};
            struct D { virtual ~D() = default; };
            static_assert(sys::is_polymorphic_v<B>);
            static_assert(sys::is_polymorphic_v<C>);
            static_assert(sys::is_polymorphic_v<D>);
            static_assert(!sys::is_polymorphic_v<A>);
        }

        // sys::is_abstract
        {
            struct A {};
            struct B { virtual void moop(); };
            struct C { virtual void moop() = 0;};
            struct D : C {};
            static_assert(sys::is_abstract_v<C>);
            static_assert(sys::is_abstract_v<D>);
            static_assert(!sys::is_abstract_v<A>);
            static_assert(!sys::is_abstract_v<B>);
        }

        // sys::is_final
        {
            struct A final {};
            struct B {};
            static_assert(sys::is_final_v<A>);
            static_assert(!sys::is_final_v<B>);
        }

        // sys::is_aggregate
        {
            class  A { public: int x; };
            union  B { int x; };
            class  C { int private_x; };
            struct D { D(int, double) {} };
            struct E { int x{0}; };     // ok since C++14
            struct F { virtual int moop(); };
            static_assert(sys::is_aggregate_v<int[]>);
            static_assert(sys::is_aggregate_v<A>);
            static_assert(sys::is_aggregate_v<A[]>);
            static_assert(sys::is_aggregate_v<B>);
            static_assert(sys::is_aggregate_v<E>);
            static_assert(!sys::is_aggregate_v<C>);
            static_assert(!sys::is_aggregate_v<int>);
            static_assert(!sys::is_aggregate_v<int*>);
            static_assert(!sys::is_aggregate_v<D>);
            static_assert(!sys::is_aggregate_v<F>);
        }

        // sys::is_signed
        {
            class C {};
            static_assert(sys::is_signed_v<int>);
            static_assert(sys::is_signed_v<short>);
            static_assert(sys::is_signed_v<long>);
            static_assert(sys::is_signed_v<long long>);
            static_assert(sys::is_signed_v<double>);
            static_assert(sys::is_signed_v<float>);
            static_assert(sys::is_signed_v<signed char>);
            static_assert(sys::is_signed_v<volatile const int>);
            static_assert(sys::is_signed_v<sys::ptrdiff_t>);
            static_assert(!sys::is_signed_v<unsigned>);
            static_assert(!sys::is_signed_v<char8_t>);
            static_assert(!sys::is_signed_v<char16_t>);
            static_assert(!sys::is_signed_v<char32_t>);
            static_assert(!sys::is_signed_v<sys::size_t>);
            static_assert(!sys::is_signed_v<C>);
        }

        // sys::is_unsigned
        {
            class C {};
            static_assert(sys::is_unsigned_v<unsigned int>);
            static_assert(sys::is_unsigned_v<unsigned short>);
            static_assert(sys::is_unsigned_v<unsigned long>);
            static_assert(sys::is_unsigned_v<unsigned long long>);
            static_assert(sys::is_unsigned_v<char8_t>);
            static_assert(sys::is_unsigned_v<char16_t>);
            static_assert(sys::is_unsigned_v<char32_t>);
            static_assert(sys::is_unsigned_v<sys::size_t>);
            static_assert(!sys::is_unsigned_v<sys::ptrdiff_t>);
            static_assert(!sys::is_unsigned_v<int>);
            static_assert(!sys::is_unsigned_v<double>);
            static_assert(!sys::is_unsigned_v<C>);
        }

        // sys::is_bounded_array
        {
            class C {};
            static_assert(sys::is_bounded_array_v<C[3]>);
            static_assert(sys::is_bounded_array_v<int[3]>);
            static_assert(sys::is_bounded_array_v<int[3][4]>);
            static_assert(!sys::is_bounded_array_v<int[]>);
            static_assert(!sys::is_bounded_array_v<int[][3]>);
            static_assert(!sys::is_bounded_array_v<int*>);
            static_assert(!sys::is_bounded_array_v<int>);
        }

        // sys::is_unbounded_array
        {
            class C {};
            static_assert(sys::is_unbounded_array_v<C[]>);
            static_assert(sys::is_unbounded_array_v<int[]>);
            static_assert(sys::is_unbounded_array_v<C[][4]>);
            static_assert(!sys::is_unbounded_array_v<C[3]>);
            static_assert(!sys::is_unbounded_array_v<C[3][4]>);
            static_assert(!sys::is_unbounded_array_v<int*>);
            static_assert(!sys::is_unbounded_array_v<int>);
        }

        // sys::is_constructible
        {
            struct A { A(int, char&, double&&) {} };
            static_assert(sys::is_constructible_v<A, int, char&, double&&>);
            static_assert(!sys::is_constructible_v<A, int, char&>);
            static_assert(!sys::is_constructible_v<A, int>);
            static_assert(!sys::is_constructible_v<A>);
        }

        // sys::is_trivially_constructible
        {
            class C { int x; public: C(int a) : x(a) {} };
            static_assert(sys::is_trivially_constructible_v<C, const C&>);
            static_assert(!sys::is_trivially_constructible_v<C, int>);
        }

        // sys::is_nothrow_constructible
        {
            struct A { A(int) noexcept        {} };
            struct B { B(int) noexcept(true)  {} };
            struct C { C(int)                 {} };
            struct D { D(int) noexcept(false) {} };
            static_assert(sys::is_nothrow_constructible_v<A, int>);
            static_assert(sys::is_nothrow_constructible_v<B, int>);
            static_assert(!sys::is_nothrow_constructible_v<C, int>);
            static_assert(!sys::is_nothrow_constructible_v<D, int>);
        }

        // sys:: is_default_constructible
        {
            struct A { };
            struct B { B() = delete; };
            static_assert(sys::is_default_constructible_v<int>);
            static_assert(sys::is_default_constructible_v<A>);
            static_assert(!sys::is_default_constructible_v<B>);
        }


        // sys::is_trivially_default_constructible
        {
            struct A { };
            struct B { int& x; B(int& a) : x(a) {} };
            struct C { virtual void moop(); };
            static_assert(sys::is_trivially_default_constructible_v<int>);
            static_assert(sys::is_trivially_default_constructible_v<A>);
            static_assert(!sys::is_trivially_default_constructible_v<B>);
            static_assert(!sys::is_trivially_default_constructible_v<C>);
        }

        // sys::is_nothrow_default_constructible
        {
            struct A {};
            struct B { B() noexcept = default; };
            struct C { C() noexcept(true) = default; };
            struct D { D() noexcept(false) {} };
            static_assert(sys::is_nothrow_default_constructible_v<int>);
            static_assert(sys::is_nothrow_default_constructible_v<A>);
            static_assert(sys::is_nothrow_default_constructible_v<B>);
            static_assert(sys::is_nothrow_default_constructible_v<C>);
            static_assert(!sys::is_nothrow_default_constructible_v<D>);
        }

        // sys::is_copy_constructible
        {
            struct A {};
            struct B { B(const B&) {} };
            struct C { C(const C&) = delete; };
            static_assert(sys::is_copy_constructible_v<int>);
            static_assert(sys::is_copy_constructible_v<A>);
            static_assert(sys::is_copy_constructible_v<B>);
            static_assert(!sys::is_copy_constructible_v<C>);
        }

        // sys::is_trivially_copy_constructible
        {
            struct A {};
            struct B { B(const B&) = default; };
            struct C { C(const C&) {} };
            struct D { D(const D&) = delete; };
            static_assert(sys::is_trivially_copy_constructible_v<int>);
            static_assert(sys::is_trivially_copy_constructible_v<A>);
            static_assert(sys::is_trivially_copy_constructible_v<B>);
            static_assert(!sys::is_trivially_copy_constructible_v<C>);
            static_assert(!sys::is_trivially_copy_constructible_v<D>);
        }

        // sys::is_nothrow_copy_constructible
        {
            struct A {};
            struct B { B(const B&) {} };
            struct C { C(const C&) = delete; };
            struct D { D(const D&) noexcept {} };
            struct E { E(const E&) noexcept(false) {} };
            static_assert(sys::is_nothrow_copy_constructible_v<int>);
            static_assert(sys::is_nothrow_copy_constructible_v<A>);
            static_assert(sys::is_nothrow_copy_constructible_v<D>);
            static_assert(!sys::is_nothrow_copy_constructible_v<B>);
            static_assert(!sys::is_nothrow_copy_constructible_v<E>);
        }

        // sys::is_move_constructible
        {
            struct A {};
            struct B { B(B&&) = default; };
            struct C { C(C&&) = delete; };
            static_assert(sys::is_move_constructible_v<int>);
            static_assert(sys::is_move_constructible_v<A>);
            static_assert(sys::is_move_constructible_v<B>);
            static_assert(!sys::is_move_constructible_v<C>);
        }

        // sys::is_trivially_move_constructible
        {
            struct A {};
            struct B { B(B&&) = default; };
            struct C { C(C&&) = delete; };
            struct D { D(D&&) {} };
            static_assert(sys::is_trivially_move_constructible_v<int>);
            static_assert(sys::is_trivially_move_constructible_v<A>);
            static_assert(sys::is_trivially_move_constructible_v<B>);
            static_assert(!sys::is_trivially_move_constructible_v<C>);
            static_assert(!sys::is_trivially_move_constructible_v<D>);
        }

        // sys::is_nothrow_move_constructible
        {
            struct A {};
            struct B { B(B&&) = default; };
            struct C { C(C&&) noexcept {} };
            struct D { D(D&&) = delete;  };
            struct E { E(E&&) noexcept(false) {} };
            static_assert(sys::is_nothrow_move_constructible_v<int>);
            static_assert(sys::is_nothrow_move_constructible_v<A>);
            static_assert(sys::is_nothrow_move_constructible_v<B>);
            static_assert(sys::is_nothrow_move_constructible_v<C>);
            static_assert(!sys::is_nothrow_move_constructible_v<D>);
            static_assert(!sys::is_nothrow_move_constructible_v<E>);
        }

        // sys::is_assignable
        {
            struct A {};
            struct B { B& operator=(int); };
            static_assert(sys::is_assignable_v<A, A>);
            static_assert(sys::is_assignable_v<B, int>);
            static_assert(sys::is_assignable_v<int&, int>);     // int a; a = 1; (ok)
            static_assert(!sys::is_assignable_v<int, double>);
            static_assert(!sys::is_assignable_v<B, A>);
            static_assert(!sys::is_assignable_v<int, int>);     // 1 = 1 (bad)
        }

        // sys::is_trivially_assignable
        {
            struct A {};
            struct B { int x; };
            static_assert(sys::is_trivially_assignable_v<A, A>);
            static_assert(sys::is_trivially_assignable_v<B&, B&>);
            static_assert(sys::is_trivially_assignable_v<int&, double>);
            static_assert(!sys::is_trivially_assignable_v<B, A>);
        }

        // sys::is_nothrow_assignable
        {
            struct A {};
            struct B { B& operator=(int) noexcept(true); };
            struct C { C& operator=(int) noexcept(false); };
            static_assert(sys::is_nothrow_assignable_v<int&, int>);
            static_assert(sys::is_nothrow_assignable_v<A, A>);
            static_assert(sys::is_nothrow_assignable_v<B, int>);
            static_assert(!sys::is_nothrow_assignable_v<C, int>);
        }

        // sys::is_copy_assignable
        {
            struct A {};
            struct B { B& operator=(const B&) = default; };
            struct C { C& operator=(const C&); };
            struct D { D& operator=(const D&) = delete; };
            static_assert(sys::is_copy_assignable_v<int>);
            static_assert(sys::is_copy_assignable_v<A>);
            static_assert(sys::is_copy_assignable_v<B>);
            static_assert(sys::is_copy_assignable_v<C>);
            static_assert(!sys::is_copy_assignable_v<D>);
            static_assert(!sys::is_copy_assignable_v<void>);
            static_assert(!sys::is_copy_assignable_v<int(int,char)>);
            static_assert(!sys::is_copy_assignable_v<int[]>);
        }

        // sys::is_trivially_copy_assignable
        {
            struct A {};
            struct B { B& operator=(const B&) = default; };
            struct C { C& operator=(const C&); };
            struct D { D& operator=(const D&) = delete; };
            static_assert(sys::is_trivially_copy_assignable_v<int>);
            static_assert(sys::is_trivially_copy_assignable_v<A>);
            static_assert(sys::is_trivially_copy_assignable_v<B>);
            static_assert(!sys::is_trivially_copy_assignable_v<C>);
            static_assert(!sys::is_trivially_copy_assignable_v<D>);
            static_assert(!sys::is_trivially_copy_assignable_v<void>);
        }

        // sys::is_nothrow_copy_assignable
        {
            struct A {};
            struct B { B& operator=(const B&) = default; };
            struct C { C& operator=(const C&) noexcept; };
            struct D { D& operator=(const D&) noexcept(false); };
            struct E { E& operator=(const E&) = delete; };
            static_assert(sys::is_nothrow_copy_assignable_v<int>);
            static_assert(sys::is_nothrow_copy_assignable_v<A>);
            static_assert(sys::is_nothrow_copy_assignable_v<B>);
            static_assert(sys::is_nothrow_copy_assignable_v<C>);
            static_assert(!sys::is_nothrow_copy_assignable_v<D>);
            static_assert(!sys::is_nothrow_copy_assignable_v<E>);
            static_assert(!sys::is_nothrow_copy_assignable_v<void>);
            static_assert(!sys::is_nothrow_copy_assignable_v<int[3]>);
        }

        // sys::is_move_assignable
        {
            struct A {};
            // B: No default move, but copy assignment can bind to rvalue argument
            struct B { B& operator=(const B&); };
            struct C { C& operator=(C&&) = delete; };
            static_assert(sys::is_move_assignable_v<int>);
            static_assert(sys::is_move_assignable_v<A>);
            static_assert(sys::is_move_assignable_v<B>);
            static_assert(!sys::is_move_assignable_v<C>);
            static_assert(!sys::is_move_assignable_v<decltype(void())>);
            static_assert(!sys::is_move_assignable_v<int[56]>);
        }

        // sys::is_trivially_move_assignable
        {
            struct A {};
            struct B { B& operator=(B&&) = default; };
            struct C { C& operator=(C&&); };
            struct D { D& operator=(D&&) = delete; };
            static_assert(sys::is_trivially_move_assignable_v<int>);
            static_assert(sys::is_trivially_move_assignable_v<A>);
            static_assert(sys::is_trivially_move_assignable_v<B>);
            static_assert(!sys::is_trivially_move_assignable_v<C>);
            static_assert(!sys::is_trivially_move_assignable_v<D>);
            static_assert(!sys::is_trivially_move_assignable_v<decltype(void())>);
            static_assert(!sys::is_trivially_move_assignable_v<void>);
            static_assert(!sys::is_trivially_move_assignable_v<int[31]>);
        }

        // sys::is_nothrow_move_assignable
        {
            struct A {};
            struct B { B& operator=(B&&) = default; };
            struct C { C& operator=(C&&) noexcept; };
            struct D { D& operator=(D&&) noexcept(false); };
            struct E { E& operator=(E&&) = delete; };
            struct F { int& ref; F(int& x) : ref(x) {} };
            static_assert(sys::is_nothrow_move_assignable_v<int>);
            static_assert(sys::is_nothrow_move_assignable_v<A>);
            static_assert(sys::is_nothrow_move_assignable_v<B>);
            static_assert(sys::is_nothrow_move_assignable_v<C>);
            static_assert(!sys::is_nothrow_move_assignable_v<D>);
            static_assert(!sys::is_nothrow_move_assignable_v<E>);
            static_assert(!sys::is_nothrow_move_assignable_v<F>);
            static_assert(!sys::is_nothrow_move_assignable_v<decltype(void())>);
            static_assert(!sys::is_nothrow_move_assignable_v<void>);
            static_assert(!sys::is_nothrow_move_assignable_v<int[1]>);
        }

        // sys::is_destructible
        {
            struct A {};                    // Yes
            struct B { ~B() = default; };   // Yes
            struct C { ~C(){} };            // Yes
            struct D { ~D() = delete; };    // No: deleted
            class  E { ~E() {} };           // No: private
            static_assert(sys::is_destructible_v<const volatile int>);
            static_assert(sys::is_destructible_v<int>);
            static_assert(sys::is_destructible_v<int&>);
            static_assert(sys::is_destructible_v<int&&>);
            static_assert(sys::is_destructible_v<int[12]>);    // Bounded
            static_assert(sys::is_destructible_v<A>);
            static_assert(sys::is_destructible_v<B>);
            static_assert(sys::is_destructible_v<C>);
            static_assert(!sys::is_destructible_v<D>);
            static_assert(!sys::is_destructible_v<E>);
            static_assert(!sys::is_destructible_v<void>);
            static_assert(!sys::is_destructible_v<decltype(void())>);
            static_assert(!sys::is_destructible_v<int[]>);      // Unbounded
        }

        // sys::is_trivially_destructible
        {
            struct A {};                    // Yes
            struct B { ~B() = default; };   // Yes
            struct C { ~C() {} };           // No: user defined
            struct D { ~D() = delete; };    // No: deleted
            static_assert(sys::is_trivially_destructible_v<int>);
            static_assert(sys::is_trivially_destructible_v<int&>);
            static_assert(sys::is_trivially_destructible_v<int&&>);
            static_assert(sys::is_trivially_destructible_v<int[12]>);   // Bounded
            static_assert(sys::is_trivially_destructible_v<A>);
            static_assert(sys::is_trivially_destructible_v<B>);
            static_assert(!sys::is_trivially_destructible_v<C>);
            static_assert(!sys::is_trivially_destructible_v<D>);
            static_assert(!sys::is_trivially_destructible_v<void>);
            static_assert(!sys::is_trivially_destructible_v<decltype(void())>);
            static_assert(!sys::is_trivially_destructible_v<int[]>);
        }

        // sys::is_nothrow_destructible
        {
            struct A {};                            // Yes
            struct B { ~B() = default; };           // Yes
            struct C { ~C() noexcept(true) {} };    // Yes
            struct D { ~D() noexcept(false) {} };   // No
            struct E { ~E() = delete; };            // No
            static_assert(sys::is_nothrow_destructible_v<int>);
            static_assert(sys::is_nothrow_destructible_v<int&>);
            static_assert(sys::is_nothrow_destructible_v<int&&>);
            static_assert(sys::is_nothrow_destructible_v<int>);
            static_assert(sys::is_nothrow_destructible_v<int[12]>);
            static_assert(sys::is_nothrow_destructible_v<A>);
            static_assert(sys::is_nothrow_destructible_v<B>);
            static_assert(sys::is_nothrow_destructible_v<C>);
            static_assert(!sys::is_nothrow_destructible_v<D>);
            static_assert(!sys::is_nothrow_destructible_v<E>);
            static_assert(!sys::is_nothrow_destructible_v<void>);
            static_assert(!sys::is_nothrow_destructible_v<decltype(void())>);
            static_assert(!sys::is_nothrow_destructible_v<int[]>);
        }

        // sys::has_virtual_destructor
        {
            struct A { virtual ~A() = default; };   // Yes: default impl
            struct B { virtual ~B() {}; };          // Yes: explicit impl
            struct C : B {};                        // Yes: Inherited
            struct D {};                            // No: default non virtual
            struct E { ~E() = default; };           // No: default non virtual
            struct F { ~F() = delete; };            // No: Deleted
            static_assert(sys::has_virtual_destructor_v<A>);
            static_assert(sys::has_virtual_destructor_v<B>);
            static_assert(sys::has_virtual_destructor_v<C>);
            static_assert(!sys::has_virtual_destructor_v<D>);
            static_assert(!sys::has_virtual_destructor_v<E>);
            static_assert(!sys::has_virtual_destructor_v<F>);
            static_assert(!sys::has_virtual_destructor_v<int>);
            static_assert(!sys::has_virtual_destructor_v<int&>);
            static_assert(!sys::has_virtual_destructor_v<int&&>);
            static_assert(!sys::has_virtual_destructor_v<void>);
        }

        // sys::rank
        {
            static_assert(sys::rank_v<int>          == 0);
            static_assert(sys::rank_v<int[1]>       == 1);
            static_assert(sys::rank_v<int[1][1]>    == 2);
            static_assert(sys::rank_v<int[1][1][1]> == 3);
            static_assert(sys::rank_v<int[ ][1][1]> == 3);
        }

        // sys::extent
        {
            static_assert(sys::extent_v<int>          == 0);
            static_assert(sys::extent_v<int[]>        == 0);
            static_assert(sys::extent_v<int[0],       0> == 0);
            static_assert(sys::extent_v<int[1],       0> == 1);
            static_assert(sys::extent_v<int[1][2],    1> == 2);
            static_assert(sys::extent_v<int[1][2][3], 2> == 3);
            static_assert(sys::extent_v<int[ ][2][3], 0> == 0);
        }

        //  sys::is_same
        {
            using alt_int = int;
            constexpr long double d1 = 12, d2 = 21;
            static_assert(sys::is_same_v<int, int>);
            static_assert(sys::is_same_v<alt_int, int>);
            static_assert(sys::is_same_v<int, signed int>);
            static_assert(sys::is_same_v<decltype(d1), decltype(d2)>);
            static_assert(sys::is_same_v<int[], int[]>);
            static_assert(sys::is_same_v<int[2], int[2]>);
            static_assert(!sys::is_same_v<int[2], int[1]>);
            static_assert(!sys::is_same_v<int, const volatile int>);
            static_assert(!sys::is_same_v<char, signed char>);
            static_assert(!sys::is_same_v<char, unsigned char>);
            static_assert(!sys::is_same_v<sys::nullptr_t, decltype(0)>);
        }

        // sys::is_base_of<base, derived>
        {
            struct A {};
            struct B {};
            struct C : B {};
            struct D : virtual B {};
            struct E : A, B {};
            struct F final : B {};
            struct G : C {};
            static_assert(sys::is_base_of_v<A, A>);
            static_assert(sys::is_base_of_v<B, C>);
            static_assert(sys::is_base_of_v<B, D>);
            static_assert(sys::is_base_of_v<A, E>);
            static_assert(sys::is_base_of_v<B, E>);
            static_assert(sys::is_base_of_v<B, F>);
            static_assert(sys::is_base_of_v<B, G>);
            static_assert(!sys::is_base_of_v<C, B>);
            static_assert(!sys::is_base_of_v<int, int>);    // Not a class
            static_assert(!sys::is_base_of_v<void, void>);  // Not a class
        }

        // sys::is_convertible
        {
            class A {};
            class B : public A {};
            class C {};
            class D { public: operator C() {return c;} C c; };
            using E = C_perfect_fwd_cx; // Can't be a local class
            static_assert(sys::is_convertible_v<int, int>);
            static_assert(sys::is_convertible_v<int, double>);
            static_assert(sys::is_convertible_v<double, int>);
            static_assert(sys::is_convertible_v<B*, A*>);
            static_assert(sys::is_convertible_v<D, C>);
            static_assert(sys::is_convertible_v<A, E>);
            static_assert(sys::is_convertible_v<B, E>);
            static_assert(sys::is_convertible_v<C, E>);
            static_assert(sys::is_convertible_v<D, E>);
            static_assert(sys::is_convertible_v<int[], int*>);
            static_assert(sys::is_convertible_v<int&&, const int&>);
            static_assert(!sys::is_convertible_v<A*, B*>);
            static_assert(!sys::is_convertible_v<B*, C*>);
        }

        // sys::remove_cv, sys::remove_const, sys::remove_volatile
        {
            struct A {};
            static_assert(sys::is_same_v<sys::remove_cv_t<const volatile int>, int>);
            static_assert(sys::is_same_v<sys::remove_cv_t<const volatile A>,   A>);
            static_assert(sys::is_same_v<sys::remove_const_t<const volatile int>, volatile int>);
            static_assert(sys::is_same_v<sys::remove_const_t<const volatile A>,   volatile A>);
            static_assert(sys::is_same_v<sys::remove_volatile_t<const volatile int>, const int>);
            static_assert(sys::is_same_v<sys::remove_volatile_t<const volatile A>,   const A>);
        }

        // sys::add_cv, sys::add_const, sys::add_volatile
        {
            struct A {};
            static_assert(sys::is_same_v<sys::add_cv_t<int>,       const volatile int>);
            static_assert(sys::is_same_v<sys::add_const_t<int>,    const int>);
            static_assert(sys::is_same_v<sys::add_volatile_t<int>, volatile int>);
            static_assert(sys::is_same_v<sys::add_cv_t<A>,         const volatile A>);
            static_assert(sys::is_same_v<sys::add_const_t<A>,      const A>);
            static_assert(sys::is_same_v<sys::add_volatile_t<A>,   volatile A>);
        }

        // sys::remove_reference
        {
            struct A {};
            static_assert(sys::is_same_v<sys::remove_reference_t<int>,   int>);
            static_assert(sys::is_same_v<sys::remove_reference_t<int&>,  int>);
            static_assert(sys::is_same_v<sys::remove_reference_t<int&&>, int>);
            static_assert(sys::is_same_v<sys::remove_reference_t<const int&>, const int>);
            static_assert(sys::is_same_v<sys::remove_reference_t<A>,   A>);
            static_assert(sys::is_same_v<sys::remove_reference_t<A&>,  A>);
            static_assert(sys::is_same_v<sys::remove_reference_t<A&&>, A>);
            static_assert(sys::is_same_v<sys::remove_reference_t<const A&>, const A>);
            static_assert(sys::is_same_v<sys::remove_reference_t<void>, void>);
        }

        // sys::add_lvalue_reference
        {
            struct A {};
            static_assert(sys::is_same_v<sys::add_lvalue_reference_t<int>,   int&>);
            static_assert(sys::is_same_v<sys::add_lvalue_reference_t<int&>,  int&>);
            static_assert(sys::is_same_v<sys::add_lvalue_reference_t<int&&>, int&>);
            static_assert(sys::is_same_v<sys::add_lvalue_reference_t<const int&>, const int&>);
            static_assert(sys::is_same_v<sys::add_lvalue_reference_t<A>,  A&>);
            static_assert(sys::is_same_v<sys::add_lvalue_reference_t<A&>, A&>);
            static_assert(sys::is_same_v<sys::add_lvalue_reference_t<const A&>, const A&>);
            static_assert(sys::is_same_v<sys::add_lvalue_reference_t<void>, void>);
            static_assert(sys::is_same_v<sys::add_lvalue_reference_t<decltype(void())>, decltype(void())>);
        }

        // sys::add_rvalue_reference
        {
            struct A {};
            static_assert(sys::is_same_v<sys::add_rvalue_reference_t<int>,   int&&>);
            static_assert(sys::is_same_v<sys::add_rvalue_reference_t<int&>,  int&>);
            static_assert(sys::is_same_v<sys::add_rvalue_reference_t<int&&>, int&&>);
            static_assert(sys::is_same_v<sys::add_rvalue_reference_t<A>,     A&&>);
            static_assert(sys::is_same_v<sys::add_rvalue_reference_t<A&>,    A&>);
            static_assert(sys::is_same_v<sys::add_rvalue_reference_t<A&&>,   A&&>);
            static_assert(sys::is_same_v<sys::add_rvalue_reference_t<void>,  void>);
            static_assert(sys::is_same_v<sys::add_rvalue_reference_t<decltype(void())>, decltype(void())>);
        }

        // sys::remove_pointer
        {
            static_assert(sys::is_same_v<sys::remove_pointer_t<int>,   int>);
            static_assert(sys::is_same_v<sys::remove_pointer_t<int*>,  int>);
            static_assert(sys::is_same_v<sys::remove_pointer_t<int**>, int*>);
            static_assert(sys::is_same_v<sys::remove_pointer_t<int&>,  int&>);
            static_assert(sys::is_same_v<sys::remove_pointer_t<int*&>, int*&>);
            static_assert(sys::is_same_v<sys::remove_pointer_t<const volatile int*>, const volatile int>);
            static_assert(sys::is_same_v<sys::remove_pointer_t<void>,  void>);
            static_assert(sys::is_same_v<sys::remove_pointer_t<sys::nullptr_t>, sys::nullptr_t>);
            static_assert(sys::is_same_v<sys::remove_pointer_t<int(*)(int)>, int(int)>);
        }

        // sys::add_pointer
        {
            static_assert(sys::is_same_v<sys::add_pointer_t<int>,  int*>);
            static_assert(sys::is_same_v<sys::add_pointer_t<int*>, int**>);
            static_assert(sys::is_same_v<sys::add_pointer_t<int&>, int*>);
            static_assert(sys::is_same_v<sys::add_pointer_t<const volatile int>, const volatile int*>);
            static_assert(sys::is_same_v<sys::add_pointer_t<void>, void*>);
            static_assert(sys::is_same_v<sys::add_pointer_t<sys::nullptr_t>, sys::nullptr_t*>);
            static_assert(sys::is_same_v<sys::add_pointer_t<int(int)>, int(*)(int)>);
        }

        // sys::remove_extent
        {
            static_assert(sys::is_same_v<sys::remove_extent_t<int[]>, int>);
            static_assert(sys::is_same_v<sys::remove_extent_t<int[1][2]>, int[2]>);
            static_assert(sys::is_same_v<sys::remove_extent_t<int[ ][3]>, int[3]>);
            static_assert(sys::is_same_v<sys::remove_extent_t<void>, void>);
            static_assert(sys::is_same_v<sys::remove_extent_t<int>, int>);
        }

        // sys::remove_all_extents
        {
            static_assert(sys::is_same_v<sys::remove_all_extents_t<int[]>, int>);
            static_assert(sys::is_same_v<sys::remove_all_extents_t<int[][1][2][3]>,  int>);
            static_assert(sys::is_same_v<sys::remove_all_extents_t<int*[][1][2][3]>, int*>);
            static_assert(sys::is_same_v<sys::remove_all_extents_t<int*>, int*>);
            static_assert(sys::is_same_v<sys::remove_all_extents_t<void>, void>);
        }

        // sys::decay
        {
            struct A {};
            static_assert(sys::is_same_v<sys::decay_t<A>,          A>);
            static_assert(sys::is_same_v<sys::decay_t<int>,        int>);
            static_assert(sys::is_same_v<sys::decay_t<int*>,       int*>);
            static_assert(sys::is_same_v<sys::decay_t<int&>,       int>);
            static_assert(sys::is_same_v<sys::decay_t<int&&>,      int>);
            static_assert(sys::is_same_v<sys::decay_t<const int&>, int>);
            static_assert(sys::is_same_v<sys::decay_t<int[]>,      int*>);
            static_assert(sys::is_same_v<sys::decay_t<int[2][5]>,  int(*)[5]>);
            static_assert(sys::is_same_v<sys::decay_t<int(int)>,   int(*)(int)>);
        }

        // sys::remove_cvref
        {
            static_assert(sys::is_same_v<sys::remove_cvref_t<const volatile int&>, int>);
            static_assert(sys::is_same_v<sys::remove_cvref_t<int>,   int>);
            static_assert(sys::is_same_v<sys::remove_cvref_t<int&>,  int>);
            static_assert(sys::is_same_v<sys::remove_cvref_t<int&&>, int>);
            static_assert(sys::is_same_v<sys::remove_cvref_t<const volatile void>, void>);
            static_assert(sys::is_same_v<sys::remove_cvref_t<const int[4]>,    int[4]>);
            static_assert(sys::is_same_v<sys::remove_cvref_t<const int(&)[4]>, int[4]>);
            static_assert(sys::is_same_v<sys::remove_cvref_t<int(int)>, int(int)>);
        }

        // sys::conditional
        {
            static_assert(sys::is_same_v<sys::conditional_t<true,  int, double>, int>);
            static_assert(sys::is_same_v<sys::conditional_t<false, int, double>, double>);
        }

        // sys::underlying_type
        {
            enum E          { Moop = 0xFFFF };  // Size not specified, but must hold 16-bit value
            enum class Ec   { Toop = 0xFFFF };  // Size not specified, but must hold 16-bit value
            enum class EcByte : sys::uint8_t {};
            enum class EcUns  : unsigned {};
            enum class EcInt  : int {};
            static_assert(sizeof(sys::underlying_type_t<E>)  >= 2);
            static_assert(sizeof(sys::underlying_type_t<Ec>) >= 2);
            static_assert(sys::is_same_v<sys::underlying_type_t<EcByte>, sys::uint8_t>);
            static_assert(sys::is_same_v<sys::underlying_type_t<EcInt>, int>);
            static_assert(sys::is_same_v<sys::underlying_type_t<EcUns>, unsigned>);
        }

        return 0;
    }
};

sys::app* CreateApp()
{
    return new TestTypeTraits();
}
