/**
 * @file include/origami/visitor.hpp
 * @brief 검색 결과 [1] "ObjectStructure에서 ConcreteElement.accept(Visitor)" 구현
 * @details ORIGAMI 재귀 구조 특화 Visitor 패턴
 */

#pragma once

#include <core/typelist.hpp>
#include <core/policy_host.hpp>
#include <origami/composite.hpp>
#include <variant>
#include <concepts>
#include <type_traits>

namespace metaloki::origami {
    
    /**
     * @brief 검색 결과 [4] "Visitor 인터페이스" 현대화
     * @details C++20 concepts 기반 Visitor 개념
     */
    template<typename T>
    concept Visitable = requires(T t) {
        typename T::visitor_result_type;
        { t.accept_visitor(std::declval<auto>()) } -> std::same_as<typename T::visitor_result_type>;
    };
    
    template<typename V, typename... ElementTypes>
    concept Visitor = requires(V v) {
        (v.visit(std::declval<ElementTypes>()), ...);
    };
    
    /**
     * @brief 검색 결과 [1] "ConcreteElement가 Visitor.visit(자기자신) 호출" 구현
     * @details 이중 디스패치를 std::variant + std::visit로 현대화
     */
    template<typename ResultType = void>
    class visitor_base {
    public:
        using result_type = ResultType;
        
        // 검색 결과 [4] "visit 메서드로 허락된 데이터 처리"
        virtual ~visitor_base() = default;
        
        // 기본 visit 구현 (오버라이드 가능)
        template<typename ElementType>
        result_type visit(const ElementType& element) {
            if constexpr (std::is_void_v<result_type>) {
                visit_impl(element);
            } else {
                return visit_impl(element);
            }
        }
        
    protected:
        // 실제 구현은 파생 클래스에서 정의
        template<typename ElementType>
        result_type visit_impl(const ElementType& element) {
            // 기본 구현: 아무것도 하지 않음
            if constexpr (!std::is_void_v<result_type>) {
                return result_type{};
            }
        }
    };
    
    /**
     * @brief 검색 결과 [4] "Unit 인터페이스" - Visitable 기반 클래스
     */
    template<typename Derived, typename ResultType = void>
    class visitable_base {
    public:
        using visitor_result_type = ResultType;
        
        // 검색 결과 [1] "accept메소드를 호출하면 모두 자기자신을 인자로 visitor에게 던져줌"
        template<Visitor<Derived> VisitorType>
        visitor_result_type accept_visitor(VisitorType&& visitor) const {
            // CRTP로 실제 타입을 visitor에게 전달
            return visitor.visit(static_cast<const Derived&>(*this));
        }
        
        template<Visitor<Derived> VisitorType>
        visitor_result_type accept_visitor(VisitorType&& visitor) {
            return visitor.visit(static_cast<Derived&>(*this));
        }
    };
    
    /**
     * @brief 검색 결과 [4] "SumVisitor" 스타일 구체적 Visitor 구현
     * @details ORIGAMI 구조에 특화된 연산 Visitor
     */
    template<typename ValueType>
    class accumulate_visitor : public visitor_base<ValueType> {
    private:
        ValueType accumulated_value_{};
        
    public:
        using result_type = ValueType;
        
        // 검색 결과 [4] "Item 클래스일 경우 값을 가져와서 더해주고"
        result_type visit(const leaf<ValueType>& leaf_element) {
            accumulated_value_ += leaf_element.value();
            return accumulated_value_;
        }
        
        // 검색 결과 [4] "아닐 경우 accept 를 하여 visit 를 허용"
        template<Component... ComponentTypes>
        result_type visit(const composite<ComponentTypes...>& composite_element) {
            // 모든 자식에 대해 재귀적으로 방문
            for (const auto& child : composite_element.children()) {
                std::visit([this](const auto& child_element) {
                    if constexpr (requires { child_element.accept_visitor(*this); }) {
                        child_element.accept_visitor(*this);
                    } else {
                        this->visit(child_element);
                    }
                }, child);
            }
            return accumulated_value_;
        }
        
        // 결과 조회
        ValueType get_result() const { return accumulated_value_; }
        void reset() { accumulated_value_ = ValueType{}; }
    };
    
    /**
     * @brief 검색 결과 [4] "AvgVisitor" 스타일 평균 계산 Visitor
     */
    template<typename ValueType>
    class average_visitor : public visitor_base<double> {
    private:
        ValueType sum_{};
        size_t count_{0};
        
    public:
        using result_type = double;
        
        result_type visit(const leaf<ValueType>& leaf_element) {
            sum_ += leaf_element.value();
            count_++;
            return static_cast<double>(sum_) / count_;
        }
        
        template<Component... ComponentTypes>
        result_type visit(const composite<ComponentTypes...>& composite_element) {
            for (const auto& child : composite_element.children()) {
                std::visit([this](const auto& child_element) {
                    if constexpr (requires { child_element.accept_visitor(*this); }) {
                        child_element.accept_visitor(*this);
                    } else {
                        this->visit(child_element);
                    }
                }, child);
            }
            return count_ > 0 ? static_cast<double>(sum_) / count_ : 0.0;
        }
        
        double get_average() const {
            return count_ > 0 ? static_cast<double>(sum_) / count_ : 0.0;
        }
        
        size_t get_count() const { return count_; }
        ValueType get_sum() const { return sum_; }
        void reset() { sum_ = ValueType{}; count_ = 0; }
    };
}
