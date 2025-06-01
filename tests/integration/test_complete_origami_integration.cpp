/**
 * @file tests/integration/test_complete_origami_integration.cpp
 * @brief 검색 결과 [1] "Composite and Visitor together" 확장
 * @details ORIGAMI 4패턴 완전 통합 테스트
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <origami/composite.hpp>
#include <origami/iterator.hpp>
#include <origami/visitor.hpp>
#include <origami/builder.hpp>
#include <origami/advanced_builder.hpp>
#include <iostream>
#include <string>
#include <chrono>

using namespace metaloki::origami;

// 검색 결과 [1] 기반 테스트 타입들
using int_leaf = leaf<int>;
using string_leaf = leaf<std::string>;
using doc_composite = iterable_composite<int_leaf, string_leaf>;

TEST_SUITE("ORIGAMI Integration Tests") {
    
    TEST_CASE("Composite + Visitor Integration") {
        // 검색 결과 [1] "Create a composite document structure" 구현
        doc_composite document("Root Document");
        document.add(int_leaf(10));
        document.add(string_leaf("Hello"));
        
        // 검색 결과 [1] "Nested composite" 구현
        doc_composite section("Section 1");
        section.add(int_leaf(20));
        section.add(string_leaf("World"));
        
        document.add_copy(section);
        
        // 검색 결과 [1] "Visitor to accumulate integer values"
        accumulate_visitor<int> visitor;
        document.accept_visitor(visitor);
        
        CHECK(visitor.get_result() == 30); // 10 + 20
        
        // 추가 검증: 평균 계산 visitor
        average_visitor<int> avg_visitor;
        document.accept_visitor(avg_visitor);
        
        CHECK(avg_visitor.get_average() == doctest::Approx(15.0));
        CHECK(avg_visitor.get_count() == 2);
    }
    
    TEST_CASE("Builder + Composite + Visitor Integration") {
        // Builder로 복합 구조 생성
        auto document = origami_builder<int_leaf, string_leaf>::create()
            .name("Integration Test Document")
            .as_composite()
            .with_description("Builder + Composite integration")
            .with_capacity(10)
            .build();
        
        // 동적으로 컨텐츠 추가
        document->add(int_leaf(100));
        document->add(string_leaf("Builder"));
        document->add(int_leaf(200));
        document->add(string_leaf("Test"));
        
        // Visitor로 검증
        accumulate_visitor<int> sum_visitor;
        document->accept_visitor(sum_visitor);
        
        CHECK(sum_visitor.get_result() == 300);
        
        // 노드 카운터 visitor로 총 개수 확인
        node_counter_visitor counter;
        auto total_nodes = document->accept_visitor(counter);
        
        CHECK(counter.get_total_count() >= 4); // 최소 4개 노드
    }
    
    TEST_CASE("Iterator + Visitor Integration") {
        // 복합 구조 생성
        doc_composite library("Digital Library");
        library.add(int_leaf(1));
        library.add(string_leaf("Book1"));
        library.add(int_leaf(2));
        library.add(string_leaf("Book2"));
        
        // DFS Iterator로 순회하며 Visitor 적용
        auto dfs_iter = library.dfs_iterator();
        accumulate_visitor<int> visitor;
        
        dfs_iter.for_each([&visitor](const auto& element) {
            std::visit([&visitor](const auto& item) {
                if constexpr (requires { item.accept_visitor(visitor); }) {
                    item.accept_visitor(visitor);
                }
            }, element);
        });
        
        CHECK(visitor.get_result() == 3); // 1 + 2
        
        // BFS Iterator로도 동일한 결과 확인
        auto bfs_iter = library.bfs_iterator();
        accumulate_visitor<int> bfs_visitor;
        
        bfs_iter.for_each([&bfs_visitor](const auto& element) {
            std::visit([&bfs_visitor](const auto& item) {
                if constexpr (requires { item.accept_visitor(bfs_visitor); }) {
                    item.accept_visitor(bfs_visitor);
                }
            }, element);
        });
        
        CHECK(bfs_visitor.get_result() == 3); // 동일한 결과
    }
    
    TEST_CASE("Builder + Iterator + Visitor Full Chain Integration") {
        // 1. Builder로 복잡한 구조 생성
        auto documentation = create_complex_structure<int_leaf, string_leaf>("Technical Documentation")
            .described_as("Complete integration test")
            .contains("chapter1", int_leaf(10))
            .contains("title1", string_leaf("Introduction"))
            .contains("chapter2", int_leaf(20))
            .contains("title2", string_leaf("Advanced Topics"))
            .requires_component("conclusion", int_leaf(30))
            .with_post_build_action([](auto& doc) {
                std::cout << "Post-build validation completed" << std::endl;
            })
            .build();
        
        CHECK(documentation->children().size() == 5);
        
        // 2. Iterator로 순회
        auto iter = documentation->dfs_iterator();
        std::vector<int> collected_ints;
        std::vector<std::string> collected_strings;
        
        iter.for_each([&](const auto& element) {
            std::visit([&](const auto& item) {
                if constexpr (std::is_same_v<std::decay_t<decltype(item)>, int_leaf>) {
                    collected_ints.push_back(item.value());
                } else if constexpr (std::is_same_v<std::decay_t<decltype(item)>, string_leaf>) {
                    collected_strings.push_back(item.value());
                }
            }, element);
        });
        
        CHECK(collected_ints.size() == 3);
        CHECK(collected_strings.size() == 2);
        
        // 3. Visitor로 최종 검증
        accumulate_visitor<int> final_visitor;
        documentation->accept_visitor(final_visitor);
        
        CHECK(final_visitor.get_result() == 60); // 10 + 20 + 30
    }
    
    TEST_CASE("Immutable Builder + Composite + Visitor Integration") {
        // 불변 Builder로 구조 생성
        auto document = immutable_origami_builder<int_leaf, string_leaf>{}
            .name("Immutable Test")
            .description("Testing immutable builder pattern")
            .as_composite()
            .with_capacity(5)
            .with_validator([]() {
                std::cout << "Immutable builder validation passed" << std::endl;
            })
            .build();
        
        // 타입 확인 (unique_ptr<composite<...>>)
        CHECK(document != nullptr);
        
        // 컨텐츠 추가 후 visitor 테스트는 mutable operations이므로
        // 별도의 mutable composite 생성하여 테스트
        doc_composite mutable_doc("Mutable Copy");
        mutable_doc.add(int_leaf(50));
        mutable_doc.add(string_leaf("Immutable"));
        
        mock_visitor<std::string> mock;
        auto result = mock.visit(int_leaf(42));
        
        CHECK(mock.get_visit_log().size() == 1);
        CHECK(mock.was_visited(typeid(int_leaf).name()));
    }
}
