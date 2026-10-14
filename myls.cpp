#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#define PATH_SIZE 4096


void getPermissions(mode_t mode, char permissions[])
{
    if (S_ISREG(mode))
        permissions[0] = '-';
    else if (S_ISDIR(mode))
        permissions[0] = 'd';
    else if (S_ISLNK(mode))
        permissions[0] = 'l';
    else
        permissions[0] = '?';

    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';

    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';

    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';

    permissions[10] = '\0';
}


void printLong(const char *name, const struct stat *st, int numericIds)
{
    char permissions[11];
    getPermissions(st->st_mode, permissions);

    printf("%s ", permissions);
    printf("%lu ", (unsigned long)st->st_nlink);

    if (numericIds) {
        printf("%u ", (unsigned int)st->st_uid);
        printf("%u ", (unsigned int)st->st_gid);
    }
    else {
        struct passwd *user = getpwuid(st->st_uid);
        struct group *group = getgrgid(st->st_gid);

        if (user != NULL)
            printf("%s ", user->pw_name);
        else
            printf("%u ", (unsigned int)st->st_uid);

        if (group != NULL)
            printf("%s ", group->gr_name);
        else
            printf("%u ", (unsigned int)st->st_gid);
    }

    printf("%lld ", (long long)st->st_size);

    char timeBuffer[64];
    struct tm *timeInfo = localtime(&st->st_mtime);

    if (timeInfo != NULL) {
        strftime(timeBuffer, sizeof(timeBuffer), "%b %d %H:%M", timeInfo);
        printf("%s ", timeBuffer);
    }

    printf("%s\n", name);
}


void printDirectory(const char *directoryName, int showAll,
                    int longFormat, int showInode, int numericIds)
{
    DIR *dir = opendir(directoryName);

    if (dir == NULL) {
        fprintf(stderr, "myls: %s: %s\n", directoryName, strerror(errno));
        return;
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (!showAll && entry->d_name[0] == '.')
            continue;

        char path[PATH_SIZE];
        struct stat st;

        snprintf(path, sizeof(path), "%s/%s", directoryName, entry->d_name);

        if (lstat(path, &st) < 0) {
            fprintf(stderr, "myls: %s: %s\n", path, strerror(errno));
            continue;
        }

        if (showInode)
            printf("%lu ", (unsigned long)st.st_ino);

        if (longFormat)
            printLong(entry->d_name, &st, numericIds);
        else
            printf("%s\n", entry->d_name);
    }

    closedir(dir);
}


int main(int argc, char *argv[])
{
    int showAll = 0;
    int longFormat = 0;
    int showInode = 0;
    int numericIds = 0;

    int option;

    while ((option = getopt(argc, argv, "ali1n")) != -1) {
        switch (option) {
            case 'a':
                showAll = 1;
                break;

            case 'l':
                longFormat = 1;
                break;

            case 'i':
                showInode = 1;
                break;

            case '1':
                break; // и так каждый объект выводится с новой строки

            case 'n':
                numericIds = 1;
                longFormat = 1;
                break;

            default:
                return 1;
        }
    }

    if (optind == argc) {
        printDirectory(".", showAll, longFormat, showInode, numericIds);
    }
    else {
        for (int i = optind; i < argc; ++i)
            printDirectory(argv[i], showAll, longFormat, showInode, numericIds);
    }

    return 0;
}
