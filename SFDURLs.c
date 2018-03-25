//All the functions for initially loading up the URLs found in rankfiles and their respective ranks.
//By George Fidler and Eddie Belokopytov
//4/10/17

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include "SFD.h"

//Each URL ranked is stored in a linked list
SFDURLNode newSFDURLNode(SFDURLList uList, char *URL, int URLs_total, int URL_rank) {
    if (uList->head) {
        for (SFDURLNode curr = uList->head; curr; curr = curr->next) {                  //time complexity = num of unique URLs already read from rankfiles = O(U)
            if (strcmp(curr->URL, URL) == 0) {                                       //if a node already exists for a given URL, just add the new rank to that node.
                addRank(curr->ranks, URLs_total, URL_rank);
                return uList->head;
            }
        }
    }

    SFDURLNode new = malloc(sizeof(SFDURLNode));
    assert(new);

    new->URL = calloc(strlen(URL) , sizeof(char));
    assert(new->URL);
    strcpy(new->URL, URL);

    new->ranks = newRankList();
    addRank(new->ranks, URLs_total, URL_rank);                                      //first rank of new URL node added
    new->next = uList->head;
    uList->length += 1;
    return new;
}

//Adds a rank for a URL from a rankfile to a pre-existing SFDURLNode
void addRank(RankList ranks, int URLs_total, int URL_rank) {
    RankNode new = malloc(sizeof(rank));
    assert(new);

    new->rank_no = (double)URL_rank / URLs_total;
    new->next = NULL;
    new->prev = NULL;
    RankNode curr = NULL;

    if (!ranks->head) {
        ranks->head = new;
    } 
    else {
        for (curr = ranks->head; curr; curr = curr->next) {              //ranks are entered in SFDURLNode in non-decreasing order so the middlemost rank value(s) can be found easily later.
            if (new->rank_no <= curr->rank_no) {                                 //time complexity = number of ranks for that URL squared = O(urlRanks^2)
                if (curr == ranks->head) {
                    new->next = curr;
                    curr->prev = new;
                    ranks->head = new;
                    break;
                } 
                else {
                    curr->prev->next = new;
                    new->prev = curr->prev;
                    curr->prev = new;
                    new->next = curr;
                    break;
                }
            }
            if (!curr->next) {
                new->prev = curr;
                curr->next = new;
                break;
            } 
        }
        
    }
    ranks->length += 1;
}


//RankList within a SFDURLNode used to store all ranks for that node.
RankList newRankList() {
    RankList new = malloc(sizeof(ranklist));
    assert(new);
    new->head = NULL;
    new->length = 0;
    return new;
}


//List of all SFDURLNodes
SFDURLList newSFDURLList() {
    SFDURLList new = malloc(sizeof(SFDURLList));
    assert(new);
    new->length = 0;
    new->head = NULL;
    return new;
}


//Frees SFDURLList and all associated allocated memory
void freeSFDURLList(SFDURLList l) {
    SFDURLNode uNode = l->head;
    while (uNode) {
        RankNode rNode = uNode->ranks->head;
        while (rNode){
            RankNode rPrev = rNode;
            rNode = rNode->next;
            free(rPrev);
        }
        free(uNode->ranks);
        SFDURLNode uPrev = uNode;
        uNode = uNode->next;
        free(uPrev);    
    }
    free(l);
}
