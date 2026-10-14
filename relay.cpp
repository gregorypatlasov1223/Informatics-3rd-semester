#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define REG 1
#define FINISH 2
#define STICK_BASE 100

struct Message {
    long mtype;
    int runnerId;
};

/*
    начинаем регистрацию
    print: жду регистрацию i-ого участника
    print: закончилась регистрация
    передаю палочку 1-ому бегуну
    жду палочку от последнего бегуна

*/


void judge(int msgId, int n) {
    struct Message msg;

    // ждём регистрацию всех бегунов
    for (int i = 0; i < n; ++i) {
        msgrcv(msgId, &msg, sizeof(msg.runnerId), REG, 0);
    }

    // передаём палочку первому бегуну
    msg.mtype = STICK_BASE;
    msg.runnerId = 0;

    msgsnd(msgId, &msg, sizeof(msg.runnerId), 0);

    // ждём финиша последнего бегуна
    msgrcv(msgId, &msg, sizeof(msg.runnerId), FINISH, 0);
}

/*
    print: i-й участник зарегистрировался
    сообщаю об этом судье
    жду палочку от i-ого бегуна
    print: бегу этап
    если я не последний, то передаю палочку
    иначе сообщаю судье о финише

*/

void runner(int msgId, int i, int n)
{
    struct Message msg;

    // сообщаем судье, что бегун зарегистрирован
    msg.mtype = REG;
    msg.runnerId = i;

    msgsnd(msgId, &msg, sizeof(msg.runnerId), 0);

    // ждём палочку именно для этого бегуна
    msgrcv(msgId, &msg, sizeof(msg.runnerId), STICK_BASE + i, 0);

    // если это не последний бегун, то передаём палочку следующему
    if (i < n - 1) {
        msg.mtype = STICK_BASE + i + 1;
        msg.runnerId = i;

        msgsnd(msgId, &msg, sizeof(msg.runnerId), 0);
    }
    else {
        // последний бегун сообщает судье о финише
        msg.mtype = FINISH;
        msg.runnerId = i;

        msgsnd(msgId, &msg, sizeof(msg.runnerId), 0);
    }
}

int main() {
    int n = 0;

    printf("Enter number of runners: ");
    scanf("%d", &n);

    // создать очередь
    int msgId = msgget(IPC_PRIVATE, 0666);
    if (msgId == -1) {
        perror("msgget");
        return 1;
    }

    // создать судью
    pid_t pid = fork();
    if (pid == 0) {
        judge(msgId, n);
        return 0;
    }

    // создаем n бегунов
    for (int i = 0; i < n; ++i) {
        pid = fork();

        if (pid == 0) {
            runner(msgId, i, n);
            return 0;
        }
    }

    // родитель ждет судью и всех n бегунов
    for (int i = 0; i < n + 1; ++i) {
        wait(NULL);
    }

    // удалить очередь
    msgctl(msgId, IPC_RMID, NULL);;

    return 0;
}
