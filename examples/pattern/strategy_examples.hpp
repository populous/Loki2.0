/**
 * @file include/patterns/strategy_example.hpp
 * @brief 검색 결과 [1] "arithmetic operations" 확장 구현
 */

#pragma once

#include <patterns/strategy.hpp>
#include <cmath>
#include <string>
#include <iostream>

namespace metaloki::patterns::algorithms {
    
    /**
     * @brief 검색 결과 [1] "different variants of an algorithm" 구현
     * @details 수학 연산 전략들
     */
    
    // 검색 결과 [2] "ConcreteStrategy implements an algorithm"
    struct addition_strategy {
        template<typename T>
        constexpr T execute(T a, T b) const {
            return a + b;
        }
        
        void execute(const std::string& msg) const {
            std::cout << "Addition: " << msg << std::endl;
        }
    };
    
    struct multiplication_strategy {
        template<typename T>
        constexpr T execute(T a, T b) const {
            return a * b;
        }
        
        void execute(const std::string& msg) const {
            std::cout << "Multiplication: " << msg << std::endl;
        }
    };
    
    struct power_strategy {
        template<typename T>
        T execute(T base, T exponent) const {
            return std::pow(base, exponent);
        }
        
        void execute(const std::string& msg) const {
            std::cout << "Power: " << msg << std::endl;
        }
    };
    
    /**
     * @brief 검색 결과 [2] "Default values for the policy parameters" 구현
     * @details 정책 기반 수학 계산기
     */
    template<typename OperationPolicy = addition_strategy>
    class math_calculator : public compile_time_strategy<
        OperationPolicy,
        core::policies::single_thread_policy
    > {
        using base = compile_time_strategy<OperationPolicy, core::policies::single_thread_policy>;
        
    public:
        template<typename T>
        constexpr T calculate(T a, T b) {
            return base::execute(a, b);
        }
        
        void log_operation(const std::string& description) {
            base::execute(description);
        }
    };
    
    /**
     * @brief 검색 결과 [1] "massive conditional statement" 대체
     * @details 조건문 지옥을 Strategy로 해결
     */
    enum class operation_type {
        ADD, MULTIPLY, POWER
    };
    
    class dynamic_calculator {
    private:
        strategy_context<addition_strategy, multiplication_strategy, power_strategy> context_;
        
    public:
        void set_operation(operation_type op) {
            // 검색 결과 [1] "do away with such a conditional"
            switch (op) {
                case operation_type::ADD:
                    context_.set_strategy(addition_strategy{});
                    break;
                case operation_type::MULTIPLY:
                    context_.set_strategy(multiplication_strategy{});
                    break;
                case operation_type::POWER:
                    context_.set_strategy(power_strategy{});
                    break;
            }
        }
        
        template<typename T>
        T calculate(T a, T b) {
            return context_.execute(a, b);
        }
        
        void log_operation(const std::string& msg) {
            context_.execute(msg);
        }
    };
    
    /**
     * @brief 검색 결과 [3] "behaviors to be modified, extended, or replaced" 구현
     * @details 확장 가능한 Strategy 시스템
     */
    template<typename... Strategies>
    class extensible_strategy_system {
        using context_type = strategy_context<Strategies...>;
        context_type context_;
        
    public:
        template<typename StrategyType>
        void add_strategy(StrategyType&& strategy) {
            context_.set_strategy(std::forward<StrategyType>(strategy));
        }
        
        template<typename... Args>
        auto execute_current(Args&&... args) {
            return context_.execute(std::forward<Args>(args)...);
        }
        
        template<typename StrategyType, typename... Args>
        auto execute_specific(StrategyType&& strategy, Args&&... args) {
            context_.set_strategy(std::forward<StrategyType>(strategy));
            return context_.execute(std::forward<Args>(args)...);
        }
    };
}
