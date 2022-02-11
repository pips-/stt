#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <time.h>
#include <string.h>

#include "arg.h"

#ifndef VERSION
#define VERSION "dev"
#endif

#define free_and_null(x) do { \
    if (x) { free (x); x = NULL; } \
} while (0);

char           *argv0;

static char    *timesfile = ".ttimes";

struct timesnode {
	char           *task;
	time_t 		starttime;
	time_t 		endtime;
	struct timesnode *left;
	struct timesnode *right;
	signed int 	height;
};

/* functions declarations */
static void 	usage(void);

FILE           *opentimesfile();

struct timesnode *loadtimes(FILE *, struct timesnode *);
void 		writetimes(FILE *, struct timesnode *);
struct timesnode *timesnode_add(struct timesnode *, const char *, time_t, time_t);
struct timesnode *timesnode_stop(struct timesnode *, time_t);
int 		timesnode_max(int, int);
int 		timesnode_heigh(struct timesnode *);
int 		timesnode_getbalance(struct timesnode *);
struct timesnode *timesnode_rightrotate(struct timesnode *);
struct timesnode *timesnode_leftrotate(struct timesnode *);
void 		timesnode_print(struct timesnode *, time_t);
void 		timesnode_free(struct timesnode *);

void
usage()
{
	fprintf(stderr, "usage: %s [-v] [-h] [-a task | -s | -l]\n", argv0);
	exit(1);
}

FILE           *
opentimesfile()
{
	FILE           *fp;

	fp = fopen(timesfile, "r+");

	if (fp == NULL) {
		//int 		oldcwd;
		struct passwd  *passwd;

		//oldcwd = dirfd(opendir("."));

		passwd = getpwuid(getuid());
		chdir(passwd->pw_dir);

		fp = fopen(timesfile, "a+");

		if (fp == NULL) {
			perror("fopen failed");
		}
		//fchdir(oldcwd);
	}
	if (fp == NULL) {
		fprintf(stderr, "%s: Error opening times file\n", argv0);
		exit(1);
	}
	return fp;
}

struct timesnode *
loadtimes(FILE * fp, struct timesnode * p)
{
	char           *readline, *line, *tmp, *tmpline;
	time_t 		starttime, endtime;
	char           *task;
	size_t 		linesize;
	ssize_t 	linelen;

	rewind(fp);

	line = readline = NULL;
	linesize = 0;
	while ((linelen = getline(&readline, &linesize, fp)) != -1) {
		line = strdup(readline);
		tmpline = line;	/* Stores the pointer to allow buffer free */
		tmp = strsep(&line, ";");
		starttime = atoi(tmp);

		if (starttime == 0) {
			free_and_null(tmpline);
			continue;
		}
		tmp = strsep(&line, ";");
		endtime = atoi(tmp);

		task = strdup(line);
		task[strlen(task) - 1] = '\0';

		p = timesnode_add(p, task, starttime, endtime);

		free_and_null(task);
		free_and_null(tmpline);
	}

	free_and_null(readline);

	return p;
}

void
writetimes(FILE * fp, struct timesnode * p)
{
	if (p == NULL) {
		return;
	}
	if (p->left != NULL) {
		writetimes(fp, p->left);
	}
	fprintf(fp, "%lld;%lld;%s\n", (long long) p->starttime, (long long) p->endtime, p->task);

	if (p->right != NULL) {
		writetimes(fp, p->right);
	}
	return;
}

int
timesnode_max(int a, int b)
{
	return (a > b) ? a : b;
}

int
timesnode_heigh(struct timesnode * p)
{
	if (p == NULL) {
		return 0;
	}
	return p->height;
}

int
timesnode_getbalance(struct timesnode * p)
{
	if (p == NULL) {
		return 0;
	}
	return (timesnode_heigh(p->left) + (p->left != NULL)) - (timesnode_heigh(p->right) + (p->right != NULL));
}

struct timesnode *
timesnode_rightrotate(struct timesnode * p)
{
	struct timesnode *leftchild;
	struct timesnode *subrightchild;

	leftchild = p->left;
	subrightchild = leftchild->right;

	leftchild->right = p;
	p->left = subrightchild;

	p->height = (p->left != NULL || p->right != NULL) ? 1 + timesnode_max(timesnode_heigh(p->left), timesnode_heigh(p->right)) : 0;
	leftchild->height = 1 + timesnode_max(timesnode_heigh(leftchild->left), timesnode_heigh(leftchild->right));

	return leftchild;
}

struct timesnode *
timesnode_leftrotate(struct timesnode * p)
{
	struct timesnode *rightchild;
	struct timesnode *subleftchild;

	rightchild = p->right;
	subleftchild = rightchild->left;

	rightchild->left = p;
	p->right = subleftchild;

	p->height = (p->left != NULL || p->right != NULL) ? 1 + timesnode_max(timesnode_heigh(p->left), timesnode_heigh(p->right)) : 0;
	rightchild->height = 1 + timesnode_max(timesnode_heigh(rightchild->left), timesnode_heigh(rightchild->right));

	return rightchild;
}

