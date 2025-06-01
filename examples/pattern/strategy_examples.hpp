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

/**
 * @file examples/patterns/strategy_examples.cpp
 * @brief 검색 결과 [2] "three concrete strategies" 확장 예제
 */

#include <metaloki/patterns/strategy.hpp>
#include <metaloki/patterns/strategy_algorithms.hpp>
#include <metaloki/patterns/policy_strategies.hpp>
#include <iostream>
#include <string>

using namespace metaloki::patterns;
using namespace metaloki::patterns::algorithms;
using namespace metaloki::patterns::policies;

void basic_strategy_example() {
    std::cout << "\n=== Basic Strategy Example ===\n";
    
    // 검색 결과 [2] "Context k; k.setStrategy" 스타일
    strategy_context<addition_strategy, multiplication_strategy, power_strategy> context;
    
    // 검색 결과 [1] "client passes the desired strategy"
    context.set_strategy(addition_strategy{});
    auto result1 = context.execute(5, 3);
    std::cout << "Addition result: " << result1 << std::endl;
    
    context.set_strategy(multiplication_strategy{});
    auto result2 = context.execute(5, 3);
    std::cout << "Multiplication result: " << result2 << std::endl;
    
    context.set_strategy(power_strategy{});
    auto result3 = context.execute(5.0, 3.0);
    std::cout << "Power result: " << result3 << std::endl;
}

void compile_time_strategy_example() {
    std::cout << "\n=== Compile-time Strategy Example ===\n";
    
    // 검색 결과 [2] "Policy-Based Design" 구현
    math_calculator<addition_strategy> add_calc;
    auto add_result = add_calc.calculate(10, 5);
    std::cout << "Compile-time addition: " << add_result << std::endl;
    
    math_calculator<multiplication_strategy> mul_calc;
    auto mul_result = mul_calc.calculate(10, 5);
    std::cout << "Compile-time multiplication: " << mul_result << std::endl;
    
    // 로깅 테스트
    add_calc.log_operation("Testing addition strategy");
    mul_calc.log_operation("Testing multiplication strategy");
}

void hybrid_strategy_example() {
    std::cout << "\n=== Hybrid Strategy Example ===\n";
    
    // 검색 결과 [3] "runtime and compile-time" 조합
    hybrid_strategy_context<double, addition_strategy, multiplication_strategy> hybrid;
    
    // 런타임 사용
    hybrid.set_runtime_strategy(addition_strategy{});
    auto runtime_result = hybrid.execute_runtime(7.5, 2.5);
    std::cout << "Runtime addition: " << runtime_result << std::endl;
    
    // 컴파일 타임 사용
    auto compile_result = hybrid.execute_compile_time<multiplication_strategy>(7.5, 2.5);
    std::cout << "Compile-time multiplication: " << compile_result << std::endl;
}

void policy_container_example() {
    std::cout << "\n=== Policy Container Example ===\n";
    
    // 검색 결과 [2] "default values for the policy parameters"
    policy_container<int> standard_container;
    standard_container.push_back(1);
    standard_container.push_back(2);
    standard_container.log_allocator_info("Standard allocator used");
    
    // 풀 할당자 사용
    policy_container<int, pool_allocator_policy> pool_container;
    pool_container.push_back(10);
    pool_container.push_back(20);
    pool_container.log_allocator_info("Pool allocator used");
    
    // 해시 테스트
    auto hash1 = standard_container.hash_element(42);
    auto hash2 = pool_container.hash_element(42);
    std::cout << "Hash values: " << hash1 << ", " << hash2 << std::endl;
}

void dynamic_calculator_example() {
    std::cout << "\n=== Dynamic Calculator Example ===\n";
    
    // 검색 결과 [1] "massive conditional statement" 해결
    dynamic_calculator calc;
    
    calc.set_operation(operation_type::ADD);
    auto add_result = calc.calculate(15, 25);
    calc.log_operation("Dynamic addition operation");
    std::cout << "Dynamic addition: " << add_result << std::endl;
    
    calc.set_operation(operation_type::MULTIPLY);
    auto mul_result = calc.calculate(6, 7);
    calc.log_operation("Dynamic multiplication operation");
    std::cout << "Dynamic multiplication: " << mul_result << std::endl;
}

int main() {
    try {
        std::cout << "===== Strategy Pattern Examples =====\n";
        
        basic_strategy_example();
        compile_time_strategy_example();
        hybrid_strategy_example();
        policy_container_example();
        dynamic_calculator_example();
        
        std::cout << "\n✅ All Strategy examples completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}


