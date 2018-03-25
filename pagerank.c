//Calculates the page rank of each 'webpage' using the PageRank alogrithm
//By George Fidler and Eddie Belokopytov
//10/10/17

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "graph.h"
#include "URL.h"
#include "utility.h"

typedef enum { In, Out } LinkType; //a definition of link types for clarity and the validation of parameters

void calculatePageRank(double,double,int);
double getWeightedLinkValues(double[],Graph,char*);
double getLinks(LinkType,char*,Graph);
double getLinksReferencedBy(LinkType,char*,Graph);
void outputPageRanks(double[],Graph);

int main(int argc, char *argv[]) {
	if (argc != 4) {
		fprintf(stderr, "Usage: <dampening> <minDiff> <maxIterations>\n");
		return 1;
	}
	calculatePageRank(atof(argv[1]), atof(argv[2]), atoi(argv[3]));
	return 0;
}

//Goes through "arr" with given "size" and returns the index of the largest element
int getLargest(double arr[], int size) {
	double currLargest = 0; int currIndex = 0;
	for (int i = 0; i < size; i++) {
		if (arr[i] > currLargest) {
			currLargest = arr[i];
			currIndex = i;
		}
	}
	return currIndex;
}

/*
Goes through every URL file in the given queue and reads section 1 to find its outlinks.
Each outlink represents a directed edge from the current url to the url linked to.
*/
Graph getGraph(URLQueue urls) {
	Graph graph = newGraph(urls->len); //create an empty graph with max vertices equal to the numbers of urls
	char buffer[MAX_LINE], *link;
	URLNode mover = urls->head;
	while (mover) {
		char *urlFileName = concat(mover->URL, ".txt"); //append .txt to the URL name to find the file to read
		FILE *fp = fopen(urlFileName, "r"); assert(fp);
		free(urlFileName);
		while (fgets(buffer, MAX_LINE, fp)) {
			if (strEQ(buffer, "#start Section-1\n")) continue; //skip the line that says #start Section-1
			if (buffer[0] == '#' && buffer[1] == 'e') break; //if the buffer starts with #e then the line is "#end Section" * so we've finished reading

			link = strtok(buffer, " \n"); //splits the buffer on any number of spaces and newlines - meaning empty lines will be disregarded
			while (link) { //since link points to the split buffer
				if (!strEQ(mover->URL, link)) addEdge(graph, mover->URL, link); //add an edge from the current url to its link ensuring no self-loops (duplicates handled by ADT)
				link = strtok(NULL, " \n"); //get the next part of the split buffer
			}
		}
		mover = mover->next;
		fclose(fp);
	}
	return graph;
}

/***********************************************
create arrays to store the current and previous iterations pageranks
initialise the pageranks via the formula because it's iteration 0

for iterations going from 0 to maxIterations
	if the difference of the pageranks for this iteration is >= to the minimumDifference
		exit
	otherwise
		copy over the previous iterations pageranks
		calculate this iterations pageranks for all urls
		calculate the aggregate difference in pageranks between this iteration and the previous

output the pageranks
free associated memory
***********************************************/
void calculatePageRank(double dampening, double minDiff, int maxIterations) {
	URLQueue urls = getURLS(); //get all the urls in collection.txt
	Graph g = getGraph(urls); freeURLQueue(urls);
	double diff = minDiff;
	double *pageRanks = calloc(g->nV, sizeof(double)), *prevRanks = calloc(g->nV, sizeof(double));
	for (int i = 0; i < g->nV; i++) pageRanks[i] = (double)1/g->nV; //initlaise the pageranks in the base iteration

	for (int iteration = 0; iteration < maxIterations && diff >= minDiff; iteration++) {
		for (int i = 0; i < g->nV; i++) prevRanks[i] = pageRanks[i]; //copy over the previous iterations pageranks
		for (int i = 0; i < g->nV; i++) {
			pageRanks[i] = (double)(1-dampening)/g->nV + dampening*getWeightedLinkValues(prevRanks, g, g->vertex[i]); //calculate the pageranks for this iteration
		}
		diff = 0;
		for (int i = 0; i < g->nV; i++) diff += fabs(pageRanks[i] - prevRanks[i]); //find the difference between the new pageranks and the old ones
	}
	
	outputPageRanks(pageRanks, g);
	free(pageRanks); free(prevRanks);
}

/*
This represents the sigma term in the equation 
It goes through all urls to find those that link to the subjectURL (p(i))
If it finds one then it adds the product of WIn, WOut and previous pagerank to the total
*/
double getWeightedLinkValues(double prevRanks[], Graph g, char *subjectURL) {
	double total = 0, WIn = 0, WOut = 0;
	for (int i = 0; i < g->nV; i++) {
		if (isConnected(g, g->vertex[i], subjectURL)) { //finding p(j) in M(p(i))
			WIn = (double)getLinks(In, subjectURL, g)/getLinksReferencedBy(In, g->vertex[i], g); //inlinks to p(i)/(sum of inlinks of nodes with inlinks from p(j))
			WOut = (double)getLinks(Out, subjectURL, g)/getLinksReferencedBy(Out, g->vertex[i], g); //as above but replacing inlinks for outlinks
			total += prevRanks[i]*WIn*WOut;
		}
	}
	return total;
}

//Goes through every node to find the number of inlinks or outlinks to the node
double getLinks(LinkType type, char *url, Graph g) {
	int total = 0, isInLinks = type == In;
	for (int i = 0; i < g->nV; i++) {
		if (isInLinks ? isConnected(g, g->vertex[i], url) : isConnected(g, url, g->vertex[i])) { //handling different link direction for inlinks and outlinks
			total += 1;
		}
	}
	return isInLinks ? total : (total == 0 ? 0.5 : total); //if it's for inlinks or (outlinks and the total is not 0), return the total, otherwise return 0.5
}

//Goes through every node to find ones that the referer links to and gets their inlinks or outlinks
double getLinksReferencedBy(LinkType type, char *referer, Graph g) {
	double total = 0;
	for (int i = 0; i < g->nV; i++) {
		if (isConnected(g, referer, g->vertex[i])) { //nodes linked to by the current referer of p(i)
			total += getLinks(type, g->vertex[i], g); //sum the inlinks or outlinks of the referred nodes
		}
	}
	return total;
}	

//Goes through the pageranks array and finds the next largest value to output
void outputPageRanks(double pageRanks[], Graph g) {
	FILE *fp = fopen("pagerankList.txt", "w"); assert(fp);
	int largest;
	for (int i = 0; i < g->nV; i++) {
		largest = getLargest(pageRanks, g->nV);
		fprintf(fp, "%s, %d, %.7lf\n", g->vertex[largest], (int)getLinks(Out, g->vertex[largest], g), pageRanks[largest]);
		pageRanks[largest] = -1; /*after finding the current largest we should set it to a small value so that we can find the next largest.
								   -1 is valid for this because pageranks cannot be negative by calculation (i can prove this if you like)*/
	}
	fclose(fp);
}