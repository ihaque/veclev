#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

long time_ms(void) {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

extern int levenshtein_scalar(const int N,
		               const uint8_t* strings, const uint8_t* refstr,
				uint8_t* distances);

extern int levenshtein_avx2_16(const int N,
		               const uint8_t* strings, const uint8_t* refstr,
				uint8_t* distances);

void fill_string_array(unsigned char* strings, const int base, const int N, const int length) {
	/* Fill in row-wise */
	unsigned char alpha[4] = {'A', 'C', 'G', 'T'};
	for (int row = 0; row < length; row++) {
		// Every stride cells we advance character
		int stride = 1 << (2*row);
		// It's a bunch of wsated work to do this for everything up to base but
		// // it's too much work to think about the actual math
		for (int i=0, chidx=-1; i < base+N; i++) {
			if (i % stride == 0) chidx = (chidx + 1) % 4;
			// Reerse the write order
			if (i >= base) strings[(length - row - 1)*N+i-base] = alpha[chidx];
		}
	}
	return;
}
int main(int argc, char** argv) {
	const int length = 16;
	unsigned char refstr[] = "AAAATTTTAAAATTTT";
	const int N = 10485760;
	uint8_t *distances = (uint8_t*) malloc(N);
	unsigned char* strings = (unsigned char*) malloc(N * length);
	if (strings == NULL || distances == NULL) {printf("couldn't allocate!\n"); return -1;}

	fill_string_array(strings, 0, N, length);

	long start_scalar = time_ms();
	levenshtein_scalar(N, strings, refstr, distances);
	long end_scalar = time_ms();

	long start_avx2 = time_ms();
	levenshtein_avx2_16(N, strings, refstr,distances);
	long end_avx2 = time_ms();
	
	double scalar_per_s = N / ((end_scalar - start_scalar) / 1000.0);
	double avx2_per_s = N / ((end_avx2 - start_avx2) / 1000.0);
	printf("%ld ms scalar, %ld ms avx2\n", (end_scalar - start_scalar), (end_avx2 - start_avx2));
	printf("%f /s scalar, %f /s avx2\n", scalar_per_s, avx2_per_s);

	for (int i = 0; i < N; i++) {
		char buf[length+1];
		buf[length] = '\0';
		for (int j = 0; j < length; j++) {
			buf[j] = strings[j*N + i];
		}
		//printf("%s vs %s %d\n",buf, refstr, distances[i]);
	}

	free(strings);
	free(distances);
}
