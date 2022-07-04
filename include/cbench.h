#pragma once

#include <time.h>

enum { CBENCH_SAMPLES = 5 };

typedef struct {
	// CONFIGURATION

	// minimal time required for reliable measurement
	// defaults to 0 sec
	double min_time;

	// maximal measurement time in seconds, defaults to INFINITY
	double max_time;

	// stop meansurement if estimate relative 3-sigma error of `unit_time` is
	// below `relerr_3sigma`. Defaults to 0.05 (5%).  Valid only if cumulative time
	// is above `min_time`
	double relerr_3sigma;

	// a number of warm runs, defaults to 0
	size_t warm_runs;

	// RESULTS

	// the estimate `unit_time`, this is the result
	double unit_time;
	// estimated squared relative error
	double unit_err_sq;
	// cumulative time of measurements
	double total_time;
	// cumulative number of samples
	size_t total_count;

	// PRIVATE data, don't use it

	size_t left_;
	struct timespec ts_;
	size_t count_;
	size_t n_samples_;
	int sample_pos_;
	double samples_[CBENCH_SAMPLES];
	size_t sizes_[CBENCH_SAMPLES];
} cbench;

cbench* cbench_start(cbench *b);

static inline
int cbench_is_done(cbench *b) {
	if (b->left_-- != 0) // hot-path
		return 0;
	extern int cbench__cold_path(cbench *b);
	return cbench__cold_path(b);
}

#define CBENCH_LOOP(b) \
	for (cbench* _cbench = cbench_start(b); !cbench_is_done(_cbench); )

#ifdef CBENCH_IMPLEMENTATION

#include <math.h>

typedef struct {
	double c;
	double m;
	double sigma2;
	double var_m;
} cbench__params;

/** Find the maximum likelihood estimator for parameters `c`, 'm' and `sigma`
 * from K samples  t[i] assuming distribution t[i] ~ Normal(c + n[i] * m, sigma^2)
 * if all `n[i]` are the same (or K == 1) then paramteres cannot be estimated
 * To compute a relevant estimator of error of `m`, the K must be more than 2
 */
static 
cbench__params cbench__estimate(size_t K, size_t n[static K], double t[static K]) {
	double sum_t = 0.0;
	double sum_n = 0.0;
	double sum_n2 = 0.0;
	double sum_nt = 0.0;

	for (size_t i = 0; i < K; ++i) {
		sum_t += t[i];
		sum_n += n[i];
		sum_n2 += (double)n[i] * n[i];
		sum_nt += n[i] * t[i];
	}

	double det = K * sum_n2 - sum_n * sum_n;
	double c = (sum_n2 * sum_t - sum_n * sum_nt) / det;
	double m = (-sum_n * sum_t + K * sum_nt) / det;

	double sigma2 = 0.0;
	for (size_t i = 0; i < K; ++i) {
		double err = t[i] - c - n[i] * m;
		sigma2 += err * err;
	}
	// compute unbiased estimator of variance
	sigma2 /= K - 2;

#if 0
	for (size_t i = 0; i < K; ++i)
		printf(" (%zu,%g)", n[i], t[i]);
	printf(" - c=%g m=%g +/-%g%%\n", c, m, sqrt(sigma2*K/det)/m*100);
#endif
	return (cbench__params) {
		.m = m,
		.c = c,
		.sigma2 = sigma2,
		.var_m = sigma2 * K / det,
	};
}

int cbench__cold_path(cbench *b) {
	struct timespec toc;
	clock_gettime(CLOCK_MONOTONIC, &toc);

	if (b->count_ == 0) { // warm-run
		b->ts_ = toc;
		b->count_ = b->left_ = 1;
		return 0;
	}

	struct timespec tic = b->ts_;
	double elapsed = (toc.tv_sec - tic.tv_sec) + 1e-9 * (toc.tv_nsec - tic.tv_nsec);

	b->samples_[b->sample_pos_] = elapsed;
	b->sizes_[b->sample_pos_] = b->count_;
	if (++b->sample_pos_ >= CBENCH_SAMPLES)
		b->sample_pos_ = 0;

	if (b->n_samples_ < CBENCH_SAMPLES)
		b->n_samples_++;

	b->total_time += elapsed;
	b->total_count += b->count_;

	cbench__params params = cbench__estimate(b->n_samples_, b->sizes_,  b->samples_);

	double exp_abs_err = b->relerr_3sigma * 0.33333 * params.m;

	if ((params.var_m < exp_abs_err * exp_abs_err && b->total_time > b->min_time) ||
	    b->total_time > b->max_time) {
		b->unit_time = params.m;
		b->unit_err_sq = params.var_m / (params.m * params.m);
		return 1;
	}

	b->count_ = 2 * b->count_ / 1 + 1;
	b->left_ = b->count_ - 1;
	clock_gettime(CLOCK_MONOTONIC, &b->ts_);
	return 0;
}

cbench* cbench_start(cbench *b) {
	if (b->max_time == 0.0)
		b->max_time = INFINITY;
	if (b->relerr_3sigma == 0.0)
		b->relerr_3sigma = 0.05;
	b->total_time = 0.0;
	b->total_count = 0;
	b->left_ = b->warm_runs;
	b->count_ = 0;
	b->n_samples_ = 0;
	b->sample_pos_ = 0;
	return b;
}

#endif
