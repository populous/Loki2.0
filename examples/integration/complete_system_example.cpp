/**
 * @file examples/integration/complete_system_example.cpp
 * @brief 검색 결과 [3] "disposable collection" + 8패턴 실용 예제
 */

#include <core/typelist.hpp>
#include <origami/composite.hpp>
#include <origami/iterator.hpp>
#include <origami/visitor.hpp>
#include <origami/builder.hpp>
#include <patterns/factory.hpp>
#include <patterns/strategy.hpp>
#include <patterns/observer.hpp>
#include <patterns/command.hpp>
#include <optimization/performance_patterns.hpp>
#include <iostream>
#include <memory>



/**
 * @brief 검색 결과 [3] "disposable collection" 패턴
 */
class resource_manager {
private:
    std::vector<std::function<void()>> cleanup_actions_;
    std::vector<std::exception_ptr> cleanup_exceptions_;
    
public:
    template<typename Resource>
    void register_resource(std::shared_ptr<Resource> resource) {
        cleanup_actions_.push_back([resource]() mutable {
            resource.reset(); // 자원 해제
        });
    }
    
    void cleanup_all() {
        cleanup_exceptions_.clear();
        
        for (auto& action : cleanup_actions_) {
            try {
                action();
            } catch (...) {
                cleanup_exceptions_.push_back(std::current_exception());
            }
        }
        
        cleanup_actions_.clear();
        
        // 검색 결과 [3] "save the exceptions for later"
        if (!cleanup_exceptions_.empty()) {
            std::string error_msg = "Cleanup failed with " + 
                std::to_string(cleanup_exceptions_.size()) + " errors";
            throw std::runtime_error(error_msg);
        }
    }
    
    ~resource_manager() {
        try {
            cleanup_all();
        } catch (...) {
            // 소멸자에서는 예외 억제
        }
    }
};

/**
 * @brief 실제 비즈니스 시나리오: 웹 컨텐츠 관리 시스템
 */
class web_content_management_system {
public:
    // 컨텐츠 타입들
    struct article {
        std::string title;
        std::string body;
        std::string author;
        
        article(std::string t = "", std::string b = "", std::string a = "")
            : title(std::move(t)), body(std::move(b)), author(std::move(a)) {}
    };
    
    struct media {
        std::string filename;
        std::string type;
        size_t size_bytes;
        
        media(std::string f = "", std::string t = "", size_t s = 0)
            : filename(std::move(f)), type(std::move(t)), size_bytes(s) {}
    };
    
    // ORIGAMI 구조
    using article_leaf = origami::leaf<article>;
    using media_leaf = origami::leaf<media>;
    using content_composite = origami::iterable_composite<article_leaf, media_leaf>;
    
private:
    // 8개 패턴 통합 시스템
    patterns::factory<article, media> content_factory_;
    std::unique_ptr<content_composite> website_structure_;
    patterns::subject<patterns::functional_observer<std::string>> event_system_;
    patterns::command_invoker<patterns::functional_command> action_system_;
    patterns::strategy_context<
        patterns::algorithms::addition_strategy,
        patterns::algorithms::multiplication_strategy
    > processing_strategy_;
    
    resource_manager resources_;
    
public:
    web_content_management_system() {
        initialize_system();
    }
    
    void initialize_system() {
        std::cout << "=== Initializing Web CMS with 8 Design Patterns ===" << std::endl;
        
        // 1. Factory 설정
        content_factory_.register_creator<article>("article", []() {
            return article("Default Article", "Content", "System");
        });
        
        content_factory_.register_creator<media>("media", []() {
            return media("default.jpg", "image/jpeg", 1024);
        });
        
        // 2. Builder로 웹사이트 구조 생성
        website_structure_ = origami::origami_builder<article_leaf, media_leaf>::create()
            .name("MetaLoki Website")
            .as_composite()
            .with_description("Demonstration website using 8 design patterns")
            .with_capacity(100)
            .build();
        
        // 3. Strategy 설정 (컨텐츠 점수 계산용)
        processing_strategy_.set_strategy(patterns::algorithms::addition_strategy{});
        
        // 4. Observer 설정 (이벤트 로깅)
        auto logger = std::make_shared<patterns::functional_observer<std::string>>(
            [](const std::string& event) {
                std::cout << "[CMS EVENT] " << event << std::endl;
            }
        );
        event_system_.add_observer(logger);
        resources_.register_resource(logger);
        
        event_system_.notify_all("Web CMS system initialized");
    }
    
