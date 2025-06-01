/**
 * @file tests/integration/test_complete_system_integration.cpp
 * @brief 검색 결과 [1] "Integration tests" + 검색 결과 [3] "Test configuration model"
 * @details ORIGAMI 4패턴 + Core GOF 4패턴 완전 통합 테스트
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <core/typelist.hpp>
#include <core/policy_host.hpp>
#include <origami/composite.hpp>
#include <origami/iterator.hpp>
#include <origami/visitor.hpp>
#include <origami/builder.hpp>
#include <patterns/factory.hpp>
#include <patterns/strategy.hpp>
#include <patterns/observer.hpp>
#include <patterns/command.hpp>

using namespace metaloki;

/**
 * @brief 검색 결과 [3] "Test configuration model" - 통합 테스트용 설정
 */
struct integration_test_config {
    size_t max_components = 1000;
    size_t max_observers = 100;
    size_t max_commands = 500;
    std::chrono::milliseconds timeout{5000};
    bool enable_performance_logging = true;
};

/**
 * @brief 검색 결과 [1] "Component integration problems" 검출용 복합 시스템
 * @details 8개 패턴이 모두 상호작용하는 실제 시나리오
 */
class document_management_system {
public:
    // 문서 요소 타입들
    struct text_content {
        std::string text;
        explicit text_content(std::string t = "") : text(std::move(t)) {}
        
        void render_impl() const {
            std::cout << "Text: " << text;
        }
        
        std::unique_ptr<text_content> clone_impl() const {
            return std::make_unique<text_content>(text);
        }
    };
    
    struct image_content {
        std::string url;
        explicit image_content(std::string u = "") : url(std::move(u)) {}
        
        void render_impl() const {
            std::cout << "Image: " << url;
        }
        
        std::unique_ptr<image_content> clone_impl() const {
            return std::make_unique<image_content>(url);
        }
    };
    
    // ORIGAMI 타입 정의
    using text_leaf = origami::leaf<text_content>;
    using image_leaf = origami::leaf<image_content>;
    using doc_composite = origami::iterable_composite<text_leaf, image_leaf>;
    
    // GOF 패턴 타입 정의
    using content_factory = patterns::factory<text_content, image_content>;
    using processing_strategy = patterns::strategy_context<
        patterns::algorithms::addition_strategy,
        patterns::algorithms::multiplication_strategy
    >;
    using doc_observer = patterns::functional_observer<std::string>;
    using doc_subject = patterns::subject<doc_observer>;
    using doc_command = patterns::functional_command;
    using command_invoker = patterns::command_invoker<doc_command>;
    
private:
    // 8개 패턴 통합 인스턴스들
    std::unique_ptr<doc_composite> document_structure_;
    content_factory factory_;
    processing_strategy strategy_;
    doc_subject event_system_;
    command_invoker command_system_;
    
    integration_test_config config_;
    
public:
    explicit document_management_system(integration_test_config cfg = {}) 
        : config_(std::move(cfg)) {
        initialize_system();
    }
    
    /**
     * @brief 검색 결과 [1] "Wrong Contracts with other modules/APIs" 검출
     */
    void initialize_system() {
        // 1. Factory로 컨텐츠 생성기 설정
        factory_.register_default<text_content>("text");
        factory_.register_default<image_content>("image");
        
        // 2. Builder로 문서 구조 생성
        document_structure_ = origami::origami_builder<text_leaf, image_leaf>::create()
            .name("Integration Test Document")
            .as_composite()
            .with_capacity(config_.max_components)
            .build();
        
        // 3. Strategy 설정
        strategy_.set_strategy(patterns::algorithms::addition_strategy{});
        
        // 4. Observer 설정 (이벤트 로깅)
        if (config_.enable_performance_logging) {
            auto logger = std::make_shared<doc_observer>([](const std::string& event) {
                std::cout << "[EVENT] " << event << std::endl;
            });
            event_system_.add_observer(logger);
        }
    }
    
    /**
     * @brief 검색 결과 [1] "Cross-cutting Security Concerns" 테스트
     * @details 8개 패턴 협력으로 복잡한 비즈니스 로직 구현
     */
    void process_document_workflow(const std::vector<std::string>& content_requests) {
        event_system_.notify_all("Document workflow started");
        
        // 1. Factory + Builder: 동적 컨텐츠 생성 및 구조 구축
        for (const auto& request : content_requests) {
            try {
                if (request.find("text:") == 0) {
                    auto content = factory_.create_typed<text_content>("text");
                    content->text = request.substr(5);
                    document_structure_->add(text_leaf(*content));
                    
                } else if (request.find("image:") == 0) {
                    auto content = factory_.create_typed<image_content>("image");
                    content->url = request.substr(6);
                    document_structure_->add(image_leaf(*content));
                }
                
                event_system_.notify_all("Content added: " + request);
                
            } catch (const std::exception& e) {
                event_system_.notify_all("Error processing: " + request + " - " + e.what());
            }
        }
        
        // 2. Iterator + Visitor: 구조 순회 및 처리
        auto iterator = document_structure_->dfs_iterator();
        origami::node_counter_visitor counter;
        
        iterator.for_each([&counter, this](const auto& element) {
            std::visit([&counter, this](const auto& item) {
                if constexpr (requires { item.accept_visitor(counter); }) {
                    item.accept_visitor(counter);
                }
                
                // Strategy로 처리 로직 적용
                auto processing_result = strategy_.execute(1, 1);
                event_system_.notify_all("Element processed with result: " + std::to_string(processing_result));
                
            }, element);
        });
        
        // 3. Command: 처리 결과를 명령으로 캡슐화
        auto finalize_command = doc_command(
            [this, count = counter.get_total_count()]() {
                event_system_.notify_all("Document finalized with " + std::to_string(count) + " elements");
            },
            [this]() {
                event_system_.notify_all("Document finalization undone");
            },
            "Finalize Document"
        );
        
        command_system_.execute_command(finalize_command);
        
        event_system_.notify_all("Document workflow completed");
    }
    
