/**
 * @file examples/policy_host_examples.cpp
 * @brief 검색 결과 [4] HelloWorld 스타일 Policy Host 활용
 */

#include <metaloki/core/policy_host.hpp>
#include <metaloki/core/policy_concepts.hpp>
#include <iostream>

using namespace metaloki::core;
using namespace metaloki::core::policies;

/**
 * @brief 검색 결과 [1] "Host classes" 실제 구현 예시
 */
template<ThreadingPolicy ThreadPolicy, 
         MemoryPolicy MemoryPolicy, 
         ValidationPolicy ValidPolicy,
         LoggingPolicy LogPolicy>
class pattern_host : public policy_host<ThreadPolicy, MemoryPolicy, ValidPolicy, LogPolicy> {
    using base = policy_host<ThreadPolicy, MemoryPolicy, ValidPolicy, LogPolicy>;
    
public:
    using base::base;  // 생성자 상속
    
    /**
     * @brief 검색 결과 [4] "behavior method" - 실제 비즈니스 로직
     */
    template<typename PatternImpl>
    void execute_pattern() {
        // 검색 결과 [1] Policy methods 활용
        auto lock = this->template get_policy<ThreadPolicy>().get_lock();
        
        this->template get_policy<ValidPolicy>().assert_that(
            true, "Pattern execution started");
        
        this->template get_policy<LogPolicy>().template log<typename LogPolicy::level::info>(
            "Executing pattern...");
        
        // 실제 패턴 실행
        PatternImpl pattern;
        pattern.execute();
        
        this->template get_policy<LogPolicy>().template log<typename LogPolicy::level::info>(
            "Pattern execution completed");
    }
    
    /**
     * @brief 검색 결과 [4] "orthogonal design decisions" 분리
     */
    template<typename T>
    std::unique_ptr<T> create_object() {
        auto* raw_ptr = this->template get_policy<MemoryPolicy>().template allocate<T>(1);
        
        this->template get_policy<ValidPolicy>().assert_that(
            raw_ptr != nullptr, "Memory allocation failed");
        
        return std::unique_ptr<T>(new(raw_ptr) T{});
    }
};

// 검색 결과 [4] "using typedef" - 편의 별칭들
using StandardPatternHost = pattern_host<
    single_thread_policy,
    cpu_memory_policy, 
    validation_policy,
    logging_policy
>;

using PerformancePatternHost = pattern_host<
    single_thread_policy,
    cpu_memory_policy,
    policies::no_validation_policy,  // 성능을 위해 검증 제거
    policies::no_logging_policy      // 성능을 위해 로깅 제거
>;

// 사용 예시
void example_usage() {
    std::cout << "=== MetaLoki 2.0 Policy Host Examples ===" << std::endl;
    
    // 검색 결과 [4] "Example 1" 스타일
    StandardPatternHost standard_host;
    
    // Policy 정보 출력
    std::cout << "Policy count: " << standard_host.policy_count << std::endl;
    std::cout << "Single thread: " << standard_host.is_single_thread << std::endl;
    std::cout << "CPU only: " << standard_host.is_cpu_only << std::endl;
    
    // Policy 개별 접근
    const auto& thread_policy = standard_host.get_policy<single_thread_policy>();
    const auto& memory_policy = standard_host.get_policy<cpu_memory_policy>();
    
    std::cout << "Thread safe: " << thread_policy.is_thread_safe << std::endl;
    std::cout << "Cache line size: " << memory_policy.cache_line_size << std::endl;
    
    // 실제 패턴 실행 (가상의 패턴)
    struct DummyPattern {
        void execute() { 
            std::cout << "Pattern executed!" << std::endl; 
        }
    };
    
    standard_host.execute_pattern<DummyPattern>();
}
