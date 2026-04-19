#ifndef LIB_INCLUDE_DEVICETREE_HPP
#define LIB_INCLUDE_DEVICETREE_HPP

// #include "Interrupt.hpp"
#include "Typelist.hpp"

#include <span>
#include <cstddef>

// extern I2c i2c0;
// extern CaptureTimG timG8;
// extern Tlc59208f ledDriver;

// static constexpr DeviceTree dt {I2c0, timG8};

template <std::size_t N>
struct ConstString {
    // constexpr ConstString(const char* str) noexcept { std::copy_n(str, std::strlen(str), arr.begin()); }
    // constexpr ConstString(std::string_view str) noexcept { std::copy(str.begin(), str.end(), arr.begin()); }
    constexpr ConstString(const char (&str)[N]) noexcept { std::copy_n(str, N, arr.begin()); }

    std::array<char, N> arr;
};

// ConstString(const char *) -> ConstString<std::string_view>;

// template <std::size_t SIZE_V>
// struct ConstString->ConstString<SIZE_V>;

// struct Properties {
//     const char* name;
// };

template <ConstString>
struct Label { };

// template <std::size_t N>
// struct Label<ConstString<N>> { };

template <typename DRIVER_T, typename...>
struct Node {
    // DRIVER_T dev;
};

// template <typename DRIVER_T, typename LABEL_T>
// struct DevNode {    // TODO rename to namedNode? namedDevice?
//     DRIVER_T dev;
// };

// template <template<typename...>typename DRIVER_T, typename...>
// struct Node { };

// template <typename DRIVER_T, std::size_t N>
// struct Node<DRIVER_T, Label<ConstString<N>>> { };

// template <typename DRIVER_T>
// struct Node<DRIVER_T, Label> { };

// template <typename... CHILD_NODE_Ts>
// struct Children { };

template <template <typename...> typename DRIVER_T, typename...>
struct Child { };

template <typename CHILD_T, typename PARENT_T>
struct ConvertChildToNode;

template <template <typename...> typename CHILD_DRIVER_T, typename... CHILD_ARG_Ts, typename PARENT_DRIVER_T,
          typename... PARENT_ARG_Ts>
struct ConvertChildToNode<Child<CHILD_DRIVER_T, CHILD_ARG_Ts...>, Node<PARENT_DRIVER_T, PARENT_ARG_Ts...>> {
    using Type = Node<CHILD_DRIVER_T<PARENT_DRIVER_T>, CHILD_ARG_Ts...>;
};

template <typename CHILD_DRIVER_T, typename... CHILD_ARG_Ts, typename PARENT_DRIVER_T, typename... PARENT_ARG_Ts>
struct ConvertChildToNode<Node<CHILD_DRIVER_T, CHILD_ARG_Ts...>, Node<PARENT_DRIVER_T, PARENT_ARG_Ts...>> {
    using Type = Node<CHILD_DRIVER_T, CHILD_ARG_Ts...>;
};

template <std::uint8_t>
struct Address { };

template <std::uint8_t>
struct Interrupt { };

// static constexpr auto dt = createDevicetree<Node<I2c, {.name = "i2c0"}>, Node<CaptureTim, {.name = "timG8"}>,
//                                             Node<Tlc59208f, {.name = "ledDriver"}>>();

template <typename DRIVER_T>
struct CtorArgs {
    template <typename...>
    using type = Typelist<>;
};

template <typename T>
struct Test {
    using Type = T::afkjhnioud;
};

template <typename T>
struct IsChild : std::false_type { };

template <template <typename...> typename DRIVER_T, typename... ARG_Vs>
struct IsChild<Child<DRIVER_T, ARG_Vs...>> : std::true_type { };

template <typename DRIVER_T, typename... ARG_Vs>
struct IsChild<Node<DRIVER_T, ARG_Vs...>> : std::true_type { };    // TODO hack to make it work with completedNodes

// namespace Concepts {

// template <typename T>
// concept Child = IsChild<T>::value;

// }

template <typename PARENT_T>
struct CollectChildren {
    using Type = Typelist<>;
};

// template <typename DRIVER_T, typename... ARG1_Ts, Concepts::Child... CHILD_Ts, typename... ARG2_Ts>
// struct CollectChildren<Node<DRIVER_T, ARG1_Ts..., CHILD_Ts..., ARG2_Ts...>> {
//     using Type = std::tuple<CHILD_Ts..., typename CollectChildren<CHILD_Ts>::Type...>;
// };

