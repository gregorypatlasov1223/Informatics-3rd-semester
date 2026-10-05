#include <stdio.h>
#include <unistd.h>
#include <string.h>

ssize_t write_all(int fd, const char *buf, size_t len)
{
	size_t total_written = 0;

	while (total_written < len)
	{
		ssize_t n = write(fd, buf + total_written, len - total_written);

		if (n < 0)
			return -1;

		total_written += (size_t)n;
	}

	return (ssize_t)total_written;
}

int main(int argc, char *argv[])
{
	int new_line_flag = 0;
	int start_index = 1;

	if (argc > 1 && strcmp(argv[1], "-n") == 0)
	{
		new_line_flag = 1;
		start_index = 2;
	}

	for (int i = start_index; i < argc; i++)
	{
		write_all(STDOUT_FILENO, argv[i], strlen(argv[i]));

		if (i < argc - 1)
			write_all(STDOUT_FILENO, " ", 1);
	}

	if (!new_line_flag)
		write_all(STDOUT_FILENO, "\n", 1);

	write_all(STDOUT_FILENO, "Hello, World\n\n", 14);

	return 0;
}
