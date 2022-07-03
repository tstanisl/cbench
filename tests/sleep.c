#include <cbench.h>

#include <unistd.h>
#include <stdio.h>
#include <math.h>

int main(void) {
	cbench b = { .min_time = 0, .relerr_3sigma = 0.05, .max_time = 10, .warm_runs = 1 };
	const int usec = 10 * 1000;

	CBENCH_LOOP(&b) {
		usleep(usec);
	}

	printf("%g +/- %g%% ", b.unit_time, sqrt(b.unit_err_sq) * 100);
	if (0.90 * usec * 1e-6 < b.unit_time && b.unit_time < 1.1 * usec * 1e-6)
		puts("- passed");
	else
		puts("- failed");


	return 0;
}
