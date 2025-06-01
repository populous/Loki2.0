/**
 * @file include/utility/simple_state.hpp
 * @brief MetaLoki 2.0 Simple State Utility
 * @details Modern C++ Design 철학 기반 경량 상태 관리 유틸리티
 * @author MetaLoki Team
 * @version 2.0.0
 */

#pragma once

#include <type_traits>
#include <concepts>
#include <functional>
#include <optional>
#include <variant>
#include <string>
#include <unordered_map>
#include <chrono>

namespace utility {
    
    /**
     * @brief 검색 결과 [1] "State pattern" 경량 구현
     * @details Andrei Alexandrescu 스타일 템플릿 기반 상태 관리
     */
    template<typename StateID>
    concept SimpleStateIdentifier = requires {
        std::is_enum_v<StateID> || std::is_integral_v<StateID> || std::is_same_v<StateID, std::string>;
        requires std::equality_comparable<StateID>;
        requires std::hash<StateID>;
    };
    
    /**
     * @brief 기본 상태 정보 구조체
     */
    template<SimpleStateIdentifier StateID>
    struct state_info {
        StateID id;
        std::string name;
        std::chrono::steady_clock::time_point entered_at;
        std::optional<std::chrono::milliseconds> duration;
        
        explicit state_info(StateID state_id, std::string state_name = "")
            : id(state_id), name(std::move(state_name))
            , entered_at(std::chrono::steady_clock::now()) {}
        
        void mark_exit() {
            auto now = std::chrono::steady_clock::now();
            duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - entered_at);
        }
        
