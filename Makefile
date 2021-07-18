SRC_DIR   := src
TEST_DIR  := test
BUILD_DIR := build

SRCS           := $(wildcard $(SRC_DIR)/*)
UNIT_TEST_SRCS := $(wildcard $(TEST_DIR)/unit/*.cc)
UNIT_TEST_EXE  := $(BUILD_DIR)/test
BENCH_SRCS     := $(wildcard $(TEST_DIR)/bench/*.cc)
BENCH_EXE      := $(BUILD_DIR)/bench

GOOGLE_BENCHMARK_DIR := $(BUILD_DIR)/benchmark
GOOGLE_BENCHMARK_LIB := $(GOOGLE_BENCHMARK_DIR)/build/src/libbenchmark.a

CXX            := g++
CXXFLAGS       := --std=c++17 -Wall -Wextra -pedantic -I$(SRC_DIR)
CXXFLAGS.test  := $(CXXFLAGS) -g -fsanitize=address -fsanitize=undefined
CXXFLAGS.bench := $(CXXFLAGS) -Ofast -I$(GOOGLE_BENCHMARK_DIR)/include
LDFLAGS        := -lboost_graph
LDFLAGS.test   := $(LDFLAGS) -lboost_unit_test_framework
LDFLAGS.bench  := $(LDFLAGS) -L$(dir $(GOOGLE_BENCHMARK_LIB)) -lbenchmark -lpthread

ifdef COVERAGE
	CXXFLAGS.test += --coverage
endif

.PHONY: default clean run_tests run_benchmarks

default: $(UNIT_TEST_EXE) $(BENCH_EXE)

run_tests: $(UNIT_TEST_EXE)
	./$< -l test_suite -r detailed
ifdef COVERAGE
	gcov *.gcda 
endif

run_benchmarks: $(BENCH_EXE)
	./$<

$(UNIT_TEST_EXE): $(UNIT_TEST_SRCS) $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS.test) $(filter %.cc,$^) $(LDFLAGS.test) -o $@

$(BENCH_EXE): $(GOOGLE_BENCHMARK_LIB) $(BENCH_SRCS) $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS.bench) $(filter %.cc,$^) $(LDFLAGS.bench) -o $@

$(GOOGLE_BENCHMARK_DIR):
	git clone https://github.com/google/benchmark.git $@ && \
	cd $@ && \
	git checkout v1.5.5

$(GOOGLE_BENCHMARK_LIB): $(GOOGLE_BENCHMARK_DIR)
	cd $(GOOGLE_BENCHMARK_DIR) && \
		cmake -E make_directory "build" && \
		cmake -E chdir "build" cmake \
			-DCMAKE_BUILD_TYPE=Release \
			-DBENCHMARK_ENABLE_GTEST_TESTS=OFF \
			-DBENCHMARK_DOWNLOAD_DEPENDENCIES=ON \
			-DBENCHMARK_ENABLE_LTO=true \
			../ && \
		cmake --build "build" --config Release

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -fr $(UNIT_TEST_EXE) $(BENCH_EXE) *.gcno *.gcda *.gcov

dist-clean:
	rm -fr $(BUILD_DIR) *.gcno *.gcda *.gcov

