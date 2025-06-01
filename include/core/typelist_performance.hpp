/**
 * @file include/core/typelist_performance.hpp
 * @brief 검색 결과 [4] Type traits 기반 성능 최적화
 */

namespace metaloki::core::performance {
    
    /**
     * @brief 검색 결과 [4] "if constexpr" 활용 최적화
     */
    template<typename... Types>
    class optimized_typelist : public typelist<Types...> {
        using base = typelist<Types...>;
        
    public:
        /**
         * @brief 캐시 친화적 순회 (검색 결과 [1] 벤치마크 참조)
         */
        template<typename F>
        static constexpr void fast_for_each(F&& f) {
            if constexpr (sizeof...(Types) <= 4) {
                // 작은 리스트: 완전 인라이닝
                (f.template operator()<Types>(), ...);
            } else if constexpr (sizeof...(Types) <= 16) {
                // 중간 리스트: 분할 처리
                constexpr size_t mid = sizeof...(Types) / 2;
                using first_half = typename base::template take<mid>;
                using second_half = typename base::template drop<mid>;
                
                first_half::fast_for_each(f);
                second_half::fast_for_each(f);
            } else {
                // 큰 리스트: 일반 순회
                base::for_each_type(f);
            }
        }
        
        /**
         * @brief 검색 결과 [4] "static_assert" 활용 타입 검증
         */
        template<typename T>
        static constexpr bool safe_contains() {
            static_assert(!std::is_void_v<T>, "Cannot check for void type");
            static_assert(!std::is_reference_v<T>, "Use decay_t for reference types");
            
            return base::template contains<T>();
        }
        
        /**
         * @brief 컴파일 타임 해시 기반 빠른 검색
         */
        template<typename T>
        static constexpr size_t fast_index_of() {
            constexpr auto hash = typehash<T>();
            return hash_lookup<hash>();
        }
        
    private:
        // 타입 해시 (컴파일 타임)
        template<typename T>
        static constexpr size_t typehash() {
            // 간단한 해시 함수 (실제로는 더 정교한 구현 필요)
            return sizeof(T) * 31 + alignof(T);
        }
        
        // 해시 기반 룩업 테이블
        template<size_t Hash>
        static constexpr size_t hash_lookup() {
            // 컴파일 타임 해시 테이블 구현
            size_t index = 0;
            bool found = false;
            ((typehash<Types>() == Hash ? (found = true) : (found ? false : ++index)), ...);
            return found ? index : sizeof...(Types);
        }
    };
}