    /**
     * @brief 8개 패턴 협력으로 컨텐츠 발행 워크플로우
     */
    void publish_content_workflow() {
        event_system_.notify_all("Starting content publication workflow");
        
        // 1. Factory로 컨텐츠 생성
        auto article_content = content_factory_.create_typed<article>("article");
        article_content->title = "MetaLoki 2.0 Release";
        article_content->body = "Comprehensive design pattern library with ORIGAMI and GOF patterns";
        article_content->author = "MetaLoki Team";
        
        auto media_content = content_factory_.create_typed<media>("media");
        media_content->filename = "metaloki_logo.png";
        media_content->type = "image/png";
        media_content->size_bytes = 2048;
        
        // 2. Composite로 구조에 추가
        website_structure_->add(article_leaf(*article_content));
        website_structure_->add(media_leaf(*media_content));
        
        event_system_.notify_all("Content added to website structure");
        
        // 3. Iterator + Visitor로 SEO 점수 계산
        auto iterator = website_structure_->dfs_iterator();
        int total_score = 0;
        
        iterator.for_each([&](const auto& element) {
            std::visit([&](const auto& item) {
                if constexpr (requires { item.value(); }) {
                    // Strategy로 점수 계산
                    int score = processing_strategy_.execute(1, 1);
                    total_score += score;
                    
                    event_system_.notify_all("Content scored: " + std::to_string(score));
                }
            }, element);
        });
        
        // 4. Command로 발행 작업 캡슐화
        auto publish_command = patterns::functional_command(
            [this, total_score]() {
                event_system_.notify_all("Content published with SEO score: " + std::to_string(total_score));
            },
            [this]() {
                event_system_.notify_all("Content publication reverted");
            },
            "Publish Content"
        );
        
        action_system_.execute_command(publish_command);
        
        // 5. 성능 최적화 적용
        auto& optimizer = optimization::performance_optimizer::get_instance();
        optimizer.optimize_all_patterns();
        
        event_system_.notify_all("Content publication workflow completed");
    }
    
    /**
     * @brief 시스템 상태 보고서
     */
    void generate_system_report() {
        std::cout << "\n=== Web CMS System Report ===" << std::endl;
        
        // Composite 구조 분석
        origami::node_counter_visitor counter;
        website_structure_->accept_visitor(counter);
        
        std::cout << "Website Elements: " << counter.get_total_count() << std::endl;
        std::cout << "Observer Count: " << event_system_.observer_count() << std::endl;
        std::cout << "Command History: " << action_system_.undo_stack_size() << std::endl;
        std::cout << "Factory Products: " << content_factory_.get_product_names().size() << std::endl;
        
        // Iterator로 컨텐츠 목록 표시
        std::cout << "\nContent Structure:" << std::endl;
        auto iter = website_structure_->dfs_iterator();
        iter.for_each([](const auto& element) {
            std::visit([](const auto& item) {
                if constexpr (requires { item.value(); }) {
                    std::cout << "  - Content item found" << std::endl;
                }
            }, element);
        });
    }
    
    void cleanup() {
        event_system_.notify_all("Cleaning up Web CMS system");
        resources_.cleanup_all();
    }
};

int main() {
    try {
        std::cout << "===== MetaLoki 2.0 Complete System Integration Example =====" << std::endl;
        
        web_content_management_system cms;
        
        // 실제 비즈니스 워크플로우 실행
        cms.publish_content_workflow();
        
        // 시스템 상태 보고
        cms.generate_system_report();
        
        // 정리 작업
        cms.cleanup();
        
        std::cout << "\n✅ Complete system integration example completed successfully!" << std::endl;
        std::cout << "\n🎉 MetaLoki 2.0 - 8 Design Patterns Working Together!" << std::endl;
        std::cout << "ORIGAMI: Composite ✓ Iterator ✓ Visitor ✓ Builder ✓" << std::endl;
        std::cout << "GOF Core: Factory ✓ Strategy ✓ Observer ✓ Command ✓" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ System Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
