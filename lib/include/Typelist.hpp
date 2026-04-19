#ifndef LIB_INCLUDE_TYPELIST_HPP
#define LIB_INCLUDE_TYPELIST_HPP

#include <type_traits>

#include <cstdint>       // TODO remove, only for testing
#include <functional>    // TODO remove, only for testing

template <typename... Ts>
struct Typelist { };

template <typename T, typename LIST_T>
struct Append;

template <typename T, typename... Ts>
struct Append<T, Typelist<Ts...>> : std::type_identity<Typelist<Ts..., T>> {
    // using type = Typelist<Ts..., T>;
};

template <typename T, typename LIST_T>
struct Prepend;

template <typename T, typename... Ts>
struct Prepend<T, Typelist<Ts...>> : std::type_identity<Typelist<T, Ts...>> {
    // using type = Typelist<T, Ts...>;
};

// template <typename, typename>
// struct Concat;
template <typename...>
struct Concat;

template <typename... T1s, typename... T2s>
struct Concat<Typelist<T1s...>, Typelist<T2s...>> : std::type_identity<Typelist<T1s..., T2s...>> {
    // using type = Typelist<T1s..., T2s...>;
};

template <typename... T1s, typename... T2s, typename... REST_LIST_Ts>
struct Concat<Typelist<T1s...>, Typelist<T2s...>, REST_LIST_Ts...> {
    using type = typename Concat<Typelist<T1s..., T2s...>, REST_LIST_Ts...>::type;
};

template <typename>
struct Trim;

template <typename F_T, typename... R_Ts>
struct Trim<Typelist<F_T, R_Ts...>> {
    using type = typename Prepend<F_T, typename Trim<Typelist<R_Ts...>>::type>::type;
};

template <typename T>
struct Trim<Typelist<T>> {
    using type = Typelist<>;
};

template <typename>
struct Size;

template <typename... Ts>
struct Size<Typelist<Ts...>> {
    static constexpr std::size_t value = sizeof...(Ts);
};

template <std::size_t, typename>
struct At;

// TODO c++26:
// template <std::size_t IDX_V, typename... Ts>
// struct At<IDX_V, Typelist<Ts...>> {
//     using type = Ts...[IDX_V];
// };

template <std::size_t IDX_V, typename F_T, typename... R_Ts>
struct At<IDX_V, Typelist<F_T, R_Ts...>> {
    using type = typename At<IDX_V - 1, Typelist<R_Ts...>>::type;
};

template <typename F_T, typename... R_Ts>
struct At<0, Typelist<F_T, R_Ts...>> {
    using type = F_T;
};

template <typename LIST_T>
struct First {
    using type = typename At<0, LIST_T>::type;
};

template <typename LIST_T>
struct Last {
    using type = typename At<Size<LIST_T>::value - 1, LIST_T>::type;
};

// template<std::size_t ,typename, typename>
// struct RemoveHelper;

// template<std::size_t IDX_V,typename TARGET_T, typename F_T, typename... R_Ts>
// struct RemoveHelper<IDX_V, TARGET_T, Typelist<F_T, R_Ts...>> {

// }

template <typename, typename>
struct Remove;

template <typename TARGET_T, typename F_T, typename... R_Ts>
struct Remove<TARGET_T, Typelist<F_T, R_Ts...>> {
    using type = typename Prepend<F_T, typename Remove<TARGET_T, Typelist<R_Ts...>>::type>::type;
};

template <typename TARGET_T, typename... R_Ts>
struct Remove<TARGET_T, Typelist<TARGET_T, R_Ts...>> {
    using type = typename Remove<TARGET_T, Typelist<R_Ts...>>::type;
};

template <typename TARGET_T>
struct Remove<TARGET_T, Typelist<>> {
    using type = Typelist<>;
};

template <typename>
struct IsEmpty;

template <>
struct IsEmpty<Typelist<>> : std::true_type { };

template <typename T, typename... Ts>
struct IsEmpty<Typelist<T, Ts...>> : std::false_type { };

template <template <typename> typename PRED_T, typename LIST_T>
struct Filter;

template <template <typename> typename PRED_T, typename FIRST_T, typename... REST_Ts>
struct Filter<PRED_T, Typelist<FIRST_T, REST_Ts...>> {
    using type = typename Filter<PRED_T, Typelist<REST_Ts...>>::type;
};

template <template <typename> typename PRED_T, typename FIRST_T, typename... REST_Ts>
    requires PRED_T<FIRST_T>::value
struct Filter<PRED_T, Typelist<FIRST_T, REST_Ts...>> {
    using type = Append<FIRST_T, typename Filter<PRED_T, Typelist<REST_Ts...>>::type>::type;
};

template <template <typename> typename PRED_T>
struct Filter<PRED_T, Typelist<>> {
    using type = Typelist<>;
};

