SRC_DIR   := src
TEST_DIR  := test
BUILD_DIR := build

SRCS           := $(wildcard $(SRC_DIR)/*)
UNIT_TEST_SRCS := $(wildcard $(TEST_DIR)/unit/*.cc)
UNIT_TEST_EXE  := $(BUILD_DIR)/test
BENCH_TIME_SRC := $(TEST_DIR)/bench/bench_time.cc
BENCH_TIME_EXE := $(BUILD_DIR)/bench_time
BENCH_TIME_OUT := $(BUILD_DIR)/bench_time_out.json
BENCH_MEM_SRCS := $(wildcard $(TEST_DIR)/bench/bench_mem*)
BENCH_MEM_EXE  := $(BUILD_DIR)/bench_mem
BENCH_MEM_OUT  := $(BUILD_DIR)/bench_mem_out.txt
BENCH_PLOT_EXE := $(TEST_DIR)/bench/plot.py

GOOGLE_BENCHMARK_DIR := $(BUILD_DIR)/benchmark
GOOGLE_BENCHMARK_LIB := $(GOOGLE_BENCHMARK_DIR)/build/src/libbenchmark.a

CXX            := g++
CXXFLAGS       := --std=c++17 -Wall -Wextra -pedantic -I$(SRC_DIR)
CXXFLAGS.test  := $(CXXFLAGS) -g -fsanitize=address -fsanitize=undefined
CXXFLAGS.bench := $(CXXFLAGS) -Ofast -I$(GOOGLE_BENCHMARK_DIR)/include -Itest/bench
LDFLAGS        := -lboost_graph
LDFLAGS.test   := $(LDFLAGS) -lboost_unit_test_framework
LDFLAGS.bench  := $(LDFLAGS) -L$(dir $(GOOGLE_BENCHMARK_LIB)) -lbenchmark -lpthread

ifdef COVERAGE
	CXXFLAGS.test += --coverage
endif

.PHONY: default clean tests benchmarks run_tests run_benchmarks run_time_benchmarks run_mem_benchmarks

default: tests benchmarks

tests: $(UNIT_TEST_EXE)

benchmarks: $(BENCH_TIME_EXE) $(BENCH_MEM_EXE)

run_tests: $(UNIT_TEST_EXE)
	./$< -l test_suite -r detailed
ifdef COVERAGE
	gcov *.gcda
endif

run_benchmarks: $(BENCH_TIME_OUT) $(BENCH_MEM_OUT)

run_time_benchmarks: $(BENCH_TIME_OUT)

run_mem_benchmarks: $(BENCH_MEM_OUT)

plot_benchmarks: $(BENCH_TIME_OUT) $(BENCH_MEM_OUT) | $(BUILD_DIR)
	$(BENCH_PLOT_EXE) $^ $(BUILD_DIR)

$(UNIT_TEST_EXE): $(UNIT_TEST_SRCS) $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS.test) $(filter %.cc,$^) $(LDFLAGS.test) -o $@

$(BENCH_TIME_EXE): $(BENCH_TIME_SRC) $(GOOGLE_BENCHMARK_LIB) $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS.bench) $< $(LDFLAGS.bench) -o $@

$(BENCH_MEM_EXE): $(BENCH_MEM_SRCS) $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS.bench) -Wno-deprecated-declarations $(filter %.cc,$^) $(LDFLAGS) -o $@

$(BENCH_TIME_OUT): $(BENCH_TIME_EXE)
	./$< --benchmark_out=$@ --benchmark_out_format=json

$(BENCH_MEM_OUT): $(BENCH_MEM_EXE)
	./$< | tee $@

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
	rm -fr $(UNIT_TEST_EXE) $(BENCH_TIME_EXE) $(BENCH_TIME_OUT) $(BENCH_MEM_OUT) $(BUILD_DIR)/*.png *.gcno *.gcda *.gcov

dist-clean:
	rm -fr $(BUILD_DIR) *.gcno *.gcda *.gcov