template <typename DRIVER_T, typename... ARG_Ts>
struct CollectChildren<Node<DRIVER_T, ARG_Ts...>> {
    using Type = typename Filter<IsChild, Typelist<ARG_Ts...>>::type;
};

template <typename PARENT_T>
struct HasChildren {
    static constexpr bool value = !IsEmpty<typename CollectChildren<PARENT_T>::Type>::value;
};

// template <typename T, template<typename>typename PRED_T>
// struct Filter;

// template <typename... Ts, template<typename>typename PRED_T>
// struct Filter<std::tuple<Ts...>, PRED_T>{
//     using Type = std::tuple<>
// }

// WORKING:
// template <typename DRIVER_T, typename LABEL_T, Concepts::Child... CHILD_Ts>
// struct CollectChildren<Node<DRIVER_T, LABEL_T, CHILD_Ts...>> {
//     using Type = std::tuple<CHILD_Ts...>;
// };

// NOT WORKING:
// template <typename DRIVER_T, typename FIRST_T, typename... REST_Ts>
// struct CollectChildren<Node<DRIVER_T, FIRST_T, REST_Ts...>> {
//     using Type = CollectChildren<Node<DRIVER_T, REST_Ts...>>::Type;
// };

// template <typename DRIVER_T, typename... ARG1_Ts, Concepts::Child... CHILD_Ts>
// struct CollectChildren<Node<DRIVER_T, ARG1_Ts..., CHILD_Ts...>> {
//     using Type = std::tuple<CHILD_Ts...>;
// };

// template <typename DRIVER_T, Concepts::Child... CHILD_Ts, typename... REST_Ts>
// struct CollectChildren<Node<DRIVER_T, CHILD_Ts..., REST_Ts...>> {
//     using Type = std::tuple<CHILD_Ts..., typename CollectChildren<CHILD_Ts>::Type...>;
// };

// template <typename DRIVER_T, typename... ARG_Ts>
// struct CollectDevices<std::tuple<Node<DRIVER_T, ARG_Ts...>>> {
//     using Type = typename Filter<IsChild, std::tuple<ARG_Ts...>>::Type;
// };

template <typename>
struct Flatten;

template <typename FIRST_NODE_T, typename... REST_NODE_Ts>
struct Flatten<Typelist<FIRST_NODE_T, REST_NODE_Ts...>> {
    // using type = typename Concat<Typelist<FIRST_NODE_T>, typename CollectChildren<FIRST_NODE_T>::Type,
    //                              typename Flatten<Typelist<REST_NODE_Ts...>>::type>::type;
    using type =
        typename Prepend<FIRST_NODE_T, typename Concat<typename CollectChildren<FIRST_NODE_T>::Type,
                                                       typename Flatten<Typelist<REST_NODE_Ts...>>::type>::type>::type;
};

template <>
struct Flatten<Typelist<>> {
    using type = Typelist<>;
};

// template <typename CHILDREN_T, typename PARENT_T>
// struct AssembleChildren;

// template <typename... CHILD_Ts, typename PARENT_T>
// struct AssembleChildren<std::tuple<CHILD_Ts...>, PARENT_T> {
//     using Type = std::tuple<ConvertChildToNode<CHILD_Ts, PARENT_T>::Type...>;
// };

// template <typename>
// struct AssembleDriverTypes;

// template <typename FIRST_NODE_T, typename... REST_NODE_Ts>
// struct AssembleDriverTypes<std::tuple<FIRST_NODE_T, REST_NODE_Ts...>, PARENT_T> {
//     using Type =
//         std::tuple<std::conditional_t<IsChild<FIRST_NODE_T>::value,
//                                       typename ConvertChildToNode<FIRST_NODE_T, PARENT_T>::Type, FIRST_NODE_T>>;

//     using Children = HasChildren<FIRST_NODE_T>::value
//                          ? AssembleDriverTypes<typename CollectChildren<FIRST_NODE_T>::Type, FIRST_NODE_T>::Type
//                          :

