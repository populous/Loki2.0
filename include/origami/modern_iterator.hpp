/**
 * @file include/origami/modern_iterator.hpp
 * @brief 검색 결과 [3][5] C++20 Concepts 완전 구현
 */

#pragma once

#include <origami/iterator.hpp>
#include <ranges>
#include <algorithm>

namespace metaloki::origami {
    
    /**
     * @brief 검색 결과 [3] "C++20 Concepts for custom container" 구현
     */
    template<typename ElementType>
    class origami_container {
    public:
        using value_type = ElementType;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        
        // 검색 결과 [5] "Iterator with C++20 Concepts" 구현
        class iterator {
        public:
            // 검색 결과 [5] "iterator_concept is a reserved name"
            using iterator_concept = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = ElementType;
            using pointer = value_type*;
            using reference = value_type&;
            
        private:
            pointer ptr_;
            const origami_container* container_;
            std::vector<size_t> position_stack_; // 위치 추적용
            
        public:
            // 검색 결과 [3] "Default constructor is required"
            iterator() : ptr_(nullptr), container_(nullptr) {}
            
            iterator(pointer p, const origami_container* c) 
                : ptr_(p), container_(c) {}
            
            // 검색 결과 [8] "reference operator*() const"
            reference operator*() const { return *ptr_; }
            pointer operator->() const { return ptr_; }
            
            // 검색 결과 [3] "++operator" 구현
            iterator& operator++() {
                advance();
                return *this;
            }
            
            iterator operator++(int) {
                iterator tmp = *this;
                ++(*this);
                return tmp;
            }
            
            // 검색 결과 [3] "==operator" (C++20 자동 생성 !=)
            bool operator==(const iterator& other) const {
                return ptr_ == other.ptr_;
            }
            
            auto operator<=>(const iterator& other) const = default;
            
        private:
            void advance() {
                if (!ptr_ || !container_) return;
                
                // ORIGAMI 특화 순회 로직
                // (실제 구현에서는 컨테이너 구조에 따라 결정)
                ++ptr_;
            }
        };
        
        // 검색 결과 [8] "begin() and end() methods"
        iterator begin() { 
            if (data_.empty()) return end();
            return iterator(&data_[0], this); 
        }
        
        iterator end() { 
            if (data_.empty()) return iterator(nullptr, this);
            return iterator(&data_[data_.size()], this); 
        }
        
        // const 버전
        iterator begin() const { 
            if (data_.empty()) return end();
            return iterator(const_cast<value_type*>(&data_[0]), this); 
        }
        
        iterator end() const { 
            if (data_.empty()) return iterator(nullptr, this);
            return iterator(const_cast<value_type*>(&data_[data_.size()]), this); 
        }
        
        // 요소 추가
        void push_back(const value_type& value) {
            data_.push_back(value);
        }
        
        void push_back(value_type&& value) {
            data_.push_back(std::move(value));
        }
        
        // 크기 정보
        size_type size() const { return data_.size(); }
        bool empty() const { return data_.empty(); }
        
        // 검색 결과 [3] "ranges support"
        auto as_range() const {
            return std::ranges::subrange(begin(), end());
        }
        
    private:
        std::vector<value_type> data_;
    };
    
    // 검색 결과 [5] "static_assert" 검증
    template<typename T>
    void verify_iterator_concepts() {
        using container_type = origami_container<T>;
        using iterator_type = typename container_type::iterator;
        
        static_assert(std::input_iterator<iterator_type>);
        static_assert(std::forward_iterator<iterator_type>);
        static_assert(std::sentinel_for<iterator_type, iterator_type>);
    }
    
    /**
     * @brief 검색 결과 [1] "traverse operator" 구현
     * @details Oxford 논문의 traverse 개념 적용
     */
    template<Traversable Container, typename Operation>
    void traverse(Container&& container, Operation&& op) {
        // 검색 결과 [1] "element-by-element access to a collection"
        std::ranges::for_each(container, std::forward<Operation>(op));
    }
    
    /**
     * @brief 검색 결과 [6] "collect and disperse" 고차 함수
     */
    template<Traversable Container, typename Accumulator, typename Transform>
    auto collect_and_transform(Container&& container, 
                              Accumulator&& accumulator, 
                              Transform&& transform) {
        using element_type = typename std::decay_t<Container>::value_type;
        using result_type = std::invoke_result_t<Transform, element_type>;
        
        std::vector<result_type> results;
        results.reserve(container.size());
        
        for (const auto& element : container) {
            accumulator(element);
            results.push_back(transform(element));
        }
        
        return results;
    }
}

// 검색 결과 [5] 컴파일 타임 검증
namespace metaloki::origami::tests {
    void verify_concepts() {
        verify_iterator_concepts<int>();
        verify_iterator_concepts<std::string>();
        verify_iterator_concepts<double>();
        
        using test_container = origami_container<int>;
        static_assert(std::ranges::range<test_container>);
        static_assert(Traversable<test_container>);
    }
}
