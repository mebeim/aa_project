SRC_DIR   := src
TEST_DIR  := test
BUILD_DIR := build

SRCS           := $(wildcard $(SRC_DIR)/*)
UNIT_TEST_SRCS := $(wildcard $(TEST_DIR)/unit/*.cc)
UNIT_TEST_EXE  := $(BUILD_DIR)/test
BENCH_SRCS     := $(wildcard $(TEST_DIR)/bench/*.cc)
BENCH_EXE      := $(BUILD_DIR)/bench

CXX            := g++
CXXFLAGS       := --std=c++17 -Wall -Wextra -pedantic -I$(SRC_DIR)
CXXFLAGS.test  := $(CXXFLAGS) -O2 -g -fsanitize=address -fsanitize=undefined
CXXFLAGS.bench := $(CXXFLAGS) -Ofast -Ibenchmark/include
LDFLAGS        := -lboost_graph
LDFLAGS.test   := $(LDFLAGS) -lboost_unit_test_framework
LDFLAGS.bench  := $(LDFLAGS) -Lbenchmark/build/src -lbenchmark -lpthread

.PHONY: default clean run_tests run_benchmark

default: $(UNIT_TEST_EXE) $(BENCH_EXE)

run_tests: $(UNIT_TEST_EXE)
	./$< -l test_suite -r detailed

run_benchmark: $(BENCH_EXE)
	./$<

$(UNIT_TEST_EXE): $(UNIT_TEST_SRCS) $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS.test) $(filter %.cc,$^) $(LDFLAGS.test) -o $@

$(BENCH_EXE): $(BENCH_SRCS) $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS.bench) $(filter %.cc,$^) $(LDFLAGS.bench) -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -fr $(BUILD_DIR)
