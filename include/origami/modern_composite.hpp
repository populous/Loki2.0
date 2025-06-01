/**
 * @file include/origami/modern_composite.hpp
 * @brief 검색 결과 [6] "값 기반 다형성" 구현
 * @details 상속 없는 현대적 Composite
 */

#pragma once

#include <core/typelist.hpp>
#include <variant>
#include <vector>
#include <iostream>
#include <memory>

namespace metaloki::origami {
    
    /**
     * @brief 검색 결과 [6] "C++20: std::variant로 Leaf/Composite를 값 기반 다형성"
     */
    template<typename LeafValueType>
    class modern_composite {
    private:
        // 자기 참조 타입을 위한 트릭
        struct composite_impl;
        
        // 값 타입과 복합 타입을 포함하는 variant
        using node_variant = std::variant<LeafValueType, std::unique_ptr<composite_impl>>;
        
        // 복합 노드 구현
        struct composite_impl {
            std::string name;
            std::vector<node_variant> children;
            
            explicit composite_impl(std::string n = "Composite") : name(std::move(n)) {}
            
            void add_leaf(LeafValueType value) {
                children.push_back(std::move(value));
            }
            
            void add_composite(std::unique_ptr<composite_impl> child) {
                children.push_back(std::move(child));
            }
            
            // 포워딩 참조를 통한 완벽한 전달
            template<typename... Args>
            composite_impl& emplace_composite(Args&&... args) {
                auto child = std::make_unique<composite_impl>(std::forward<Args>(args)...);
                auto& ref = *child;
                children.push_back(std::move(child));
                return ref;
            }
        };
        
        // 루트 노드
        composite_impl root_;
        
    public:
        // 생성자
        explicit modern_composite(std::string name = "Root") : root_(std::move(name)) {}
        
        // 리프 노드 추가
        void add_leaf(LeafValueType value) {
            root_.add_leaf(std::move(value));
        }
        
        // 새 복합 노드 추가
        modern_composite& add_composite(std::string name = "Composite") {
            return root_.emplace_composite(std::move(name));
        }
        
        // 검색 결과 [6] "std::visit로 render 함수 구현"
        void render() const {
            std::cout << "Modern Composite '" << root_.name << "' {\n";
            
            for (const auto& child : root_.children) {
                std::visit([](const auto& value) {
                    using T = std::decay_t<decltype(value)>;
                    
                    if constexpr (std::is_same_v<T, LeafValueType>) {
                        if constexpr (requires { std::cout << value; }) {
                            std::cout << "  Leaf: " << value << '\n';
                        } else {
                            std::cout << "  Leaf: [value]\n";
                        }
                    } else if constexpr (std::is_same_v<T, std::unique_ptr<composite_impl>>) {
                        std::cout << "  Composite: '" << value->name << "' with " 
                                  << value->children.size() << " children\n";
                    }
                }, child);
            }
            
            std::cout << "}" << std::endl;
        }
        
        // 깊이 우선 순회
        template<typename LeafOperation, typename CompositeOperation>
        void traverse_depth_first(LeafOperation&& leaf_op, CompositeOperation&& composite_op) const {
            // 루트 복합 노드에 작업 적용
            composite_op(root_.name, root_.children.size());
            
            // 자식 순회 함수
            std::function<void(const composite_impl&)> visit_composite = 
                [&](const composite_impl& node) {
                    for (const auto& child : node.children) {
                        std::visit([&](const auto& value) {
                            using T = std::decay_t<decltype(value)>;
                            
                            if constexpr (std::is_same_v<T, LeafValueType>) {
                                leaf_op(value);
                            } else if constexpr (std::is_same_v<T, std::unique_ptr<composite_impl>>) {
                                composite_op(value->name, value->children.size());
                                visit_composite(*value);
                            }
                        }, child);
                    }
                };
            
            // 루트부터 순회 시작
            visit_composite(root_);
        }
    };
}
