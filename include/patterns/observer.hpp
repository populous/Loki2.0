/**
 * @file include/patterns/observer.hpp
 * @brief 검색 결과 [1] "invalidation problems" 해결 + 검색 결과 [2] "RAII" 적용
 * @details MetaLoki 2.0 Observer Pattern - TypeList + Thread Safe
 */

#pragma once

#include <core/typelist.hpp>
#include <core/policy_host.hpp>
#include <memory>
#include <vector>
#include <variant>
#include <concepts>
#include <functional>
#include <algorithm>

namespace metaloki::patterns {
    
    /**
     * @brief 검색 결과 [4] "Observer concept" 정의
     */
    template<typename T, typename EventType = void>
    concept Observer = requires(T observer, EventType event) {
        observer.notify(event);
    } || requires(T observer) {
        observer.notify();
    };
    
    /**
     * @brief 검색 결과 [2] "Subject manages its collection of observers"
     * @details 검색 결과 [1] invalidation 문제 해결된 Subject
     */
    template<Observer... ObserverTypes>
    class subject : public core::policy_host<
        core::policies::single_thread_policy,
        core::policies::validation_policy
    > {
        using policy_base = core::policy_host<
            core::policies::single_thread_policy,
            core::policies::validation_policy
        >;
        
    public:
        using observer_list = core::typelist<ObserverTypes...>;
        using observer_variant = std::variant<std::shared_ptr<ObserverTypes>...>;
        
    private:
        std::vector<observer_variant> observers_;
        bool notification_in_progress_ = false;
        
    public:
        /**
         * @brief 검색 결과 [2] "allows the observers to register"
         */
        template<Observer ObserverType>
        void add_observer(std::shared_ptr<ObserverType> observer) {
            static_assert(observer_list::template contains<ObserverType>(), 
                "ObserverType must be in the observer list");
            
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            
            this->template get_policy<core::policies::validation_policy>().assert_that(
                observer != nullptr, "Observer cannot be null");
            
            observers_.push_back(observer);
        }
        
        /**
         * @brief 검색 결과 [2] "allows the observers to unregister"
         */
        template<Observer ObserverType>
        void remove_observer(std::shared_ptr<ObserverType> observer) {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            
            observers_.erase(
                std::remove_if(observers_.begin(), observers_.end(),
                    [&observer](const observer_variant& var) {
                        return std::visit([&observer](const auto& obs) {
                            if constexpr (std::is_same_v<std::decay_t<decltype(obs)>, std::shared_ptr<ObserverType>>) {
                                return obs == observer;
                            }
                            return false;
                        }, var);
                    }
                ),
                observers_.end()
            );
        }
        
        /**
         * @brief 검색 결과 [1] "make a snapshot" - invalidation 문제 해결
         */
        template<typename EventType = void>
        void notify_all(const EventType& event = EventType{}) {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            
            // 검색 결과 [1] "snapshot of the registered observers list"
            auto observers_snapshot = observers_;
            notification_in_progress_ = true;
            
            lock.unlock(); // snapshot 생성 후 lock 해제
            
            // 검색 결과 [1] "try/catch mechanism" 적용
            for (const auto& observer_var : observers_snapshot) {
                std::visit([&event](const auto& observer) {
                    try {
                        if (observer && !observer.expired()) { // weak_ptr 체크는 shared_ptr에서 직접 확인
                            if constexpr (std::is_void_v<EventType>) {
                                observer->notify();
                            } else {
                                observer->notify(event);
                            }
                        }
                    } catch (const std::exception& e) {
                        // 검색 결과 [1] "print notification failed"
                        std::cerr << "Notification failed: " << e.what() << std::endl;
                    }
                }, observer_var);
            }
            
            notification_in_progress_ = false;
        }
        
        /**
         * @brief Observer 개수 조회
         */
        size_t observer_count() const {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            return observers_.size();
        }
        
        /**
         * @brief 모든 Observer 제거
         */
        void clear_observers() {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            observers_.clear();
        }
    };
    
    /**
     * @brief 검색 결과 [2] "RAII Idiom" - 자동 등록/해제
     * @details ConcreteObserver 기반 클래스
     */
    template<typename Derived, typename SubjectType>
    class auto_observer {
    private:
        SubjectType& subject_;
        std::shared_ptr<Derived> self_;
        
    public:
        explicit auto_observer(SubjectType& subject) 
            : subject_(subject) {
            // 검색 결과 [2] "registers itself in its constructor"
            self_ = std::static_pointer_cast<Derived>(
                std::shared_ptr<auto_observer>(this, [](auto_observer*) {})
            );
            subject_.add_observer(self_);
        }
        
        virtual ~auto_observer() noexcept {
            // 검색 결과 [2] "unregisters itself in its destructor"
            try {
                subject_.remove_observer(self_);
            } catch (...) {
                // 소멸자에서는 예외 발생 억제
            }
        }
        
        // 복사/이동 방지 (RAII 객체의 안전성)
        auto_observer(const auto_observer&) = delete;
        auto_observer& operator=(const auto_observer&) = delete;
        auto_observer(auto_observer&&) = delete;
        auto_observer& operator=(auto_observer&&) = delete;
    };
    
    /**
     * @brief 검색 결과 [4] "lambda as a subscriber" - 함수형 Observer
     * @details Modern C++ 스타일 functional observer
     */
    template<typename EventType = void>
    class functional_observer {
    public:
        using callback_type = std::conditional_t<
            std::is_void_v<EventType>,
            std::function<void()>,
            std::function<void(const EventType&)>
        >;
        
    private:
        callback_type callback_;
        
    public:
        explicit functional_observer(callback_type callback) 
            : callback_(std::move(callback)) {}
        
        void notify() requires std::is_void_v<EventType> {
            if (callback_) {
                callback_();
            }
        }
        
        void notify(const EventType& event) requires (!std::is_void_v<EventType>) {
            if (callback_) {
                callback_(event);
            }
        }
    };
    
    /**
     * @brief 검색 결과 [3] "subscription mechanism" - Publisher/Subscriber
     * @details 검색 결과 [2] "Publisher-Subscriber (short Pub/Sub)" 구현
     */
    template<typename EventType>
    class publisher {
    public:
        using subscriber_callback = std::function<void(const EventType&)>;
        using functional_obs = functional_observer<EventType>;
        
    private:
        subject<functional_obs> subject_;
        
    public:
        /**
         * @brief 검색 결과 [3] "subscribe to or unsubscribe from a stream of events"
         */
        std::shared_ptr<functional_obs> subscribe(subscriber_callback callback) {
            auto observer = std::make_shared<functional_obs>(std::move(callback));
            subject_.add_observer(observer);
            return observer;
        }
        
        void unsubscribe(std::shared_ptr<functional_obs> observer) {
            subject_.remove_observer(observer);
        }
        
        /**
         * @brief 검색 결과 [3] "publish events"
         */
        void publish(const EventType& event) {
            subject_.notify_all(event);
        }
        
        size_t subscriber_count() const {
            return subject_.observer_count();
        }
    };
}
