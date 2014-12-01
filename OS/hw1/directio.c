#define _GNU_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

#define BUFF_SIZE 1024*1024

void write_func(int, char[], char[], int);

int size[] = { 4, 16, 64, 256, 1024 };
int w_s = BUFF_SIZE;

int main(int argc, char** argv) {
	int fd, n, i;
	struct stat sb;

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

	// Re-write with O_DIRECT and at Aligned spots.
	printf("-----Aligned-----\n\n");
	write_func(fd, buf, argv[1], 0);

	printf("\n-----Unligned-----\n\n");
	write_func(fd, buf, argv[1], 1);

	close(fd);
	fd = open(argv[1], O_RDWR);

	printf("\n-----Aligned no O_DIRECT-----\n\n");
	write_func(fd, buf, argv[1], 1);
	
	// Re-write with O_DIRECT and at aligned spots.

	close(fd);
	return 0;
}


void write_func(int fd, char buf[], char path[], int mod) {
	int i, j, blocks, seconds, useconds, seek_res, f_size;
	double time, res;

	struct timeval start, end;
	struct stat sb;

	stat(path, &sb);

	f_size = (int)sb.st_size;

	printf("w_s\ttime\tthroughput\n");

	for (j = 0; j < 5; j++) {
		w_s = 1024 * size[j];

		blocks = ((128 * BUFF_SIZE) / w_s);
		printf("%dkb\t", size[j]);

		gettimeofday(&start, NULL);

		for(i = 1; i < blocks; i++) {
			if (mod == 0)
				seek_res = lseek(fd, ((random() % blocks) * w_s), SEEK_SET);
			else
				seek_res = lseek(fd, (random() % f_size), SEEK_SET);
			if(seek_res < 0) {
				printf("\nseek error\n");
				close(fd);
				return;
			}
			if (write(fd, buf, w_s) < 0) {
				printf("\nwrite error\n");
				return;
			}
		}

		gettimeofday(&end, NULL);

		seconds = end.tv_sec - start.tv_sec;
		useconds = end.tv_usec - start.tv_usec;
		time = seconds + useconds/1000000.0;

		printf("%lf\t", time);

		// Throughput
		res = (((double)w_s * blocks)/(BUFF_SIZE * time));
		printf("%lf\tMB/s\n", res);
	}
}
