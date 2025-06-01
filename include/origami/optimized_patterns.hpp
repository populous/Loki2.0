/**
 * @file include/origami/optimized_patterns.hpp
 * @brief ORIGAMI 패턴 성능 최적화 버전
 */

#pragma once

#include <origami/composite.hpp>
#include <origami/iterator.hpp>
#include <origami/visitor.hpp>
#include <origami/builder.hpp>

namespace origami::optimized {
    
    /**
     * @brief 성능 최적화된 Composite - 메모리 레이아웃 최적화
     */
    template<typename... ComponentTypes>
    class fast_composite : public composite<ComponentTypes...> {
        using base = composite<ComponentTypes...>;
        
        // 캐시 친화적 메모리 레이아웃
        static constexpr size_t cache_line_size = 64;
        alignas(cache_line_size) typename base::child_list optimized_children_;
        
    public:
        using base::base;
        
        // 벌크 추가 최적화
        template<typename... Elements>
        void add_bulk(Elements&&... elements) {
            optimized_children_.reserve(optimized_children_.size() + sizeof...(elements));
            (optimized_children_.push_back(std::forward<Elements>(elements)), ...);
        }
        
        // 메모리 사전 할당
        void reserve_capacity(size_t capacity) {
            optimized_children_.reserve(capacity);
        }
        
        // 이동 의미론 최적화
        template<typename Element>
        void add_move(Element&& element) {
            optimized_children_.push_back(std::move(element));
        }
    };
    
    /**
     * @brief 성능 최적화된 Iterator - 분기 예측 최적화
     */
    template<typename... ComponentTypes>
    class fast_iterator {
        using composite_type = fast_composite<ComponentTypes...>;
        
        // 분기 예측 힌트
        static constexpr bool likely_has_children = true;
        
    public:
        template<typename Operation>
        void fast_for_each(const composite_type& composite, Operation&& op) {
            const auto& children = composite.children();
            
            // 벡터화 가능한 루프
            for (size_t i = 0; i < children.size(); ++i) {
                [[likely]] if constexpr (likely_has_children) {
                    std::visit([&op](const auto& element) {
                        op(element);
                    }, children[i]);
                }
            }
        }
    };
    
    /**
     * @brief 성능 최적화된 Visitor - 가상 함수 제거
     */
    template<typename ResultType = void>
    class fast_visitor {
        // 함수 포인터 대신 std::function 최적화
        using visitor_func = std::function<ResultType(const auto&)>;
        
    public:
        template<typename Element>
        constexpr ResultType visit(const Element& element) {
            // 컴파일 타임에 타입 결정, 런타임 디스패치 없음
            if constexpr (std::is_void_v<ResultType>) {
                process_element(element);
            } else {
                return process_element(element);
            }
        }
        
    private:
        template<typename Element>
        constexpr ResultType process_element(const Element& element) {
            // 타입별 특화 처리 (컴파일 타임 분기)
            if constexpr (requires { element.value(); }) {
                if constexpr (!std::is_void_v<ResultType>) {
                    return static_cast<ResultType>(element.value());
                }
            }
            
            if constexpr (!std::is_void_v<ResultType>) {
                return ResultType{};
            }
        }
    };
    
    /**
     * @brief 성능 최적화된 Builder - RAII 및 이동 의미론
     */
    template<typename... ComponentTypes>
    class fast_builder {
    private:
        std::unique_ptr<fast_composite<ComponentTypes...>> result_;
        bool finalized_ = false;
        
    public:
        fast_builder() : result_(std::make_unique<fast_composite<ComponentTypes...>>("Fast Built")) {
            result_->reserve_capacity(16); // 기본 용량 사전 할당
        }
        
        // 이동 체이닝
        fast_builder&& add(auto&& element) && {
            if (!finalized_) {
                result_->add_move(std::forward<decltype(element)>(element));
            }
            return std::move(*this);
        }
        
        // 벌크 추가 최적화
        template<typename... Elements>
        fast_builder&& add_all(Elements&&... elements) && {
            if (!finalized_) {
                result_->add_bulk(std::forward<Elements>(elements)...);
            }
            return std::move(*this);
        }
        
        // RAII 기반 자동 완료
        std::unique_ptr<fast_composite<ComponentTypes...>> finish() && {
            finalized_ = true;
            return std::move(result_);
        }
    };
}
