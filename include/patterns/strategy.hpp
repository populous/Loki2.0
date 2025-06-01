/**
 * @file include/patterns/strategy.hpp
 * @brief 검색 결과 [1] "Context + Strategy Interface" 구현
 * @details MetaLoki 2.0 Strategy Pattern - TypeList + Policy Host 통합
 */

#pragma once

#include <core/typelist.hpp>
#include <core/policy_host.hpp>
#include <memory>
#include <variant>
#include <concepts>
#include <functional>

namespace patterns {
    
    /**
     * @brief 검색 결과 [4] "Policy classes" 개념 적용
     * @details Strategy가 구현해야 할 기본 개념
     */
    template<typename T, typename... Args>
    concept Strategy = requires(T strategy, Args... args) {
        { strategy.execute(args...) } -> std::same_as<void>;
    } || requires(T strategy, Args... args) {
        strategy.execute(args...);
    };
    
    /**
     * @brief 검색 결과 [1] "Context delegates the work to a linked strategy object"
     * @details TypeList 기반 Modern Strategy Context
     */
    template<Strategy... StrategyTypes>
    class strategy_context : public core::policy_host<
        core::policies::single_thread_policy,
        core::policies::validation_policy
    > {
        using policy_base = core::policy_host<
            core::policies::single_thread_policy,
            core::policies::validation_policy
        >;
        
    public:
        using strategy_list = core::typelist<StrategyTypes...>;
        using strategy_variant = std::variant<StrategyTypes...>;
        
    private:
        strategy_variant current_strategy_;
        
    public:
        /**
         * @brief 검색 결과 [1] "context exposes a setter" 구현
         */
        template<Strategy StrategyType>
        void set_strategy(StrategyType&& strategy) {
            static_assert(strategy_list::template contains<std::decay_t<StrategyType>>(), 
                "StrategyType must be in the strategy list");
            
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            current_strategy_ = std::forward<StrategyType>(strategy);
        }
        
        /**
         * @brief 검색 결과 [1] "context calls the execution method" 구현
         */
        template<typename... Args>
        auto execute(Args&&... args) {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            
            return std::visit([&args...](auto&& strategy) {
                return strategy.execute(std::forward<Args>(args)...);
            }, current_strategy_);
        }
        
        /**
         * @brief 검색 결과 [1] "context doesn't know what type of strategy" 확인
         */
        template<Strategy StrategyType>
        bool is_current_strategy() const {
            return std::holds_alternative<StrategyType>(current_strategy_);
        }
        
        /**
         * @brief 현재 전략 타입 조회
         */
        size_t get_strategy_index() const {
            return current_strategy_.index();
        }
    };
    
    /**
     * @brief 검색 결과 [2] "Policy-Based Design" C++ 스타일 구현
     * @details 컴파일 타임 Strategy (Policy-Based Design)
     */
    template<template<typename...> class StrategyPolicy, typename... PolicyArgs>
    class compile_time_strategy : public StrategyPolicy<PolicyArgs...>,
                                 public core::policy_host<
                                     core::policies::single_thread_policy
                                 > {
        using strategy_type = StrategyPolicy<PolicyArgs...>;
        using policy_base = core::policy_host<core::policies::single_thread_policy>;
        
    public:
        using strategy_type::execute; // Strategy 인터페이스 노출
        
        /**
         * @brief 검색 결과 [2] "configured behavior" 구현
         */
        template<typename... Args>
        constexpr auto execute_strategy(Args&&... args) {
            return strategy_type::execute(std::forward<Args>(args)...);
        }
    };
    
    /**
     * @brief 검색 결과 [3] "Policy-Based Design solves the same problem" 
     * @details 하이브리드 접근법 - 런타임 + 컴파일타임
     */
    template<typename ResultType = void, Strategy... StrategyTypes>
    class hybrid_strategy_context {
    public:
        using strategy_list = core::typelist<StrategyTypes...>;
        using strategy_variant = std::variant<StrategyTypes...>;
        using result_type = ResultType;
        
    private:
        strategy_variant current_strategy_;
        
    public:
        /**
         * @brief 런타임 Strategy 설정
         */
        template<Strategy StrategyType>
        void set_runtime_strategy(StrategyType&& strategy) {
            static_assert(strategy_list::template contains<std::decay_t<StrategyType>>(), 
                "StrategyType must be in the strategy list");
            
            current_strategy_ = std::forward<StrategyType>(strategy);
        }
        
        /**
         * @brief 런타임 실행
         */
        template<typename... Args>
        result_type execute_runtime(Args&&... args) {
            return std::visit([&args...](auto&& strategy) -> result_type {
                if constexpr (std::is_void_v<result_type>) {
                    strategy.execute(std::forward<Args>(args)...);
                } else {
                    return strategy.execute(std::forward<Args>(args)...);
                }
            }, current_strategy_);
        }
        
        /**
         * @brief 검색 결과 [2] "templates" 컴파일 타임 실행
         */
        template<Strategy StrategyType, typename... Args>
        constexpr result_type execute_compile_time(Args&&... args) {
            static_assert(strategy_list::template contains<StrategyType>(), 
                "StrategyType must be in the strategy list");
            
            StrategyType strategy{};
            if constexpr (std::is_void_v<result_type>) {
                strategy.execute(std::forward<Args>(args)...);
            } else {
                return strategy.execute(std::forward<Args>(args)...);
            }
        }
    };
}
