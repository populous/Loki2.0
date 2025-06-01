/**
 * @file examples/performance/origami_performance_demo.cpp
 * @brief 실제 사용 시나리오의 성능 데모
 */

#include <origami/optimized_patterns.hpp>
#include <chrono>
#include <iostream>
#include <iomanip>

using namespace metaloki::origami;
using namespace metaloki::origami::optimized;

void performance_comparison_demo() {
    std::cout << "\n=== ORIGAMI Performance Comparison Demo ===\n";
    
    constexpr size_t test_size = 10000;
    
    // 1. 일반 구현 성능
    {
        auto start = std::chrono::high_resolution_clock::now();
        
        auto document = origami_builder<int_leaf, string_leaf>::create()
            .name("Standard Implementation")
            .as_composite()
            .build();
        
        for (size_t i = 0; i < test_size; ++i) {
            document->add(int_leaf(static_cast<int>(i)));
        }
        
        accumulate_visitor<int> visitor;
        document->accept_visitor(visitor);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Standard Implementation: " << duration.count() << " μs" << std::endl;
        std::cout << "Result: " << visitor.get_result() << std::endl;
    }
    
    // 2. 최적화된 구현 성능
    {
        auto start = std::chrono::high_resolution_clock::now();
        
        auto optimized_doc = fast_builder<int_leaf, string_leaf>{}
            .add_all([test_size]() {
                std::vector<int_leaf> elements;
                elements.reserve(test_size);
                for (size_t i = 0; i < test_size; ++i) {
                    elements.emplace_back(static_cast<int>(i));
                }
                return elements;
            }()...)
            .finish();
        
        fast_visitor<int> fast_vis;
        int sum = 0;
        fast_iterator<int_leaf, string_leaf> fast_iter;
        fast_iter.fast_for_each(*optimized_doc, [&](const auto& element) {
            if constexpr (requires { element.value(); }) {
                sum += element.value();
            }
        });
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Optimized Implementation: " << duration.count() << " μs" << std::endl;
        std::cout << "Result: " << sum << std::endl;
    }
}

int main() {
    try {
        performance_comparison_demo();
        std::cout << "\n✅ Performance comparison completed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