//     // using Type = decltype(std::tuple_cat(
//     //     std::declval<std::tuple<FIRST_NODE_T>>(),
//     //     std::declval<typename AssembleChildren<typename CollectChildren<FIRST_NODE_T>::Type>::Type>(),
//     //     std::declval<typename AssembleDriverTypes<std::tuple<REST_NODE_Ts...>>::Type>()));
// };

// template <bool PRED_V, typename IF_T, typename ELSE_T>
// struct LazyConditional : std::type_identity<ELSE_T> { };

// template <bool PRED_V, typename IF_T, typename ELSE_T>
//     requires(PRED_V)
// struct LazyConditional<PRED_V, IF_T, ELSE_T> : std::type_identity<IF_T> { };

template <typename>
struct AssembleDriverTypes;

template <typename DRIVER_T, typename... ARG_Ts>
struct AssembleDriverTypes<Node<DRIVER_T, ARG_Ts...>> {
  private:
    template <typename T>
    static constexpr auto conditionallyConvertChild() noexcept    // TODO needed for lazy evaluation of
                                                                  // ConvertChildToNode
    {
        if constexpr (IsChild<T>::value) {
            return std::type_identity<
                typename AssembleDriverTypes<typename ConvertChildToNode<T, Node<DRIVER_T>>::Type>::Type>{};
        } else {
            return std::type_identity<T>{};
        }
    }

  public:
    // recursively assemble all children:
    // using Type = Node<DRIVER_T, std::conditional_t<IsChild<ARG_Ts>::value, typename
    // AssembleDriverTypes<ARG_Ts>::Type, ARG_Ts>...>;

    // using Type = Node<
    //     DRIVER_T,
    //     typename std::contidional_t<
    //         IsChild<ARG_Ts>::value,
    //         typename AssembleDriverTypes<typename ConvertChildToNode<ARG_Ts, Node<DRIVER_T>>::Type>::Type,
    //         ARG_Ts>...>;

    using Type = Node<DRIVER_T, typename decltype(conditionallyConvertChild<ARG_Ts>())::type...>;

    // using Type = Node<DRIVER_T, decltype([]() {
    //                       if constexpr (IsChild<ARG_T>::value) {
    //                           return std::type_identity<typename AssembleDriverTypes<
    //                               typename ConvertChildToNode<ARG_Ts, Node<DRIVER_T>>::Type>::Type>{};
    //                       } else {
    //                           return std::type_identity<ARG_T>{};
    //                       }
    //                   }()::type)...>;

    // using Type = Node<DRIVER_T, [:if constexpr (IsChild<ARG_Ts>::value) {
    //     return ^^typename AssembleDriverTypes<typename ConvertChildToNode<ARG_Ts, Node<DRIVER_T>>::Type>::Type;
    // } else { return ^^ARG_Ts; }:]...>;
};

// template <typename DRIVER_T, typename FIRST_ARG_T, typename... REST_ARG_Ts>
// struct AssembleDriverTypes<Node<DRIVER_T, FIRST_ARG_T, REST_ARG_Ts...>> {
//     using Type = typename AssembleDriverTypes<Node<DRIVER_T, REST_ARG_Ts...>>::Type;

//     auto args = std::array{mp::meta(ARG_Ts)...};

//     for (auto& arg : args) {
//         if (isChild(arg)) {
//             arg = assembleDriverTypes(mp::meta(DRIVER_T), );
//         }
//     }

// };

// template <typename DRIVER_T, template <typename...> typename CHILD_DRIVER_T, typename... CHILD_ARG_Ts,
//           typename... ARG_Ts>
// struct AssembleDriverTypes<Node<DRIVER_T, Child<CHILD_DRIVER_T, CHILD_ARG_Ts...>, ARG_Ts...>> {
//     // recursively assemble all children:
//     using AssembledChild = typename AssembleDriverTypes<Node<CHILD_DRIVER_T<DRIVER_T>>>::Type;
//     using Type = typename AssembleDriverTypes<Node<DRIVER_T, AssembledChild, CHILD_ARG_Ts...>, ARG_Ts...>::Type;
// };

// template <typename DRIVER_T>
// struct AssembleDriverTypes<Node<DRIVER_T>> {
//     using Type = Node<DRIVER_T>;
// };

template <typename>
struct CompleteNodes;

template <typename... NODE_Ts>
struct CompleteNodes<Typelist<NODE_Ts...>> {
    using type = Typelist<typename AssembleDriverTypes<NODE_Ts>::Type...>;
};

