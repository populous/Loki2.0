/**
 * @file examples/origami/basic_composite_example.cpp
 * @brief 검색 결과 [5] 예제 코드 기반 Composite 사용법
 */

#include <origami/composite.hpp>
#include <origami/origami_composite.hpp>
#include <origami/modern_composite.hpp>
#include <iostream>
#include <string>

using namespace metaloki::origami;

// 기본 Composite 패턴 예제
void basic_composite_example() {
    std::cout << "\n=== Basic Composite Example ===\n";
    
    // 검색 결과 [5] 예제 기반 사용법
    using string_leaf = leaf<std::string>;
    using int_leaf = leaf<int>;
    using mixed_composite = composite<string_leaf, int_leaf>;
    
    // 복합 구조 생성
    mixed_composite root("Root");
    
    // 리프 노드 추가
    root.add(string_leaf("Hello"));
    root.add(int_leaf(42));
    
    // 중첩 복합 구조
    mixed_composite nested("Nested");
    nested.add(string_leaf("World"));
    nested.add(int_leaf(100));
    
    // 복합 노드에 복합 노드 추가
    using nested_composite = composite<string_leaf, int_leaf, mixed_composite>;
    nested_composite complex("Complex");
    complex.add(string_leaf("Top level"));
    complex.add(nested);
    
    // 구조 렌더링
    std::cout << "Rendering basic structure:\n";
    root.render();
    std::cout << "\n\nRendering complex structure:\n";
    complex.render();
    std::cout << std::endl;
}

// ORIGAMI 특화 Composite 예제
void origami_composite_example() {
    std::cout << "\n=== ORIGAMI Composite Example ===\n";
    
    // 검색 결과 [1] Miura-ori 패턴 기반
    origami_composite<std::string> miura_pattern("Miura-ori");
    
    // 검색 결과 [1] "corrugated structure" 생성
    std::cout << "Creating 3x3 Miura-ori pattern...\n";
    miura_pattern.create_miura_pattern(3, 3);
    
    // 노드 내용 설정
    for (size_t i = 0; i < 9; ++i) {
        miura_pattern.get_element(i) = "Node-" + std::to_string(i);
    }
    
    // 패턴 렌더링
    miura_pattern.render();
    
    // 연결 순회
    std::cout << "\nTraversing node 4 connections:\n";
    miura_pattern.visit_connections(4, [](size_t from, size_t to, 
                                      const std::string& from_element, 
                                      const std::string& to_element) {
        std::cout << from_element << " -> " << to_element << std::endl;
    });
}

// 현대적 값 기반 Composite 예제
void modern_composite_example() {
    std::cout << "\n=== Modern Composite Example ===\n";
    
    // 검색 결과 [6] 값 기반 다형성
    modern_composite<std::string> document("Document");
    
    // 내용 추가
    document.add_leaf("Title: ORIGAMI Design Patterns");
    document.add_leaf("Author: MetaLoki 2.0");
    
    // 섹션 추가
    auto& section1 = document.add_composite("Introduction");
    section1.add_leaf("This is the introduction section");
    section1.add_leaf("It contains basic information");
    
    auto& section2 = document.add_composite("Main Content");
    section2.add_leaf("This is the main content");
    section2.add_composite("Subsection").add_leaf("Nested content");
    
    // 문서 렌더링
    document.render();
    
    // 깊이 우선 순회
    std::cout << "\nDepth-first traversal:\n";
    document.traverse_depth_first(
        [](const std::string& leaf) {
            std::cout << "  Leaf: " << leaf << std::endl;
        },
        [](const std::string& name, size_t child_count) {
            std::cout << "Composite '" << name << "' with " 
                      << child_count << " children" << std::endl;
        }
    );
}

int main() {
    try {
        std::cout << "===== ORIGAMI Composite Pattern Examples =====\n";
        
        basic_composite_example();
        origami_composite_example();
        modern_composite_example();
        
        std::cout << "\n✅ All Composite examples completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
