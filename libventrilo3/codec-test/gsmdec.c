#include <gsm.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main() {
	gsm handle;
	gsm_frame buf;
	gsm_signal sample[160];
	int cc, soundfd;

	if (!(handle = gsm_create())) {
		fprintf(stderr, "fail handle\n");
		exit(0);
	}
	while (cc = fread((char *)buf, sizeof buf, 1, stdin)) {
		if (cc != 1) {
			fprintf(stderr, "fail read %d: %s\n", cc, strerror(errno));
			exit(0);
		}
		if (gsm_decode(handle, buf, sample) < 0) {
			fprintf(stderr, "fail decode\n");
			exit(0);
		}
		if (fwrite(sample, sizeof sample, 1, stdout) != 1) {
			fprintf(stderr, "fail write\n");
			exit(0);
		}
	}
	gsm_destroy(handle);
}