// template <>
// struct AssembleDriverTypes<std::tuple<>> {
//     using Type = std::tuple<>;
// };

// template <typename, typename>
// struct BuildDevice;

// template <typename DRIVER_T, typename... ARG_Ts, typename DT_T>
// struct BuildDevice<Node<DRIVER_T, ARG_Ts...>, DT_T> {
//     using Type = DRIVER_T;
// };

// template <typename NODE_T, typename DT_T>
// struct GetParent {
//     using Type = ;    // TODO
// };

// template <template <typename...> typename DRIVER_T, typename... ARG_Ts, typename DT_T>
// struct BuildDevice<Child<DRIVER_T, ARG_Ts...>, DT_T> {
//     using Parent = typename GetParent<Child<DRIVER_T, ARG_Ts...>, DT_T>::Type;
//     using Type = DRIVER_T<Parent>;
// };

template <typename>
struct GetDriver;

template <typename DRIVER_T, typename... ARG_Ts>
struct GetDriver<Node<DRIVER_T, ARG_Ts...>> {
    using type = DRIVER_T;
};

// template <template <typename...> typename DRIVER_T, typename... ARG_Ts, typename PARENT_T>
// struct GetDriver<Child<DRIVER_T, ARG_Ts...>, PARENT_T> {
//     using Type = DRIVER_T<PARENT_T>;
// };

template <typename>
struct GetLabel;

template <typename DRIVER_T, typename FIRST_ARG_T, typename... REST_ARG_Ts>
struct GetLabel<Node<DRIVER_T, FIRST_ARG_T, REST_ARG_Ts...>> {
    using Type = typename GetLabel<Node<DRIVER_T, REST_ARG_Ts...>>::Type;
};

template <typename DRIVER_T, ConstString STR_V, typename... ARG_Ts>
struct GetLabel<Node<DRIVER_T, Label<STR_V>, ARG_Ts...>> {
    using Type = Label<STR_V>;
};

// template <typename DEVNODE_T>
// struct DeviceBuilder;

// template <typename DRIVER_T, typename LABEL_T>
// struct DeviceBuilder<DevNode<DRIVER_T, LABEL_T>> {
//     static constexpr DevNode<DRIVER_T, LABEL_T> create(auto& parent) noexcept { return {parent}; }
// };

template <typename NODE_T>
struct DeviceBuilder;

template <typename DRIVER_T, typename LABEL_T>
struct DeviceBuilder<Node<DRIVER_T, LABEL_T>> {
    static constexpr Node<DRIVER_T, LABEL_T> create(auto& parent) noexcept { return {parent}; }
};

template <typename NODE_T, typename PARENT_T>
static constexpr auto createBranch(PARENT_T& parent) noexcept
{
    // using Device = DevNode<typename GetDriver<NODE_T, PARENT_T>::Type, typename GetLabel<NODE_T>::Type>;
    // Device dev = DeviceBuilder<Device>::create(parent);

    // using Driver = typename GetDriver<NODE_T, PARENT_T>::Type;
    // using Label = typename GetLabel<NODE_T>::Type;
    // DevNode<Driver, Label> dev = DeviceBuilder<Driver>::create(parent);
    NODE_T node{parent.dev};

    if constexpr (HasChildren<NODE_T>::value) {
        using Children = typename CollectChildren<NODE_T>::Type;

        std::tuple children = [&node]<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
            return std::tuple_cat(createBranch<std::tuple_element_t<IDX_Vs, Children>>(node)...);
        }(std::make_index_sequence<std::tuple_size_v<Children>>());

        return std::tuple_cat(std::make_tuple(node), children);
    } else {
        return std::make_tuple(node);
    }

    // if constexpr (IsChild<NODE_T>::value) {
    //     using Parent = typename GetParent<NODE_T, DT_T>::Type;
    //     using Driver = typename GetDriver<NODE_T>::Type;
    //     return Driver<Parent>{parent};
    // } else {
    //     using Driver = typename GetDriver<NODE_T>::Type;
    //     return Driver{};
    // }
}