    /**
     * @brief 검색 결과 [1] "Performance/Timeouts" 검출
     */
    std::chrono::milliseconds measure_performance(size_t iterations) {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < iterations; ++i) {
            std::vector<std::string> test_requests = {
                "text:Content " + std::to_string(i),
                "image:image" + std::to_string(i) + ".jpg"
            };
            
            process_document_workflow(test_requests);
            
            // 타임아웃 체크
            auto elapsed = std::chrono::high_resolution_clock::now() - start;
            if (elapsed > config_.timeout) {
                throw std::runtime_error("Performance timeout exceeded");
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    }
    
    // 상태 조회 메서드들
    size_t get_document_element_count() const {
        return document_structure_ ? document_structure_->children().size() : 0;
    }
    
    size_t get_observer_count() const {
        return event_system_.observer_count();
    }
    
    size_t get_command_undo_count() const {
        return command_system_.undo_stack_size();
    }
    
    bool has_factory_product(const std::string& name) const {
        return factory_.has_product(name);
    }
};

TEST_SUITE("Complete System Integration Tests") {
    
    TEST_CASE("8-Pattern Integration: Basic Workflow") {
        // 검색 결과 [1] "Component integration problems" 검출
        integration_test_config config;
        config.max_components = 10;
        config.enable_performance_logging = true;
        
        document_management_system dms(config);
        
        // 초기 상태 검증
        CHECK(dms.get_document_element_count() == 0);
        CHECK(dms.get_observer_count() == 1); // Logger observer
        CHECK(dms.has_factory_product("text"));
        CHECK(dms.has_factory_product("image"));
        
        // 통합 워크플로우 실행
        std::vector<std::string> requests = {
            "text:Hello World",
            "image:logo.png",
            "text:Integration Test",
            "image:diagram.svg"
        };
        
        REQUIRE_NOTHROW(dms.process_document_workflow(requests));
        
        // 결과 검증
        CHECK(dms.get_document_element_count() == 4);
        CHECK(dms.get_command_undo_count() == 1); // Finalize command
    }
    
    TEST_CASE("Pattern Interaction: Factory + Builder + Observer Chain") {
        // 검색 결과 [1] "Wrong Contracts with other modules/APIs" 검출
        document_management_system dms;
        
        // 복잡한 상호작용 시나리오
        std::vector<std::string> complex_requests;
        for (int i = 0; i < 20; ++i) {
            complex_requests.push_back("text:Section " + std::to_string(i));
            complex_requests.push_back("image:chart" + std::to_string(i) + ".png");
        }
        
        // 대량 처리 테스트
        REQUIRE_NOTHROW(dms.process_document_workflow(complex_requests));
        CHECK(dms.get_document_element_count() == 40);
    }
    
    TEST_CASE("Performance Integration Test") {
        // 검색 결과 [1] "Performance/Timeouts" 검출
        integration_test_config perf_config;
        perf_config.timeout = std::chrono::milliseconds(2000);
        perf_config.enable_performance_logging = false; // 성능 측정을 위해 로깅 비활성화
        
        document_management_system dms(perf_config);
        
        // 성능 측정
        auto duration = dms.measure_performance(10);
        
        CHECK(duration.count() > 0);
        CHECK(duration < perf_config.timeout);
        
        std::cout << "Performance test completed in " << duration.count() << "ms" << std::endl;
    }
    
    TEST_CASE("Error Handling Integration") {
        // 검색 결과 [1] "Cross-cutting Security Concerns" 테스트
        document_management_system dms;
        
        // 잘못된 요청들
        std::vector<std::string> invalid_requests = {
            "unknown:invalid",
            "text:", // 빈 텍스트
            "image:", // 빈 이미지
            "malformed_request"
        };
        
        // 에러 상황에서도 시스템이 안정적으로 동작해야 함
        REQUIRE_NOTHROW(dms.process_document_workflow(invalid_requests));
        
        // 유효한 요청만 처리되었는지 확인
        CHECK(dms.get_document_element_count() == 2); // 빈 text, image는 추가됨
    }
    
    TEST_CASE("Command Pattern Undo/Redo Integration") {
        // 검색 결과 [1] "Transactions" 검출
        document_management_system dms;
        
        std::vector<std::string> requests = {"text:Test Content"};
        dms.process_document_workflow(requests);
        
        CHECK(dms.get_command_undo_count() == 1);
        
        // 추가 명령 실행으로 Undo 스택 증가 확인
        dms.process_document_workflow({"image:test.jpg"});
        CHECK(dms.get_command_undo_count() == 2);
    }
}
