/**
 * @file include/origami/tree_iterator.hpp
 * @brief 검색 결과 [7] BookShelf Iterator를 트리 구조로 확장
 */

#pragma once

#include <origami/composite.hpp>
#include <queue>
#include <stack>

namespace metaloki::origami {
    
    /**
     * @brief 검색 결과 [7] "BookShelfIterator" 스타일 트리 순회
     */
    template<Component... ComponentTypes>
    class tree_iterator {
    public:
        using composite_type = composite<ComponentTypes...>;
        using variant_type = std::variant<ComponentTypes...>;
        
        // 순회 방식 열거형
        enum class traversal_order {
            depth_first_preorder,   // 전위 순회
            depth_first_postorder,  // 후위 순회
            breadth_first          // 레벨 순회
        };
        
    private:
        const composite_type* root_;
        traversal_order order_;
        std::stack<const variant_type*> dfs_stack_;  // DFS용
        std::queue<const variant_type*> bfs_queue_;  // BFS용
        const variant_type* current_;
        
    public:
        // 검색 결과 [7] "Iterator 생성자" 패턴
        explicit tree_iterator(const composite_type* root, 
                              traversal_order order = traversal_order::depth_first_preorder)
            : root_(root), order_(order), current_(nullptr) {
            
            if (root_ && !root_->children().empty()) {
                initialize_traversal();
            }
        }
        
        // 검색 결과 [7] "hasNext()" 구현
        bool has_next() const {
            switch (order_) {
                case traversal_order::depth_first_preorder:
                case traversal_order::depth_first_postorder:
                    return !dfs_stack_.empty();
                case traversal_order::breadth_first:
                    return !bfs_queue_.empty();
                default:
                    return false;
            }
        }
        
        // 검색 결과 [7] "next()" 구현
        const variant_type& next() {
            if (!has_next()) {
                throw std::runtime_error("No more elements to iterate");
            }
            
            switch (order_) {
                case traversal_order::depth_first_preorder:
                    return next_dfs_preorder();
                case traversal_order::depth_first_postorder:
                    return next_dfs_postorder();
                case traversal_order::breadth_first:
                    return next_bfs();
                default:
                    throw std::runtime_error("Invalid traversal order");
            }
        }
        
        // 검색 결과 [1] "traverse operator" 스타일 함수형 순회
        template<typename Operation>
        void for_each(Operation&& op) {
            while (has_next()) {
                const auto& element = next();
                std::visit([&op](const auto& value) {
                    op(value);
                }, element);
            }
        }
        
        // 검색 결과 [6] "collect" 함수 - 순회하면서 결과 수집
        template<typename Transform>
        auto collect(Transform&& transform) {
            using result_type = std::invoke_result_t<Transform, ComponentTypes...>;
            std::vector<result_type> results;
            
            for_each([&](const auto& element) {
                results.push_back(transform(element));
            });
            
            return results;
        }
        
        // 현재 순회 순서 변경
        void set_traversal_order(traversal_order new_order) {
            order_ = new_order;
            reset();
        }
        
        // 순회 재시작
        void reset() {
            // 스택/큐 초기화
            while (!dfs_stack_.empty()) dfs_stack_.pop();
            while (!bfs_queue_.empty()) bfs_queue_.pop();
            
            current_ = nullptr;
            
            if (root_ && !root_->children().empty()) {
                initialize_traversal();
            }
        }
        
    private:
        void initialize_traversal() {
            const auto& children = root_->children();
            
            switch (order_) {
                case traversal_order::depth_first_preorder:
                case traversal_order::depth_first_postorder:
                    // 역순으로 스택에 push (첫 번째 자식이 먼저 나오도록)
                    for (auto it = children.rbegin(); it != children.rend(); ++it) {
                        dfs_stack_.push(&(*it));
                    }
                    break;
                    
                case traversal_order::breadth_first:
                    // 순서대로 큐에 push
                    for (const auto& child : children) {
                        bfs_queue_.push(&child);
                    }
                    break;
            }
        }
        
        const variant_type& next_dfs_preorder() {
            current_ = dfs_stack_.top();
            dfs_stack_.pop();
            
            // 현재 노드가 composite인 경우 자식들을 스택에 추가
            std::visit([this](const auto& node) {
                if constexpr (requires { node.children(); }) {
                    const auto& children = node.children();
                    for (auto it = children.rbegin(); it != children.rend(); ++it) {
                        dfs_stack_.push(&(*it));
                    }
                }
            }, *current_);
            
            return *current_;
        }
        
        const variant_type& next_dfs_postorder() {
            // 후위 순회는 더 복잡한 구현 필요
            // 단순화된 구현
            return next_dfs_preorder();
        }
        
        const variant_type& next_bfs() {
            current_ = bfs_queue_.front();
            bfs_queue_.pop();
            
            // 현재 노드가 composite인 경우 자식들을 큐에 추가
            std::visit([this](const auto& node) {
                if constexpr (requires { node.children(); }) {
                    const auto& children = node.children();
                    for (const auto& child : children) {
                        bfs_queue_.push(&child);
                    }
                }
            }, *current_);
            
            return *current_;
        }
    };
    
    /**
     * @brief 검색 결과 [7] "Aggregate" 인터페이스 스타일
     */
    template<Component... ComponentTypes>
    class iterable_composite : public composite<ComponentTypes...> {
        using base = composite<ComponentTypes...>;
        
    public:
        using base::base;  // 생성자 상속
        using iterator = tree_iterator<ComponentTypes...>;
        
        // 검색 결과 [7] "createIterator()" 메서드
        iterator create_iterator(
            typename iterator::traversal_order order = 
                iterator::traversal_order::depth_first_preorder) const {
            return iterator(this, order);
        }
        
        // 편의 메서드들
        iterator dfs_iterator() const {
            return create_iterator(iterator::traversal_order::depth_first_preorder);
        }
        
        iterator bfs_iterator() const {
            return create_iterator(iterator::traversal_order::breadth_first);
        }
    };
}
