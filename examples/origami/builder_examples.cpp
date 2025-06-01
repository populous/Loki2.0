/**
 * @file examples/origami/builder_examples.cpp
 * @brief 검색 결과 [6] "Desktop PC builder" ORIGAMI 버전
 */

#include <origami/builder.hpp>
#include <origami/advanced_builder.hpp>
#include <origami/complex_builder.hpp>
#include <iostream>
#include <string>

using namespace origami;

// 검색 결과 [7] "Order" 스타일 도메인 객체들
class document_section {
    std::string title_;
    std::string content_;
    
public:
    document_section(std::string title = "", std::string content = "")
        : title_(std::move(title)), content_(std::move(content)) {}
    
    const std::string& title() const { return title_; }
    const std::string& content() const { return content_; }
    
    void render_impl() const {
        std::cout << "Section: " << title_ << " - " << content_;
    }
    
    std::unique_ptr<document_section> clone_impl() const {
        return std::make_unique<document_section>(title_, content_);
    }
};

class document_image {
    std::string url_;
    std::string alt_text_;
    
public:
    document_image(std::string url = "", std::string alt = "")
        : url_(std::move(url)), alt_text_(std::move(alt)) {}
    
    const std::string& url() const { return url_; }
    const std::string& alt_text() const { return alt_text_; }
    
    void render_impl() const {
        std::cout << "Image: " << url_ << " (alt: " << alt_text_ << ")";
    }
    
    std::unique_ptr<document_image> clone_impl() const {
        return std::make_unique<document_image>(url_, alt_text_);
    }
};

void basic_fluent_builder_example() {
    std::cout << "\n=== Basic Fluent Builder Example ===\n";
    
    // 검색 결과 [2] "interface chain" - 강제된 순서대로 구성
    using doc_builder = origami_builder<document_section, document_image>;
    
    // 검색 결과 [1] "step by step" - 필수 속성부터 단계별 설정
    auto document = doc_builder::create()
        .name("User Manual")           // 필수: 이름 설정
        .as_composite()               // 필수: 구조 타입 선택
        .with_description("Software user manual")  // 선택: 설명
        .with_capacity(10)            // 선택: 초기 용량
        .build();                     // 최종: 객체 생성
    
    std::cout << "Created document structure successfully" << std::endl;
    
    // 검색 결과 [7] "more expressive approach" - 자연스러운 문법
    auto quick_document = doc_builder::create()
        .named("Quick Guide")
        .container()
        .described_as("Quick reference guide")
        .ready()
        .build();
    
    std::cout << "Created quick document successfully" << std::endl;
}

void immutable_fluent_builder_example() {
    std::cout << "\n=== Immutable Fluent Builder Example ===\n";
    
    // 검색 결과 [4] "return a new instance each time" - 불변 객체 패턴
    immutable_origami_builder<document_section, document_image> builder;
    
    // 모든 메서드가 새 인스턴스를 반환 - 안전한 병렬 사용 가능
    auto step1 = builder.name("Technical Spec");
    auto step2 = step1.description("Technical specifications document");
    auto step3 = step2.as_composite();
    auto step4 = step3.with_capacity(20);
    auto step5 = step4.with_validator([]() {
        std::cout << "Validating document structure..." << std::endl;
    });
    
    auto final_document = step5.build();
    
    std::cout << "Created immutable builder document successfully" << std::endl;
    
    // 검색 결과 [5] "plain English" - 체이닝으로도 사용 가능
    auto chained_document = immutable_origami_builder<document_section, document_image>{}
        .named("API Reference")
        .described_as("Complete API documentation")
        .container()
        .reserves(50)
        .with_metadata("version", "2.0")
        .with_metadata("author", "MetaLoki Team")
        .build();
    
    std::cout << "Created chained immutable document successfully" << std::endl;
}

void complex_builder_example() {
    std::cout << "\n=== Complex Builder Example ===\n";
    
    // 검색 결과 [5] "Personal & Professional" 스타일 - 복합 구조 구성
    auto documentation = create_complex_structure<document_section, document_image>("Software Documentation")
        .described_as("Complete software documentation package")
        .contains("introduction", document_section("Introduction", "Getting started guide"))
        .contains("features", document_section("Features", "Feature overview"))
        .requires_component("conclusion", document_section("Conclusion", "Final thoughts"))
        .contains("logo", document_image("logo.png", "Company logo"))
        .with_post_build_action([](auto& doc) {
            std::cout << "Post-build: Adding table of contents..." << std::endl;
        })
        .build();
    
    std::cout << "Complex documentation created with " 
              << documentation->children().size() << " components" << std::endl;
    
    // 검색 결과 [6] "Desktop PC builder" 스타일 - 팩토리 활용
    using structure_builder = structure_factory<document_section, document_image>;
    
    auto handbook = structure_builder::create_with(
        "Employee Handbook",
        "policies", 
        document_section("Policies", "Company policies and procedures")
    )
    .contains("benefits", document_section("Benefits", "Employee benefits"))
    .contains("contact", document_section("Contact", "Contact information"))
    .with_post_build_action([](auto& handbook) {
        std::cout << "Post-build: Finalizing handbook structure..." << std::endl;
    })
    .create();
    
    std::cout << "Employee handbook created successfully" << std::endl;
}

void fluent_naming_conventions_example() {
    std::cout << "\n=== Fluent Naming Conventions Example ===\n";
    
    // 검색 결과 [7] "instead of WithReleaseDate, simply use ReleaseDate"
    auto modern_document = immutable_origami_builder<document_section, document_image>{}
        .name("Modern Design Guide")           // 간결한 메서드명
        .description("UI/UX design guidelines") // With 접두사 제거
        .as_composite()
        .reserves(30)
        .with_validator([]() {
            std::cout << "Validating design guidelines..." << std::endl;
        })
        .build();
    
    // 검색 결과 [5] "more intuitive, natural & plain English"
    auto natural_document = create_complex_structure<document_section, document_image>("Release Notes")
        .described_as("Version 2.0 release notes")
        .contains("new_features", document_section("New Features", "What's new in 2.0"))
        .contains("improvements", document_section("Improvements", "Performance improvements"))
        .contains("screenshots", document_image("v2-screenshot.png", "Version 2.0 interface"))
        .create();
    
    std::cout << "Natural language style documents created successfully" << std::endl;
}

int main() {
    try {
        std::cout << "===== ORIGAMI Fluent Builder Pattern Examples =====\n";
        
        basic_fluent_builder_example();
        immutable_fluent_builder_example();
        complex_builder_example();
        fluent_naming_conventions_example();
        
        std::cout << "\n✅ All Fluent Builder examples completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
