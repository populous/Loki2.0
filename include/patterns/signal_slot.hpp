/**
 * @file include/patterns/signal_slot.hpp
 * @brief 검색 결과 [4] "signal & slot" 구현
 */

#pragma once

#include <patterns/observer.hpp>
#include <functional>
#include <vector>

namespace patterns::signals {
    
    /**
     * @brief 검색 결과 [4] "signal & slot" - Qt 스타일 신호
     */
    template<typename... Args>
    class signal {
    public:
        using slot_type = std::function<void(Args...)>;
        using connection_id = size_t;
        
    private:
        std::vector<std::pair<connection_id, slot_type>> slots_;
        connection_id next_id_ = 0;
        
    public:
        /**
         * @brief Slot 연결
         */
        connection_id connect(slot_type slot) {
            auto id = next_id_++;
            slots_.emplace_back(id, std::move(slot));
            return id;
        }
        
        /**
         * @brief Lambda를 slot으로 연결
         */
        template<typename Lambda>
        connection_id connect(Lambda&& lambda) {
            return connect(slot_type(std::forward<Lambda>(lambda)));
        }
        
        /**
         * @brief 멤버 함수를 slot으로 연결
         */
        template<typename Object, typename Method>
        connection_id connect(Object* obj, Method method) {
            return connect([obj, method](Args... args) {
                (obj->*method)(args...);
            });
        }
        
        /**
         * @brief Connection 해제
         */
        void disconnect(connection_id id) {
            slots_.erase(
                std::remove_if(slots_.begin(), slots_.end(),
                    [id](const auto& pair) { return pair.first == id; }
                ),
                slots_.end()
            );
        }
        
        /**
         * @brief 모든 연결 해제
         */
        void disconnect_all() {
            slots_.clear();
        }
        
        /**
         * @brief 신호 발생 (emit)
         */
        void emit(Args... args) {
            // 검색 결과 [1] snapshot을 만들어 안전하게 호출
            auto slots_snapshot = slots_;
            
            for (const auto& [id, slot] : slots_snapshot) {
                try {
                    slot(args...);
                } catch (const std::exception& e) {
                    std::cerr << "Signal emission failed: " << e.what() << std::endl;
                }
            }
        }
        
        /**
         * @brief operator() 오버로딩으로 emit
         */
        void operator()(Args... args) {
            emit(args...);
        }
        
        /**
         * @brief 연결된 slot 개수
         */
        size_t connection_count() const {
            return slots_.size();
        }
    };
    
    /**
     * @brief 검색 결과 [4] "Boost, Qt" 스타일 편의 매크로
     */
    #define METALOKI_SIGNAL(name, ...) \
        metaloki::patterns::signals::signal<__VA_ARGS__> name
    
    #define METALOKI_EMIT(signal, ...) \
        signal.emit(__VA_ARGS__)
    
    #define METALOKI_CONNECT(signal, slot) \
        signal.connect(slot)
}
