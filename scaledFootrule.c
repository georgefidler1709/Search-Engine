//Given a set of ranklists, finds a final rank arrangement with minimum scaled footrule distance
//By Geroge Fidler
//16/10/17

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "assert.h"
#include "SFD.h"



/* My algorithm is based on a heuristic approach in which I devised a few postulates. By thinking about scaled footrule distance (SFD) geomterically 
with each rank scaled to a position on the number line, it becomes apparent that SFD is a literal measure of distance between the chosen rank and all
other ranks given for an object (in this case, a URL). With this in mind it can be determined that:

1) If a URL is mentioned in an odd number of rankfiles, its minimum SFD position will be closest to its middlemost rank value, scaled to the size of the final ranklist.
e.g. URL1 has ranks 1/8, 2/8 and 7/8 with a total of 8 URLs ranked. The equivalent position to the middlemost rank = 2/8 * 8 = 2. 
Any movement from this position in either direction would mean moving closer to 1 node while moving further away from 2 nodes, all in equal measure. Thus increasing total distance.

2) If a URL is mentioned in an even number of rankfiles, its minimum SFD lies anywhere between the 2 middlemost rank values scaled to the size of the final ranklist.
e.g. URL1 has ranks, 2/10, 3/10, 6/10 and 9/10 with a total of 10 yrls ranked. The 2 middlemost ranks are 3/10 and 6/10, scaled to the size of the final ranklist = 3 and 6. 
Therefore the minimum SFD position form URL1 is anywhere between (and including) 3 to 6.
Any movement between 3 and 6 will mean moving further away from 2 ranks and closer to 2 ranks in equal measure, maintaining a constant total distance. Once it is beyond the bounds (3 and 6)
it will be moving further away from 3 ranks and closer to 1 rank, therefore increasing SFD.

If every URL can be placed using this approach then a guaranteed min SFD is found in 'n' time and the program will return this.  However in most cases eventually a URL will have its range of min SFD positions (range = 1 for URLs with an odd number of ranks) filled by previously placed URLs.
To combat this when a node wants to be placed in a position which has already been filled, the new URL and the current tenant are compared as to minimise SFD if one was to move one spot to the left. 
Whichever URL would be best to keep in the current spot is stored in a temporary ranklist while the other URL is used as the new URL to repeat the process on the position 1 to the left. This continues iteratively
until a free position is found.

This process is then repeated but with comparisons one to the right rather than one to the left. After both approaches the total SFD increase of both prospective resolutions is compared and whichever holds the minimum 
SFD increase is implemented on the actual ranklist. 

While this secondary approach does not guarantee absolute minimum SFD it is correct in many simple cases (small datasets or when the preferred rank of all URLs is nicely spread) and for all cases it gives axtremely good 'bar' case
(on average within 1% of min SFD) for the factorial step. In this way any arrangement of URLs can be discredited as a min SFD arrangement as soon as its total SFD exceeds that of the 'bar' arrangement, saving us for completeing unnecessary SFD calculations,
for this suboptimal case.

In conclusion, while this is still a O(n!) algorithm it beats the brute force approach in 2 major ways:
1) In simple cases (where all URLs can be placed at a rank/position within their optimal range) it finishes in 'n' time complexity.
2) By using the above method to find a 'bar' rankset and SFD approximating the min SFD, in the factorial step in calculating a ranksets total SFD, as soon as that ranksets SFD exceeds that of the 'bar', 
that rankset can be excluded immediately and all SFD calculations on that rankset are avoided.
*/

