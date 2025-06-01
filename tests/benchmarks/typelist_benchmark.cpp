/**
 * @file tests/benchmarks/typelist_benchmark.cpp
 * @brief 검색 결과 [1] 벤치마크 개선 및 성능 측정
 */

#include <core/typelist.hpp>
#include <core/typelist_performance.hpp>
#include <chrono>
#include <iostream>
#include <string>

using namespace metaloki::core;

// 테스트용 타입들
struct A {}; struct B {}; struct C {}; struct D {}; struct E {};
struct F {}; struct G {}; struct H {}; struct I {}; struct J {};

// 벤치마크 타입들
using SmallTypeList = typelist<int, double, std::string>;
using MediumTypeList = typelist<A, B, C, D, E, F, G, H>;
using LargeTypeList = typelist<A, B, C, D, E, F, G, H, I, J, 
                              int, double, float, char, bool,
                              std::string, std::vector<int>>;

/**
 * @brief 컴파일 타임 성능 측정 (검색 결과 [4] 참조)
 */
template<typename TypeList>
void benchmark_compile_time_operations() {
    std::cout << "=== Compile-time Benchmark: " << TypeList::size << " types ===" << std::endl;
    
    // contains 테스트
    auto start = std::chrono::high_resolution_clock::now();
    
    constexpr bool test1 = TypeList::template contains<int>();
    constexpr bool test2 = TypeList::template contains<double>();
    constexpr bool test3 = TypeList::template contains<std::string>();
    constexpr bool test4 = TypeList::template contains<void>(); // false
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    std::cout << "Contains operations: " << (test1 + test2 + test3 + test4) 
              << " results in " << duration.count() << " ns" << std::endl;
    
    // index_of 테스트
    start = std::chrono::high_resolution_clock::now();
    
    constexpr size_t idx1 = TypeList::template index_of<int>();
    constexpr size_t idx2 = TypeList::template index_of<double>();
    constexpr size_t idx3 = TypeList::template index_of<void>(); // not found
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    std::cout << "Index operations: " << (idx1 + idx2 + idx3) 
              << " total in " << duration.count() << " ns" << std::endl;
}

/**
 * @brief 런타임 성능 측정 (검색 결과 [7] C++20 lambdas 활용)
 */
template<typename TypeList>
void benchmark_runtime_operations() {
    std::cout << "=== Runtime Benchmark: " << TypeList::size << " types ===" << std::endl;
    
    constexpr int iterations = 1000000;
    
    // for_each_type 벤치마크
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        TypeList::for_each_type([]<typename T>() {
            // 간단한 작업 (타입 크기 계산)
            volatile size_t size = sizeof(T);
            (void)size; // 사용하지 않는 변수 경고 제거
        });
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "for_each_type (" << iterations << " iterations): " 
              << duration.count() << " μs" << std::endl;
    
    // enumerate_types 벤치마크 (검색 결과 [7] 참조)
    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        TypeList::enumerate_types([]<typename T, auto Index>() {
            volatile size_t idx = Index;
            volatile size_t size = sizeof(T);
            (void)idx; (void)size;
        });
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "enumerate_types (" << iterations << " iterations): " 
              << duration.count() << " μs" << std::endl;
}

/**
 * @brief 검색 결과 [5] "for_each_and_collect" 성능 테스트
 */
template<typename TypeList>
void benchmark_advanced_operations() {
    std::cout << "=== Advanced Operations Benchmark ===" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // for_each_and_collect 테스트
    auto result = TypeList::template for_each_and_collect<std::tuple>([]<typename T>() {
        return sizeof(T);
    });
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    std::cout << "for_each_and_collect: " << duration.count() << " ns" << std::endl;
    
    // 집합 연산 테스트
    using OtherList = typelist<int, float, char>;
    
    start = std::chrono::high_resolution_clock::now();
    
    using Intersection = typename TypeList::template intersection<OtherList>;
    using Difference = typename TypeList::template difference<OtherList>;
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    std::cout << "Set operations: " << Intersection::size << " intersection, " 
              << Difference::size << " difference in " << duration.count() << " ns" << std::endl;
}

int main() {
    std::cout << "🚀 MetaLoki 2.0 TypeList Performance Benchmark" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // 컴파일 타임 벤치마크
    benchmark_compile_time_operations<SmallTypeList>();
    benchmark_compile_time_operations<MediumTypeList>();
    benchmark_compile_time_operations<LargeTypeList>();
    
    std::cout << std::endl;
    
    // 런타임 벤치마크
    benchmark_runtime_operations<SmallTypeList>();
    benchmark_runtime_operations<MediumTypeList>();
    benchmark_runtime_operations<LargeTypeList>();
    
    std::cout << std::endl;
    
    // 고급 기능 벤치마크
    benchmark_advanced_operations<MediumTypeList>();
    
    std::cout << "\n✅ 모든 벤치마크 완료!" << std::endl;
    
    return 0;
}
