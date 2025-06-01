/**
 * @file include/origami/composite.hpp
 * @brief MetaLoki 2.0 ORIGAMI Composite Pattern
 * @details 검색 결과 [4] "tree structures" + 검색 결과 [1] "origami cores"
 */

#pragma once

#include <core/typelist.hpp>
#include <core/policy_host.hpp>
#include <memory>
#include <vector>
#include <algorithm>
#include <concepts>

namespace metaloki::origami {
    
    /**
     * @brief 검색 결과 [4] "Component - Defines the interface"
     * @details 모든 컴포넌트의 공통 인터페이스
     */
    template<typename T>
    concept Component = requires(T t) {
        { t.render() } -> std::same_as<void>;
        { t.clone() } -> std::convertible_to<std::unique_ptr<T>>;
    };
    
    /**
     * @brief 검색 결과 [5] "Base component" 구현
     * @details Component 인터페이스 템플릿
     */
    template<typename Derived>
    class component_base {
    public:
        // 검색 결과 [5] 필수 작업
        void render() const {
            // CRTP 패턴으로 derived 클래스의 구현 호출
            static_cast<const Derived*>(this)->render_impl();
        }
        
        // 자신의 복제본 생성
        std::unique_ptr<Derived> clone() const {
            return static_cast<const Derived*>(this)->clone_impl();
        }
        
        // 가상 소멸자 (검색 결과 [5] "virtual ~TextComponent() = default;")
        virtual ~component_base() = default;
    };
    
    /**
     * @brief 검색 결과 [5] "Leaf component" 구현
     * @details 단말 노드 (자식이 없는 기본 요소)
     */
    template<typename T>
    class leaf : public component_base<leaf<T>> {
    private:
        T value_;
        
    public:
        // 값 생성자
        explicit leaf(T value) : value_(std::move(value)) {}
        
        // Getter
        const T& value() const { return value_; }
        T& value() { return value_; }
        
        // 검색 결과 [5] "print" -> render_impl
        void render_impl() const {
            // 기본 구현은 단순히 값 출력
            if constexpr (requires { std::cout << value_; }) {
                std::cout << value_;
            }
        }
        
        // 복제 구현
        std::unique_ptr<leaf<T>> clone_impl() const {
            return std::make_unique<leaf<T>>(value_);
        }
    };
    
    /**
     * @brief 검색 결과 [4] "Composite - Represents the composite object"
     * @details 복합 노드 (자식을 포함하는 요소)
     */
    template<Component... ChildTypes>
    class composite : public component_base<composite<ChildTypes...>> {
    private:
        using child_variant = std::variant<ChildTypes...>;
        using child_list = std::vector<child_variant>;
        
        child_list children_;
        std::string name_;
        
    public:
        // 생성자
        explicit composite(std::string name = "Composite") : name_(std::move(name)) {}
        
        // 검색 결과 [5] "void add(Component* component)"
        template<Component ChildType>
        void add(ChildType&& child) {
            static_assert((std::is_same_v<std::decay_t<ChildType>, ChildTypes> || ...), 
                "Child type must be one of the supported types");
            
            children_.push_back(std::forward<ChildType>(child));
        }
        
        // 템플릿 버전 - 복사본 추가
        template<Component ChildType>
        void add_copy(const ChildType& child) {
            static_assert((std::is_same_v<ChildType, ChildTypes> || ...), 
                "Child type must be one of the supported types");
            
            children_.push_back(child);
        }
        
        // 검색 결과 [5] "std::vector<std::shared_ptr<TextComponent>> children;"
        template<Component ChildType, typename... Args>
        void emplace(Args&&... args) {
            static_assert((std::is_same_v<ChildType, ChildTypes> || ...), 
                "Child type must be one of the supported types");
            
            children_.push_back(ChildType(std::forward<Args>(args)...));
        }
        
        // 자식 접근
        const child_list& children() const { return children_; }
        child_list& children() { return children_; }
        
        // 검색 결과 [5] "void print() const override"
        void render_impl() const {
            std::cout << "Composite '" << name_ << "' {\n";
            
            // 검색 결과 [6] std::visit 활용
            for (const auto& child : children_) {
                std::visit([](const auto& c) {
                    c.render();
                }, child);
                std::cout << '\n';
            }
            
            std::cout << "}";
        }
        
        // 복제 구현
        std::unique_ptr<composite<ChildTypes...>> clone_impl() const {
            auto clone = std::make_unique<composite<ChildTypes...>>(name_);
            clone->children_ = children_;  // 복사 가능한 std::variant 사용
            return clone;
        }
        
        // 재귀적 작업 수행 (검색 결과 [2] "recursive structure")
        template<typename Operation>
        void traverse(Operation&& op) const {
            // 자신에게 작업 적용
            op(*this);
            
            // 모든 자식에게 작업 적용
            for (const auto& child : children_) {
                std::visit([&op](const auto& c) {
                    if constexpr (requires { c.traverse(op); }) {
                        c.traverse(op);
                    } else {
                        op(c);
                    }
                }, child);
            }
        }
        
        // 이름 설정/조회
        void set_name(std::string name) { name_ = std::move(name); }
        const std::string& name() const { return name_; }
    };
}
