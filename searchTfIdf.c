#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include "URL.h"
#include "search.h"
#include "utility.h"

void findTfIdf(URLQueue,char*);
void findTf(URLQueue,char*);
void multiplyByIdf(URLQueue,int);
void printFunction(URLNode);

int main(int argc, char *argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "Usage: <searchTerm> <searchTerm> ...\n");
		return 1;
	}

	URLQueue URLsWithSearchTerms = getURLsWithSearchTerms(argc, argv, findTfIdf); //pass findTfIdf function because tfidf needs to be calculated per term
	URLNode *sortedNodePointersArray = sortResults(URLsWithSearchTerms);
	outputResults(sortedNodePointersArray, URLsWithSearchTerms->len, printFunction);

	free(sortedNodePointersArray);
	freeURLQueue(URLsWithSearchTerms);
	return 0;
}

//For each term we receive the urls for that term and calculate the tfidifs for them
void findTfIdf(URLQueue urlsForTerm, char *searchTerm) {
	findTf(urlsForTerm, searchTerm);
	multiplyByIdf(urlsForTerm, urlsForTerm->len);
}

//calculates the term frequency of  given term in a given URL file.
void findTf(URLQueue list, char *term) {
	char buffer[MAX_LINE] = {0};
	for (URLNode mover = list->head; mover; mover = mover->next) {
		int startRead = 0, numTerms = 0, numWords = 0;

		char *urlFileName = concat(mover->URL, ".txt");
		FILE *fp = fopen(urlFileName, "r"); assert(fp);		//opens the text file with the URL stored in the URLnode
		free(urlFileName);
		while (fgets(buffer, MAX_LINE, fp)) {
			if (startRead != 1) {
				if (strEQ(buffer, "#start Section-2\n")) startRead = 1;		//only starts reading at the start of section 2
				continue;
			}
			if (buffer[0] == '#' && buffer[1] == 'e') break;				//finishes at the end of section 2

			char *token = strtok(buffer, " \n");							//reads all words in part2, keeping track of total read and total of the term in question
			while (token) {
				normaliseWord(token);
				if (strEQ(token, term)) numTerms++;
				numWords++;		
			
				token = strtok(NULL, " \n");
			}
		}
		mover->tf = (double)numTerms/numWords;							//uses this to calculate term frequency
		fclose(fp);
	}
}

//As idf is constant across all files, simply need to find the product of tf and idf for a complete tf-idf score for a URL file
void multiplyByIdf(URLQueue list, int termURLs) {
	URLQueue urls = getURLS(); 
	int totalURLs = urls->len; freeURLQueue(urls);
	for (URLNode mover = list->head; mover; mover = mover->next) {
		mover->rankScore += mover->tf * log10((double)totalURLs/termURLs);	//tf-idf calculation
	}
}

void printFunction(URLNode urlNode) {
	printf("%s %.6lf\n", urlNode->URL, urlNode->rankScore);
}
