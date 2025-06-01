/**
 * @file include/patterns/mvc_observer.hpp
 * @brief 검색 결과 [2] "Model-View-Controller" 구현
 */

#pragma once

#include <patterns/observer.hpp>
#include <string>
#include <iostream>

namespace patterns::mvc {
    
    /**
     * @brief 검색 결과 [2] "model represents the data and its logic"
     */
    struct model_event {
        std::string property_name;
        std::string old_value;
        std::string new_value;
    };
    
    class model : public subject<functional_observer<model_event>> {
    private:
        std::string data_;
        
    public:
        void set_data(const std::string& new_data) {
            std::string old_data = data_;
            data_ = new_data;
            
            // 검색 결과 [2] "model notifies its dependent component"
            model_event event{
                .property_name = "data",
                .old_value = old_data,
                .new_value = new_data
            };
            
            notify_all(event);
        }
        
        const std::string& get_data() const { return data_; }
    };
    
    /**
     * @brief 검색 결과 [2] "views are responsible for representing the data"
     */
    class view : public auto_observer<view, model> {
    private:
        std::string view_name_;
        
    public:
        view(const std::string& name, model& model) 
            : auto_observer<view, model>(model), view_name_(name) {}
        
        void notify(const model_event& event) {
            std::cout << "[" << view_name_ << "] Model changed: " 
                      << event.property_name << " = " << event.new_value << std::endl;
        }
        
        void render(const std::string& data) {
            std::cout << "[" << view_name_ << "] Rendering: " << data << std::endl;
        }
    };
    
    /**
     * @brief 검색 결과 [2] "controller is for the user input"
     */
    class controller {
    private:
        model& model_;
        
    public:
        explicit controller(model& model) : model_(model) {}
        
        void handle_user_input(const std::string& input) {
            // 사용자 입력을 모델에 반영
            model_.set_data(input);
        }
        
        void request_view_update(view& view) {
            view.render(model_.get_data());
        }
    };
}
