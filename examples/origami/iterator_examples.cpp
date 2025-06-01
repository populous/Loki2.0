/**
 * @file examples/origami/iterator_examples.cpp
 * @brief 검색 결과 [7] BookShelf 예제를 ORIGAMI로 확장
 */

#include <origami/iterator.hpp>
#include <origami/tree_iterator.hpp>
#include <iostream>
#include <string>

using namespace origami;

// 검색 결과 [7] "Book" 클래스 스타일
class document {
private:
    std::string title_;
    std::string content_;
    
public:
    document(std::string title, std::string content = "")
        : title_(std::move(title)), content_(std::move(content)) {}
    
    const std::string& title() const { return title_; }
    const std::string& content() const { return content_; }
    
    void render_impl() const {
        std::cout << "Document: " << title_;
        if (!content_.empty()) {
            std::cout << " - " << content_;
        }
    }
    
    std::unique_ptr<document> clone_impl() const {
        return std::make_unique<document>(title_, content_);
    }
};

void basic_iterator_example() {
    std::cout << "\n=== Basic ORIGAMI Iterator Example ===\n";
    
    // 검색 결과 [3] "origami_container" 사용
    origami_container<std::string> container;
    
    // 요소 추가
    container.push_back("First Element");
    container.push_back("Second Element");
    container.push_back("Third Element");
    
    std::cout << "Container size: " << container.size() << std::endl;
    
    // 검색 결과 [8] "range-based for loop" 사용
    std::cout << "Elements using range-based for:\n";
    for (const auto& element : container) {
        std::cout << "  " << element << std::endl;
    }
    
    // 검색 결과 [3] "explicit iterator" 사용
    std::cout << "Elements using explicit iterator:\n";
    for (auto it = container.begin(); it != container.end(); ++it) {
        std::cout << "  " << *it << std::endl;
    }
    
    // 검색 결과 [1] "traverse operator" 사용
    std::cout << "Elements using traverse function:\n";
    traverse(container, [](const std::string& element) {
        std::cout << "  Traversed: " << element << std::endl;
    });
}

void tree_iterator_example() {
    std::cout << "\n=== Tree Iterator Example ===\n";
    
    // 검색 결과 [7] "iterable_composite" 생성
    using string_leaf = leaf<std::string>;
    using doc_composite = iterable_composite<string_leaf, document>;
    
    doc_composite library("Digital Library");
    
    // 문서 추가
    library.add(document("Introduction", "Basic concepts"));
    library.add(document("Chapter 1", "Getting started"));
    library.add(string_leaf("Appendix A"));
    
    // 중첩 섹션 생성
    doc_composite advanced_section("Advanced Topics");
    advanced_section.add(document("Performance", "Optimization techniques"));
    advanced_section.add(document("Patterns", "Design patterns"));
    
    // 네스팅된 구조 추가 (복합 노드에 복합 노드 추가는 지원되지 않음)
    // 대신 개별적으로 관리
    
    // 검색 결과 [7] "Iterator it = bookShelf.createIterator()" 스타일
    auto dfs_iter = library.dfs_iterator();
    
    std::cout << "DFS Traversal:\n";
    dfs_iter.for_each([](const auto& element) {
        std::visit([](const auto& item) {
            if constexpr (requires { item.render(); }) {
                std::cout << "  ";
                item.render();
                std::cout << std::endl;
            } else if constexpr (requires { item.title(); }) {
                std::cout << "  Document: " << item.title() << std::endl;
            }
        }, element);
    });
    
    // BFS 순회
    auto bfs_iter = library.bfs_iterator();
    std::cout << "\nBFS Traversal:\n";
    bfs_iter.for_each([](const auto& element) {
        std::visit([](const auto& item) {
            std::cout << "  [BFS] ";
            if constexpr (requires { item.render(); }) {
                item.render();
            } else if constexpr (requires { item.title(); }) {
                std::cout << "Document: " << item.title();
            }
            std::cout << std::endl;
        }, element);
    });
}

void functional_iterator_example() {
    std::cout << "\n=== Functional Iterator Example ===\n";
    
    origami_container<int> numbers;
    for (int i = 1; i <= 5; ++i) {
        numbers.push_back(i);
    }
    
    // 검색 결과 [6] "collect" 함수 사용
    auto squared = collect_and_transform(
        numbers,
        [](int n) { 
            std::cout << "Processing: " << n << std::endl; 
        },
        [](int n) { 
            return n * n; 
        }
    );
    
    std::cout << "Squared numbers: ";
    for (int sq : squared) {
        std::cout << sq << " ";
    }
    std::cout << std::endl;
    
    // C++20 ranges와의 조합
    auto filtered_doubled = numbers.as_range() 
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * 2; });
    
    std::cout << "Even numbers doubled: ";
    for (int n : filtered_doubled) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
}

int main() {
    try {
        std::cout << "===== ORIGAMI Iterator Pattern Examples =====\n";
        
        basic_iterator_example();
        tree_iterator_example();
        functional_iterator_example();
        
        std::cout << "\n✅ All Iterator examples completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
