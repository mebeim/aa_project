#ifndef BENCH_MEM_TRACE_H
#define BENCH_MEM_TRACE_H

#include <iostream>
#include <malloc.h>

static const char *cur_trace_name;
static size_t max_allocated_memory;
static size_t allocated_memory;

static void *malloc_hook(size_t size, const __attribute__((unused)) void *caller) {
	__typeof__(__malloc_hook) old_malloc_hook;
	void *res;

	old_malloc_hook = __malloc_hook;
	__malloc_hook = NULL;
	res = malloc(size);
	__malloc_hook = old_malloc_hook;

	if (res)
		allocated_memory += malloc_usable_size(res);

	if (allocated_memory > max_allocated_memory)
		max_allocated_memory = allocated_memory;

	return res;
}

static void *realloc_hook(void *ptr, size_t size, const __attribute__((unused)) void *caller) {
	__typeof__(__realloc_hook) old_realloc_hook;
	void *res;

	size_t old_size = malloc_usable_size(ptr);

	old_realloc_hook = __realloc_hook;
	__realloc_hook = NULL;
	res = realloc(ptr, size);
	__realloc_hook = old_realloc_hook;

	if (res) {
		allocated_memory -= old_size;
		allocated_memory += malloc_usable_size(res);

		if (allocated_memory > max_allocated_memory)
			max_allocated_memory = allocated_memory;
	}

	return res;
}

static void free_hook(void *ptr, const __attribute__((unused)) void *caller) {
	__typeof__(__free_hook) old_free_hook;

	size_t old_size = malloc_usable_size(ptr);

	old_free_hook = __free_hook;
	__free_hook = NULL;
	free(ptr);
	__free_hook = old_free_hook;

	allocated_memory -= old_size;
}

static void start_trace(const char *name) {
	__malloc_hook        = malloc_hook;
	__realloc_hook       = realloc_hook;
	__free_hook          = free_hook;
	cur_trace_name       = name;
	allocated_memory     = 0;
	max_allocated_memory = 0;
}

static void stop_trace(void) {
	__malloc_hook  = NULL;
	__realloc_hook = NULL;
	__free_hook    = NULL;

	std::cout << cur_trace_name << ": max " << max_allocated_memory << " bytes" << std::endl;
}

#endif // BENCH_MEM_TRACE_H