struct timesnode *
timesnode_add(struct timesnode * p, const char *task, time_t starttime, time_t endtime)
{
	int 		balance;

	if (p == NULL) {
		p = (struct timesnode *) malloc(sizeof(struct timesnode));

		p->task = strdup(task);
		p->starttime = starttime;
		p->endtime = endtime;
		p->left = p->right = NULL;
		p->height = 0;

		return p;
	} else if (p->starttime > starttime) {
		p->left = timesnode_add(p->left, task, starttime, endtime);
	} else {
		p->right = timesnode_add(p->right, task, starttime, endtime);
	}

	p->height = 1 + timesnode_max(timesnode_heigh(p->left), timesnode_heigh(p->right));

	balance = timesnode_getbalance(p);

	if (balance > 1 && starttime < p->left->starttime) {
		return timesnode_rightrotate(p);
	}
	if (balance < -1 && starttime > p->right->starttime) {
		return timesnode_leftrotate(p);
	}
	if (balance > 1 && starttime > p->left->starttime) {
		p->left = timesnode_leftrotate(p->left);
		return timesnode_rightrotate(p);
	}
	if (balance < -1 && starttime < p->right->starttime) {
		p->right = timesnode_rightrotate(p->right);
		return timesnode_leftrotate(p);
	}
	return p;
}

struct timesnode *
timesnode_stop(struct timesnode * p, time_t endtime)
{
	if (p == NULL) {
		return NULL;
	}
	if (p->endtime == 0) {
		p->endtime = endtime;
	}
	if (p->left != NULL) {
		timesnode_stop(p->left, endtime);
	}
	if (p->right != NULL) {
		timesnode_stop(p->right, endtime);
	}
	return p;
}

void
timesnode_print(struct timesnode * p, time_t aftertime)
{
	time_t         *nowtime;
	int 		duration = 0;

	if (p == NULL) {
		return;
	}
	if (p->left != NULL) {
		timesnode_print(p->left, aftertime);
	}
	if (p->starttime > aftertime || p->endtime > aftertime || p->endtime == 0) {
		printf("task: %s\n", p->task);
		printf("started at: %s", ctime(&p->starttime));

		if (p->endtime != 0) {
			printf("ended at: %s", ctime(&p->endtime));

			duration = difftime(p->endtime, p->starttime);

			printf("duration(hours): %.2f\n\n", duration / 3600.0);
		} else {
			nowtime = malloc(sizeof(time_t));
			time(nowtime);
			printf("still running\n");

			duration = difftime(*nowtime, p->starttime);

			printf("duration(hours): %.2f\n\n", duration / 3600.0);
			free(nowtime);
		}
	}
	if (p->right != NULL) {
		timesnode_print(p->right, aftertime);
	}
}

void
timesnode_free(struct timesnode * p)
{
	if (p->left) {
		timesnode_free(p->left);
		p->left = NULL;
	}
	if (p->right) {
		timesnode_free(p->right);
		p->right = NULL;
	}
	free_and_null(p->task);
	free_and_null(p);
}

int
main(int argc, char *argv[])
{
	unsigned int 	start, stop, list;
	char           *task;
	FILE           *fp;
	struct timesnode *timestree;
	time_t         *nowtime;
	int 		changed;
	struct tm 	today;

	timestree = NULL;
	task = NULL;
	changed = start = stop = 0;
	list = 1;

	nowtime = malloc(sizeof(time_t));

	ARGBEGIN {
case 'a':			/* start new task - stop current if any */
		start = 1;
		stop = 1;
		list = 0;

		task = EARGF(usage());
		break;
case 's':			/* stop current task */
		start = 0;
		stop = 1;
		list = 0;
		break;
case 'l':			/* list task */
		start = 0;
		stop = 0;
		list = 1;
		break;
case 'v':
		fprintf(stderr, "%s-" VERSION " © 2016 Simon Lieb, see LICENSE for details\n", argv0);
		return 1;
		break;
case 'h':
default:
		usage();
	} ARGEND;

	fp = opentimesfile();

	time(nowtime);
	timestree = loadtimes(fp, timestree);

	if (stop) {
		timesnode_stop(timestree, *nowtime);
		changed = 1;
	}
	if (task != NULL && start) {
		timestree = timesnode_add(timestree, task, *nowtime, 0);
		changed = 1;
	}
	if (changed) {
		freopen(timesfile, "w", fp);
		writetimes(fp, timestree);
	}
	if (list) {
		localtime_r(nowtime, &today);

		today.tm_sec = 0;
		today.tm_min = 0;
		today.tm_hour = 0;

		timesnode_print(timestree, mktime(&today));
	}
	fclose(fp);

	timesnode_free(timestree);

	free_and_null(nowtime);

	return 0;
}
