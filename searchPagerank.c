//Gives output for pagerank evalutation for testing
//By George Fidler and Eddie Belokopytov
//21/10/17

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "set.h"
#include "URL.h"
#include "search.h"

void setPageRanks(URLQueue);
void printFunction(URLNode);

int main(int argc, char *argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "Usage: <searchTerm> <searchTerm> ...\n");
		return 1;
	}

	URLQueue URLsWithSearchTerms = getURLsWithSearchTerms(argc, argv, NULL); //we pass NULL becasue we dont the URLs for each term, just the final list of unique URLs for all terms
	setPageRanks(URLsWithSearchTerms);
	URLNode *sortedNodePointersArray = sortResults(URLsWithSearchTerms);
	outputResults(sortedNodePointersArray, URLsWithSearchTerms->len, printFunction);

	free(sortedNodePointersArray);
	freeURLQueue(URLsWithSearchTerms);
	return 0;
}

//Goes through the URLQueue and sets the corresponding pagerank for the current URL
void setPageRanks(URLQueue urls) {
	char buffer[MAX_LINE], string[MAX_URL]; double pageRank = 0;
	FILE *fp = fopen("pagerankList.txt", "r"); assert(fp);

	for (URLNode curr = urls->head; curr; curr = curr->next) {
		while (fgets(buffer, MAX_LINE, fp)) {
			sscanf(buffer, "%[^,], %*d, %lf", string, &pageRank); //from the buffer, read a string stoppping at a "," then find but dont read an int then read a double (all comma-space separated) 
			if (strEQ(string, curr->URL)) {
				curr->rankScore = pageRank;
				break;
			}
		}
		if (curr->next) rewind(fp); //we only need to rewind the file pointer if it's not the last iteration
	}
	fclose(fp);
}

void printFunction(URLNode urlNode) {
	printf("%s\n", urlNode->URL);
}