template <std::size_t OFFSET_V, std::size_t LEN_V, typename LIST_T>
struct Sublist;

// TODO c++26:
// template <std::size_t IDX_V, typename... Ts>
// struct At<IDX_V, Typelist<Ts...>> {
//     using type = Ts...[IDX_V];
// };

template <std::size_t OFFSET_V, std::size_t LEN_V, typename F_T, typename... R_Ts>
    requires(OFFSET_V > 0)
struct Sublist<OFFSET_V, LEN_V, Typelist<F_T, R_Ts...>> {
    using type = Sublist<OFFSET_V - 1, LEN_V, Typelist<R_Ts...>>::type;
};

template <std::size_t LEN_V, typename... Ts>
struct Sublist<0, LEN_V, Typelist<Ts...>> {
    using type = Sublist<0, LEN_V, typename Trim<Typelist<Ts...>>::type>::type;
};

template <std::size_t LEN_V, typename... Ts>
    requires(sizeof...(Ts) <= LEN_V)
struct Sublist<0, LEN_V, Typelist<Ts...>> {
    using type = Typelist<Ts...>;
};

// template <typename LIST_T, auto COMP_V, template <typename> typename PROJ_T, std::size_t PIVOT_POS_V = 0>
// struct Sort;

// // template <typename... Ts, auto COMP_V, template <typename> typename PROJ_T>
// // struct Sort<Typelist<Ts...>, COMP_V, PROJ_T> {

// //     using LeftHalf = typename Sublist<0, Size<Typelist<Ts...>>::value / 2, Typelist<Ts...>>::type;
// //     using RightHalf =
// //         typename Sublist<Size<Typelist<Ts...>>::value / 2, Size<Typelist<Ts...>>::value, Typelist<Ts...>>::type;

// //     using LeftSorted = typename Sort<LeftHalf, COMP_V, PROJ_T>::type;
// //     using RightSorted = typename Sort<RightHalf, COMP_V, PROJ_T>::type;

// //     using type = std::conditional_t<
// //         COMP_V(PROJ_T<typename At<0, LeftSorted>::type>::value, PROJ_T<typename At<0, RightSorted>::type>::value),
// //         typename Concat<LeftSorted, RightSorted>::type, typename Concat<RightSorted, LeftSorted>::type>;
// // };

// // template <typename F_T, typename L_T, auto COMP_V, template <typename> typename PROJ_T>
// // struct Sort<Typelist<F_T, L_T>, COMP_V, PROJ_T> {
// //     using type =
// //         std::conditional_t<COMP_V(PROJ_T<F_T>::value, PROJ_T<L_T>::value), Typelist<F_T, L_T>, Typelist<L_T,
// F_T>>;
// // };

// // template <typename T, auto COMP_V, template <typename> typename PROJ_T>
// // struct Sort<Typelist<T>, COMP_V, PROJ_T> {
// //     using type = Typelist<T>;
// // };

// template <typename... Ts, auto COMP_V, template <typename> typename PROJ_T, std::size_t PIVOT_POS_V>
// struct Sort<Typelist<Ts...>, COMP_V, PROJ_T, PIVOT_POS_V> {

//     static constexpr auto pivot = PROJ_T<typename At<PIVOT_POS_V, Typelist<Ts...>>::type>::value;

//     using Left =
//         typename Remove<void,
//                         Typelist<std::conditional_t<COMP_V(PROJ_T<Ts>::value, pivot), Ts, void>...>>::type;    //
//                         TODO
//                                                                                                                //
//                                                                                                                remove
//                                                                                                                //
//                                                                                                                void?

//     using Right = typename Remove<
//         void, Typelist<std::conditional_t<!COMP_V(PROJ_T<Ts>::value, pivot), Ts, void>...>>::type;    // TODO
//                                                                                                       // remove
//                                                                                                       // void?

//     // move pivot if list is same as input to prevent infinite recursion:
//     using LeftSorted =
//         typename Sort<Left, COMP_V, PROJ_T, std::is_same_v<Left, Typelist<Ts...>> ? PIVOT_POS_V + 1 : 0>::type;
//     using RightSorted =
//         typename Sort<Right, COMP_V, PROJ_T, std::is_same_v<Right, Typelist<Ts...>> ? PIVOT_POS_V + 1 : 0>::type;

//     using type = typename Concat<LeftSorted, RightSorted>::type;
// };

// template <typename F_T, typename L_T, auto COMP_V, template <typename> typename PROJ_T, std::size_t PIVOT_POS_V>
// struct Sort<Typelist<F_T, L_T>, COMP_V, PROJ_T, PIVOT_POS_V> {
//     using type =
//         std::conditional_t<COMP_V(PROJ_T<F_T>::value, PROJ_T<L_T>::value), Typelist<F_T, L_T>, Typelist<L_T, F_T>>;
// };

