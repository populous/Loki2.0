/**
 * @file include/patterns/command.hpp
 * @brief 검색 결과 [2] "std::variant and std::visit" + 검색 결과 [1] "Command Interface" 통합
 * @details MetaLoki 2.0 Command Pattern - TypeList + Modern C++
 */

#pragma once

#include <core/typelist.hpp>
#include <core/policy_host.hpp>
#include <memory>
#include <variant>
#include <concepts>
#include <functional>
#include <queue>
#include <stack>

namespace patterns {
    
    /**
     * @brief 검색 결과 [1] "Command Interface with execute() method" 개념
     */
    template<typename T>
    concept Command = requires(T cmd) {
        cmd.execute();
    } || requires(T cmd) {
        { cmd.execute() } -> std::same_as<void>;
    };
    
    /**
     * @brief 검색 결과 [2] "POD structs that represent commands" 구현
     * @details Modern C++ Command 기반 클래스
     */
    template<typename Derived>
    class command_base {
    public:
        virtual ~command_base() = default;
        
        // 검색 결과 [1] "execute() method"
        virtual void execute() = 0;
        
        // 검색 결과 [3] "undo() or redo() methods"
        virtual void undo() {
            // 기본 구현: 아무것도 하지 않음
        }
        
        virtual bool can_undo() const { return false; }
        
        // Command 복제 (Prototype 패턴)
        virtual std::unique_ptr<command_base> clone() const = 0;
        
        // 명령 설명
        virtual std::string description() const {
            return "Command";
        }
    };
    
    /**
     * @brief 검색 결과 [2] "CommandExecutor" 스타일 Invoker
     * @details TypeList 기반 Command Executor
     */
    template<Command... CommandTypes>
    class command_invoker : public core::policy_host<
        core::policies::single_thread_policy,
        core::policies::validation_policy
    > {
        using policy_base = core::policy_host<
            core::policies::single_thread_policy,
            core::policies::validation_policy
        >;
        
    public:
        using command_list = core::typelist<CommandTypes...>;
        using command_variant = std::variant<CommandTypes...>;
        
    private:
        std::queue<command_variant> command_queue_;
        std::stack<command_variant> undo_stack_;
        std::stack<command_variant> redo_stack_;
        
    public:
        /**
         * @brief 검색 결과 [4] "execute any command it's given"
         */
        template<Command CommandType>
        void execute_command(CommandType&& command) {
            static_assert(command_list::template contains<std::decay_t<CommandType>>(), 
                "CommandType must be in the command list");
            
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            
            // 검색 결과 [2] "std::visit" 활용
            auto cmd_variant = command_variant{std::forward<CommandType>(command)};
            
            std::visit([this](auto&& cmd) {
                cmd.execute();
                
                // Undo 가능한 명령이면 스택에 저장
                if constexpr (requires { cmd.can_undo(); }) {
                    if (cmd.can_undo()) {
                        undo_stack_.push(cmd);
                        
                        // Redo 스택 클리어 (새 명령 실행 시)
                        while (!redo_stack_.empty()) {
                            redo_stack_.pop();
                        }
                    }
                }
            }, cmd_variant);
        }
        
        /**
         * @brief 검색 결과 [3] "queue operations"
         */
        template<Command CommandType>
        void queue_command(CommandType&& command) {
            static_assert(command_list::template contains<std::decay_t<CommandType>>(), 
                "CommandType must be in the command list");
            
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            command_queue_.push(command_variant{std::forward<CommandType>(command)});
        }
        
        /**
         * @brief 큐에 있는 모든 명령 실행
         */
        void execute_queued_commands() {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            
            while (!command_queue_.empty()) {
                auto command = command_queue_.front();
                command_queue_.pop();
                
                lock.unlock(); // 실행 중에는 lock 해제
                
                std::visit([](auto&& cmd) {
                    cmd.execute();
                }, command);
                
                lock.lock(); // 다시 lock
            }
        }
        
        /**
         * @brief 검색 결과 [3] "undo operations"
         */
        bool undo() {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            
            if (undo_stack_.empty()) {
                return false;
            }
            
            auto command = undo_stack_.top();
            undo_stack_.pop();
            
            std::visit([this](auto&& cmd) {
                if constexpr (requires { cmd.undo(); }) {
                    cmd.undo();
                    redo_stack_.push(cmd);
                }
            }, command);
            
            return true;
        }
        
        /**
         * @brief 검색 결과 [3] "redo operations"
         */
        bool redo() {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            
            if (redo_stack_.empty()) {
                return false;
            }
            
            auto command = redo_stack_.top();
            redo_stack_.pop();
            
            std::visit([this](auto&& cmd) {
                cmd.execute();
                undo_stack_.push(cmd);
            }, command);
            
            return true;
        }
        
        // 상태 조회
        size_t queued_command_count() const {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            return command_queue_.size();
        }
        
        size_t undo_stack_size() const {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            return undo_stack_.size();
        }
        
        size_t redo_stack_size() const {
            auto lock = this->template get_policy<core::policies::single_thread_policy>().get_lock();
            return redo_stack_.size();
        }
    };
    
    /**
     * @brief 검색 결과 [1] "Receiver" 구현
     * @details Command가 실제 작업을 위임할 수신자
     */
    template<typename Derived>
    class command_receiver {
    public:
        virtual ~command_receiver() = default;
        
        // 파생 클래스에서 구체적인 동작 구현
    };
    
    /**
     * @brief 검색 결과 [4] "decouple methods from object" 구현
     * @details 함수형 Command 래퍼
     */
    class functional_command {
    private:
        std::function<void()> execute_func_;
        std::function<void()> undo_func_;
        std::string description_;
        
    public:
        functional_command(std::function<void()> execute_func, 
                          std::function<void()> undo_func = nullptr,
                          std::string description = "Functional Command")
            : execute_func_(std::move(execute_func))
            , undo_func_(std::move(undo_func))
            , description_(std::move(description)) {}
        
        void execute() {
            if (execute_func_) {
                execute_func_();
            }
        }
        
        void undo() {
            if (undo_func_) {
                undo_func_();
            }
        }
        
        bool can_undo() const {
            return static_cast<bool>(undo_func_);
        }
        
        const std::string& description() const {
            return description_;
        }
    };
}
