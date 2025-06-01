/**
 * @file include/origami/advanced_builder.hpp
 * @brief 검색 결과 [4] "immutability aspect" 구현
 * @details 불변 객체 기반 고급 Fluent Builder
 */

#pragma once

#include <origami/builder.hpp>
#include <vector>
#include <functional>

namespace metaloki::origami {
    
    /**
     * @brief 검색 결과 [4] "return a new instance each time" 구현
     * @details 불변 Fluent Builder - 모든 메서드가 새 인스턴스 반환
     */
    template<typename... ComponentTypes>
    class immutable_origami_builder {
    private:
        struct build_state {
            std::string name;
            std::string description;
            bool is_composite = false;
            size_t capacity = 0;
            std::vector<std::function<void()>> validators;
            std::vector<std::pair<std::string, std::string>> metadata;
        };
        
        build_state state_;
        
        explicit immutable_origami_builder(build_state state) : state_(std::move(state)) {}
        
    public:
        // 기본 생성자
        immutable_origami_builder() = default;
        
        // 검색 결과 [4] "new instance each time" - 모든 메서드가 새 인스턴스 반환
        immutable_origami_builder name(std::string name) const {
            auto new_state = state_;
            new_state.name = std::move(name);
            return immutable_origami_builder(std::move(new_state));
        }
        
        immutable_origami_builder description(std::string desc) const {
            auto new_state = state_;
            new_state.description = std::move(desc);
            return immutable_origami_builder(std::move(new_state));
        }
        
        immutable_origami_builder as_composite() const {
            auto new_state = state_;
            new_state.is_composite = true;
            return immutable_origami_builder(std::move(new_state));
        }
        
        immutable_origami_builder as_leaf() const {
            auto new_state = state_;
            new_state.is_composite = false;
            return immutable_origami_builder(std::move(new_state));
        }
        
        immutable_origami_builder with_capacity(size_t capacity) const {
            auto new_state = state_;
            new_state.capacity = capacity;
            return immutable_origami_builder(std::move(new_state));
        }
        
        // 검색 결과 [1] "validation logic ensuring the object is always in valid State"
        template<typename Validator>
        immutable_origami_builder with_validator(Validator&& validator) const {
            auto new_state = state_;
            new_state.validators.emplace_back([validator](){ validator(); });
            return immutable_origami_builder(std::move(new_state));
        }
        
        // 메타데이터 추가
        immutable_origami_builder with_metadata(std::string key, std::string value) const {
            auto new_state = state_;
            new_state.metadata.emplace_back(std::move(key), std::move(value));
            return immutable_origami_builder(std::move(new_state));
        }
        
        // 검색 결과 [4] "build() method call" - 최종 빌드
        auto build() const {
            // 검색 결과 [1] "validation logic" 실행
            for (const auto& validator : state_.validators) {
                validator();
            }
            
            if (state_.is_composite) {
                auto result = std::make_unique<composite<ComponentTypes...>>(state_.name);
                // 메타데이터 및 기타 설정 적용
                return result;
            } else {
                using FirstType = typename core::typelist<ComponentTypes...>::template at<0>;
                return std::make_unique<leaf<FirstType>>(FirstType{});
            }
        }
        
        // 체이닝 편의 메서드들 (검색 결과 [5] "plain English" 스타일)
        immutable_origami_builder named(std::string name) const { return this->name(std::move(name)); }
        immutable_origami_builder described_as(std::string desc) const { return description(std::move(desc)); }
        immutable_origami_builder container() const { return as_composite(); }
        immutable_origami_builder element() const { return as_leaf(); }
        immutable_origami_builder reserves(size_t capacity) const { return with_capacity(capacity); }
    };
}