template <typename NODE_T>
static constexpr auto createBranch() noexcept    // TODO combine with above fn?
{
    // using Device = DevNode<typename GetDriver<NODE_T, void>::Type, typename GetLabel<NODE_T>::Type>;    // TODO void
    // // because no
    // // parent...
    // Device dev = DeviceBuilder<Device>::create(parent);

    // using Driver = typename GetDriver<NODE_T, void>::Type;    // TODO void because no parent...
    // // using Label = typename GetLabel<NODE_T>::Type;
    // DevNode<Driver, Label> dev = DeviceBuilder<Driver>::create();

    NODE_T node{};

    if constexpr (HasChildren<NODE_T>::value) {
        using Children = typename CollectChildren<NODE_T>::Type;

        std::tuple children = [&node]<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
            return std::tuple_cat(createBranch<std::tuple_element_t<IDX_Vs, Children>>(node)...);
        }(std::make_index_sequence<std::tuple_size_v<Children>>());

        return std::tuple_cat(std::make_tuple(node), children);
    } else {
        return std::make_tuple(node);
    }

    // if constexpr (IsChild<NODE_T>::value) {
    //     using Parent = typename GetParent<NODE_T, DT_T>::Type;
    //     using Driver = typename GetDriver<NODE_T>::Type;
    //     return Driver<Parent>{parent};
    // } else {
    //     using Driver = typename GetDriver<NODE_T>::Type;
    //     return Driver{};
    // }
}

// template <typename T>
// struct GetDriver;

// template <typename DRIVER_T, typename... ARG_Ts>
// struct GetDriver<Node<DRIVER_T, ARG_Ts...>> {
//     using type = DRIVER_T;
// };

template <typename T>
struct DriverSize {
    static constexpr std::size_t value = sizeof(typename GetDriver<T>::type);
};

// template <typename DRIVER_T, typename... ARG_Ts>
// struct DriverSize<Node<DRIVER_T, ARG_Ts...>> {
//     static constexpr std::size_t value = sizeof(DRIVER_T);
// };

namespace detail {

// template <typename T>
// static constexpr T roundUpTo(T val, T x) noexcept
// {    // TODO naming
//     return val + val % x;
// }

// static_assert(roundUpTo(0, 2) == 0);
// static_assert(roundUpTo(1, 2) == 2);
// static_assert(roundUpTo(2, 2) == 2);
// static_assert(roundUpTo(3, 2) == 4);

template <typename, typename>
struct StorageOffsetImpl;

// template <typename TARGET_NODE_T, typename... NODE_Ts>
// struct StorageOffset<TARGET_NODE_T, Typelist<NODE_Ts...>> {
//     // TODO could sort drivers by size to optimize padding due to alignment
//     static constexpr std::uintptr_t value = 0;    // TODO implement
// };

template <typename TARGET_NODE_T, typename... R_Ts>
struct StorageOffsetImpl<TARGET_NODE_T, Typelist<TARGET_NODE_T, R_Ts...>> {
    static constexpr std::uintptr_t value = 0;
};

template <typename TARGET_NODE_T, typename F_T, typename... R_Ts>
struct StorageOffsetImpl<TARGET_NODE_T, Typelist<F_T, R_Ts...>> {
    static constexpr std::size_t driverSize = DriverSize<F_T>::value;
    static constexpr std::size_t alignmentPadding = driverSize % alignof(typename GetDriver<F_T>::type);
    static constexpr std::uintptr_t value =
        driverSize + alignmentPadding + StorageOffsetImpl<TARGET_NODE_T, Typelist<R_Ts...>>::value;
};

}    // namespace detail

// template <typename TARGET_NODE_T, typename NODE_LIST_T>
// struct StorageOffset {
//     // Sort drivers by size to optimize padding due to alignment:
//     using SortedNodes = typename Sort<NODE_LIST_T, std::greater{}, DriverSize>::type;
//     static constexpr std::uintptr_t value = detail::StorageOffsetImpl<TARGET_NODE_T, SortedNodes>::value;
// };

// template <typename NODE_LIST_T>
// struct StorageSize {
//     static constexpr auto maxOffset = std::max(StorageOffset<NODE_Ts, Typelist<NODE_Ts...>>::value...);
//     using LastNode = std::conditional_t<, typename StorageSize<Typelist<R_Ts...>>::LastNode>;
//     static constexpr std::size_t value = maxOffset + sizeof(LastNode);
// };