int main(int argc, char *argv[]) {
    char buffer[MAX_LINE] = {0};
    int URLsInFile = 0;
    int URLRank = 0;
    SFDURLList uList = newSFDURLList();                                                	//url linked list used to store all urls ranked in rankfiles.

    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");

        while (fgets(buffer, MAX_LINE, fp)) URLsInFile++;                           //total number of URLs in a given rankfile are counted so that ranks can be written as the appropriate fraction for SFD calculation.
        rewind(fp);                                                                //time complexity = total number of ranks across all rankfiles = O(R)
        while (fgets(buffer, MAX_LINE, fp)) {                                       //now urls are actually added to the list with their ranks (or rank added to a URL node if the URL read already has a node)
            URLRank++;                                                             //time complexity = total number of ranks across all rankfiles = O(R)
            char *token = strtok(buffer, " \n");
            uList->head = newSFDURLNode(uList, token, URLsInFile, URLRank);
        }
        URLsInFile = 0;
        URLRank = 0;
    }

    SFDURLNode *bestRanks = malloc((uList->length + 1) * sizeof(SFDURLNode));
    assert(bestRanks);

    int barIsOptimal = barRanking(bestRanks, uList);                                           				//integrated ranklist found
    double totalSFD = 0;
 
    for (int i = 1; i < uList->length + 1; i++) totalSFD += calculateSFD(bestRanks[i], i, uList->length);	//final SFDs added to find totalSFD
    																										//time complexity = total number of URLs ranked = O(U)   
    if (barIsOptimal == FALSE) {																			//If left and right search were never applied then bestRanks at its current state is guaranteed to be a min SFD arrangement (from postulates at the top of the page).
        SFDURLNode *workingRanks = malloc((uList->length + 1) * sizeof(SFDURLNode));
        assert(workingRanks);

	    for (int i = 1; i < uList->length + 1; i++) workingRanks[i] = bestRanks[i];  			//workingRanks set up to be permuted in factorial checking step. //time complexity = O(U)
	    checkMinimum(bestRanks, workingRanks, &totalSFD, 1, uList->length);             //factorial checking step
        free(workingRanks);			                                     
	}

    printf("minSFD = %.6f\n" , totalSFD);
    for (int i = 0; i < uList->length + 1; i++) {                               		//time complexity = total number of URLs ranked = O(U)
        if (bestRanks[i]) printf("%s\n" , bestRanks[i]->URL);
    }

    free(bestRanks);
    freeSFDURLList(uList);
    return 0;
}


//Actual integreated ranking algorithm for the bar case.
int barRanking(SFDURLNode bestRanks[], SFDURLList uList) {
	int guaranteedMinSFD = TRUE;
    SFDURLNode *leftChanges = malloc(uList->length * sizeof(SFDURLNode));
    assert(leftChanges);

    SFDURLNode *rightChanges = malloc(uList->length * sizeof(SFDURLNode));
    assert(rightChanges);

    int chosenRank[2] = {0};
    for (SFDURLNode curr = uList->head; curr; curr = curr->next) {          			//URLs with an odd number of ranks are found positions first as they only have one min SFD position while those with an even number of
                                                                                    		//ranks can have many, thereby minimizing the number of time URLs already placed in final_ranking will need to be changed
        if (curr->ranks->length % 2 == 1) {
            RankNode middleRank = curr->ranks->head;
            for (int i = 1; i != (curr->ranks->length/2 + 1); i++) {
                middleRank = middleRank->next;
            }
            chosenRank[0] = fabs(middleRank->rank_no * uList->length);           			//rank scaled to length of final ranklist and rounded to an integer value to represent a position in this list
            if (!bestRanks[chosenRank[0]]) {                               			//if this optimal position for the URL in the final ranklist is empty, place the URL at that position.
                bestRanks[chosenRank[0]] = curr;
            } 
            else {
                FindSFDRank(bestRanks, uList, curr, chosenRank, leftChanges, rightChanges); //if not we may need to move URLs from their optimal position.
               	guaranteedMinSFD = FALSE;													//FIndSFDRank removes the guarantee that bestRanks is a min SFD arrangement therefore the factorial checking method will be necessary.
                chosenRank[1] = 0;
            }
        }
    }
    for (SFDURLNode curr = uList->head; curr; curr = curr->next) {           			//Now attempt to position URls with an even number of ranks.
        if (curr->ranks->length % 2 == 0) {
            RankNode L_middleRank = curr->ranks->head;
            for (int i = 1; i != (curr->ranks->length/2); i++) {
                L_middleRank = L_middleRank->next;
            }
           	RankNode R_middleRank = L_middleRank->next;                             		//L & R_middleRank are the Left and Right boundaries of the min SFD range
            if (L_middleRank->rank_no * uList->length == (int)(L_middleRank->rank_no * uList->length)) 
                chosenRank[0] = L_middleRank->rank_no * uList->length; 						//takes the ceiling
            else chosenRank[0] = (int)(L_middleRank->rank_no* uList->length) + 1;

            chosenRank[1] = (int)(R_middleRank->rank_no * uList->length); 					//takes the floor, this is to ensure the range only contains min SFD ranks 
            																				//e.g. if a URL had 2 ranks (scaled to final list length) 4.1 and 8.9, its min SFD range is positions 5 to 8.
            int positionFound = FALSE;
            for (int i = chosenRank[0]; i <= chosenRank[1]; i++) {							//searches min SFD range for the URL, inserts node if possible.
                if (!bestRanks[i]) {
                    bestRanks[i] = curr;
                    positionFound = TRUE;
                    break;
                }
            }
            if (positionFound == FALSE) {
            	FindSFDRank(bestRanks, uList, curr, chosenRank, leftChanges, rightChanges);	//if not we may need to move URLs from their optimal position.
            	guaranteedMinSFD = FALSE;													//FIndSFDRank removes the guarantee that bestRanks is a min SFD arrangement therefore the factorial checking method will be necessary.
            }																						
        }
    }
    free(leftChanges);
    free(rightChanges);
    return guaranteedMinSFD;
}


