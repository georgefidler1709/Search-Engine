//Sets up the URL structs which will be how we store necessary information from each 'webpage' and how these structs themselves will be stored and accessed
//By George Fidler and Eddie Belokopytov
//28/9/17

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "URL.h"
#include "set.h"

URLQueue newURLQueue() {
	URLQueue new = malloc(sizeof(struct _URLqueue));
	assert(new);
	new->head = NULL;
	new->tail = NULL;
	new->len = 0;
	return new;
}

void newURLNode(char *str, URLQueue q) {
	URLNode new = malloc(sizeof(urlnode));
	assert(new);

	new->URL = calloc(strlen(str)+1, sizeof(char)); //+1 for null terminator
	assert(new->URL);
	strcpy(new->URL, str);

	new->rankScore = NOT_SET;
	new->tf = NOT_SET;
	new->termMatches = NOT_SET;
	new->next = NULL;

	if (!q->head) {
		q->head = new;
		q->tail = new;
	}
	else {
		q->tail->next = new;
		q->tail = new;
	}
	q->len++;
}

//Go through collection.txt and create a queue from the urls inside
URLQueue getURLS() {
	URLQueue q = newURLQueue(); //the final queue containing the unique URLs in collection.txt
	Set seenURLs = newSet();
	FILE *fp = fopen("collection.txt" , "r"); assert(fp);
	char buffer[MAX_LINE] = {0};

	while (fgets(buffer, MAX_LINE, fp)) {
		char *token = strtok(buffer, " \n"); //(explained in detail in pagerank.c)
		while (token) {
			if (!isElem(seenURLs, token)) { //prevent queue from having duplicate URLs
				newURLNode(token, q);
				insertInto(seenURLs, token);
			}
			token = strtok(NULL, " \n");
		}
	}

	fclose(fp);
	disposeSet(seenURLs);
	return q;
}

//Go through the queue, free every url, every node and then the struct
void freeURLQueue(URLQueue q) {
    URLNode uNode = q->head;
    while (uNode) {
        URLNode uPrev = uNode;
        uNode = uNode->next;
        free(uPrev->URL);
        free(uPrev);
    }
    free(q);
}
