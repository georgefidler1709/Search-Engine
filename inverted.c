#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include "URL.h"
#include "utility.h"

//Creates an inverted index file of all words in the URL files named in collection.txt
//By George Fidler
//9/10/17

typedef struct _wordnode *WordNode;

struct _wordnode {
    char *word;
    URLQueue URLs;
    WordNode next;
    WordNode prev;

} wordnode;

typedef struct _wordlist *WordList;

struct _wordlist {
    WordNode head;
    WordNode tail;
} wordlist;

WordList newWordList();
WordNode newWordNode(char*,char*,WordList,URLQueue);
void addURL(char*,WordNode,URLQueue);
void insertNode(WordNode,WordNode,WordList);
void addWord(char*,char*,WordList,URLQueue);
void freeWordList(WordList);

int main(void) {
    URLQueue urls = getURLS();     //creates linked list of all URLs in collection.txt
    WordList list = newWordList(); //list of all words in URL files
    char buffer[MAX_LINE] = {0};
    int startRead = 0;

    URLNode mover = urls->head;
    while (mover) {
        char *urlFileName = concat(mover->URL, ".txt"); //adds .txt to actually open the file related to each URL.
        FILE *fp = fopen(urlFileName, "r"); assert(fp);
        free(urlFileName);
        while (fgets(buffer, MAX_LINE, fp)) {  //only start reading words from that file once in section-2
            if (startRead != 1) {
                if (strEQ(buffer, "#start Section-2\n")) startRead = 1;
                continue;
            }
            if (buffer[0] == '#' && buffer[1] == 'e') break;  //finish at the end of section 2

            char *token = strtok(buffer, " \n");   //reads each word out of section 2 individually
            while (token) {
                normaliseWord(token);     //normalises them
                addWord(mover->URL, token, list, urls); //and adds them to the word list (with the current URL inside the wordnode)
                token = strtok(NULL, " \n");
            }
        }

        mover = mover->next;
        fclose(fp);
        startRead = 0;
    }

    FILE *fp = fopen("invertedIndex.txt" , "w+");
    WordNode curr = list->head;
    while (curr) {   //prints words and all URLs they're present in in alphabetical order
        fprintf(fp, "%s  ", curr->word);
        for (URLNode mover = curr->URLs->head; mover; mover = mover->next) fprintf(fp, "%s ", mover->URL);
        fprintf(fp, "\n");
        curr = curr->next;
    }

    fclose(fp);
    freeWordList(list);
    freeURLQueue(urls);
    return 0;
}


//List of all words in the URL files, stored alphabetically.
WordList newWordList() { 
    WordList new = malloc(sizeof(wordlist));
    assert(new);
    new->head = NULL;
    new->tail = NULL;
    return new;
}

//Individual word stored here with a an array of URLs it appears in. 
WordNode newWordNode(char *currURL, char *word, WordList list, URLQueue urls) {    
    WordNode new =  malloc(sizeof(wordnode));
    assert(new);

    new->word = calloc(strlen(word)+1, sizeof(char)); //+1 for null terminator
    assert(new->word);
     
    strcpy(new->word, word);

    new->URLs = newURLQueue();
    addURL(currURL, new, urls);    //adds URL in which the word was first sighted

    new->next = NULL;
    new->prev = NULL;

    if(list->head == NULL) {
        list->head = new;
        list->tail = new;
    } 

    return new;
}

void addWord(char *currURL, char *word, WordList list, URLQueue urls) {
    if (!list->head) {        //if this is the first wordNode, no need to sort.
        newWordNode(currURL, word, list, urls);
        return;
    }

    WordNode mover = list->head;
    WordNode newNode = NULL;

    while (mover) {       //finds the appropriate alphabetical spot for the word.
        int cmp = strcmp(word, mover->word);
        if (cmp == 0) {
            addURL(currURL, mover, urls);
            return;
        } 
        else if (cmp < 0) {
            newNode = newWordNode(currURL, word, list, urls);
            insertNode(newNode, mover, list);     //inserts wordNode into spot in the list
            return;
        }

        mover = mover->next;
    }
    newNode = newWordNode(currURL, word, list, urls);
    insertNode(newNode, NULL, list);
}

//Inserts wordNode into the list in spot determined in addWord
void insertNode(WordNode newNode, WordNode afterNode, WordList list) {
    if (!afterNode) {
        list->tail->next = newNode;
        newNode->prev = list->tail;
        list->tail = newNode;
    } 
    else if (afterNode == list->head) {
        list->head->prev = newNode;
        newNode->next = list->head;
        list->head = newNode;
    } 
    else {
        afterNode->prev->next = newNode;
        newNode->prev = afterNode->prev;
        afterNode->prev = newNode;
        newNode->next = afterNode;
    }
}

//Adds URL to wordNode
void addURL(char *URL, WordNode presentWord, URLQueue urls) {
    for (URLNode mover = presentWord->URLs->head; mover; mover = mover->next) {                            
        if (strEQ(URL, mover->URL)) return;   //if URL is already present in wordNode, no need to add it.
    }
    newURLNode(URL, presentWord->URLs);
}

void freeWordList(WordList l) {
    WordNode wNode = l->head;
    while (wNode) {
        WordNode wPrev = wNode;
        wNode = wNode->next;
        free(wPrev->word);
        freeURLQueue(wPrev->URLs);
        free(wPrev);
    }
    free(l);
}