        bool has_duration() const { return duration.has_value(); }
        auto get_duration() const { return duration.value_or(std::chrono::milliseconds{0}); }
    };
    
    /**
     * @brief Modern C++ Design 스타일 Simple State 관리자
     * @details 경량화된 상태 머신, FSM의 기반 클래스로 활용 가능
     */
    template<SimpleStateIdentifier StateID>
    class simple_state {
    public:
        using state_id_type = StateID;
        using state_info_type = state_info<StateID>;
        using transition_callback = std::function<void(StateID, StateID)>;
        using enter_callback = std::function<void(StateID)>;
        using exit_callback = std::function<void(StateID)>;
        
    private:
        std::optional<StateID> current_state_;
        std::unordered_map<StateID, std::string> state_names_;
        std::vector<state_info_type> state_history_;
        
        // 콜백 함수들
        std::unordered_map<StateID, enter_callback> enter_callbacks_;
        std::unordered_map<StateID, exit_callback> exit_callbacks_;
        std::optional<transition_callback> transition_callback_;
        
        // 통계 정보
        std::unordered_map<StateID, size_t> state_visit_count_;
        std::unordered_map<StateID, std::chrono::milliseconds> total_time_in_state_;
        
    public:
        simple_state() = default;
        
        /**
         * @brief 상태 등록
         */
        void register_state(StateID id, std::string name = "") {
            state_names_[id] = name.empty() ? std::to_string(static_cast<int>(id)) : std::move(name);
            state_visit_count_[id] = 0;
            total_time_in_state_[id] = std::chrono::milliseconds{0};
        }
        
        /**
         * @brief 상태 전환
         */
        void transition_to(StateID new_state) {
            StateID old_state{};
            bool had_previous_state = false;
            
            // 이전 상태 처리
            if (current_state_.has_value()) {
                old_state = current_state_.value();
                had_previous_state = true;
                
                // Exit callback 호출
                if (auto it = exit_callbacks_.find(old_state); it != exit_callbacks_.end()) {
                    it->second(old_state);
                }
                
                // 이전 상태 히스토리 완료
                if (!state_history_.empty()) {
                    auto& last_state = state_history_.back();
                    if (last_state.id == old_state && !last_state.has_duration()) {
                        last_state.mark_exit();
                        total_time_in_state_[old_state] += last_state.get_duration();
                    }
                }
            }
            
            // 새 상태 설정
            current_state_ = new_state;
            state_visit_count_[new_state]++;
            
            // 새 상태 히스토리 추가
            auto state_name = get_state_name(new_state);
            state_history_.emplace_back(new_state, state_name);
            
            // Enter callback 호출
            if (auto it = enter_callbacks_.find(new_state); it != enter_callbacks_.end()) {
                it->second(new_state);
            }
            
            // Transition callback 호출
            if (transition_callback_.has_value() && had_previous_state) {
                transition_callback_.value()(old_state, new_state);
            }
        }
        
        /**
         * @brief 현재 상태 조회
         */
        std::optional<StateID> get_current_state() const {
            return current_state_;
        }
        
        /**
         * @brief 특정 상태인지 확인
         */
        bool is_in_state(StateID state) const {
            return current_state_.has_value() && current_state_.value() == state;
        }
        
        /**
         * @brief 상태 이름 조회
         */
        std::string get_state_name(StateID state) const {
            if (auto it = state_names_.find(state); it != state_names_.end()) {
                return it->second;
            }
            return "Unknown State";
        }
        
        /**
         * @brief 현재 상태 이름 조회
         */
        std::string get_current_state_name() const {
            if (current_state_.has_value()) {
                return get_state_name(current_state_.value());
            }
            return "No State";
        }
        
        /**
         * @brief 콜백 등록
         */
        void on_enter(StateID state, enter_callback callback) {
            enter_callbacks_[state] = std::move(callback);
        }
        
        void on_exit(StateID state, exit_callback callback) {
            exit_callbacks_[state] = std::move(callback);
        }
        
        void on_transition(transition_callback callback) {
            transition_callback_ = std::move(callback);
        }
        
        /**
         * @brief 상태 히스토리 조회
         */
        const std::vector<state_info_type>& get_state_history() const {
            return state_history_;
        }
        
        /**
         * @brief 통계 정보
         */
        size_t get_visit_count(StateID state) const {
            if (auto it = state_visit_count_.find(state); it != state_visit_count_.end()) {
                return it->second;
            }
            return 0;
        }
        
        std::chrono::milliseconds get_total_time_in_state(StateID state) const {
            if (auto it = total_time_in_state_.find(state); it != total_time_in_state_.end()) {
                return it->second;
            }
            return std::chrono::milliseconds{0};
        }
        
        /**
         * @brief 현재 상태에서 경과 시간
         */
        std::chrono::milliseconds get_current_state_duration() const {
            if (!state_history_.empty()) {
                const auto& current = state_history_.back();
                if (!current.has_duration()) {
                    auto now = std::chrono::steady_clock::now();
                    return std::chrono::duration_cast<std::chrono::milliseconds>(now - current.entered_at);
                }
            }
            return std::chrono::milliseconds{0};
        }
        
        /**
         * @brief 등록된 모든 상태 조회
         */
        std::vector<StateID> get_all_states() const {
            std::vector<StateID> states;
            states.reserve(state_names_.size());
            
            for (const auto& [id, name] : state_names_) {
                states.push_back(id);
            }
            
            return states;
        }
        
        /**
         * @brief 상태 초기화
         */
        void reset() {
            // 현재 상태가 있다면 종료 처리
            if (current_state_.has_value()) {
                auto current = current_state_.value();
                
                if (auto it = exit_callbacks_.find(current); it != exit_callbacks_.end()) {
                    it->second(current);
                }
                
                if (!state_history_.empty()) {
                    auto& last_state = state_history_.back();
                    if (!last_state.has_duration()) {
                        last_state.mark_exit();
                        total_time_in_state_[current] += last_state.get_duration();
                    }
                }
            }
            
            current_state_.reset();
        }
        
        /**
         * @brief 디버그 정보 출력
         */
        void print_debug_info() const {
            std::cout << "=== Simple State Debug Info ===" << std::endl;
            std::cout << "Current State: " << get_current_state_name() << std::endl;
            std::cout << "Current Duration: " << get_current_state_duration().count() << "ms" << std::endl;
            
            std::cout << "\nState Statistics:" << std::endl;
            for (const auto& [state, count] : state_visit_count_) {
                auto total_time = get_total_time_in_state(state);
                std::cout << "  " << get_state_name(state) 
                          << ": " << count << " visits, " 
                          << total_time.count() << "ms total" << std::endl;
            }
            
            std::cout << "\nState History (last 10):" << std::endl;
            size_t start = state_history_.size() > 10 ? state_history_.size() - 10 : 0;
            for (size_t i = start; i < state_history_.size(); ++i) {
                const auto& info = state_history_[i];
                std::cout << "  " << i << ": " << get_state_name(info.id);
                if (info.has_duration()) {
                    std::cout << " (" << info.get_duration().count() << "ms)";
                } else {
                    std::cout << " (current)";
                }
                std::cout << std::endl;
            }
        }
    };
    
    /**
     * @brief 열거형 기반 편의 타입
     */
    template<typename StateEnum>
    using enum_simple_state = simple_state<StateEnum>;
    
    /**
     * @brief 문자열 기반 편의 타입
     */
    using string_simple_state = simple_state<std::string>;
    
    /**
     * @brief 정수 기반 편의 타입
     */
    using int_simple_state = simple_state<int>;
}
