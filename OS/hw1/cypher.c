#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define BUFF_SIZE 1024*1024
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

int main(int argc, char** argv) {
	int fd_in, fd_key, fd_out, read_in, read_key, pos_in, pos_key, input_end, key_end, i, write_num;
	char buf_in[BUFF_SIZE], buf_key[BUFF_SIZE], buf_out[BUFF_SIZE];

	// Checking input
	if (argc != 4) {
		printf("Error: Wrong number of arguments.\n");
		return 0;
	}

	fd_in = open(argv[1], O_RDONLY);

	if (fd_in < 0) {
		printf("Error: Input file cannot be opened.\n");
		return 0;
	}

	fd_key = open(argv[2], O_RDONLY);

	if (fd_key < 0) {
		close(fd_in);
		printf("Error: Key file cannot be opened.\n");
		return 0;
	}

	fd_out = open(argv[3], O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);

	if (fd_out < 0) {
		close(fd_in);
		close(fd_key);
		printf("Error: Output file cannot be opened.\n");
		return 0;
	}
	
	input_end = lseek(fd_in, 0, SEEK_END);
	key_end = lseek(fd_key, 0, SEEK_END);

	// Return to file beginnig and check for errors
	if (lseek(fd_in, 0, SEEK_SET) < 0 || input_end < 0 || lseek(fd_key, 0, SEEK_SET) < 0 || key_end < 0) {
		printf("Error: canot find input file end\n");
		close(fd_in);
		close(fd_key);
		close(fd_out);
		return 0;
	}

	do {
		read_in = read(fd_in, &buf_in, BUFF_SIZE);
		read_key = read(fd_key, &buf_key, BUFF_SIZE);

		if (read_in < 0 || read_key < 0) {
			printf("Error reading one of the input files\n");
			break;
		} else if (read_in == 0) {
			break;
		}

		pos_in += read_in;

		write_num = 0;

		// Fill write buffer with all the read data from input file
		while(write_num < read_in) {
			for (i = 0; i < MIN(read_in, read_key); i++) {
				buf_out[i] = buf_in[i] ^ buf_key[i];
				write_num++;
			}

			if (read_key < read_in) {
				if (pos_key + read_key == key_end)
					// Reached EOF -> go to start
					lseek(fd_key, 0, SEEK_SET);
				else {
					pos_key += read_key;
					lseek(fd_key, pos_key, SEEK_SET);
				}
				
				read_key = read(fd_key, &buf_key, read_in - read_key);
				read_in -= read_key;
			}
		}
		
		pos_key += read_key;
		
		write(fd_out, buf_out, write_num);
		
		lseek(fd_in, pos_in, SEEK_SET);
		lseek(fd_key, pos_key, SEEK_SET);
	} while (pos_in < input_end);


	close(fd_in);
	close(fd_key);
	close(fd_out);
	return 0;
}
