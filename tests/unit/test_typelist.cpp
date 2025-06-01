/**
 * @file tests/unit/test_typelist.cpp
 * @brief Core TypeList 단위 테스트
 */

#include <core/typelist.hpp>
#include <string>
#include <vector>

using namespace metaloki::core;

// 기본 기능 테스트
void test_basic_functionality() {
    using TestTypes = typelist<int, double, std::string>;
    
    // 크기 확인
    static_assert(TestTypes::size == 3);
    static_assert(!TestTypes::empty);
    
    // 타입 접근
    static_assert(std::is_same_v<TestTypes::at<0>, int>);
    static_assert(std::is_same_v<TestTypes::at<1>, double>);
    static_assert(std::is_same_v<TestTypes::at<2>, std::string>);
    
    // 타입 검색
    static_assert(TestTypes::contains<int>());
    static_assert(TestTypes::contains<double>());
    static_assert(!TestTypes::contains<float>());
    
    // 인덱스 검색
    static_assert(TestTypes::index_of<int>() == 0);
    static_assert(TestTypes::index_of<double>() == 1);
    static_assert(TestTypes::index_of<float>() == TestTypes::size);
}

// 변환 테스트
void test_transformations() {
    using TestTypes = typelist<int, double>;
    
    // append
    using AppendedTypes = TestTypes::append<std::string>;
    static_assert(AppendedTypes::size == 3);
    static_assert(std::is_same_v<AppendedTypes::at<2>, std::string>);
    
    // prepend
    using PrependedTypes = TestTypes::prepend<char>;
    static_assert(PrependedTypes::size == 3);
    static_assert(std::is_same_v<PrependedTypes::at<0>, char>);
    
    // transform (포인터 변환 예시)
    template<typename T>
    using add_pointer = T*;
    
    using PointerTypes = TestTypes::transform<add_pointer>;
    static_assert(std::is_same_v<PointerTypes::at<0>, int*>);
    static_assert(std::is_same_v<PointerTypes::at<1>, double*>);
}

int main() {
    test_basic_functionality();
    test_transformations();
    
    // 런타임 테스트
    using TestTypes = typelist<int, double, std::string>;
    
    // 순회 테스트
    int type_count = 0;
    TestTypes::for_each_type([&type_count]<typename T>() {
        type_count++;
        // 각 타입에 대한 처리
    });
    
    assert(type_count == 3);
    
    std::cout << "✅ Core TypeList 모든 테스트 통과!" << std::endl;
    return 0;
}