template <typename NODE_LIST_T>
struct DriverStorage {
    // Sort drivers by size to optimize padding due to alignment:
    using SortedNodes = typename Sort<NODE_LIST_T, std::greater{}, DriverSize>::type;

    // template<typename T>
    // static constexpr std::uintptr_t getOffset() noexcept{
    //     // = detail::StorageOffsetImpl<TARGET_NODE_T, SortedNodes>::value;

    //     if constexpr (std::is_same_v<T, typename At<0, SortedNodes>::type>) {
    //         return
    //     }
    // }

    template <typename T>
    static constexpr auto offset = detail::StorageOffsetImpl<T, SortedNodes>::value;

    static constexpr auto size =
        offset<typename Last<SortedNodes>::type> + DriverSize<typename Last<SortedNodes>::type>::value;
};

// template <typename F_T, typename... R_Ts>
// struct StorageSize<Typelist<F_T, R_Ts...>> {
//     static constexpr auto maxOffset = std::max(StorageOffset<NODE_Ts, Typelist<NODE_Ts...>>::value...);
//     using LastNode = std::conditional_t<, typename StorageSize<Typelist<R_Ts...>>::LastNode>;
//     static constexpr std::size_t value = maxOffset + sizeof(LastNode);
// };

template <typename, typename>
struct Abc;    // TODO rename

// template <typename DRIVER_T, typename... ARG_Ts>
// struct NodeAssembler;

// template <typename DRIVER_T, typename... ARG_Ts>
// struct NodeAssembler<Node<DRIVER_T, ARG_Ts...>> {
//     static constexpr DRIVER_T& construct(std::uintptr_t location) noexcept
//     {
//         return *std::construct_at(reinterpret_cast<DRIVER_T*>(location), args..., );
//     }
// }
// ;

template <typename NODE_T, typename NODE_LIST_T>
static constexpr void constructBranch(std::span<std::byte> storage, auto&... args) noexcept    // TODO combine with
                                                                                               // above fn?
{
    using Driver = typename GetDriver<NODE_T>::type;
    auto& driver =
        // TODO bit cast or reinterpret cast?
        *std::construct_at(reinterpret_cast<Driver*>(&storage[0] + DriverStorage<NODE_LIST_T>::template offset<NODE_T>),
                           args...);
    // *std::construct_at(std::bit_cast<Driver*>(&storage[0] + StorageOffset<NODE_T, NODE_LIST_T>::value), args...);
    // *std::construct_at(static_cast<Driver*>(static_cast<std::uintptr_t>(static_cast<void*>(&storage[0])) +
    //                                         StorageOffset<NODE_T, NODE_LIST_T>::value),
    //                    args...);

    if constexpr (HasChildren<NODE_T>::value) {
        using Children = typename CollectChildren<NODE_T>::Type;

        // [&driver]<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
        //     (constructBranch<std::tuple_element_t<IDX_Vs, Children>>(driver)...);
        // }(std::make_index_sequence<std::tuple_size_v<Children>>());
        Abc<Children, NODE_LIST_T>::foo(storage, driver);
    }
}

template <typename... NODE_Ts, typename NODE_LIST_T>
struct Abc<Typelist<NODE_Ts...>, NODE_LIST_T> {
    static constexpr auto foo(std::span<std::byte> storage, auto&... args) noexcept
    {    // TODO rename
        (constructBranch<NODE_Ts, NODE_LIST_T>(storage, args...), ...);
    }
};

template <typename LABEL_T, typename LIST_T>
struct FindByLabel;

template <typename LABEL_T, typename FIRST_T, typename... REST_Ts>
struct FindByLabel<LABEL_T, Typelist<FIRST_T, REST_Ts...>> {
    using Type = typename FindByLabel<LABEL_T, Typelist<REST_Ts...>>::Type;
};

template <typename LABEL_T, typename FIRST_T, typename... REST_Ts>
    requires(std::is_same_v<LABEL_T, typename GetLabel<FIRST_T>::Type>)
struct FindByLabel<LABEL_T, Typelist<FIRST_T, REST_Ts...>> {
    using Type = FIRST_T;
};

template <typename DT_T>
class DeviceContainer {
  public:
    constexpr DeviceContainer() noexcept { Abc<typename DT_T::Tree, typename DT_T::Nodes>::foo(_storage); }

