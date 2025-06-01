/**
 * @file examples/origami/visitor_examples.cpp
 * @brief 검색 결과 [1] "Car 예제" ORIGAMI 확장
 */

#include <origami/visitor.hpp>
#include <origami/modern_visitor.hpp>
#include <origami/advanced_visitor.hpp>
#include <iostream>
#include <string>

using namespace metaloki::origami;

// 검색 결과 [4] "Item, ItemList" 스타일의 ORIGAMI 요소들
class document_element : public visitable_base<document_element> {
private:
    std::string content_;
    
public:
    explicit document_element(std::string content) : content_(std::move(content)) {}
    
    const std::string& content() const { return content_; }
    
    void render_impl() const {
        std::cout << "Document: " << content_;
    }
    
    std::unique_ptr<document_element> clone_impl() const {
        return std::make_unique<document_element>(content_);
    }
};

void basic_visitor_example() {
    std::cout << "\n=== Basic ORIGAMI Visitor Example ===\n";
    
    // 검색 결과 [4] "SumVisitor" 스타일 구조 생성
    using int_leaf = leaf<int>;
    using string_leaf = leaf<std::string>;
    using doc_composite = composite<int_leaf, string_leaf, document_element>;
    
    doc_composite document("Main Document");
    document.add(int_leaf(42));
    document.add(string_leaf("Hello"));
    document.add(document_element("ORIGAMI Visitor"));
    
    // 검색 결과 [4] "SumVisitor" 적용
    accumulate_visitor<int> sum_visitor;
    document.accept_visitor(sum_visitor);
    
    std::cout << "Sum of numeric values: " << sum_visitor.get_result() << std::endl;
    
    // 검색 결과 [4] "AvgVisitor" 적용
    average_visitor<int> avg_visitor;
    document.accept_visitor(avg_visitor);
    
    std::cout << "Average: " << avg_visitor.get_average() 
              << " (count: " << avg_visitor.get_count() << ")" << std::endl;
}

void modern_visitor_example() {
    std::cout << "\n=== Modern Variant Visitor Example ===\n";
    
    // 검색 결과 [2] "test visitor object" 스타일
    using int_leaf = leaf<int>;
    using string_leaf = leaf<std::string>;
    
    variant_visitor<int_leaf, string_leaf> visitor;
    
    // 각 타입에 대한 visitor 등록
    visitor.register_visitor_for_type<int_leaf>([](const int_leaf& element) {
        std::cout << "Visiting int leaf: " << element.value() << std::endl;
    });
    
    visitor.register_visitor_for_type<string_leaf>([](const string_leaf& element) {
        std::cout << "Visiting string leaf: " << element.value() << std::endl;
    });
    
    // 검색 결과 [2] "test that it visited the right things"
    visitor.enable_visit_tracking();
    
    int_leaf num(100);
    string_leaf text("Test");
    
    visitor.visit(num);
    visitor.visit(text);
    
    std::cout << "Visit history size: " << visitor.get_visit_history().size() << std::endl;
}

void mock_visitor_example() {
    std::cout << "\n=== Mock Visitor Testing Example ===\n";
    
    // 검색 결과 [2] "mock objects" 사용
    mock_visitor<std::string> mock;
    
    using int_leaf = leaf<int>;
    using string_leaf = leaf<std::string>;
    
    // Mock 결과 설정
    mock.set_result_for_type(typeid(int_leaf).name(), "Processed int leaf");
    mock.set_result_for_type(typeid(string_leaf).name(), "Processed string leaf");
    
    int_leaf num(42);
    string_leaf text("Hello");
    
    auto result1 = mock.visit(num);
    auto result2 = mock.visit(text);
    
    std::cout << "Mock result 1: " << result1 << std::endl;
    std::cout << "Mock result 2: " << result2 << std::endl;
    
    // 검색 결과 [2] "test that the right actions were performed"
    std::cout << "Was int_leaf visited? " << 
        (mock.was_visited(typeid(int_leaf).name()) ? "Yes" : "No") << std::endl;
    
    const auto& log = mock.get_visit_log();
    std::cout << "Total visits: " << log.size() << std::endl;
}

void advanced_tree_visitor_example() {
    std::cout << "\n=== Advanced Tree Visitor Example ===\n";
    
    using int_leaf = leaf<int>;
    using nested_composite = composite<int_leaf>;
    
    nested_composite root("Root");
    
    // 중첩 구조 생성
    nested_composite section1("Section 1");
    section1.add(int_leaf(10));
    section1.add(int_leaf(20));
    
    nested_composite section2("Section 2");
    section2.add(int_leaf(30));
    
    root.add_copy(section1);
    root.add_copy(section2);
    root.add(int_leaf(5));
    
    // 값 수집 visitor
    collect_values_visitor<int> collector;
    collector.set_traversal_mode(tree_visitor<std::vector<int>>::traversal_mode::breadth_first);
    
    root.accept_visitor(collector);
    
    const auto& values = collector.get_results();
    std::cout << "Collected values: ";
    for (const auto& value_vec : values) {
        for (int val : value_vec) {
            std::cout << val << " ";
        }
    }
    std::cout << std::endl;
    
    // 노드 카운터
    node_counter_visitor counter;
    auto total_nodes = root.accept_visitor(counter);
    std::cout << "Total nodes: " << counter.get_total_count() << std::endl;
    
    // 깊이 계산
    depth_calculator_visitor depth_calc;
    auto max_depth = root.accept_visitor(depth_calc);
    std::cout << "Maximum depth: " << max_depth << std::endl;
}

int main() {
    try {
        std::cout << "===== ORIGAMI Visitor Pattern Examples =====\n";
        
        basic_visitor_example();
        modern_visitor_example();
        mock_visitor_example();
        advanced_tree_visitor_example();
        
        std::cout << "\n✅ All Visitor examples completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
