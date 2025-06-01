/**
 * @file include/origami/complex_builder.hpp
 * @brief 검색 결과 [5] "sophisticated example" ORIGAMI 확장
 */

#pragma once

#include <origami/advanced_builder.hpp>
#include <functional>
#include <initializer_list>

namespace metaloki::origami {
    
    /**
     * @brief 검색 결과 [5] "PersonBuilder" 스타일 복합 ORIGAMI Builder
     * @details 재귀적 구조와 컴포넌트를 동시에 구성
     */
    template<typename... ComponentTypes>
    class complex_origami_builder {
    private:
        using component_variant = std::variant<ComponentTypes...>;
        using composite_type = composite<ComponentTypes...>;
        
        struct component_config {
            std::string name;
            std::function<component_variant()> factory;
            bool is_required = false;
        };
        
        std::string root_name_;
        std::string description_;
        std::vector<component_config> components_;
        std::vector<std::function<void(composite_type&)>> post_build_actions_;
        
    public:
        // 검색 결과 [5] "create(std::string name)" 스타일 시작
        explicit complex_origami_builder(std::string root_name) 
            : root_name_(std::move(root_name)) {}
        
        // 검색 결과 [1] "descriptive names" - 구조 정보 설정
        complex_origami_builder& described_as(std::string description) {
            description_ = std::move(description);
            return *this;
        }
        
        // 검색 결과 [5] "Personal & Professional" 스타일 섹션 구성
        template<typename ComponentType>
        complex_origami_builder& with_component(std::string name, ComponentType component, bool required = false) {
            static_assert((std::is_same_v<ComponentType, ComponentTypes> || ...), 
                "ComponentType must be one of the supported types");
            
            components_.push_back({
                std::move(name),
                [component = std::move(component)]() -> component_variant { 
                    return component; 
                },
                required
            });
            return *this;
        }
        
        // 검색 결과 [1] "method chaining" - 컴포넌트 팩토리 추가
        template<typename ComponentType, typename Factory>
        complex_origami_builder& with_component_factory(std::string name, Factory&& factory, bool required = false) {
            static_assert((std::is_same_v<ComponentType, ComponentTypes> || ...), 
                "ComponentType must be one of the supported types");
            
            components_.push_back({
                std::move(name),
                [factory = std::forward<Factory>(factory)]() -> component_variant { 
                    return ComponentType(factory()); 
                },
                required
            });
            return *this;
        }
        
        // 검색 결과 [5] "lives() & works()" 스타일 - 자연스러운 언어
        template<typename... Args>
        complex_origami_builder& contains(std::string component_name, Args&&... args) {
            if constexpr (sizeof...(args) == 1) {
                return with_component(std::move(component_name), std::forward<Args>(args)...);
            } else {
                // 여러 인자로 컴포넌트 생성
                using FirstType = typename core::typelist<ComponentTypes...>::template at<0>;
                return with_component(std::move(component_name), FirstType{std::forward<Args>(args)...});
            }
        }
        
        // 필수 컴포넌트 추가
        template<typename ComponentType>
        complex_origami_builder& requires_component(std::string name, ComponentType component) {
            return with_component(std::move(name), std::move(component), true);
        }
        
        // 검색 결과 [1] "validation logic" - 빌드 후 액션 추가
        template<typename Action>
        complex_origami_builder& with_post_build_action(Action&& action) {
            post_build_actions_.emplace_back([action = std::forward<Action>(action)](composite_type& comp) {
                action(comp);
            });
            return *this;
        }
        
        // 검색 결과 [7] "Build() => new Order" - 최종 빌드
        std::unique_ptr<composite_type> build() {
            // 검색 결과 [1] "validation" - 필수 컴포넌트 확인
            for (const auto& config : components_) {
                if (config.is_required && !config.factory) {
                    throw std::runtime_error("Required component '" + config.name + "' not provided");
                }
            }
            
            auto result = std::make_unique<composite_type>(root_name_);
            
            // 모든 컴포넌트 추가
            for (const auto& config : components_) {
                if (config.factory) {
                    auto component = config.factory();
                    result->add(std::move(component));
                }
            }
            
            // 검색 결과 [1] "post_build_actions" 실행
            for (const auto& action : post_build_actions_) {
                action(*result);
            }
            
            return result;
        }
        
        // 검색 결과 [3] "Build()" - unique_ptr 반환
        auto create() {
            return build();
        }
    };
    
    /**
     * @brief 검색 결과 [5] "do not worry, it will be eliminated in optimization"
     * @details 컴파일러 최적화를 고려한 편의 팩토리 함수들
     */
    
    // 검색 결과 [7] "OrderBuilder.Create()" 스타일 진입점
    template<typename... ComponentTypes>
    complex_origami_builder<ComponentTypes...> create_complex_structure(std::string name) {
        return complex_origami_builder<ComponentTypes...>(std::move(name));
    }
    
    // 검색 결과 [5] "forcing user's to use builder" - private 생성자 패턴 적용
    template<typename... ComponentTypes>
    class structure_factory {
    public:
        static complex_origami_builder<ComponentTypes...> create(std::string name) {
            return complex_origami_builder<ComponentTypes...>(std::move(name));
        }
        
        // 템플릿 특화로 다양한 생성 방법 제공
        template<typename FirstComponent>
        static auto create_with(std::string name, std::string first_name, FirstComponent&& first) {
            return create(std::move(name))
                .contains(std::move(first_name), std::forward<FirstComponent>(first));
        }
    };
}
