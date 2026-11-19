#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

int fd;
pid_t writer_pid;

unsigned char input_byte;
int bit_pos = -1;

unsigned char output_byte = 0;
int bit_count = 0;


void send_next_bit(void) {
    if (bit_pos < 0) {
        ssize_t n = read(fd, &input_byte, 1);

        if (n < 0) {
            kill(writer_pid, SIGTERM);
            wait(NULL);
            close(fd);
            _exit(1);
        }

        if (n == 0) {
            kill(writer_pid, SIGTERM);
            wait(NULL);
            close(fd);
            _exit(0);
        }

        bit_pos = 7;
    }

    int bit = (input_byte >> bit_pos) & 1;
    bit_pos--;

    if (bit == 0) {
        kill(writer_pid, SIGUSR1);
    } else {
        kill(writer_pid, SIGUSR2);
    }
}


void ready_for_next_bit(int signo) {
    send_next_bit();
}


void receive_bit_handler(int signo) {
    int bit;

    if (signo == SIGUSR1) {
        bit = 0;
    } else {
        bit = 1;
    }

    output_byte = (output_byte << 1) | bit;
    bit_count++;

    if (bit_count == 8) {
        write(STDOUT_FILENO, &output_byte, 1);

        output_byte = 0;
        bit_count = 0;
    }

    kill(getppid(), SIGUSR1);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        return 1;
    }

    fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        return 1;
    }

    signal(SIGUSR1, receive_bit_handler);
    signal(SIGUSR2, receive_bit_handler);

    pid_t pid = fork();

    if (pid == -1) {
        close(fd);
        return 1;
    }

    if (pid == 0) {
        close(fd);

        while (1) {
            pause();
        }
    }

    writer_pid = pid;

    signal(SIGUSR1, ready_for_next_bit);

    send_next_bit();  // первый бит отправляем вручную

    while (1) {
        pause();
    }
}
