#pragma once

#include <time.h>

typedef struct {
	size_t left;
	size_t count;
	double min_time;
	double unit_time;
	struct timespec ts;
} cbench;

static inline
cbench* cbench_start(cbench *b) {
	b->count = 0;
	b->left = 1;
	return b;
}

static inline
int cbench_is_done(cbench *b) {
	if (b->left-- != 0) // hot-path
		return 0;
	extern int cbench__cold_path(cbench *b);
	return cbench__cold_path(b);
}

#define CBENCH_LOOP(b) \
	for (cbench* _cbench = cbench_start(b); !cbench_is_done(_cbench); )

#ifdef CBENCH_IMPLEMENTATION

int cbench__cold_path(cbench *b) {
	struct timespec toc;
	clock_gettime(CLOCK_MONOTONIC, &toc);
	if (b->count == 0) { // warm-run
		b->ts = toc;
		b->count = b->left = 1;
		return 0;
	}

	struct timespec tic = b->ts;
	double elapsed = (toc.tv_sec - tic.tv_sec) + 1e-9 * (toc.tv_nsec - tic.tv_nsec);
	if (elapsed < b->min_time) {
		b->count *= 10;
		b->left = b->count - 1;
		clock_gettime(CLOCK_MONOTONIC, &b->ts);
		return 0;
	}

	b->unit_time = elapsed / b->count;

	return 1;
}

#endif
