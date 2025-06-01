/**
 * @file include/metaloki/origami/advanced_visitor.hpp
 * @brief 검색 결과 [4] "집합데이터" 처리를 ORIGAMI 트리로 확장
 */

#pragma once

#include <origami/visitor.hpp>
#include <origami/iterator.hpp>
#include <functional>
#include <stack>
#include <queue>

namespace metaloki::origami {
    
    /**
     * @brief 검색 결과 [4] "ItemList" 개념의 트리 구조 확장
     * @details 재귀적 데이터 구조 전용 고급 Visitor
     */
    template<typename ResultType = void>
    class tree_visitor : public visitor_base<ResultType> {
    public:
        using result_type = ResultType;
        
        // 순회 방식 열거형
        enum class traversal_mode {
            depth_first_preorder,
            depth_first_postorder,
            breadth_first,
            custom
        };
        
    private:
        traversal_mode mode_ = traversal_mode::depth_first_preorder;
        std::function<void(const auto&)> pre_visit_hook_;
        std::function<void(const auto&)> post_visit_hook_;
        std::vector<result_type> results_;
        
    public:
        // 순회 방식 설정
        void set_traversal_mode(traversal_mode mode) { mode_ = mode; }
        
        // 방문 전후 훅 설정
        template<typename PreHook>
        void set_pre_visit_hook(PreHook&& hook) {
            pre_visit_hook_ = std::forward<PreHook>(hook);
        }
        
        template<typename PostHook>
        void set_post_visit_hook(PostHook&& hook) {
            post_visit_hook_ = std::forward<PostHook>(hook);
        }
        
        // 검색 결과 [4] "단일데이터" 처리 (Leaf)
        template<typename ValueType>
        result_type visit(const leaf<ValueType>& leaf_element) {
            if (pre_visit_hook_) pre_visit_hook_(leaf_element);
            
            result_type result = process_leaf(leaf_element);
            
            if constexpr (!std::is_void_v<result_type>) {
                results_.push_back(result);
            }
            
            if (post_visit_hook_) post_visit_hook_(leaf_element);
            
            if constexpr (!std::is_void_v<result_type>) {
                return result;
            }
        }
        
        // 검색 결과 [4] "집합데이터" 처리 (Composite)
        template<Component... ComponentTypes>
        result_type visit(const composite<ComponentTypes...>& composite_element) {
            if (pre_visit_hook_) pre_visit_hook_(composite_element);
            
            result_type result = process_composite(composite_element);
            
            if (post_visit_hook_) post_visit_hook_(composite_element);
            
            if constexpr (!std::is_void_v<result_type>) {
                return result;
            }
        }
        
        // 결과 조회
        const std::vector<result_type>& get_results() const requires (!std::is_void_v<result_type>) {
            return results_;
        }
        
        void clear_results() requires (!std::is_void_v<result_type>) {
            results_.clear();
        }
        
    protected:
        // 파생 클래스에서 구현할 메서드들
        template<typename ValueType>
        virtual result_type process_leaf(const leaf<ValueType>& leaf_element) {
            // 기본 구현: 아무것도 하지 않음
            if constexpr (!std::is_void_v<result_type>) {
                return result_type{};
            }
        }
        
        template<Component... ComponentTypes>
        virtual result_type process_composite(const composite<ComponentTypes...>& composite_element) {
            // 기본 구현: 자식들을 순회하며 방문
            result_type aggregate_result{};
            
            switch (mode_) {
                case traversal_mode::depth_first_preorder:
                    aggregate_result = visit_depth_first_preorder(composite_element);
                    break;
                case traversal_mode::depth_first_postorder:
                    aggregate_result = visit_depth_first_postorder(composite_element);
                    break;
                case traversal_mode::breadth_first:
                    aggregate_result = visit_breadth_first(composite_element);
                    break;
                case traversal_mode::custom:
                    aggregate_result = visit_custom(composite_element);
                    break;
            }
            
            if constexpr (!std::is_void_v<result_type>) {
                return aggregate_result;
            }
        }
        
    private:
        template<Component... ComponentTypes>
        result_type visit_depth_first_preorder(const composite<ComponentTypes...>& composite_element) {
            result_type result{};
            
            for (const auto& child : composite_element.children()) {
                std::visit([this, &result](const auto& child_element) {
                    if constexpr (!std::is_void_v<result_type>) {
                        result = this->visit(child_element);
                    } else {
                        this->visit(child_element);
                    }
                }, child);
            }
            
            if constexpr (!std::is_void_v<result_type>) {
                return result;
            }
        }
        
        template<Component... ComponentTypes>
        result_type visit_depth_first_postorder(const composite<ComponentTypes...>& composite_element) {
            // 후위 순회: 자식 먼저, 그 다음 부모
            return visit_depth_first_preorder(composite_element); // 단순화된 구현
        }
        
        template<Component... ComponentTypes>
        result_type visit_breadth_first(const composite<ComponentTypes...>& composite_element) {
            std::queue<std::variant<ComponentTypes...>> visit_queue;
            
            // 현재 레벨의 모든 자식을 큐에 추가
            for (const auto& child : composite_element.children()) {
                visit_queue.push(child);
            }
            
            result_type result{};
            
            while (!visit_queue.empty()) {
                auto current = visit_queue.front();
                visit_queue.pop();
                
                std::visit([this, &visit_queue, &result](const auto& element) {
                    if constexpr (!std::is_void_v<result_type>) {
                        result = this->visit(element);
                    } else {
                        this->visit(element);
                    }
                    
                    // 복합 요소인 경우 자식들을 큐에 추가
                    if constexpr (requires { element.children(); }) {
                        for (const auto& child : element.children()) {
                            visit_queue.push(child);
                        }
                    }
                }, current);
            }
            
            if constexpr (!std::is_void_v<result_type>) {
                return result;
            }
        }
        
        template<Component... ComponentTypes>
        result_type visit_custom(const composite<ComponentTypes...>& composite_element) {
            // 사용자 정의 순회 (기본은 DFS)
            return visit_depth_first_preorder(composite_element);
        }
    };
    
    /**
     * @brief 검색 결과 [4] "SumVisitor, AvgVisitor" 스타일의 구체적 구현들
     */
    
    // 트리의 모든 수치 값을 수집하는 Visitor
    template<typename ValueType>
    class collect_values_visitor : public tree_visitor<std::vector<ValueType>> {
    public:
        using result_type = std::vector<ValueType>;
        
    protected:
        result_type process_leaf(const leaf<ValueType>& leaf_element) override {
            return {leaf_element.value()};
        }
    };
    
    // 트리의 깊이를 계산하는 Visitor
    class depth_calculator_visitor : public tree_visitor<size_t> {
    private:
        size_t current_depth_ = 0;
        size_t max_depth_ = 0;
        
    public:
        using result_type = size_t;
        
        template<typename ElementType>
        result_type visit(const ElementType& element) {
            current_depth_++;
            max_depth_ = std::max(max_depth_, current_depth_);
            
            auto result = tree_visitor<size_t>::visit(element);
            
            current_depth_--;
            return max_depth_;
        }
    };
    
    // 노드 개수를 세는 Visitor
    class node_counter_visitor : public tree_visitor<size_t> {
    private:
        size_t count_ = 0;
        
    public:
        using result_type = size_t;
        
        template<typename ElementType>
        result_type visit(const ElementType& element) {
            count_++;
            tree_visitor<size_t>::visit(element);
            return count_;
        }
        
        size_t get_total_count() const { return count_; }
        void reset() { count_ = 0; }
    };
}
