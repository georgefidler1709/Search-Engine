//Creates a ranking list for a variable number of search terms
//By George Fidler and Eddie Belokopytov
//17/10/17

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "set.h"
#include "URL.h"
#include "search.h"
#include "utility.h"

static void incrementTotalTermMatches(char*,URLQueue);
static void copyChanges(URLQueue,URLQueue);
static int compareFunction(const void*,const void*);

/*************************************************************************
for each search term
	go through inverted index and find the line with that term
	add all the urls to a queue for the current term
	if a url is in the master queue (for all search terms):
		increment its matches
	else:
		add it to the master queue and set matches to 1
	
	copy the data from the master queue into the term's queue
	call the given function if not null with the term given its relevant queue
	copy any changes that function may have made to the term's queue into the master queue
*************************************************************************/
URLQueue getURLsWithSearchTerms(int argc, char *argv[], void (*functionForSearchTerm) (URLQueue URLsForTerm, char *searchTerm)) {
	URLQueue URLsWithSearchTerms = newURLQueue(); //the master queue which will hold all URLs found for all search terms
	Set seenURLs = newSet();
	FILE *fp = fopen("invertedIndex.txt", "r"); assert(fp);

	for (int i = 1; i < argc; i++) {
		URLQueue URLsForTerm = newURLQueue(); //a queue that should the urls for the current search term
		int found = 0; char string[MAX_LINE]; //needs to handle both words and urls

		normaliseWord(argv[i]); //normalise the search term as the terms in invertedIndex.txt are normalised
		while (fscanf(fp, "%s", string) == 1) {
			if (found) {
				if (isURL(string)) { //then we're on the line with the search term
					newURLNode(string, URLsForTerm); //can insert without checking because urls are unique on a line in invertedIndex.txt
					if (!isElem(seenURLs, string)) { //so that we dont get duplicates in the master queue
						newURLNode(string, URLsWithSearchTerms);
						URLsWithSearchTerms->tail->termMatches = 1;
						insertInto(seenURLs, string);
					}
					else incrementTotalTermMatches(string, URLsWithSearchTerms);
				}
				else break; //then we've moved to the next line and we have finished reading the relevant line
			}
			else if (strEQ(string, argv[i])) found = 1; //we havent found the search term line yet but we may find it now
		}

		copyChanges(URLsWithSearchTerms, URLsForTerm); //so that we can keep the rankScore for aggregation
		if (functionForSearchTerm) functionForSearchTerm(URLsForTerm, argv[i]);
		copyChanges(URLsForTerm, URLsWithSearchTerms); //so that our final list that gets sorted has the updated values after calculations
		
		if (i+1 != argc) rewind(fp); //we only need to rewind the file pointer if it's not the last iteration
		freeURLQueue(URLsForTerm);
	}

	fclose(fp);
	disposeSet(seenURLs);
	return URLsWithSearchTerms;
}

//Go through the URLQueue until you find the node with the matching url, then increment its matches
static void incrementTotalTermMatches(char *url, URLQueue urls) {
	URLNode curr = urls->head;
	while (!strEQ(curr->URL, url)) curr = curr->next;
	curr->termMatches += 1;
}

//Go through the nodes in the newQ, find the matching node in the oldQ if it exists and copy over the data
static void copyChanges(URLQueue newQ, URLQueue oldQ) {
	for (URLNode currForNew = newQ->head; currForNew; currForNew = currForNew->next) {
		URLNode currForOld = oldQ->head;
		while (currForOld && !strEQ(currForOld->URL, currForNew->URL)) currForOld = currForOld->next;
		if (currForOld) {
			//keep oldQ updated except for termMatches because this is maintained in the calling function
			currForOld->tf = currForNew->tf;
			currForOld->rankScore = currForNew->rankScore;
		}
	}
}

//Populate an array with the pointers to the nodes in the URLQueue, then quicksort this array and return the sorted array
URLNode * sortResults(URLQueue urls) {
 	int index = 0;
	URLNode *nodePointersArray = calloc(urls->len, sizeof(URLNode)); assert(nodePointersArray);
	for (URLNode curr = urls->head; curr; curr = curr->next) nodePointersArray[index++] = curr;
	
	qsort(nodePointersArray, urls->len, sizeof(URLNode), compareFunction);
	return nodePointersArray;
}

//Compare two pointers to pointers to URLNodes and sort them based on termMatches and then rankScore if needed
static int compareFunction(const void *element1, const void *element2) {
	URLNode node1 = *((URLNode *)element1); URLNode node2 = *((URLNode *)element2); //our elements were URLNode so we've been passed pointers to URLNode
	//compare on the primary sorting field
	double comparison = node2->termMatches - node1->termMatches;
	if (comparison > 0) return 1;
	if (comparison < 0) return -1;
	//compare on the secondary sorting field because the primary sorting field is equal
	comparison = node2->rankScore - node1->rankScore;
	if (comparison > 0) return 1;
	if (comparison < 0) return -1;
	return 0;
}

//Go through the nodePointers array and print the top 30 or less results with the given print function 
void outputResults(URLNode * nodePointers, int size, void (*printFp) (URLNode urlNode)) {
	for (int i = 0; i < (size > MAX_PRINT ? MAX_PRINT : size); i++) printFp(nodePointers[i]);
}