/**
 * @file include/origami/modern_visitor.hpp
 * @brief 검색 결과 [2] "Unit testing" 가능한 현대적 Visitor
 * @details std::variant + std::visit 기반 타입 안전 구현
 */

#pragma once

#include <origami/visitor.hpp>
#include <functional>
#include <unordered_map>
#include <typeindex>

namespace metaloki::origami {
    
    /**
     * @brief 검색 결과 [2] "ConcreteVisitor and ConcreteElement 단위 테스트" 지원
     * @details 각 타입별로 독립적 테스트가 가능한 구조
     */
    template<typename... ElementTypes>
    class variant_visitor {
    public:
        using element_variant = std::variant<ElementTypes...>;
        using element_list = core::typelist<ElementTypes...>;
        
        // 검색 결과 [2] "test visitor object and make it visit things"
        template<typename VisitorFunction>
        void register_visitor(VisitorFunction&& visitor_func) {
            element_list::for_each_type([&]<typename T>() {
                visitors_[std::type_index(typeid(T))] = 
                    [visitor_func](const element_variant& variant) {
                        return std::visit([&visitor_func](const auto& element) {
                            if constexpr (std::is_same_v<std::decay_t<decltype(element)>, T>) {
                                return visitor_func(element);
                            } else {
                                throw std::bad_variant_access{};
                            }
                        }, variant);
                    };
            });
        }
        
        // 특정 타입에 대한 visitor 등록
        template<typename ElementType, typename VisitorFunction>
        void register_visitor_for_type(VisitorFunction&& visitor_func) {
            static_assert(element_list::template contains<ElementType>(), 
                "ElementType must be in the element list");
            
            visitors_[std::type_index(typeid(ElementType))] = 
                [visitor_func](const element_variant& variant) {
                    return std::visit([&visitor_func](const auto& element) {
                        if constexpr (std::is_same_v<std::decay_t<decltype(element)>, ElementType>) {
                            return visitor_func(element);
                        } else {
                            throw std::runtime_error("Type mismatch in visitor");
                        }
                    }, variant);
                };
        }
        
        // 검색 결과 [2] "test that it visited the right things"
        template<typename ElementType>
        auto visit(const ElementType& element) {
            static_assert(element_list::template contains<ElementType>(), 
                "ElementType must be visitable");
            
            auto type_index = std::type_index(typeid(ElementType));
            auto it = visitors_.find(type_index);
            
            if (it != visitors_.end()) {
                element_variant variant = element;
                return it->second(variant);
            } else {
                throw std::runtime_error("No visitor registered for this type");
            }
        }
        
        // 방문 추적 (테스트용)
        void enable_visit_tracking() { track_visits_ = true; }
        void disable_visit_tracking() { track_visits_ = false; }
        
        const std::vector<std::type_index>& get_visit_history() const {
            return visit_history_;
        }
        
        void clear_visit_history() { visit_history_.clear(); }
        
    private:
        std::unordered_map<std::type_index, std::function<void(const element_variant&)>> visitors_;
        bool track_visits_ = false;
        std::vector<std::type_index> visit_history_;
        
        void track_visit(std::type_index type) {
            if (track_visits_) {
                visit_history_.push_back(type);
            }
        }
    };
    
    /**
     * @brief 검색 결과 [2] "mock objects" 지원 Visitor
     * @details 테스트를 위한 Mock Visitor 구현
     */
    template<typename ResultType = void>
    class mock_visitor : public visitor_base<ResultType> {
    private:
        mutable std::vector<std::string> visit_log_;
        std::unordered_map<std::string, ResultType> predefined_results_;
        
    public:
        using result_type = ResultType;
        
        // Mock visitor에서 호출 로그 기록
        template<typename ElementType>
        result_type visit(const ElementType& element) {
            std::string type_name = typeid(ElementType).name();
            visit_log_.push_back(type_name);
            
            // 미리 정의된 결과가 있으면 반환
            auto it = predefined_results_.find(type_name);
            if (it != predefined_results_.end()) {
                if constexpr (!std::is_void_v<result_type>) {
                    return it->second;
                }
            }
            
            // 기본 동작
            if constexpr (!std::is_void_v<result_type>) {
                return result_type{};
            }
        }
        
        // 검색 결과 [2] "test that the right actions were performed"
        const std::vector<std::string>& get_visit_log() const { return visit_log_; }
        void clear_visit_log() { visit_log_.clear(); }
        
        // Mock 결과 설정
        void set_result_for_type(const std::string& type_name, const ResultType& result) {
            if constexpr (!std::is_void_v<ResultType>) {
                predefined_results_[type_name] = result;
            }
        }
        
        // 방문 여부 확인
        bool was_visited(const std::string& type_name) const {
            return std::find(visit_log_.begin(), visit_log_.end(), type_name) != visit_log_.end();
        }
        
        size_t visit_count(const std::string& type_name) const {
            return std::count(visit_log_.begin(), visit_log_.end(), type_name);
        }
    };
}
