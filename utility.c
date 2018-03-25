//Utility Functions
//By George Fidler and Eddie Belokopytov
//18/10/17
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "URL.h"

//Removes everything but letters from the word and reduces all letter to lower case.
void normaliseWord(char *word) {
    for (int i = 0; i < strlen(word); i++) {
        if (word[i] >= 'A' || word[i] <= 'Z') word[i] = tolower(word[i]);
        if (word[i] < 'a' || word[i] > 'z') {
            word[i] = '\0';
            return;
        }
    }
}

//concatenates two strings, returning the resulting string
char * concat(char *str1, char *str2) {
	char *result = calloc(strlen(str1)+strlen(str2)+1, sizeof(char)); //+1 for null terminator
	assert(result);
	strcat(result, str1);
	strcat(result, str2);
	return result;
}

//If the string is found in collection.txt then it is a url, otherwise it is not
int isURL(char *string) {
	URLQueue urls = getURLS();
	int found = 0;
	for (URLNode curr = urls->head; curr && !found; curr = curr->next) {
		if (strEQ(curr->URL, string)) found = 1;
	}
	free(urls);
	return found;
}


char *strdup (const char *s) {
    char *d = malloc (strlen (s) + 1);
    assert(d);
    strcpy (d,s);
    return d;
}