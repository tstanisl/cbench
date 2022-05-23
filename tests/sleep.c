#include <cbench.h>

#include <unistd.h>
#include <stdio.h>

int main(void) {
	cbench b = { .min_time = 1 };
	const int usec = 50 * 1000;

	CBENCH_LOOP(&b) {
		usleep(usec);
	}

	printf("%f: ", b.unit_time);
	if (0.95 * usec * 1e-6 < b.unit_time && b.unit_time < 1.05 * usec * 1e-6)
		puts("- passed");
	else
		puts("- failed");


	return 0;
}
