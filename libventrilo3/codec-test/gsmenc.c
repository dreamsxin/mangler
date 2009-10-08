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
	}
	while (cc = fread(sample, sizeof sample, 1, stdin)) {
		if (cc != 1) {
                        fprintf(stderr, "fail read %d: %s\n", cc, strerror(errno));
			exit(0);
		}
		gsm_encode(handle, sample, buf);
		if (fwrite((char *)buf, sizeof buf, 1, stdout) != 1) {
			fprintf(stderr, "fail write\n");
			exit(0);
		}
	}
	gsm_destroy(handle);
}