//Attempts to minimize total SFD increase when moving URL from its min SFD position using only 2 node comparisons.
void FindSFDRank(SFDURLNode bestRanks[], SFDURLList uList, SFDURLNode curr, int chosenRank[], SFDURLNode leftChanges[], SFDURLNode rightChanges[]) {
    double leftSFDinc = 0;
    double rightSFDinc = 0;
    int leftFinish = 0;
    int rightFinish = 0;

    if (chosenRank[1] == 0) chosenRank[1] = chosenRank[0];
    leftFinish = searchLeft(bestRanks, uList->length, curr, chosenRank[0], leftChanges, &leftSFDinc);					//attempts to minimize total SFD increase in moving a URL to the closest free position on the left.
    rightFinish = searchRight(bestRanks, uList->length, curr, chosenRank[1], rightChanges, &rightSFDinc, leftSFDinc);	//attempts to minimize total SFD increase in moving a URL to the closest free position on the left.
    
    if (rightFinish != FAIL){																							//returned if rightSearch could not minimize SFD increase better than leftSearch																							//
        for (int i = chosenRank[1]; i <= rightFinish; i++) bestRanks[i] = rightChanges[i];
    } 
    else {
        for (int i = chosenRank[0]; i >= leftFinish; i--) bestRanks[i] = leftChanges[i];
    }
}

//Attempts to minimize total SFD increase when moving URL from its min SFD position using only comparisons between a URL and its left neighbour in the ranklist.
//This left neighbour is the blocknode, blocking the URL from being placed where it wants to to minimize its SFD.
int searchLeft(SFDURLNode ranks[], int listLength, SFDURLNode currNode, int chosenRank, 
    SFDURLNode changes[], double *leftSFDinc) {
	*leftSFDinc = 0;

    for (int i = chosenRank; i >= 1; i--) {
        SFDURLNode blockNode = ranks[i];
        if (!blockNode) {													    //If there is no blockNode for the chosenRank, insert the URL
            *leftSFDinc += calculateSFD(currNode, i, listLength);				//and report the SFD increase back to FindSFDRank
            changes[i] = currNode;
            return i;
        }
        double curr_bestSFD = calculateSFD(currNode, i, listLength);			//else undergo a comparison of which is best at the current position based on which calculating the total SFD both both 2 URL arrangements
        double block_bestSFD = calculateSFD(blockNode, i, listLength);			//i.e. the SFD if URL1 was to stay and URL2 was to move one to the left vs the SFD if URL2 was to stay and URL1 was to move one to the left
        double curr_nextSFD = calculateSFD(currNode, i - 1, listLength);		//this is based on the idea that if URL1 increases the total SFD by jumping one to the left than URL2 does, this will continue for 'n' jumps to the left. 
        double block_nextSFD = calculateSFD(blockNode, i - 1, listLength);		//This is only true in simple cases where ranks well distributed but serves as a good, quick estimate of a min SFD arrangement.

        double currDiff = curr_bestSFD - curr_nextSFD;
        double blockDiff = block_bestSFD - block_nextSFD;
        double totalSFDdiff = blockDiff - currDiff;
         
        if (totalSFDdiff > 0) {													//if the new URL being positioned at chosenRank reduced SFD better than blockNode, the two are swapped, the change recorded in changes[], 
        	changes[i] = currNode;												//and the process continues with blockNode now being compared to its left neighbour
            *leftSFDinc += curr_bestSFD - block_bestSFD;
            currNode = blockNode;
        } 
        else changes[i] = blockNode;
    }

    *leftSFDinc = NO_FREE_RANK;
    return FAIL;
}

