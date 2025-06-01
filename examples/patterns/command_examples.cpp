/**
 * @file examples/patterns/command_examples.cpp
 * @brief 검색 결과 [1][2][3][4][5] 종합 Command 예제
 */

#include <patterns/command.hpp>
#include <patterns/document_commands.hpp>
#include <patterns/lightbulb_commands.hpp>
#include <iostream>

using namespace patterns;
using namespace patterns::examples;
using namespace patterns::iot;

void document_editor_example() {
    std::cout << "\n=== Document Editor Command Example ===\n";
    
    // 검색 결과 [1] "receiver objects"
    document doc("Hello World");
    
    // 검색 결과 [1] "command objects"
    copy_command copy_cmd(&doc);
    paste_command paste_cmd(&doc);
    cut_command cut_cmd(&doc);
    append_text_command append_cmd(&doc, " - MetaLoki!");
    
    // 검색 결과 [1] "sender objects" - Command Invoker
    command_invoker<copy_command, paste_command, cut_command, append_text_command> editor;
    
    std::cout << "Initial content: '" << doc.content() << "'" << std::endl;
    
    // 명령 실행
    editor.execute_command(copy_cmd);
    editor.execute_command(append_cmd);
    editor.execute_command(paste_cmd);
    
    std::cout << "Final content: '" << doc.content() << "'" << std::endl;
    
    // 검색 결과 [3] "undo operations"
    std::cout << "\nUndo operations:" << std::endl;
    std::cout << "Can undo " << editor.undo_stack_size() << " operations" << std::endl;
    
    editor.undo(); // Undo paste
    editor.undo(); // Undo append
    
    std::cout << "After undo: '" << doc.content() << "'" << std::endl;
    
    // 검색 결과 [3] "redo operations"
    std::cout << "\nRedo operations:" << std::endl;
    editor.redo(); // Redo append
    std::cout << "After redo: '" << doc.content() << "'" << std::endl;
}

void lightbulb_iot_example() {
    std::cout << "\n=== IoT Lightbulb Command Example ===\n";
    
    // 검색 결과 [2] "Lightbulb" receiver
    lightbulb smart_bulb;
    lightbulb_controller controller;
    
    // 검색 결과 [2] "SetBrightness, SetColor" commands
    set_brightness_command brightness_cmd(&smart_bulb, 75);
    set_color_command color_cmd(&smart_bulb, 255, 100, 50);
    toggle_command toggle_cmd(&smart_bulb);
    
    // 명령 실행
    controller.execute(toggle_cmd);          // Turn on
    controller.execute(brightness_cmd);      // Set brightness
    controller.execute(color_cmd);          // Set color
    
    std::cout << "\nCurrent state:" << std::endl;
    std::cout << "On: " << smart_bulb.is_on() << std::endl;
    std::cout << "Brightness: " << smart_bulb.brightness() << std::endl;
    std::cout << "Color: RGB(" << smart_bulb.red() << ", " << smart_bulb.green() << ", " << smart_bulb.blue() << ")" << std::endl;
    
    // Undo 테스트
    std::cout << "\nUndo operations:" << std::endl;
    controller.undo(); // Undo color
    controller.undo(); // Undo brightness
    controller.undo(); // Undo toggle (turn off)
}

void queued_commands_example() {
    std::cout << "\n=== Queued Commands Example ===\n";
    
    lightbulb bulb;
    lightbulb_controller controller;
    
    // 검색 결과 [3] "queue operations"
    controller.queue(toggle_command(&bulb));
    controller.queue(set_brightness_command(&bulb, 50));
    controller.queue(set_color_command(&bulb, 0, 255, 0)); // Green
    controller.queue(set_brightness_command(&bulb, 100));
    
    std::cout << "Queued " << controller.queued_count() << " commands" << std::endl;
    
    // 모든 큐 명령 실행
    std::cout << "Executing all queued commands:" << std::endl;
    controller.execute_all_queued();
    
    std::cout << "Queue size after execution: " << controller.queued_count() << std::endl;
}

void functional_command_example() {
    std::cout << "\n=== Functional Command Example ===\n";
    
    // 검색 결과 [4] "decouple methods from object"
    int counter = 0;
    
    functional_command increment_cmd(
        [&counter]() { 
            ++counter; 
            std::cout << "Counter incremented to: " << counter << std::endl;
        },
        [&counter]() { 
            --counter; 
            std::cout << "Counter decremented to: " << counter << std::endl;
        },
        "Increment Counter"
    );
    
    functional_command decrement_cmd(
        [&counter]() { 
            --counter; 
            std::cout << "Counter decremented to: " << counter << std::endl;
        },
        [&counter]() { 
            ++counter; 
            std::cout << "Counter incremented to: " << counter << std::endl;
        },
        "Decrement Counter"
    );
    
    command_invoker<functional_command> func_invoker;
    
    func_invoker.execute_command(increment_cmd);
    func_invoker.execute_command(increment_cmd);
    func_invoker.execute_command(decrement_cmd);
    
    std::cout << "Final counter: " << counter << std::endl;
    
    // Undo 테스트
    std::cout << "\nUndo operations:" << std::endl;
    func_invoker.undo(); // Undo decrement (increment)
    func_invoker.undo(); // Undo increment (decrement)
    
    std::cout << "After undo, counter: " << counter << std::endl;
}

void button_example() {
    std::cout << "\n=== Button (Menu Item) Example ===\n";
    
    // 검색 결과 [5] "Menu_Item" 스타일
    document doc("Initial Content");
    
    // 버튼들 생성
    button copy_button("Copy", copy_command(&doc));
    button append_button("Append", append_text_command(&doc, " [ADDED]"));
    button paste_button("Paste", paste_command(&doc));
    
    // 버튼 클릭 시뮬레이션
    copy_button.click();
    append_button.click();
    paste_button.click();
    
    std::cout << "Final document: '" << doc.content() << "'" << std::endl;
}

int main() {
    try {
        std::cout << "===== Command Pattern Examples =====\n";
        
        document_editor_example();
        lightbulb_iot_example();
        queued_commands_example();
        functional_command_example();
        button_example();
        
        std::cout << "\n✅ All Command examples completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
