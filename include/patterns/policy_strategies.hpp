/**
 * @file include/patterns/policy_strategies.hpp
 * @brief 검색 결과 [2] "STL policies" 스타일 구현
 */

#pragma once

#include <patterns/strategy.hpp>
#include <vector>
#include <memory>

namespace patterns::policies {
    
    /**
     * @brief 검색 결과 [2] "std::allocator" 스타일 정책들
     */
    
    // 메모리 할당 전략들
    template<typename T>
    struct standard_allocator_policy {
        using value_type = T;
        
        T* allocate(size_t n) {
            return static_cast<T*>(std::malloc(sizeof(T) * n));
        }
        
        void deallocate(T* ptr, size_t) {
            std::free(ptr);
        }
        
        void execute(const std::string& msg) {
            std::cout << "Standard allocator: " << msg << std::endl;
        }
    };
    
    template<typename T>
    struct pool_allocator_policy {
        using value_type = T;
        
        T* allocate(size_t n) {
            // 풀 할당 시뮬레이션
            static std::vector<T> pool(1000);
            static size_t offset = 0;
            
            if (offset + n > pool.size()) {
                throw std::bad_alloc{};
            }
            
            T* result = &pool[offset];
            offset += n;
            return result;
        }
        
        void deallocate(T*, size_t) {
            // 풀에서는 개별 해제 없음
        }
        
        void execute(const std::string& msg) {
            std::cout << "Pool allocator: " << msg << std::endl;
        }
    };
    
    /**
     * @brief 검색 결과 [2] "default hash function" 스타일
     */
    template<typename Key>
    struct default_hash_policy {
        size_t execute(const Key& key) const {
            return std::hash<Key>{}(key);
        }
    };
    
    template<typename Key>
    struct custom_hash_policy {
        size_t execute(const Key& key) const {
            // 사용자 정의 해시 함수
            return static_cast<size_t>(key) * 31;
        }
    };
    
    /**
     * @brief 검색 결과 [2] "Policy is a generic function or class" 구현
     * @details MetaLoki Container with Policy Support
     */
    template<typename T, 
             template<typename> class AllocatorPolicy = standard_allocator_policy,
             template<typename> class HashPolicy = default_hash_policy>
    class policy_container : public AllocatorPolicy<T>, 
                           public HashPolicy<T>,
                           public core::policy_host<core::policies::single_thread_policy> {
        
        using allocator_type = AllocatorPolicy<T>;
        using hash_type = HashPolicy<T>;
        using policy_base = core::policy_host<core::policies::single_thread_policy>;
        
    private:
        T* data_;
        size_t size_;
        size_t capacity_;
        
    public:
        explicit policy_container(size_t initial_capacity = 10) 
            : size_(0), capacity_(initial_capacity) {
            data_ = allocator_type::allocate(capacity_);
        }
        
        ~policy_container() {
            for (size_t i = 0; i < size_; ++i) {
                data_[i].~T();
            }
            allocator_type::deallocate(data_, capacity_);
        }
        
        void push_back(const T& value) {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            
            if (size_ >= capacity_) {
                resize();
            }
            
            new(data_ + size_) T(value);
            ++size_;
        }
        
        size_t hash_element(const T& element) const {
            return hash_type::execute(element);
        }
        
        void log_allocator_info(const std::string& msg) {
            allocator_type::execute(msg);
        }
        
    private:
        void resize() {
            size_t new_capacity = capacity_ * 2;
            T* new_data = allocator_type::allocate(new_capacity);
            
            for (size_t i = 0; i < size_; ++i) {
                new(new_data + i) T(std::move(data_[i]));
                data_[i].~T();
            }
            
            allocator_type::deallocate(data_, capacity_);
            data_ = new_data;
            capacity_ = new_capacity;
        }
    };
}
