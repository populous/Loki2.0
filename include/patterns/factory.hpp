/**
 * @file include/patterns/factory.hpp
 * @brief MetaLoki 2.0 Factory Pattern - TypeList 기반 현대적 구현
 * @details Single Thread + C++20 concepts + Policy Host 통합
 */

#pragma once

#include <core/typelist.hpp>
#include <core/policy_host.hpp>
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>
#include <concepts>

namespace patterns {
    
    /**
     * @brief 생산 가능한 타입에 대한 개념
     */
    template<typename T>
    concept Producible = requires {
        std::is_default_constructible_v<T>;
    } && requires(T t) {
        std::unique_ptr<T>(std::make_unique<T>());
    };
    
    /**
     * @brief TypeList 기반 Modern Factory
     * @details 컴파일 타임 타입 검증 + 런타임 동적 생성
     */
    template<Producible... ProductTypes>
    class factory : public core::policy_host<
        core::policies::single_thread_policy,
        core::policies::validation_policy
    > {
        using policy_base = core::policy_host<
            core::policies::single_thread_policy,
            core::policies::validation_policy
        >;
        
    public:
        using product_list = core::typelist<ProductTypes...>;
        using product_variant = std::variant<std::unique_ptr<ProductTypes>...>;
        
        // 생성자 함수 타입
        template<typename T>
        using creator_function = std::function<std::unique_ptr<T>()>;
        
    private:
        // 타입별 생성자 함수 저장
        std::unordered_map<std::string, std::function<product_variant()>> creators_;
        
    public:
        /**
         * @brief 생성자 함수 등록
         */
        template<Producible ProductType, typename CreatorFunc>
        void register_creator(const std::string& name, CreatorFunc&& creator) {
            static_assert(product_list::template contains<ProductType>(), 
                "ProductType must be in the product list");
            
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            
            this->template get_policy<core::policies::validation_policy>().assert_that(
                !name.empty(), "Product name cannot be empty");
            
            creators_[name] = [creator = std::forward<CreatorFunc>(creator)]() -> product_variant {
                return std::make_unique<ProductType>(creator());
            };
        }
        
        /**
         * @brief 기본 생성자 등록 (편의 메서드)
         */
        template<Producible ProductType>
        void register_default(const std::string& name) {
            register_creator<ProductType>(name, []() { return ProductType{}; });
        }
        
        /**
         * @brief 인자를 받는 생성자 등록
         */
        template<Producible ProductType, typename... Args>
        void register_with_args(const std::string& name, Args... args) {
            register_creator<ProductType>(name, [args...]() { return ProductType{args...}; });
        }
        
        /**
         * @brief 제품 생성
         */
        product_variant create(const std::string& name) {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            
            auto it = creators_.find(name);
            this->template get_policy<core::policies::validation_policy>().assert_that(
                it != creators_.end(), "Unknown product: " + name);
            
            return it->second();
        }
        
        /**
         * @brief 특정 타입으로 생성 (타입 안전)
         */
        template<Producible ProductType>
        std::unique_ptr<ProductType> create_typed(const std::string& name) {
            static_assert(product_list::template contains<ProductType>(), 
                "ProductType must be in the product list");
            
            auto variant_result = create(name);
            
            return std::visit([](auto&& ptr) -> std::unique_ptr<ProductType> {
                if constexpr (std::is_same_v<std::decay_t<decltype(ptr)>, std::unique_ptr<ProductType>>) {
                    return std::move(ptr);
                } else {
                    throw std::bad_variant_access{};
                }
            }, variant_result);
        }
        
        /**
         * @brief 등록된 제품 목록 조회
         */
        std::vector<std::string> get_product_names() const {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            
            std::vector<std::string> names;
            names.reserve(creators_.size());
            
            for (const auto& [name, creator] : creators_) {
                names.push_back(name);
            }
            
            return names;
        }
        
        /**
         * @brief 제품 존재 여부 확인
         */
        bool has_product(const std::string& name) const {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            return creators_.find(name) != creators_.end();
        }
        
        /**
         * @brief 모든 제품 타입의 기본 생성자 일괄 등록
         */
        void register_all_defaults() {
            product_list::for_each_type([this]<typename T>() {
                std::string type_name = typeid(T).name();
                register_default<T>(type_name);
            });
        }
    };
    
    /**
     * @brief Abstract Factory 패턴 구현
     * @details 관련된 제품군을 생성하는 팩토리들의 팩토리
     */
    template<typename... FactoryTypes>
    class abstract_factory {
        using factory_list = core::typelist<FactoryTypes...>;
        using factory_variant = std::variant<FactoryTypes...>;
        
    private:
        std::unordered_map<std::string, factory_variant> factories_;
        
    public:
        /**
         * @brief 구체적 팩토리 등록
         */
        template<typename FactoryType>
        void register_factory(const std::string& family_name, FactoryType factory) {
            static_assert(factory_list::template contains<FactoryType>(), 
                "FactoryType must be in the factory list");
            
            factories_[family_name] = std::move(factory);
        }
        
        /**
         * @brief 팩토리 조회
         */
        template<typename FactoryType>
        FactoryType& get_factory(const std::string& family_name) {
            static_assert(factory_list::template contains<FactoryType>(), 
                "FactoryType must be in the factory list");
            
            auto it = factories_.find(family_name);
            if (it == factories_.end()) {
                throw std::runtime_error("Unknown factory family: " + family_name);
            }
            
            return std::get<FactoryType>(it->second);
        }
    };
}
