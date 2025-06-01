/**
 * @file include/optimization/performance_patterns.hpp
 * @brief 검색 결과 [8] "system performance" 최적화 패턴들
 */

#pragma once

#include <core/typelist.hpp>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace optimization {
    
    /**
     * @brief 검색 결과 [8] "Singleton Pattern" - 전역 자원 관리
     */
    template<typename T>
    class singleton {
    private:
        static std::unique_ptr<T> instance_;
        static std::once_flag once_flag_;
        
        singleton() = default;
        
    public:
        static T& get_instance() {
            std::call_once(once_flag_, []() {
                instance_ = std::make_unique<T>();
            });
            return *instance_;
        }
        
        // 복사/이동 방지
        singleton(const singleton&) = delete;
        singleton& operator=(const singleton&) = delete;
        singleton(singleton&&) = delete;
        singleton& operator=(singleton&&) = delete;
    };
    
    template<typename T>
    std::unique_ptr<T> singleton<T>::instance_ = nullptr;
    
    template<typename T>
    std::once_flag singleton<T>::once_flag_;
    
    /**
     * @brief 검색 결과 [8] "Flyweight Pattern" - 메모리 최적화
     */
    template<typename KeyType, typename ValueType>
    class flyweight_factory {
    private:
        std::unordered_map<KeyType, std::weak_ptr<ValueType>> flyweights_;
        mutable std::shared_mutex mutex_;
        
    public:
        std::shared_ptr<ValueType> get_flyweight(const KeyType& key) {
            std::shared_lock lock(mutex_);
            
            auto it = flyweights_.find(key);
            if (it != flyweights_.end()) {
                if (auto shared = it->second.lock()) {
                    return shared;
                }
            }
            
            // 새로운 flyweight 생성
            lock.unlock();
            std::unique_lock unique_lock(mutex_);
            
            // Double-checked locking
            it = flyweights_.find(key);
            if (it != flyweights_.end()) {
                if (auto shared = it->second.lock()) {
                    return shared;
                }
            }
            
            auto flyweight = std::make_shared<ValueType>(key);
            flyweights_[key] = flyweight;
            return flyweight;
        }
        
        void cleanup_expired() {
            std::unique_lock lock(mutex_);
            
            for (auto it = flyweights_.begin(); it != flyweights_.end();) {
                if (it->second.expired()) {
                    it = flyweights_.erase(it);
                } else {
                    ++it;
                }
            }
        }
    };
    
    /**
     * @brief 검색 결과 [8] "Proxy Pattern" - 지연 로딩 및 캐싱
     */
    template<typename T>
    class lazy_proxy {
    private:
        mutable std::unique_ptr<T> real_object_;
        mutable std::once_flag initialized_;
        std::function<std::unique_ptr<T>()> factory_;
        
    public:
        explicit lazy_proxy(std::function<std::unique_ptr<T>()> factory)
            : factory_(std::move(factory)) {}
        
        const T& get() const {
            std::call_once(initialized_, [this]() {
                real_object_ = factory_();
            });
            return *real_object_;
        }
        
        T& get() {
            std::call_once(initialized_, [this]() {
                real_object_ = factory_();
            });
            return *real_object_;
        }
        
        bool is_initialized() const {
            return real_object_ != nullptr;
        }
    };
    
    /**
     * @brief MetaLoki 특화 성능 최적화 관리자
     */
    class performance_optimizer : public singleton<performance_optimizer> {
    private:
        // Pattern별 성능 캐시
        flyweight_factory<std::string, origami::composite<>> composite_cache_;
        std::unordered_map<std::string, std::chrono::steady_clock::time_point> timing_cache_;
        
    public:
        /**
         * @brief 검색 결과 [9] "Performance Optimization" 전략 적용
         */
        template<typename PatternType>
        auto create_optimized_pattern(const std::string& key) {
            // Flyweight로 동일한 패턴 재사용
            return composite_cache_.get_flyweight(key);
        }
        
        /**
         * @brief 성능 측정 및 캐싱
         */
        template<typename Function>
        auto measure_and_cache(const std::string& operation, Function&& func) {
            auto start = std::chrono::steady_clock::now();
            auto result = func();
            auto end = std::chrono::steady_clock::now();
            
            timing_cache_[operation] = end - start;
            return result;
        }
        
        /**
         * @brief 검색 결과 [9] "Refactoring Strategies for Optimization"
         */
        void optimize_all_patterns() {
            // 사용하지 않는 flyweight 정리
            composite_cache_.cleanup_expired();
            
            // 오래된 타이밍 캐시 정리
            auto now = std::chrono::steady_clock::now();
            for (auto it = timing_cache_.begin(); it != timing_cache_.end();) {
                if (now - it->second > std::chrono::minutes(5)) {
                    it = timing_cache_.erase(it);
                } else {
                    ++it;
                }
            }
        }
        
        // 성능 통계 조회
        std::chrono::nanoseconds get_operation_time(const std::string& operation) const {
            auto it = timing_cache_.find(operation);
            return it != timing_cache_.end() ? 
                std::chrono::duration_cast<std::chrono::nanoseconds>(it->second.time_since_epoch()) :
                std::chrono::nanoseconds::zero();
        }
    };
}
