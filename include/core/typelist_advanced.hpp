/**
 * @file include/core/typelist_advanced.hpp
 * @brief 검색 결과 [2] VTLL 기반 고급 기능 구현
 * @details "Vienna Type List Library" 알고리즘 참조
 */


#pragma once

#include <metaloki/core/typelist.hpp>
#include <variant>
#include <array>

namespace metaloki::core {
    
    /**
     * @brief 검색 결과 [2] VTLL 고급 기능 확장
     */
    template<typename... Types>
    class typelist {
        // 기존 기본 기능들...
        
    public:
        /**
         * @brief 검색 결과 [2] "front/back" 구현
         */
        using front = std::conditional_t<(size > 0), at<0>, void>;
        using back = std::conditional_t<(size > 0), at<size - 1>, void>;
        
        /**
         * @brief 검색 결과 [2] "sublist" 구현
         */
        template<size_t Start, size_t Count = size - Start>
        using sublist = typename detail::sublist_impl<Start, Count, Types...>::type;
        
        /**
         * @brief 검색 결과 [2] "to_variant" 구현
         */
        using to_variant = std::variant<Types...>;
        
        /**
         * @brief 검색 결과 [2] "to_ref/to_const_ref/to_ptr" 구현
         */
        using to_ref = typelist<Types&...>;
        using to_const_ref = typelist<const Types&...>;
        using to_ptr = typelist<Types*...>;
        
        /**
         * @brief 검색 결과 [2] "transform_size_t" - std::array 지원
         */
        template<template<typename, size_t> class ArrayLike, size_t Size>
        using transform_array = typelist<ArrayLike<Types, Size>...>;
        
        /**
         * @brief 검색 결과 [2] "transform_front/transform_back" 구현
         */
        template<typename T, template<typename, typename> class BinaryOp>
        using transform_front = typelist<BinaryOp<T, Types>...>;
        
        template<typename T, template<typename, typename> class BinaryOp>
        using transform_back = typelist<BinaryOp<Types, T>...>;
        
        /**
         * @brief 검색 결과 [2] "erase_Nth" 구현
         */
        template<size_t Index>
        using erase_at = typename detail::erase_at_impl<Index, Types...>::type;
        
        using pop_front = erase_at<0>;
        using pop_back = erase_at<size - 1>;
        
        /**
         * @brief 검색 결과 [5] "keep" - 인덱스 기반 필터링
         */
        template<size_t... Indices>
        using keep = typelist<at<Indices>...>;
        
        /**
         * @brief 검색 결과 [5] "enumerate" - 타입과 인덱스 조합
         */
        template<typename T, size_t Index>
        struct type_index_pair {
            using type = T;
            static constexpr size_t index = Index;
        };
        
        using enumerate = typename detail::enumerate_impl<
            std::index_sequence_for<Types...>, Types...>::type;
        
        /**
         * @brief 검색 결과 [7] "C++20 lambdas compiletime" 활용
         */
        template<typename F>
        static constexpr void enumerate_types(F&& f) {
            [&f]<auto... Is>(std::index_sequence<Is...>) {
                (f.template operator()<Types, Is>(), ...);
            }(std::index_sequence_for<Types...>{});
        }
        
        /**
         * @brief 검색 결과 [5] "for_each_and_collect" 구현
         */
        template<template<typename...> class Container, typename F>
        static constexpr auto for_each_and_collect(F&& f) {
            return Container{f.template operator()<Types>()...};
        }
        
        /**
         * @brief 고성능 find_if (컴파일 타임 최적화)
         */
        template<template<typename> class Predicate>
        static constexpr size_t find_if() {
            return detail::find_if_impl<Predicate, 0, Types...>::value;
        }
        
        /**
         * @brief 타입 집합 연산들
         */
        template<typename OtherTypeList>
        using intersection = typename detail::intersection_impl<typelist, OtherTypeList>::type;
        
        template<typename OtherTypeList>
        using difference = typename detail::difference_impl<typelist, OtherTypeList>::type;
        
        template<typename OtherTypeList>
        using symmetric_difference = typename detail::symmetric_difference_impl<typelist, OtherTypeList>::type;
        
        /**
         * @brief 검색 결과 [2] "is_same_list" 구현
         */
        template<typename OtherTypeList>
        static constexpr bool is_same_list() {
            return std::is_same_v<typelist, OtherTypeList>;
        }
    };
}

namespace metaloki::core::detail {
    
    /**
     * @brief 고급 기능 구현 상세
     */
    
    // sublist 구현 (검색 결과 [2] 참조)
    template<size_t Start, size_t Count, typename... Types>
    struct sublist_impl {
        template<size_t... Is>
        static auto make_sublist(std::index_sequence<Is...>) 
            -> typelist<std::tuple_element_t<Start + Is, std::tuple<Types...>>...>;
        
        using type = decltype(make_sublist(std::make_index_sequence<Count>{}));
    };
    
    // erase_at 구현
    template<size_t Index, typename... Types>
    struct erase_at_impl {
        template<size_t... Before, size_t... After>
        static auto make_erased(std::index_sequence<Before...>, std::index_sequence<After...>)
            -> typelist<std::tuple_element_t<Before, std::tuple<Types...>>...,
                       std::tuple_element_t<Index + 1 + After, std::tuple<Types...>>...>;
        
        using type = decltype(make_erased(
            std::make_index_sequence<Index>{},
            std::make_index_sequence<sizeof...(Types) - Index - 1>{}
        ));
    };
    
    // enumerate 구현 (검색 결과 [5] 참조)
    template<typename IndexSeq, typename... Types>
    struct enumerate_impl;
    
    template<size_t... Is, typename... Types>
    struct enumerate_impl<std::index_sequence<Is...>, Types...> {
        using type = typelist<typelist<Types...>::template type_index_pair<Types, Is>...>;
    };
    
    // find_if 고성능 구현
    template<template<typename> class Predicate, size_t Index, typename... Types>
    struct find_if_impl;
    
    template<template<typename> class Predicate, size_t Index>
    struct find_if_impl<Predicate, Index> {
        static constexpr size_t value = Index; // not found
    };
    
    template<template<typename> class Predicate, size_t Index, typename Head, typename... Tail>
    struct find_if_impl<Predicate, Index, Head, Tail...> {
        static constexpr size_t value = Predicate<Head>::value ? 
            Index : find_if_impl<Predicate, Index + 1, Tail...>::value;
    };
}

