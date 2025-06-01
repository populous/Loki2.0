/**
 * @file include/patterns/lightbulb_commands.hpp
 * @brief 검색 결과 [2] "SetBrightness, SetColor" 확장 구현
 */

#pragma once

#include <patterns/command.hpp>
#include <iostream>

namespace patterns::iot {
    
    /**
     * @brief 검색 결과 [2] "Lightbulb" Receiver
     */
    class lightbulb : public command_receiver<lightbulb> {
    private:
        unsigned brightness_ = 0;
        unsigned red_ = 0, green_ = 0, blue_ = 0;
        bool is_on_ = false;
        
    public:
        void set_brightness(unsigned val) {
            brightness_ = val;
            std::cout << "Lightbulb brightness set to " << brightness_ << std::endl;
        }
        
        void set_color(unsigned r, unsigned g, unsigned b) {
            red_ = r; green_ = g; blue_ = b;
            std::cout << "Lightbulb color set to RGB(" << red_ << ", " << green_ << ", " << blue_ << ")" << std::endl;
        }
        
        void turn_on() {
            is_on_ = true;
            std::cout << "Lightbulb turned ON" << std::endl;
        }
        
        void turn_off() {
            is_on_ = false;
            std::cout << "Lightbulb turned OFF" << std::endl;
        }
        
        // Getters
        unsigned brightness() const { return brightness_; }
        unsigned red() const { return red_; }
        unsigned green() const { return green_; }
        unsigned blue() const { return blue_; }
        bool is_on() const { return is_on_; }
    };
    
    /**
     * @brief 검색 결과 [2] "SetBrightness" Command
     */
    struct set_brightness_command {
        lightbulb* bulb;
        unsigned val = 0;
        unsigned previous_brightness = 0;
        
        set_brightness_command(lightbulb* b, unsigned brightness) 
            : bulb(b), val(brightness) {}
        
        void execute() {
            if (bulb) {
                previous_brightness = bulb->brightness();
                bulb->set_brightness(val);
            }
        }
        
        void undo() {
            if (bulb) {
                bulb->set_brightness(previous_brightness);
                std::cout << "Undo brightness change. Restored to " << previous_brightness << std::endl;
            }
        }
        
        bool can_undo() const { return true; }
        
        std::string description() const {
            return "Set Brightness to " + std::to_string(val);
        }
    };
    
    /**
     * @brief 검색 결과 [2] "SetColor" Command
     */
    struct set_color_command {
        lightbulb* bulb;
        unsigned r = 0, g = 0, b = 0;
        unsigned prev_r = 0, prev_g = 0, prev_b = 0;
        
        set_color_command(lightbulb* bulb, unsigned red, unsigned green, unsigned blue)
            : bulb(bulb), r(red), g(green), b(blue) {}
        
        void execute() {
            if (bulb) {
                prev_r = bulb->red();
                prev_g = bulb->green(); 
                prev_b = bulb->blue();
                bulb->set_color(r, g, b);
            }
        }
        
        void undo() {
            if (bulb) {
                bulb->set_color(prev_r, prev_g, prev_b);
                std::cout << "Undo color change. Restored to RGB(" << prev_r << ", " << prev_g << ", " << prev_b << ")" << std::endl;
            }
        }
        
        bool can_undo() const { return true; }
        
        std::string description() const {
            return "Set Color to RGB(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ")";
        }
    };
    
    // Toggle Command
    struct toggle_command {
        lightbulb* bulb;
        
        explicit toggle_command(lightbulb* b) : bulb(b) {}
        
        void execute() {
            if (bulb) {
                if (bulb->is_on()) {
                    bulb->turn_off();
                } else {
                    bulb->turn_on();
                }
            }
        }
        
        void undo() {
            // Toggle은 자기 자신이 undo
            execute();
        }
        
        bool can_undo() const { return true; }
        
        std::string description() const {
            return "Toggle Lightbulb";
        }
    };
    
    /**
     * @brief 검색 결과 [2] "CommandExecutor" 스타일
     */
    class lightbulb_controller {
    private:
        command_invoker<set_brightness_command, set_color_command, toggle_command> invoker_;
        
    public:
        template<typename CommandType>
        void execute(CommandType&& command) {
            invoker_.execute_command(std::forward<CommandType>(command));
        }
        
        template<typename CommandType>
        void queue(CommandType&& command) {
            invoker_.queue_command(std::forward<CommandType>(command));
        }
        
        void execute_all_queued() {
            invoker_.execute_queued_commands();
        }
        
        bool undo() {
            return invoker_.undo();
        }
        
        bool redo() {
            return invoker_.redo();
        }
        
        // 상태 조회
        size_t queued_count() const { return invoker_.queued_command_count(); }
        size_t undo_count() const { return invoker_.undo_stack_size(); }
        size_t redo_count() const { return invoker_.redo_stack_size(); }
    };
}