    constexpr DeviceContainer(const DeviceContainer&) noexcept = delete;
    constexpr DeviceContainer(DeviceContainer&&) noexcept = delete;
    constexpr DeviceContainer& operator=(const DeviceContainer&) noexcept = delete;
    constexpr DeviceContainer& operator=(DeviceContainer&&) noexcept = delete;

    constexpr ~DeviceContainer() noexcept
    {
        // TODO std::destruct_at in reverse order?
    }

    // return []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
    //     return std::tuple_cat(createBranch<std::tuple_element_t<IDX_Vs, ROOT_NODES_T>>()...);
    // }(std::make_index_sequence<std::tuple_size_v<ROOT_NODES_T>>{});

    constexpr void init() noexcept
    {
        // TODO implement
        // []<>() {

        // }();

        // std::apply([](auto&... nodes) { (nodes.dev.init(), ...); }, _devices);    // TODO order?
    }

    template <ConstString LABEL_V>
    constexpr auto& get() noexcept
    {
        // template <typename T>
        // using SameLabel = std::is_same<Label<LABEL_V, T>>;

        using TargetNode = typename FindByLabel<Label<LABEL_V>, typename DT_T::Nodes>::Type;
        // return []<>() {
        //     if constexpr (std::get<IDX_Vs>(_devices)) {

        //     } else {
        //     }
        // }();
        // return std::get<TargetNode>(_devices).dev;
        return get<TargetNode>();
    }

    template <ConstString LABEL_V>
    constexpr const auto& get() const noexcept
    {
        using TargetNode = typename FindByLabel<Label<LABEL_V>, typename DT_T::Nodes>::Type;
        // return std::get<TargetNode>(_devices).dev;
        return get<TargetNode>();
    }

  private:
    template <typename NODE_T>
    constexpr typename GetDriver<NODE_T>::type& get() noexcept
    {
        return *reinterpret_cast<typename GetDriver<NODE_T>::type*>(
            &_storage[0] + DriverStorage<typename DT_T::Nodes>::template offset<NODE_T>);
    }

    template <typename NODE_T>
    constexpr const typename GetDriver<NODE_T>::type& get() const noexcept
    {
        return *reinterpret_cast<const typename GetDriver<NODE_T>::type*>(
            &_storage[0] + DriverStorage<typename DT_T::Nodes>::template offset<NODE_T>);
    }

    // TODO must be aligned to biggest type inside
    alignas(
        std::max_align_t) std::array<std::byte, DriverStorage<typename DT_T::Nodes>::size> _storage{};    // TODO
                                                                                                          // calculate
                                                                                                          // size
};

// template <typename... NODE_Ts>
// static constexpr auto createDevices() noexcept
// {
//     // return std::make_tuple();
//     return std::tuple_cat(createBranch<NODE_Ts>()...);
// }

// template <typename NODE_T>
// static constexpr auto createDevice() noexcept
// {
//     using Driver = typename GetDriver<NODE_T>::Type;
//     Driver dev{};

//     if constexpr (HasChildren<NODE_T>::value) {
//         using ChildNodes = CollectChildren<NODE_T>;
//         std::tuple children = []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs..>) {
//             return std::make_tuple(createChild<std::get<IDX_Vs, children>, Driver>(dev));
//         }(std::make_index_sequence<std::tuple_size_v<ChildNodes>>());
//         // std::tuple children = []<typename... NODE_Ts>() { return std::make_tuple(createChild<NODE_Ts,
//         // Driver>(dev)...); }<>();

//         return std::tuple_cat(std::make_tuple(dev), children);
//     } else {
//         return std::make_tuple(dev);
//     }
// }

// template <typename NODE_T, typename PARENT_DRIVER_T>
// static constexpr auto createChild(PARENT_DRIVER_T& parent) noexcept
// {
//     using Driver = typename GetDriver<NODE_T>::Type;
//     return Driver<PARENT_DRIVER_T>{parent};
//     // return std::tuple_cat(std::make_tuple(Driver));

//     // if constexpr (HasChildren<NODE_T>::value) {
//     //     using Parent = NODE_T;
//     //     using Children = CollectChildren<NODE_T>;
//     // } else {

//     // }

