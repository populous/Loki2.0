/**
 * @file tests/benchmarks/complete_system_benchmark.cpp
 * @brief 검색 결과 [8] "system performance" 종합 벤치마크
 */

#include <benchmark/benchmark.h>
#include <optimization/performance_patterns.hpp>
#include "test_complete_system_integration.cpp" // 통합 테스트 시스템 재사용


using namespace optimization;

// 검색 결과 [9] "Strategies for Performance Optimization" 적용
static void BM_Complete_System_Integration(benchmark::State& state) {
    const size_t complexity = state.range(0);
    
    integration_test_config config;
    config.max_components = complexity;
    config.enable_performance_logging = false; // 벤치마크에서는 로깅 비활성화
    
    for (auto _ : state) {
        document_management_system dms(config);
        
        // 검색 결과 [1] "Component integration problems" 시뮬레이션
        std::vector<std::string> requests;
        for (size_t i = 0; i < complexity; ++i) {
            requests.push_back("text:Content " + std::to_string(i));
            if (i % 2 == 0) {
                requests.push_back("image:image" + std::to_string(i) + ".jpg");
            }
        }
        
        benchmark::DoNotOptimize(dms.process_document_workflow(requests));
    }
    
    state.SetComplexityN(state.range(0));
    state.SetItemsProcessed(state.iterations() * complexity);
}
BENCHMARK(BM_Complete_System_Integration)->Range(8, 1024)->Complexity();

// ORIGAMI vs GOF 패턴 성능 비교
static void BM_ORIGAMI_Patterns_Only(benchmark::State& state) {
    const size_t elements = state.range(0);
    
    for (auto _ : state) {
        // ORIGAMI 4패턴만 사용
        auto document = origami::origami_builder<origami::leaf<std::string>>::create()
            .name("ORIGAMI Only")
            .as_composite()
            .build();
        
        for (size_t i = 0; i < elements; ++i) {
            document->add(origami::leaf<std::string>("Element " + std::to_string(i)));
        }
        
        auto iter = document->dfs_iterator();
        origami::node_counter_visitor counter;
        
        iter.for_each([&counter](const auto& element) {
            std::visit([&counter](const auto& item) {
                if constexpr (requires { item.accept_visitor(counter); }) {
                    item.accept_visitor(counter);
                }
            }, element);
        });
        
        benchmark::DoNotOptimize(counter.get_total_count());
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_ORIGAMI_Patterns_Only)->Range(8, 1024)->Complexity();

static void BM_GOF_Patterns_Only(benchmark::State& state) {
    const size_t commands = state.range(0);
    
    for (auto _ : state) {
        // GOF 4패턴만 사용
        patterns::factory<std::string> factory;
        factory.register_default<std::string>("content");
        
        patterns::command_invoker<patterns::functional_command> invoker;
        patterns::subject<patterns::functional_observer<std::string>> subject;
        
        auto observer = std::make_shared<patterns::functional_observer<std::string>>(
            [](const std::string&) { /* no-op */ }
        );
        subject.add_observer(observer);
        
        for (size_t i = 0; i < commands; ++i) {
            auto content = factory.create("content");
            
            patterns::functional_command cmd(
                [&subject, i]() { 
                    subject.notify_all("Command " + std::to_string(i)); 
                }
            );
            
            invoker.execute_command(cmd);
        }
        
        benchmark::DoNotOptimize(invoker.undo_stack_size());
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_GOF_Patterns_Only)->Range(8, 1024)->Complexity();

// 성능 최적화 패턴 효과 측정
static void BM_Optimized_vs_Standard(benchmark::State& state) {
    const bool use_optimization = state.range(0);
    
    for (auto _ : state) {
        if (use_optimization) {
            // 검색 결과 [8] 최적화 패턴 사용
            auto& optimizer = performance_optimizer::get_instance();
            
            auto result = optimizer.measure_and_cache("test_operation", []() {
                return std::make_shared<origami::composite<origami::leaf<int>>>("Optimized");
            });
            
            benchmark::DoNotOptimize(result);
        } else {
            // 표준 방식
            auto result = std::make_shared<origami::composite<origami::leaf<int>>>("Standard");
            benchmark::DoNotOptimize(result);
        }
    }
}
BENCHMARK(BM_Optimized_vs_Standard)->Arg(0)->Arg(1);

// 메모리 사용량 최적화 테스트
static void BM_Memory_Optimization(benchmark::State& state) {
    const size_t objects = state.range(0);
    
    flyweight_factory<std::string, std::string> factory;
    
    for (auto _ : state) {
        std::vector<std::shared_ptr<std::string>> objects_vec;
        objects_vec.reserve(objects);
        
        // 동일한 키로 객체 생성 (Flyweight 효과 측정)
        for (size_t i = 0; i < objects; ++i) {
            std::string key = "key_" + std::to_string(i % 10); // 10개 키 재사용
            objects_vec.push_back(factory.get_flyweight(key));
        }
        
        benchmark::DoNotOptimize(objects_vec);
    }
    
    state.SetItemsProcessed(state.iterations() * objects);
}
BENCHMARK(BM_Memory_Optimization)->Range(64, 4096);

BENCHMARK_MAIN();
