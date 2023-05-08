#include "constLib.h"

#define CACHE &cache[0][0], &action[0][0], nO+1, nD+1

#ifdef DEBUG
	#define DEBUG_SET 1
#else
	#define DEBUG_SET 0
#endif

void errorLev(Cstr message) {
	printf(RED_CC"ERROR:"WHITE_CC" %s\n"LWHITE_CC, message);
	if (DEBUG_SET) return;
	exit(1);
}

Cstr actionToCstr(int at);
int min(int x, int y);
int lev(Cstr origin, Cstr destination);
void PrintCache(int *cache, int *action, int col_len, int row_len);

enum ActionType {
	IGNORE = 1, ADD, DELETE, REPLACE
};

int lev(Cstr origin, Cstr destination) {
	int nO = strlen(origin);
	int nD = strlen(destination);
	// nO = (nO + nD) - (nD = nO);
	int cache[nO+1][nD+1];
	int action[nO+1][nD+1];

	if (DEBUG_SET) printf("nO: %d\nnD: %d\n", nO, nD);

	for (int i = 0; i < nO + 1; i++)
		for(int j = 0; j < nD + 1; j++) {
			cache[i][j] = -1;
			action[i][j] = -1;
		}
	PrintCache(CACHE);

	cache[0][0] = 0;
	action[0][0] = IGNORE;
	PrintCache(CACHE);

	for (int i = 1; i < nO + 1; i++) {
		cache[i][0] = i;
		action[i][0] = DELETE;
	}
	PrintCache(CACHE);
	for (int i = 1; i < nD + 1; i++) {
		cache[0][i] = i;
		action[0][i] = ADD;
	}
	PrintCache(CACHE);

	for (int i = 1; i < nO + 1; i++)
		for (int j = 1; j < nD + 1; j++) {	
			if (origin[nO-i] == destination[nD-j]) {
				cache[i][j] = cache[i-1][j-1]; //ignore
				action[i][j] = IGNORE;
				continue;
			}
			
			int insert = cache[i][j-1];	//insert
			int delete = cache[i-1][j];	//delete
			cache[i][j] = cache[i-1][j-1];  //replace
			action[i][j] = REPLACE;

			if (cache[i][j] > delete) {
				cache[i][j] = delete;
				action[i][j] = DELETE;
			}
			if (cache[i][j] > insert) {
				cache[i][j] = insert;
				action[i][j] = ADD;
			}
			cache[i][j]++;
		}
	PrintCache(CACHE);

	return cache[nO][nD];
}
//			nO+1		nD+1
void PrintCache(int *cache, int *action, int col_len, int row_len) {
	if (!DEBUG_SET) return;

	for (int i = 0; i < row_len; i++) {
		for (int j = 0; j < col_len; j++)
			printf(" %2d (%s)", *(cache + j*row_len + i), actionToCstr(*(action + j*row_len + i)));
		printf("\n");
	}
	printf("\n");
}

Cstr actionToCstr(int at) {
	switch(at) {
		case IGNORE: return " ignored ";
		case ADD: return " inserted ";
		case DELETE: return " deleted ";
		case REPLACE: return " replaced ";
		case -1: return " ~ ";
		default: {Cstr errMsg = (Cstr)malloc(35);
			  sprintf(errMsg, "%d is NOT a valid action", at);
			  errorLev(errMsg);
			  free(errMsg); }
	}
	Error("unreachable");
	return "";
}

int min(int x, int y) {
	return (x<y) ? x : y;
}

void pushLev(int levDist[9], int indexRijeci[9], int pushVal, int pushIndex) {
	for (int i = 8; i > 0; i--) {
		levDist[i] = levDist[i-1];
		indexRijeci[i] = indexRijeci[i-1];
	}

	levDist[0] = pushVal;
	indexRijeci[0] = pushIndex;
	return;
}

int indexNajSlicnijeRijeci(int maxBrojRijeci, Cstr input, Cstr *rijeci, int rijeciLen) {

	if (maxBrojRijeci < 1) Error("Maksimalan broj ponudjenih rijeci ne moze biti manji od 1");

	//Najslicnije 4 rijeci
	int levDist[maxBrojRijeci]; 
	levDist[0] = 4;
	int indexRijeci[maxBrojRijeci];
	int countSlicnih = 0;

	int slicnostTrenutne; //Slicnost inputa svakoj od 28 mogucih rijeci
	for (int i = 0; i < rijeciLen; i++) {
		slicnostTrenutne = lev(input, rijeci[i]);
		if (slicnostTrenutne <= levDist[0]) {
			pushLev(levDist, indexRijeci, slicnostTrenutne, i);
			countSlicnih++;
		}
		
	}
	countSlicnih = (countSlicnih > maxBrojRijeci) ? maxBrojRijeci : countSlicnih;

	if (levDist[0] == 0) return indexRijeci[0];

	int odabir = 0;

	if (countSlicnih == 0 ) return -1;
	else if (countSlicnih == 1) odabir = 1;
	else {
		printf("Komanda nije ispravna, jeste li mislili:\n");
		for (int i = 0; i < countSlicnih; i++)
			printf("%s%2d%s)  %s \n", YELW_CC, i+1, LWHITE_CC, rijeci[indexRijeci[i]]);
		scanf("%d", &odabir);
	}

	return indexRijeci[odabir-1];
}
