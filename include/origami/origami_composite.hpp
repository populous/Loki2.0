/**
 * @file include/metaloki/origami/origami_composite.hpp
 * @brief 검색 결과 [1] "Multi-functional structures" 구현
 * @details ORIGAMI 특화 Composite 확장
 */

#pragma once

#include <origami/composite.hpp>
#include <core/policy_host.hpp>
#include <functional>

namespace metaloki::origami {
    
    /**
     * @brief 검색 결과 [1] "origami cores" 구현
     * @details Policy 기반 Origami Composite
     */
    template<typename ElementType, 
             typename ThreadingPolicy = core::policies::single_thread_policy,
             typename ValidationPolicy = core::policies::validation_policy>
    class origami_composite : 
        public component_base<origami_composite<ElementType, ThreadingPolicy, ValidationPolicy>>,
        public core::policy_host<ThreadingPolicy, ValidationPolicy> {
        
        using policy_base = core::policy_host<ThreadingPolicy, ValidationPolicy>;
        
    private:
        // 검색 결과 [1] "Miura-ori" 구조
        struct node {
            ElementType element;
            std::vector<size_t> connections;
            
            explicit node(ElementType e) : element(std::move(e)) {}
        };
        
        std::vector<node> nodes_;
        std::string pattern_name_;
        
    public:
        // 생성자
        explicit origami_composite(std::string pattern_name = "Miura-ori") 
            : pattern_name_(std::move(pattern_name)) {}
        
        // 요소 추가
        template<typename... Args>
        size_t add_element(Args&&... args) {
            auto lock = this->template get_policy<ThreadingPolicy>().get_lock();
            
            // 요소 생성 및 추가
            size_t index = nodes_.size();
            nodes_.emplace_back(ElementType(std::forward<Args>(args)...));
            
            return index;
        }
        
        // 연결 추가 (검색 결과 [1] "crease lines")
        void connect(size_t from, size_t to) {
            auto lock = this->template get_policy<ThreadingPolicy>().get_lock();
            
            this->template get_policy<ValidationPolicy>().assert_that(
                from < nodes_.size() && to < nodes_.size(),
                "Invalid node indices"
            );
            
            nodes_[from].connections.push_back(to);
        }
        
        // 검색 결과 [1] "Miura-derivative prismatic base patterns"
        void create_miura_pattern(size_t width, size_t height) {
            // Miura-ori 패턴 생성 - 기본 격자 구조
            std::vector<size_t> indices;
            indices.reserve(width * height);
            
            // 노드 생성
            for (size_t i = 0; i < width * height; ++i) {
                indices.push_back(add_element());
            }
            
            // 연결 생성 (가로)
            for (size_t y = 0; y < height; ++y) {
                for (size_t x = 0; x < width - 1; ++x) {
                    size_t idx = y * width + x;
                    connect(idx, idx + 1);
                    connect(idx + 1, idx);  // 양방향
                }
            }
            
            // 연결 생성 (세로)
            for (size_t y = 0; y < height - 1; ++y) {
                for (size_t x = 0; x < width; ++x) {
                    size_t idx = y * width + x;
                    connect(idx, idx + width);
                    connect(idx + width, idx);  // 양방향
                }
            }
            
            // 지그재그 대각선 (Miura-ori 특징)
            for (size_t y = 0; y < height - 1; ++y) {
                for (size_t x = 0; x < width - 1; ++x) {
                    size_t idx = y * width + x;
                    
                    if ((x + y) % 2 == 0) {
                        connect(idx, idx + width + 1);
                        connect(idx + width + 1, idx);
                    } else {
                        connect(idx + 1, idx + width);
                        connect(idx + width, idx + 1);
                    }
                }
            }
        }
        
        // 요소 접근
        const ElementType& get_element(size_t index) const {
            this->template get_policy<ValidationPolicy>().assert_that(
                index < nodes_.size(),
                "Invalid node index"
            );
            
            return nodes_[index].element;
        }
        
        ElementType& get_element(size_t index) {
            this->template get_policy<ValidationPolicy>().assert_that(
                index < nodes_.size(),
                "Invalid node index"
            );
            
            return nodes_[index].element;
        }
        
        // 검색 결과 [4] "traverse" 구현
        template<typename Operation>
        void traverse(Operation&& op) const {
            auto lock = this->template get_policy<ThreadingPolicy>().get_lock();
            
            // 모든 노드에 작업 적용
            for (size_t i = 0; i < nodes_.size(); ++i) {
                op(i, nodes_[i].element);
            }
        }
        
        // 검색 결과 [3] "connect_to" 흉내
        template<typename Function>
        void visit_connections(size_t node_index, Function&& func) const {
            auto lock = this->template get_policy<ThreadingPolicy>().get_lock();
            
            this->template get_policy<ValidationPolicy>().assert_that(
                node_index < nodes_.size(),
                "Invalid node index"
            );
            
            const auto& node = nodes_[node_index];
            for (size_t connected : node.connections) {
                func(node_index, connected, node.element, nodes_[connected].element);
            }
        }
        
        // 렌더링 구현
        void render_impl() const {
            std::cout << "Origami Pattern '" << pattern_name_ << "' with " 
                      << nodes_.size() << " elements" << std::endl;
            
            for (size_t i = 0; i < nodes_.size(); ++i) {
                std::cout << "Node " << i << ": ";
                
                if constexpr (requires { std::cout << nodes_[i].element; }) {
                    std::cout << nodes_[i].element;
                } else {
                    std::cout << "[Element]";
                }
                
                std::cout << " -> Connections: ";
                for (size_t conn : nodes_[i].connections) {
                    std::cout << conn << " ";
                }
                std::cout << std::endl;
            }
        }
        
        // 복제 구현
        std::unique_ptr<origami_composite> clone_impl() const {
            auto clone = std::make_unique<origami_composite>(pattern_name_);
            clone->nodes_ = nodes_;  // 복사 가능한 요소 사용
            return clone;
        }
    };
}
