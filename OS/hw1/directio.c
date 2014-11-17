#define _GNU_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

#define BUFF_SIZE 1024*1024


int main(int argc, char** argv) {
	int fd, n, i, j, w_s = BUFF_SIZE, blocks, seconds, useconds;
	double time;
	struct stat sb;
	struct timeval start, end;

	static char buf[1024*1024] __attribute__ ((__aligned__ (4096)));

	if (argc < 2) {
		printf("Error: No file path specified\n");
		return 0;
	}

	// Fill the buffer with random data
	for (i = 0; i < BUFF_SIZE; i++) buf[i] = random() % 255;

	fd = stat(argv[1], &sb);

	if (fd == -1) {
		// File doesn't exit - Create it
		fd = open(argv[1], O_CREAT, S_IRUSR|S_IWUSR);
		stat(argv[1], &sb);
	}

	if((int)sb.st_size < 128 * BUFF_SIZE) {
		// Make 128MB
		close(fd);
		fd = open(argv[1], O_RDWR|O_TRUNC);

		do {
			if (sb.st_size - w_s < 0) // Making sure we don't go over 128MB
				w_s = BUFF_SIZE - sb.st_size;

			n = write(fd, buf, w_s);
			if (n < 0) {
				printf("Write Error\n");
				break;
			}
			stat(argv[1], &sb);
		} while((int)sb.st_size < 128 * BUFF_SIZE);
	}

	close(fd);

	fd = open(argv[1], O_RDWR|O_DIRECT);

	if (fd < 0) {
		printf("Error: Cannot re-open file");
		return 0;
	}

	w_s = 2 * 1024;

	for (j = 0; j < 5; j++) {
		w_s *= 2;

		blocks = ((128 * BUFF_SIZE) / w_s);

		gettimeofday(&start, NULL);

		printf("# of blocks: %d\n", blocks);

		for(i = 1; i < blocks; i++) {
			if(lseek(fd, ((random() % blocks) * w_s), SEEK_SET) < 0) {
				printf("seek error\n");
				close(fd);
				return 0;
			}
			if (write(fd, buf, w_s) < 0) {
				printf("write error\n");
				return 0;
			}
		}

		gettimeofday(&end, NULL);

		seconds = end.tv_sec - start.tv_sec;
		useconds = end.tv_usec - start.tv_usec;
		time = seconds + useconds/1000000.0;

		printf("%lf\n", time);

	}

	close(fd);
	return 0;
}