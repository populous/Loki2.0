/**
 * @file examples/utility/simple_state_example.cpp
 * @brief Simple State 사용 예제
 */

#include <utility/simple_state.hpp>
#include <iostream>
#include <thread>

using namespace utility;

// 게임 상태 예제
enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

void game_state_example() {
    std::cout << "=== Game State Example ===" << std::endl;
    
    simple_state<GameState> game_state;
    
    // 상태 등록
    game_state.register_state(GameState::MENU, "Main Menu");
    game_state.register_state(GameState::PLAYING, "Playing");
    game_state.register_state(GameState::PAUSED, "Paused");
    game_state.register_state(GameState::GAME_OVER, "Game Over");
    
    // 콜백 등록
    game_state.on_enter(GameState::PLAYING, [](GameState) {
        std::cout << "Game started!" << std::endl;
    });
    
    game_state.on_exit(GameState::PLAYING, [](GameState) {
        std::cout << "Game ended!" << std::endl;
    });
    
    game_state.on_transition([](GameState from, GameState to) {
        std::cout << "Transition: " << static_cast<int>(from) 
                  << " -> " << static_cast<int>(to) << std::endl;
    });
    
    // 상태 전환 시뮬레이션
    game_state.transition_to(GameState::MENU);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    game_state.transition_to(GameState::PLAYING);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    game_state.transition_to(GameState::PAUSED);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    game_state.transition_to(GameState::PLAYING);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    game_state.transition_to(GameState::GAME_OVER);
    
    // 디버그 정보 출력
    game_state.print_debug_info();
}

// 문자열 상태 예제
void string_state_example() {
    std::cout << "\n=== String State Example ===" << std::endl;
    
    string_simple_state document_state;
    
    // 문서 편집 상태들
    document_state.register_state("editing", "Editing Document");
    document_state.register_state("saving", "Saving Document");
    document_state.register_state("saved", "Document Saved");
    
    // 워크플로우 시뮬레이션
    document_state.transition_to("editing");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    document_state.transition_to("saving");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    document_state.transition_to("saved");
    
    std::cout << "Final state: " << document_state.get_current_state_name() << std::endl;
    std::cout << "Total editing time: " 
              << document_state.get_total_time_in_state("editing").count() << "ms" << std::endl;
}

int main() {
    try {
        std::cout << "===== MetaLoki Simple State Utility Examples =====" << std::endl;
        
        game_state_example();
        string_state_example();
        
        std::cout << "\n✅ All Simple State examples completed!" << std::endl;
        std::cout << "\n🎯 Simple State Features:" << std::endl;
        std::cout << "  • Template-based state identifier support ✓" << std::endl;
        std::cout << "  • Automatic state history tracking ✓" << std::endl;
        std::cout << "  • Callback system for state transitions ✓" << std::endl;
        std::cout << "  • Built-in timing and statistics ✓" << std::endl;
        std::cout << "  • Modern C++ Design philosophy ✓" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
