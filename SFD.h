#define MAX_LINE 1024
#define NO_FREE_RANK 1000000
#define TRUE 1
#define FALSE 0
#define FAIL -1

typedef struct _rank *RankNode;
typedef struct _rank {
    double rank_no;
    RankNode next;
    RankNode prev;
}rank;

typedef struct _rankList *RankList;
typedef struct _rankList {
    RankNode head;
    int length;
}ranklist;

typedef struct _URLnode *SFDURLNode;
typedef struct _URLList *SFDURLList;
typedef struct _URLList {
    SFDURLNode head;
    int length;
} SFDurllist;

typedef struct _URLnode {
    char *URL;
    SFDURLNode next;
    RankList ranks;
} SFDurlnode;

void checkMinimum(SFDURLNode *bestRanks, SFDURLNode *workingRanks, double *barSFD, int mover, int listLength);
void swap(SFDURLNode *list, int x, int y);
void checkSFD(SFDURLNode *bestRanks, SFDURLNode *workingRanks, int totalURLs, double *barSFD);
SFDURLNode newSFDURLNode(SFDURLList uList, char *URL, int URLs_total, int URL_rank);
SFDURLList newSFDURLList();
void addRank(RankList ranks, int URLs_total, int URL_rank);
RankList newRankList();
double calculateSFD(SFDURLNode node, int givenRank, int totalURLs);
int barRanking(SFDURLNode bestRanks[], SFDURLList uList);
void FindSFDRank(SFDURLNode bestRanks[], SFDURLList uList, SFDURLNode curr, int chosenRank[], SFDURLNode leftChanges[], SFDURLNode rightChanges[]);
int searchLeft(SFDURLNode ranks[], int listLength, SFDURLNode currNode, int chosenRank, 
    SFDURLNode changes[], double *leftSFDinc);
int searchRight(SFDURLNode ranks[], int listLength, SFDURLNode currNode, int chosenRank, 
    SFDURLNode changes[], double *rightSFDinc, double leftSFDinc);
void freeSFDURLList(SFDURLList l);