// template <typename T, auto COMP_V, template <typename> typename PROJ_T, std::size_t PIVOT_POS_V>
// struct Sort<Typelist<T>, COMP_V, PROJ_T, PIVOT_POS_V> {
//     using type = Typelist<T>;
// };

// template <auto COMP_V, template <typename> typename PROJ_T, std::size_t PIVOT_POS_V>
// struct Sort<Typelist<>, COMP_V, PROJ_T, PIVOT_POS_V> {
//     using type = Typelist<>;
// };

template <typename LIST_T, auto COMP_V, template <typename> typename PROJ_T>
struct Sort;

template <typename PIVOT_T, typename... Ts, auto COMP_V, template <typename> typename PROJ_T>
struct Sort<Typelist<PIVOT_T, Ts...>, COMP_V, PROJ_T> {

    static constexpr auto pivotVal = PROJ_T<PIVOT_T>::value;

    using Left = typename Remove<
        void, Typelist<std::conditional_t<COMP_V(PROJ_T<Ts>::value, pivotVal), Ts, void>...>>::type;    // TODO
                                                                                                        // remove
                                                                                                        // void?

    using Right = typename Remove<
        void, Typelist<std::conditional_t<!COMP_V(PROJ_T<Ts>::value, pivotVal), Ts, void>...>>::type;    // TODO
                                                                                                         // remove
                                                                                                         // void?

    using LeftSorted = typename Sort<Left, COMP_V, PROJ_T>::type;
    using RightSorted = typename Sort<Right, COMP_V, PROJ_T>::type;

    // using type = typename Concat<LeftSorted, typename Prepend<PIVOT_T, RightSorted>::type>::type;
    using type = typename Concat<LeftSorted, Typelist<PIVOT_T>, RightSorted>::type;
};

template <typename F_T, typename L_T, auto COMP_V, template <typename> typename PROJ_T>
struct Sort<Typelist<F_T, L_T>, COMP_V, PROJ_T> {
    using type =
        std::conditional_t<COMP_V(PROJ_T<F_T>::value, PROJ_T<L_T>::value), Typelist<F_T, L_T>, Typelist<L_T, F_T>>;
};

template <typename T, auto COMP_V, template <typename> typename PROJ_T>
struct Sort<Typelist<T>, COMP_V, PROJ_T> {
    using type = Typelist<T>;
};

template <auto COMP_V, template <typename> typename PROJ_T>
struct Sort<Typelist<>, COMP_V, PROJ_T> {
    using type = Typelist<>;
};

namespace testing {

static_assert(std::is_same_v<Typelist<int>, typename Remove<bool, Typelist<int>>::type>);
static_assert(std::is_same_v<Typelist<>, typename Remove<int, Typelist<int>>::type>);
static_assert(std::is_same_v<Typelist<int>, typename Remove<double, Typelist<int, double>>::type>);
static_assert(std::is_same_v<Typelist<bool, double>, typename Remove<int, Typelist<int, bool, int, double>>::type>);

static_assert(std::is_same_v<Typelist<int>, typename Sublist<0, 1, Typelist<int>>::type>);
static_assert(std::is_same_v<Typelist<int>, typename Sublist<0, 1, Typelist<int, double>>::type>);
static_assert(std::is_same_v<Typelist<int, double>, typename Sublist<0, 2, Typelist<int, double>>::type>);
static_assert(std::is_same_v<Typelist<>, typename Sublist<1, 1, Typelist<int>>::type>);
static_assert(std::is_same_v<Typelist<bool, int>, typename Sublist<1, 2, Typelist<int, bool, int, double>>::type>);

template <typename T>
struct SizeProjection {
    static constexpr std::size_t value = sizeof(T);
};

static_assert(
    std::is_same_v<
        Typelist<std::uint32_t, std::uint16_t, std::uint8_t>,
        typename Sort<Typelist<std::uint16_t, std::uint8_t, std::uint32_t>, std::greater{}, SizeProjection>::type>);

static_assert(
    std::is_same_v<
        Typelist<std::uint32_t, std::uint16_t, std::uint8_t>,
        typename Sort<Typelist<std::uint32_t, std::uint16_t, std::uint8_t>, std::greater{}, SizeProjection>::type>);

static_assert(std::is_same_v<
              Typelist<std::uint8_t, std::uint16_t, std::uint32_t>,
              typename Sort<Typelist<std::uint16_t, std::uint8_t, std::uint32_t>, std::less{}, SizeProjection>::type>);

static_assert(std::is_same_v<
              Typelist<std::uint8_t, std::uint16_t, std::uint32_t>,
              typename Sort<Typelist<std::uint8_t, std::uint16_t, std::uint32_t>, std::less{}, SizeProjection>::type>);

}    // namespace testing

#endif    // LIB_INCLUDE_TYPELIST_HPP