//     // if constexpr (IsChild<NODE_T>::value) {
//     //     using Parent = typename GetParent<NODE_T, DT_T>::Type;
//     //     using Driver = typename GetDriver<NODE_T>::Type;
//     //     return Driver<Parent>{parent};
//     // } else {
//     //     using Driver = typename GetDriver<NODE_T>::Type;
//     //     return Driver{};
//     // }
// }

// template <typename... ROOT_NODE_Ts>
// static constexpr auto createDevices() noexcept
// {
//     return std::make_tuple(createBranch<ROOT_NODE_Ts>()...);
// }

// template <typename ROOT_NODES_T>
// static constexpr auto createDevices() noexcept
// {
//     return []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
//         return std::tuple_cat(createBranch<std::tuple_element_t<IDX_Vs, ROOT_NODES_T>>()...);
//     }(std::make_index_sequence<std::tuple_size_v<ROOT_NODES_T>>{});
// }

// static_assert(IsChild<Child<Tlc59208f, Label<"ledDriver2">, Address<0x21>>>::value);    // TODO test

// using Children = typename CollectChildren<Node<I2c, Label<"i2c0">, Child<Tlc59208f, Label<"ledDriver">,
// Address<0x20>>,
//                                                Child<Tlc59208f, Label<"ledDriver2">, Address<0x21>>>>::Type;
// using T = Test<Children>::Type;

template <typename... ROOT_NODE_Ts>
struct Devicetree {
    using Tree = typename CompleteNodes<Typelist<ROOT_NODE_Ts...>>::type;
    using Nodes = typename Flatten<Tree>::type;
};

// template <template <typename> typename PRED_T, typename TUPLE_T>
// struct Find;

// template <template <typename> typename PRED_T, typename FIRST_T, typename... REST_Ts>
// struct Find<PRED_T, std::tuple<FIRST_T, REST_Ts...>> {
//     using Type = typename Find<PRED_T, REST_Ts...>::Type;
// };

// template <template <typename> typename PRED_T, typename FIRST_T, typename... REST_Ts>
//     requires(PRED_T<FIRST_T>::value)
// struct Find<PRED_T, std::tuple<FIRST_T, REST_Ts...>> {
//     using Type = FIRST_T;
// };

// template<typename T, typename T>
// struct SameLabel

// template <typename DEVICETREE_T>
// class System_ {    // TODO find better name (Hardware?)
//   public:
//     constexpr System_() noexcept
//     //  : _devices()
//     { }

//     constexpr void init() noexcept
//     {
//         // TODO implement
//         // []<>() {

//         // }();

//         // std::apply([](auto&... nodes) { (nodes.dev.init(), ...); }, _devices);    // TODO order?
//     }

//     template <ConstString LABEL_V>
//     constexpr auto& get() noexcept
//     {
//         // template <typename T>
//         // using SameLabel = std::is_same<Label<LABEL_V, T>>;

//         using TargetNode = typename FindByLabel<Label<LABEL_V>, typename DEVICETREE_T::Nodes>::Type;
//         // return []<>() {
//         //     if constexpr (std::get<IDX_Vs>(_devices)) {

//         //     } else {
//         //     }
//         // }();
//         // return std::get<TargetNode>(_devices).dev;
//         return _devices.template get<TargetNode>();
//     }

//     template <ConstString LABEL_V>
//     constexpr const auto& get() const noexcept
//     {
//         using TargetNode = typename FindByLabel<Label<LABEL_V>, typename DEVICETREE_T::Nodes>::Type;
//         // return std::get<TargetNode>(_devices).dev;
//         return _devices.template get<TargetNode>();
//     }

//   private:
//     // using AssembledNodes = typename CompleteNodes<std::tuple<ROOT_NODE_Ts...>>::Type;
//     // using T = Test<typename CompleteNodes<std::tuple<ROOT_NODE_Ts...>>::Type>::Type;
//     // using DeviceNodes = typename Flatten<typename DEVICETREE_T::Nodes>::type;
//     // using T = Test<DeviceNodes>::Type;
//     // Devices _devices;
//     // DeviceNodes _devices = createDevices<typename DEVICETREE_T::Nodes>();
//     DeviceContainer<DEVICETREE_T> _devices{};
// };

// template <typename... NODE_Ts>
// static constexpr System<NODE_Ts...> createDevicetree() noexcept
// {
//     return {};
// }

#endif    // LIB_INCLUDE_DEVICETREE_HPP
