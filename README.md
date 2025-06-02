# Loki2.0
single thread 
cpu only
C++2B
Orgami
Core
pattern

# Google BechMark 의존성 해결 
# 검색 결과 [5] 기본 빌드 단계
git clone https://github.com/google/benchmark.git
cd benchmark

# Google Test 의존성 해결 (검색 결과 [5] 방법 1)
git clone https://github.com/google/googletest.git googletest

# 또는 자동 다운로드 (검색 결과 [5] 방법 2)
mkdir build && cd build
cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=ON ../
make -j$(nproc)
sudo make install

Discord
https://discord.gg/yYWPhgu6
