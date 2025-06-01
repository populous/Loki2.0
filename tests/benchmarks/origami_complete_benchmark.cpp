/**
 * @file tests/benchmarks/origami_complete_benchmark.cpp
 * @brief ORIGAMI 4패턴 통합 성능 벤치마크
 */

#include <benchmark/benchmark.h>  // where this file
#include <origami/composite.hpp>
#include <origami/iterator.hpp>
#include <origami/visitor.hpp>
#include <origami/builder.hpp>
#include <origami/advanced_builder.hpp>
#include <random>
#include <vector>

using namespace metaloki::origami;

// 벤치마크용 타입들
using int_leaf = leaf<int>;
using string_leaf = leaf<std::string>;
using doc_composite = iterable_composite<int_leaf, string_leaf>;

// 테스트 데이터 생성기
class TestDataGenerator {
private:
    std::mt19937 gen_{std::random_device{}()};
    std::uniform_int_distribution<int> int_dist_{1, 1000};
    std::uniform_int_distribution<size_t> string_len_dist_{5, 20};
    
public:
    int generate_int() { return int_dist_(gen_); }
    
    std::string generate_string() {
        size_t len = string_len_dist_(gen_);
        std::string result;
        result.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            result += static_cast<char>('a' + (gen_() % 26));
        }
        return result;
    }
};

static TestDataGenerator data_gen;

// Composite 생성 성능 벤치마크
static void BM_CompositeCreation(benchmark::State& state) {
    const size_t num_elements = state.range(0);
    
    for (auto _ : state) {
        doc_composite document("Benchmark Doc");
        
        for (size_t i = 0; i < num_elements; ++i) {
            if (i % 2 == 0) {
                document.add(int_leaf(data_gen.generate_int()));
            } else {
                document.add(string_leaf(data_gen.generate_string()));
            }
        }
        
        benchmark::DoNotOptimize(document);
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_CompositeCreation)->Range(8, 8<<10)->Complexity();

// Builder 성능 벤치마크
static void BM_BuilderConstruction(benchmark::State& state) {
    const size_t num_components = state.range(0);
    
    for (auto _ : state) {
        auto builder = create_complex_structure<int_leaf, string_leaf>("Benchmark Builder");
        
        for (size_t i = 0; i < num_components; ++i) {
            if (i % 2 == 0) {
                builder.contains("int_" + std::to_string(i), int_leaf(data_gen.generate_int()));
            } else {
                builder.contains("str_" + std::to_string(i), string_leaf(data_gen.generate_string()));
            }
        }
        
        auto result = builder.build();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_BuilderConstruction)->Range(8, 8<<8)->Complexity();

// Iterator 순회 성능 벤치마크
static void BM_IteratorTraversal(benchmark::State& state) {
    const size_t num_elements = state.range(0);
    
    // 사전에 테스트 데이터 생성
    doc_composite document("Iterator Benchmark");
    for (size_t i = 0; i < num_elements; ++i) {
        if (i % 2 == 0) {
            document.add(int_leaf(data_gen.generate_int()));
        } else {
            document.add(string_leaf(data_gen.generate_string()));
        }
    }
    
    for (auto _ : state) {
        auto iter = document.dfs_iterator();
        size_t count = 0;
        
        iter.for_each([&count](const auto& element) {
            ++count;
            benchmark::DoNotOptimize(element);
        });
        
        benchmark::DoNotOptimize(count);
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_IteratorTraversal)->Range(8, 8<<10)->Complexity();

// Visitor 패턴 성능 벤치마크
static void BM_VisitorPattern(benchmark::State& state) {
    const size_t num_elements = state.range(0);
    
    // 사전에 테스트 데이터 생성
    doc_composite document("Visitor Benchmark");
    for (size_t i = 0; i < num_elements; ++i) {
        document.add(int_leaf(data_gen.generate_int()));
    }
    
    for (auto _ : state) {
        accumulate_visitor<int> visitor;
        document.accept_visitor(visitor);
        
        auto result = visitor.get_result();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_VisitorPattern)->Range(8, 8<<10)->Complexity();

// 통합 패턴 성능 벤치마크 (Builder + Iterator + Visitor)
static void BM_IntegratedPatterns(benchmark::State& state) {
    const size_t num_elements = state.range(0);
    
    for (auto _ : state) {
        // 1. Builder로 구조 생성
        auto document = origami_builder<int_leaf, string_leaf>::create()
            .name("Integrated Benchmark")
            .as_composite()
            .with_capacity(num_elements)
            .build();
        
        // 2. 데이터 추가
        for (size_t i = 0; i < num_elements; ++i) {
            document->add(int_leaf(data_gen.generate_int()));
        }
        
        // 3. Iterator로 순회
        auto iter = document->dfs_iterator();
        size_t iteration_count = 0;
        iter.for_each([&iteration_count](const auto& element) {
            ++iteration_count;
        });
        
        // 4. Visitor로 처리
        accumulate_visitor<int> visitor;
        document->accept_visitor(visitor);
        auto sum = visitor.get_result();
        
        benchmark::DoNotOptimize(sum);
        benchmark::DoNotOptimize(iteration_count);
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_IntegratedPatterns)->Range(8, 8<<8)->Complexity();

// 불변 vs 가변 Builder 비교
static void BM_ImmutableBuilder(benchmark::State& state) {
    const size_t num_operations = state.range(0);
    
    for (auto _ : state) {
        auto builder = immutable_origami_builder<int_leaf, string_leaf>{};
        
        for (size_t i = 0; i < num_operations; ++i) {
            builder = builder.name("Step " + std::to_string(i))
                           .description("Description " + std::to_string(i))
                           .as_composite()
                           .with_capacity(i + 1);
        }
        
        auto result = builder.build();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_ImmutableBuilder)->Range(1, 64)->Complexity();

static void BM_MutableBuilder(benchmark::State& state) {
    const size_t num_operations = state.range(0);
    
    for (auto _ : state) {
        auto builder = create_complex_structure<int_leaf, string_leaf>("Mutable Test");
        
        for (size_t i = 0; i < num_operations; ++i) {
            builder.described_as("Description " + std::to_string(i));
            // Note: 체이닝이므로 각 호출이 누적됨
        }
        
        auto result = builder.build();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_MutableBuilder)->Range(1, 64)->Complexity();

BENCHMARK_MAIN();
