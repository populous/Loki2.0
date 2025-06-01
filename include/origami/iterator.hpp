/**
 * @file include/origami/iterator.hpp
 * @brief 검색 결과 [1] "Essence of Iterator Pattern" 구현
 * @details ORIGAMI 재귀 구조 특화 Iterator
 */

#pragma once

#include <core/typelist.hpp>
#include <core/policy_host.hpp>
#include <origami/composite.hpp>
#include <iterator>
#include <concepts>
#include <functional>
#include <stack>

namespace metaloki::origami {
    
    /**
     * @brief 검색 결과 [3] "C++20 Concepts" 기반 Iterator 개념
     */
    template<typename T>
    concept Traversable = requires(T t) {
        typename T::value_type;
        { t.begin() } -> std::input_or_output_iterator;
        { t.end() } -> std::sentinel_for<decltype(t.begin())>;
    };
    
    /**
     * @brief 검색 결과 [5] "iterator_concept" 태그 구현
     * @details C++20 표준 iterator 요구사항 준수
     */
    template<typename ValueType>
    class origami_iterator {
    public:
        // 검색 결과 [5] "C++20 iterator requirements"
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = ValueType;
        using pointer = value_type*;
        using reference = value_type&;
        
    private:
        // 검색 결과 [8] "private pointer m_ptr" 개념 확장
        pointer current_ptr_;
        std::stack<pointer> traversal_stack_;
        
    public:
        // 검색 결과 [3] "Default constructor is required"
        origami_iterator() : current_ptr_(nullptr) {}
        
        // 특정 요소를 가리키는 생성자
        explicit origami_iterator(pointer ptr) : current_ptr_(ptr) {}
        
        // 검색 결과 [8] "reference operator*() const"
        reference operator*() const {
            if (!current_ptr_) {
                throw std::runtime_error("Dereferencing null iterator");
            }
            return *current_ptr_;
        }
        
        // 검색 결과 [8] "pointer operator->()"
        pointer operator->() const {
            return current_ptr_;
        }
        
        // 검색 결과 [3] "++operator"
        origami_iterator& operator++() {
            advance_to_next();
            return *this;
        }
        
        // 검색 결과 [8] "Postfix increment"
        origami_iterator operator++(int) {
            origami_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        // 검색 결과 [3] "==operator" (C++20 auto-generates !=)
        auto operator<=>(const origami_iterator&) const = default;
        bool operator==(const origami_iterator& other) const {
            return current_ptr_ == other.current_ptr_;
        }
        
    private:
        // ORIGAMI 특화 순회 로직
        void advance_to_next() {
            if (!current_ptr_) return;
            
            // 복합 구조의 경우 깊이 우선 순회
            if constexpr (requires { current_ptr_->children(); }) {
                auto& children = current_ptr_->children();
                if (!children.empty()) {
                    // 현재 위치를 스택에 저장
                    traversal_stack_.push(current_ptr_);
                    // 첫 번째 자식으로 이동
                    std::visit([this](auto&& child) {
                        current_ptr_ = &child;
                    }, children[0]);
                    return;
                }
            }
            
            // 형제나 부모의 다음 형제로 이동
            move_to_next_sibling_or_parent();
        }
        
        void move_to_next_sibling_or_parent() {
            // 스택을 활용한 백트래킹
            while (!traversal_stack_.empty()) {
                auto parent = traversal_stack_.top();
                traversal_stack_.pop();
                
                // 부모의 다음 형제 찾기 로직
                if (has_next_sibling(parent)) {
                    current_ptr_ = get_next_sibling(parent);
                    return;
                }
            }
            
            // 더 이상 순회할 요소 없음
            current_ptr_ = nullptr;
        }
        
        bool has_next_sibling(pointer parent) const {
            // 실제 구현에서는 부모-자식 관계를 통해 판단
            return false; // 단순화된 구현
        }
        
        pointer get_next_sibling(pointer parent) const {
            // 실제 구현에서는 부모의 다음 자식 반환
            return nullptr; // 단순화된 구현
        }
    };
    
    /**
     * @brief 검색 결과 [1] "Internal Iterator" 구현
     * @details 컬렉션이 순회를 책임지는 방식
     */
    template<typename ContainerType>
    class internal_iterator {
    private:
        ContainerType& container_;
        
    public:
        explicit internal_iterator(ContainerType& container) 
            : container_(container) {}
        
        // 검색 결과 [1] "client needs only to provide an operation"
        template<typename Operation>
        void for_each(Operation&& op) {
            traverse_internal(container_, std::forward<Operation>(op));
        }
        
        // 검색 결과 [6] "collect" 함수 구현
        template<typename Operation, typename Transform>
        auto collect(Operation&& accumulator, Transform&& transform) {
            using result_type = std::invoke_result_t<Transform, typename ContainerType::value_type>;
            std::vector<result_type> results;
            
            for_each([&](const auto& element) {
                accumulator(element);
                results.push_back(transform(element));
            });
            
            return results;
        }
        
        // 검색 결과 [6] "disperse" 함수 구현  
        template<typename State, typename Transform>
        auto disperse(State initial_state, Transform&& transform) {
            State current_state = std::move(initial_state);
            using result_type = std::invoke_result_t<Transform, typename ContainerType::value_type, State>;
            std::vector<result_type> results;
            
            for_each([&](const auto& element) {
                auto result = transform(element, current_state);
                results.push_back(result);
                // 상태 업데이트 로직 (필요시)
            });
            
            return results;
        }
        
    private:
        template<typename Container, typename Operation>
        void traverse_internal(Container& container, Operation&& op) {
            // 재귀적 구조 순회
            if constexpr (requires { container.children(); }) {
                // 복합 객체의 경우
                op(container);
                
                for (auto& child : container.children()) {
                    std::visit([&](auto&& c) {
                        traverse_internal(c, op);
                    }, child);
                }
            } else {
                // 단말 객체의 경우
                op(container);
            }
        }
    };
}
