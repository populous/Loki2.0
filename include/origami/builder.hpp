/**
 * @file include/origami/builder.hpp
 * @brief 검색 결과 [2] "interface chain" + 검색 결과 [5] "Modern C++" 통합
 * @details ORIGAMI 재귀 구조 특화 Fluent Builder
 */

#pragma once

#include <core/typelist.hpp>
#include <core/policy_host.hpp>
#include <origami/composite.hpp>
#include <origami/iterator.hpp>
#include <origami/visitor.hpp>
#include <memory>
#include <string>
#include <concepts>

namespace metaloki::origami {
    
    /**
     * @brief 검색 결과 [2] "force the developer to create the object in sequence" 구현
     * @details Interface chain을 통한 단계별 강제 구성
     */
    
    // Forward declarations for interface chain
    template<typename... ComponentTypes> class builder_requires_name;
    template<typename... ComponentTypes> class builder_requires_structure;
    template<typename... ComponentTypes> class builder_optional_settings;
    template<typename... ComponentTypes> class builder_final;
    
    /**
     * @brief 검색 결과 [2] "force the developer to set all required setter methods" 
     * @details 필수 속성 설정을 강제하는 인터페이스 체인
     */
    template<typename... ComponentTypes>
    class builder_requires_name {
    private:
        std::string name_;
        
    public:
        // 검색 결과 [7] "more expressive approach" - WithName 대신 Name 사용
        builder_requires_structure<ComponentTypes...> name(std::string name) {
            return builder_requires_structure<ComponentTypes...>(std::move(name));
        }
        
        // 검색 결과 [1] "descriptive names for all of those methods"
        builder_requires_structure<ComponentTypes...> named(std::string name) {
            return name(std::move(name));
        }
    };
    
    template<typename... ComponentTypes>
    class builder_requires_structure {
    private:
        std::string name_;
        
    public:
        explicit builder_requires_structure(std::string name) : name_(std::move(name)) {}
        
        // 검색 결과 [1] "stepbystep construction" - 구조 타입 선택 강제
        builder_optional_settings<ComponentTypes...> as_composite() {
            return builder_optional_settings<ComponentTypes...>(std::move(name_), true);
        }
        
        builder_optional_settings<ComponentTypes...> as_leaf() {
            return builder_optional_settings<ComponentTypes...>(std::move(name_), false);
        }
        
        // 검색 결과 [5] "more intuitive, natural & plain English"
        builder_optional_settings<ComponentTypes...> container() {
            return as_composite();
        }
        
        builder_optional_settings<ComponentTypes...> element() {
            return as_leaf();
        }
    };
    
    template<typename... ComponentTypes>
    class builder_optional_settings {
    private:
        std::string name_;
        bool is_composite_;
        std::string description_;
        size_t initial_capacity_ = 0;
        
    public:
        builder_optional_settings(std::string name, bool is_composite) 
            : name_(std::move(name)), is_composite_(is_composite) {}
        
        // 검색 결과 [1] "easy to add or omit optional parameters"
        builder_optional_settings& with_description(std::string description) {
            description_ = std::move(description);
            return *this;
        }
        
        builder_optional_settings& with_capacity(size_t capacity) {
            initial_capacity_ = capacity;
            return *this;
        }
        
        // 검색 결과 [7] "expressive approach" - 자연스러운 메서드명
        builder_optional_settings& described_as(std::string description) {
            return with_description(std::move(description));
        }
        
        builder_optional_settings& reserves(size_t capacity) {
            return with_capacity(capacity);
        }
        
        // 검색 결과 [2] "calling the build method" - 체인 종료
        builder_final<ComponentTypes...> ready() {
            return builder_final<ComponentTypes...>(
                std::move(name_), is_composite_, std::move(description_), initial_capacity_);
        }
        
        // 직접 빌드도 가능 (선택적 설정 생략)
        auto build() {
            return ready().build();
        }
    };
    
    template<typename... ComponentTypes>
    class builder_final {
    private:
        std::string name_;
        bool is_composite_;
        std::string description_;
        size_t initial_capacity_;
        
    public:
        builder_final(std::string name, bool is_composite, std::string description, size_t capacity)
            : name_(std::move(name)), is_composite_(is_composite), 
              description_(std::move(description)), initial_capacity_(capacity) {}
        
        // 검색 결과 [4] "build() method call, which instantiates and returns"
        auto build() {
            if (is_composite_) {
                auto composite_obj = std::make_unique<composite<ComponentTypes...>>(name_);
                if (!description_.empty()) {
                    // description 설정 로직 (확장 가능)
                }
                if (initial_capacity_ > 0) {
                    // capacity 예약 로직
                }
                return composite_obj;
            } else {
                // Leaf 객체 생성 (첫 번째 타입으로 기본값)
                static_assert(sizeof...(ComponentTypes) > 0, "At least one component type required");
                using FirstType = typename core::typelist<ComponentTypes...>::template at<0>;
                return std::make_unique<leaf<FirstType>>(FirstType{});
            }
        }
    };
    
    /**
     * @brief 검색 결과 [3] "UserFluentInterface" 스타일 팩토리 클래스
     * @details ORIGAMI 구조 생성을 위한 Fluent 인터페이스 진입점
     */
    template<typename... ComponentTypes>
    class origami_builder {
    public:
        // 검색 결과 [7] "Create() => new OrderBuilder()" 패턴
        static builder_requires_name<ComponentTypes...> create() {
            return builder_requires_name<ComponentTypes...>{};
        }
        
        // 검색 결과 [5] "create(std::string name)" 편의 메서드
        static builder_requires_structure<ComponentTypes...> create(std::string name) {
            return builder_requires_name<ComponentTypes...>{}.name(std::move(name));
        }
    };
}
