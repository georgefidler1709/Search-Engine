#ifndef SEARCH_H
#define SEARCH_H

#include "URL.h"
#define MAX_PRINT 30

URLQueue getURLsWithSearchTerms(int,char*[],void(*)(URLQueue,char*));
URLNode *sortResults(URLQueue);
void outputResults(URLNode*,int,void(*)(URLNode));

#endif