/**
 * @file examples/patterns/observer_examples.cpp
 * @brief 검색 결과 [1][2][3][4] 종합 Observer 예제
 */

#include <patterns/observer.hpp>
#include <patterns/mvc_observer.hpp>
#include <patterns/signal_slot.hpp>
#include <iostream>
#include <string>

using namespace patterns;
using namespace patterns::mvc;
using namespace patterns::signals;

// 검색 결과 [4] "Person is observable" 스타일 예제
struct person_event {
    std::string name;
    int age;
    std::string action;
};

class person : public subject<functional_observer<person_event>> {
private:
    std::string name_;
    int age_;
    
public:
    person(std::string name, int age) : name_(std::move(name)), age_(age) {}
    
    void drive() {
        std::cout << name_ << " is driving." << std::endl;
        
        person_event event{name_, age_, "driving"};
        notify_all(event);
    }
    
    void set_age(int new_age) {
        age_ = new_age;
        person_event event{name_, age_, "age_changed"};
        notify_all(event);
    }
    
    const std::string& name() const { return name_; }
    int age() const { return age_; }
};

void basic_observer_example() {
    std::cout << "\n=== Basic Observer Example ===\n";
    
    person john("John", 25);
    
    // 검색 결과 [4] "lambda as a subscriber"
    auto traffic_admin = std::make_shared<functional_observer<person_event>>(
        [](const person_event& event) {
            if (event.action == "driving") {
                std::cout << "Traffic Admin: " << event.name << " (age " 
                          << event.age << ") is driving - checking license." << std::endl;
            }
        }
    );
    
    auto insurance_company = std::make_shared<functional_observer<person_event>>(
        [](const person_event& event) {
            if (event.action == "driving") {
                std::cout << "Insurance: Monitoring " << event.name 
                          << "'s driving behavior." << std::endl;
            }
        }
    );
    
    john.add_observer(traffic_admin);
    john.add_observer(insurance_company);
    
    std::cout << "Observer count: " << john.observer_count() << std::endl;
    
    john.drive();
    john.set_age(26);
    
    // Observer 제거
    john.remove_observer(traffic_admin);
    std::cout << "After removing traffic admin: " << john.observer_count() << std::endl;
    
    john.drive(); // insurance_company만 알림 받음
}

void mvc_example() {
    std::cout << "\n=== MVC Observer Example ===\n";
    
    // 검색 결과 [2] "Model-View-Controller"
    model data_model;
    view console_view("Console", data_model);
    view web_view("Web", data_model);
    controller app_controller(data_model);
    
    // 사용자 입력 시뮬레이션
    app_controller.handle_user_input("Hello World");
    app_controller.handle_user_input("Observer Pattern Demo");
    
    // 뷰 업데이트 요청
    app_controller.request_view_update(console_view);
    app_controller.request_view_update(web_view);
}

void signal_slot_example() {
    std::cout << "\n=== Signal-Slot Example ===\n";
    
    // 검색 결과 [4] "signal & slot"
    signal<std::string, int> button_clicked;
    signal<> application_quit;
    
    // Lambda 연결
    auto conn1 = button_clicked.connect([](const std::string& button, int clicks) {
        std::cout << "Button '" << button << "' clicked " << clicks << " times." << std::endl;
    });
    
    // 함수 객체 연결
    auto conn2 = application_quit.connect([]() {
        std::cout << "Application is quitting..." << std::endl;
    });
    
    // 신호 발생
    button_clicked.emit("OK", 1);
    button_clicked("Cancel", 2); // operator() 사용
    
    application_quit.emit();
    
    // 연결 해제
    button_clicked.disconnect(conn1);
    button_clicked.emit("OK", 3); // 아무도 받지 않음
    
    std::cout << "Remaining connections: " << button_clicked.connection_count() << std::endl;
}

void publisher_subscriber_example() {
    std::cout << "\n=== Publisher-Subscriber Example ===\n";
    
    // 검색 결과 [3] "subscription mechanism"
    publisher<std::string> news_publisher;
    
    auto subscriber1 = news_publisher.subscribe([](const std::string& news) {
        std::cout << "Subscriber 1 received: " << news << std::endl;
    });
    
    auto subscriber2 = news_publisher.subscribe([](const std::string& news) {
        std::cout << "Subscriber 2 received: " << news << std::endl;
    });
    
    news_publisher.publish("Breaking News: Observer Pattern Implemented!");
    news_publisher.publish("Weather Update: Sunny Day");
    
    // 구독 해제
    news_publisher.unsubscribe(subscriber1);
    news_publisher.publish("Only subscriber 2 will see this");
    
    std::cout << "Active subscribers: " << news_publisher.subscriber_count() << std::endl;
}

void raii_observer_example() {
    std::cout << "\n=== RAII Observer Example ===\n";
    
    person alice("Alice", 30);
    
    {
        // 검색 결과 [2] "RAII Idiom" - 자동 등록/해제
        class raii_traffic_observer : public auto_observer<raii_traffic_observer, person> {
        public:
            explicit raii_traffic_observer(person& p) : auto_observer(p) {}
            
            void notify(const person_event& event) {
                std::cout << "RAII Observer: " << event.name << " performed " << event.action << std::endl;
            }
        };
        
        std::cout << "Creating RAII observer..." << std::endl;
        raii_traffic_observer observer(alice);
        
        std::cout << "Observer count: " << alice.observer_count() << std::endl;
        alice.drive();
        
        std::cout << "Leaving scope..." << std::endl;
    } // RAII observer 자동 해제
    
    std::cout << "Observer count after scope: " << alice.observer_count() << std::endl;
    alice.drive(); // 아무도 받지 않음
}

int main() {
    try {
        std::cout << "===== Observer Pattern Examples =====\n";
        
        basic_observer_example();
        mvc_example();
        signal_slot_example();
        publisher_subscriber_example();
        raii_observer_example();
        
        std::cout << "\n✅ All Observer examples completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
