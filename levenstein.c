#include <stdint.h>
#include <immintrin.h>
static inline uint8_t min(const uint8_t x, const uint8_t y) {
	return x < y ? x : y;
}
int levenshtein_scalar(const int N,
                        const uint8_t* strings, const uint8_t* refstr,
                        uint8_t* distances) {
	uint8_t buf0[17], buf1[17];
	uint8_t *v0 = buf0, *v1 = buf1, *t;
	const uint8_t ZERO = 0;
	const uint8_t ONE = 1;
	uint8_t REF, QUERY, INS, DEL, SUB, eq, x;
	for (int querybase = 0; querybase < N; querybase += 1) {
		x = ZERO;
		for (int i=0; i < 17; i++) {
			buf0[i] = x;
			x += ONE;
		}
		for (int ri=0; ri < 16; ri++) {
			REF = refstr[ri];
			v1[0] = (ri + 1);
			for (int qi=0; qi < 16; qi++) {
				QUERY = *(strings + qi * N + querybase);
				DEL = (v0[qi + 1] + ONE);
				INS = (v1[qi] + ONE);
				eq = (REF == QUERY);
				SUB = (v0[qi] + (eq ? 0 : 1));
				v1[qi+1] = min(SUB, min(DEL, INS));
			}
			t = v0;
			v0 = v1;
			v1 = t;
		}
		distances[querybase] = v0[16];
	}
	return 0;
}
int levenshtein_avx2_16(const int N,
                        const uint8_t* strings, const uint8_t* refstr,
                        uint8_t* distances) {
	// Promote every uint8 to __m256i to work on many at a time
	__m256i buf0[17], buf1[17];
	__m256i *v0 = buf0, *v1 = buf1, *t;
	const __m256i ZERO = _mm256_setzero_si256();
	const __m256i ONE = _mm256_set1_epi8((char)1);
	__m256i REF, QUERY, INS, DEL, SUB, eq, x;
	// TODO: handle quantities that are not multiples of 32
	if (N % 32) return -1;
	for (int querybase = 0; querybase < N; querybase += 32) {
		x = ZERO;
		for (int i=0; i < 17; i++) {
			buf0[i] = x;
			x=_mm256_add_epi8(x, ONE);
		}
		for (int ri=0; ri < 16; ri++) {
			REF = _mm256_set1_epi8(refstr[ri]);
			v1[0] = _mm256_set1_epi8(ri + 1);
			for (int qi=0; qi < 16; qi++) {
				QUERY = _mm256_loadu_si256((__m256i*)(strings + qi * N + querybase));
				DEL = _mm256_add_epi8(v0[qi + 1], ONE);
				INS = _mm256_add_epi8(v1[qi], ONE);
				eq = _mm256_cmpeq_epi8(REF, QUERY);
				SUB = _mm256_add_epi8(v0[qi], _mm256_andnot_si256(eq, ONE));
				v1[qi+1] = _mm256_min_epu8(SUB, _mm256_min_epu8(DEL, INS));
			}
			t = v0;
			v0 = v1;
			v1 = t;
		}
		_mm256_storeu_si256((__m256i*)(distances + querybase), v0[16]);
	}
	return 0;
}
