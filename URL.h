#ifndef URL_H
#define URL_H

#include <string.h>
#define strEQ(g,t) (strcmp((g),(t)) == 0) //copied here from Graph.c for cross-file use
#define MAX_LINE 1024
#define MAX_URL 20
#define NOT_SET 0

typedef struct _URLnode *URLNode;

typedef struct _URLnode {
	char *URL;
	double tf; //separate to rankScore because rankScore needs to be aggregate using this as part of the calculation
	double rankScore;
	int termMatches; //number of times a URL has matched a set of search terms
	URLNode next;
} urlnode ;

typedef struct _URLqueue *URLQueue;

typedef struct _URLqueue {
	URLNode head;
	URLNode tail;
	int len;
}urlqueue;

URLQueue newURLQueue();
void newURLNode(char*,URLQueue);
URLQueue getURLS();
void freeURLQueue(URLQueue q);

#endif