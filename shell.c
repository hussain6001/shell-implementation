#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define YES 1
#define NO 0

void get_dir(char *home, char *current, int size)
{
    char *ret;
    int i = 0, count = 0;
    ret = getcwd(current, size);
    if (ret == NULL)
	error(1, errno, "cannot get current working directory");
    while (1) {
	home[i] = current[i];
	if (current[i] == '/')
	    count++;
	i++;
	if (count == 3)
	    break;
    }
    home[i] = '\0';
}


int parsing_commands(char *buf, char *argv_temp[], char redirection)
{
    int i = 0, j = 0, argc = 0;
    char temp[50];


    while (buf[i] != '\n') {
	if (redirection == YES) {
	    if (buf[i] == '>' || buf[i] == '<')
		break;
	}
	while (buf[i] != ' ' && buf[i] != '\n') {
	    temp[j] = buf[i];
	    i++;
	    j++;
	}
	temp[j] = '\0';

	argv_temp[argc] = (char *) malloc(j);
	strcpy(argv_temp[argc], temp);

	argc++;
	j = 0;
	if (buf[i] == '\n')
	    break;
	i++;
    }
    argv_temp[argc] = (char *) malloc(5);
    argv_temp[argc] = NULL;
    return argc;
}

void redirection(int argc, char *argv_temp[])
{
    int i;
    int fd;
    for (i = 0; i < argc; i++) {
	if (!strcmp(argv_temp[i], ">")) {
	    fd = open(argv_temp[i + 1],
		      O_WRONLY | O_TRUNC | O_CREAT, 0644);
	    if (fd == -1)
		error(1, errno, "error opening file");
	    dup2(fd, 1);
	    break;
	} else if (!strcmp(argv_temp[i], ">>")) {
	    fd = open(argv_temp[i + 1],
		      O_WRONLY | O_APPEND | O_CREAT, 0644);
	    if (fd == -1)
		error(1, errno, "error opening file");
	    dup2(fd, 1);
	    break;
	} else if (!strcmp(argv_temp[i], "<")) {
	    fd = open(argv_temp[i + 1], O_RDONLY);
	    if (fd == -1)
		error(1, errno, "error opening file");
	    dup2(fd, 0);
	    break;
	}
    }
}

int main()
{
    char buffer[256];
    char *argv[10];
    char *argv2[10];
    pid_t mypid;
    char home_dir[50];
    char current_dir[50];
    int status;
    char *ret;
    int sys_ret;
    int argc = 0;

    get_dir(home_dir, current_dir, sizeof(current_dir));

    while (1) {
	printf("MyShell>");
	ret = fgets(buffer, sizeof(buffer), stdin);
	if (buffer[0] == '\n') {
	    continue;
	}

	parsing_commands(buffer, argv, 1);

	if (!strcmp(argv[0], "cd")) {
	    if (argv[1] == NULL) {
		sys_ret = chdir(home_dir);
		if (sys_ret == -1)
		    error(1, errno, "cannot change directory");
	    } else {
		sys_ret = chdir(argv[1]);
		if (sys_ret == -1)
		    error(1, errno, "cannot change directory");

	    }
	}

	if (strcmp(argv[0], "cd")) {
	    mypid = fork();
	    if (mypid == -1)
		error(1, errno, "cannot create process");
	    if (mypid == 0) {
		argc = parsing_commands(buffer, argv2, 0);
		redirection(argc, argv2);
		sys_ret = execvp(argv[0], argv);
		if (sys_ret == -1)
		    error(1, errno, "");
	    }
	    wait(&status);
	}
    }
}