//Attempts to minimize total SFD increase when moving URL from its min SFD position using only comparisons between a URL and its right neighbour in the ranklist.
//This right neighbour is the blocknode, blocking the URL from being placed where it wants to to minimize its SFD.
int searchRight(SFDURLNode ranks[], int listLength, SFDURLNode currNode, int chosenRank, 
    SFDURLNode changes[], double *rightSFDinc, double leftSFDinc) {
    *rightSFDinc = 0;

    for (int i = chosenRank; i <= listLength; i++){
        SFDURLNode blockNode = ranks[i];
        if (!blockNode) {
            *rightSFDinc += calculateSFD(currNode, i, listLength);
            changes[i] = currNode;
            if (*rightSFDinc >= leftSFDinc) return FAIL;					//the major difference between leftSearch and rightSearch is that by going second rightSearch can constantly compare its total SFD increase to that of leftSearch at its conclusion.
            return i;														//as soon as the total SFD increase of rightSearch exceeds that of the left we can exit immediately as any resolution found thereafter will be worse in terms of minimizing SFD than that of leftSearch. 
        }
        double curr_bestSFD = calculateSFD(currNode, i, listLength);
        double block_bestSFD = calculateSFD(blockNode, i, listLength);
        double curr_nextSFD = calculateSFD(currNode, i + 1, listLength);
        double block_nextSFD = calculateSFD(blockNode, i + 1, listLength);

        double currDiff = curr_bestSFD - curr_nextSFD;
        double blockDiff = block_bestSFD - block_nextSFD;
        double totalSFDdiff = blockDiff - currDiff;
         
        if (totalSFDdiff >= 0) {
            *rightSFDinc += curr_bestSFD - block_bestSFD;
            if (*rightSFDinc >= leftSFDinc) return FAIL;
            changes[i] = currNode;
            currNode = blockNode;
        } 
        else changes[i] = blockNode;
    }
    return FAIL;
}


//Uses the scaled footrule algorithm to determine a URLs SFD score at any given position/rank.
double calculateSFD(SFDURLNode node, int givenRank, int totalURLs) {
    double SFD = 0;
    for (RankNode rank = node->ranks->head; rank != NULL; rank = rank->next) {
        SFD += fabs(rank->rank_no - (double)givenRank/totalURLs);
    }
    return SFD;
}


//Finds every permutation of the ranklist so it can be checked for minSFD value. O(n!)
//Based off code to print out every permutation of a string found at http://www.geeksforgeeks.org/write-a-c-program-to-print-all-permutations-of-a-given-string/
//but naturally heavily modified to fit this case
void checkMinimum(SFDURLNode *bestRanks, SFDURLNode *workingRanks, double *barSFD, int mover, int listLength) {
    if (mover == listLength) checkSFD(bestRanks, workingRanks, listLength, barSFD);	//when a new permutation is found check its SFD
    else {
        for (int i = mover; i <= listLength; i++) {
            swap(workingRanks, mover, i);
            checkMinimum(bestRanks, workingRanks, barSFD, mover + 1, listLength);
            swap(workingRanks, mover, i);       
        }
    }
}


//Swaps positions (i.e. ranks) of 2 URLs
void swap(SFDURLNode *list, int x, int y) {
    SFDURLNode temp = NULL;
    temp = list[x];
    list[x] = list[y];
    list[y] = temp;
}


//Calculates the total SFD of a rankset. As soon as this SFD calculation exceeds that of the barSFD (current best SFD found) the calculations stop.
//By finding an extremely accurate barSFD from the previous functions this will mean almost all SFD calculations will return early, skipping many unnecessary calculations.
void checkSFD(SFDURLNode *bestRanks, SFDURLNode *workingRanks, int listLength, double *barSFD) {    
    double totalSFD = 0;
    for (int i = 1; i < listLength + 1; i++) {							//O(n) adding SFDs of each URL 
        totalSFD += calculateSFD(workingRanks[i], i, listLength);
        if (totalSFD >= *barSFD) return;
    }
    for (int i = 1; i < listLength + 1; i++) {							//If there is no early return this means the current permutation is better than the bar, therefore is stored as the new bar. O(n)
        bestRanks[i] = workingRanks[i];									//this should happen extremely infrequently, less than 1% of arrangement in most cases
    }
    *barSFD = totalSFD;
}