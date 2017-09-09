//#include <bitset>
#include <random>
#include <algorithm>
#include <iostream> //FOR DEBUG

typedef long long ll;
typedef unsigned int uint;
typedef unsigned long long ull;
//typedef bitset<8> byte;

#define LOOPS_LIMIT -1 // USED TO LIMIT NUMBERS OF LOOP RUNNING GA, -1 = INFINITY, INT = # of LOOPS limit
#define GENE_LENGTH 81 // LENGTH OF STRING OF SUDOKU
#define MINVAL 1 //MIN VAL FOR SUDOKU
#define MAXVAL 9 //MAX VAL FOR SUDOKU
#define POPULATION_SIZE 100
#define MUTATION_RATE 0.1
#define CROSSOVER_RATE 0.1
#define REMAINING_INDIVIDUALS_SIZE 30
#define CROSSOVER_INDIVIDUALS_SIZE 20
#define MUTATED_INDIVIDUALS_SIZE 30
#define FILLED_INDIVIDUALS_SIZE 20
#define LOCAL_STRUCK_TIMES 10000
#define OLD_GEN_AFTER_RE_NUMS 0

#define SUDOKU_ROW 9
#define SUDOKU_COL 9
#define DUPLICATED_ROW_COST 10
#define DUPLICATED_COL_COST 10
#define DUPLICATED_TABLE_COST 100
#define UNSET_COST 1
#define ILLEGAL_COST 10000

struct chromosome {
	//WEIGHT[i] / SUM(WEIGHT[i])i=0...N
	int gene[GENE_LENGTH], fitness, preferWeight;

	//DECOMPOSE POSITION
	int getRow(int idx) {
		return idx / 9;
	}

	int getCol(int idx) {
		return idx % 9;
	}

	int getTable(int idx) {
		int r = getRow(idx);
		int c = getCol(idx);
		return 3 * (r / 3) + c / 3;
	}

	//CHECKING FUNCTION
	int checkRow(int idx) {
		int r = getRow(idx);
		int c = getCol(idx);
		int ret = 0;
		for (int i = 0; i < SUDOKU_COL; i++) {
			if (c == i) continue;
			ret += DUPLICATED_ROW_COST*(gene[idx] == gene[SUDOKU_COL*r + i]);
		}
		return ret;
	}

	int checkCol(int idx) {
		int r = getRow(idx);
		int c = getCol(idx);
		int ret = 0;
		for (int i = 0; i < SUDOKU_ROW; i++) {
			if (r == i) continue;
			ret += DUPLICATED_COL_COST*(gene[idx] == gene[SUDOKU_COL*i + c]);
		}
		return ret;
	}

	int checkTable(int idx) {
		int t = getTable(idx);
		int iniIdx = 27 * (t / 3) + 3 * (t % 3); //UPPER-LEFT CORNER OF EVERY TABLE FOR 9*9
		int r = getRow(iniIdx);
		int c = getCol(iniIdx);
		int ret = 0,i,j;
		for (i = r; i < r + 3; i++) {
			for (int j = c; j < c + 3; j++) {
				int cur = SUDOKU_COL*i + j;
				if (cur == idx) continue;
				ret += DUPLICATED_TABLE_COST*(gene[idx] == gene[cur]);
			}
		}
		return ret;
	}

	int checkUnset(int idx) {
		if (gene[idx] == 0) return UNSET_COST;
		return 0;
	}

	int checkIllegal(int idx) {
		if (prototype.gene[idx] == 0) return 0;
		if (prototype.gene[idx] != gene[idx]) return ILLEGAL_COST;
		else return 0;
	}

	//MAY CHANGE THIS FUNC TO RETURN INT LATERS.
	void fitnessCalc() {
		fitness = 0;
		for (int i = 0; i < GENE_LENGTH; i++) {
			fitness += checkRow(i);
			fitness += checkCol(i);
			fitness += checkTable(i);
			fitness += checkUnset(i);
			fitness += checkIllegal(i);//check for invalid answer from the starting point (change constant value given by the task)
		}
	}

	void clear() {
		memset(gene, 0, sizeof(gene));
		fitness = 0;
		preferWeight = 0;
	}

	void randomize() {
		std::random_device gen;
		std::uniform_int_distribution<int> distribution(MINVAL, MAXVAL);
		for (int i = 0; i < GENE_LENGTH; i++) {
			int rndval = distribution(gen);

			gene[i] = rndval;
		}
	}

	void SudokuRandomize() {
		std::random_device gen;
		std::uniform_int_distribution<int> distribution(MINVAL,MAXVAL);
		for (int i = 0; i < GENE_LENGTH; i++) {
			if (prototype.gene[i]) gene[i] = prototype.gene[i];
			else {
				int rndval = distribution(gen);
				
				gene[i] = rndval;
			}
		}
	}

	bool operator <(const chromosome &ot) {
		return fitness < ot.fitness;
	}



}population[POPULATION_SIZE],prototype;

void setPrototype(int pro[]) {
	for (int i = 0; i < GENE_LENGTH; i++)
		prototype.gene[i] = pro[i];
}

void onHold();
void initialize();
void reinitialize(int remainingNum);//reinitialize with first remainingNum-th from old generation 
void execute();
void sortToFitness();
void clearNotFitIndividuals();
int chooseIndividual();
void createCrossOverIndividuals();
void mutateIndividuals();
void generateNewIndividuals();
void calculateFitness();
chromosome crossOver(chromosome &A, chromosome &B);
chromosome mutate(chromosome &A);
void printChromosome(int idx);

std::random_device rnd;
int curPopSize = 0;

/*
int main() {
	initialize();

	return 0;
}
*/

void onHold() {
	int cmd;
	scanf_s("%d", &cmd);
}

void initialize() {
	std::random_device gen;
	//std::default_random_engine gen;
	std::uniform_int_distribution<int> distribution(MINVAL, MAXVAL);

	for (int i = 0; i < POPULATION_SIZE; i++) {
		/*
		for (int j = 0; j < GENE_LENGTH; j++) {
			int rndval = distribution(gen);
			population[i].gene[j] = rndval;
			//printf("%d\n", rndval);
		}*/
		population[i].SudokuRandomize();
	}



	//for (int i = 0; i < 1000000000; i++);
}

void reinitialize(int remainingNum) {
	std::random_device gen;
	std::uniform_int_distribution<int> distribution(MINVAL, MAXVAL);

	for (int i = remainingNum; i < POPULATION_SIZE; i++) {
		population[i].SudokuRandomize();
	}



	//for (int i = 0; i < 1000000000; i++);
}


void execute() {
	int loopTimes = 0, curans=-1,loopStill=0,bestAns=1e9;
	while (loopTimes < LOOPS_LIMIT || LOOPS_LIMIT < 0) { // TERMINATION CONDITION NOT MET , LOOPS_LIMIT < 0 is INFINITY
		
		int rem = REMAINING_INDIVIDUALS_SIZE;
		loopStill++;
		if (loopStill > LOCAL_STRUCK_TIMES) {//IF GOT STRUCK ON LOCAL > LOCAL STRUCK TIMES -> RENEW ALL
			printf("RE !\n");
			reinitialize(OLD_GEN_AFTER_RE_NUMS);
			loopStill = 0;
			continue;
		}
		calculateFitness();

		if (curans != population[0].fitness) {
			curans = population[0].fitness;
			bestAns = std::min(curans, bestAns);
			printChromosome(0);
			printf("Best Ans : %d\n", bestAns);
			//printChromosome(1);
			//onHold();
		}
		if (population[0].fitness == 0) break;
		loopTimes++;

		sortToFitness();
		clearNotFitIndividuals();
		createCrossOverIndividuals();
		mutateIndividuals();
		generateNewIndividuals(); // RANDOMLY CREATE NEW SET OF INDIVIDUALS TO FILL THE REMAINING SLOTS
	}
	onHold();
}

void sortToFitness() {
	std::sort(population, population + POPULATION_SIZE);
}

void clearNotFitIndividuals() {
	curPopSize = REMAINING_INDIVIDUALS_SIZE;
	for (int i = REMAINING_INDIVIDUALS_SIZE; i < POPULATION_SIZE; i++) {
		population[i].clear();
	}
}

int chooseIndividual() {
	std::random_device gen;
	std::uniform_int_distribution<int> genePosDist(0, curPopSize);
	return genePosDist(gen);
}

void createCrossOverIndividuals() {
	for (int i = 0; i < CROSSOVER_INDIVIDUALS_SIZE; i++) {
		int a, b;
		/*int c,a, b;
		a = i, b = i + 1;
		if (chooseIndividual() % 2) {//random swap
			c = a;
			a = b;
			b = c;
		}*/
		a = chooseIndividual();
		//b = chooseIndividual(); //DUPLICATED ELEMENT?
		do {
			b = chooseIndividual();
		} while (a == b);
		population[curPopSize] = crossOver(population[a], population[b]);
		//population[curPopSize] = crossOver(population[a], population[b]);//MUTATE FIRST k-th PAIR
		curPopSize++;
	}
	//curPopSize += CROSSOVER_INDIVIDUALS_SIZE;
}

void mutateIndividuals() {
	for (int i = 0; i < MUTATED_INDIVIDUALS_SIZE; i++) {
		int a = chooseIndividual();
		population[curPopSize] = mutate(population[a]);
		curPopSize++;
	}
	//curPopSize += MUTATED_INDIVIDUALS_SIZE;
}

void generateNewIndividuals() {
	for (int i = 0; i < FILLED_INDIVIDUALS_SIZE; i++) {
		population[curPopSize].SudokuRandomize();
		curPopSize++;
	}
	//curPopSize += FILLED_INDIVIDUALS_SIZE;
}

void calculateFitness() {
	for (int i = 0; i < POPULATION_SIZE; i++) {
		population[i].fitnessCalc();
	}
}

chromosome crossOver(chromosome &A, chromosome &B) {
	std::random_device gen;
	std::uniform_int_distribution<int> distribution(0, GENE_LENGTH - 1);
	int rndval = distribution(gen);

	chromosome ret;

	for (int i = 0; i < GENE_LENGTH; i++) {
		//std::swap(population[idxA].gene[i], population[idxB].gene[i]);
		if (i <= rndval)
			ret.gene[i] = A.gene[i];
		else
			ret.gene[i] = B.gene[i];
	}
	return ret;
}

chromosome mutate_old(chromosome &A) {
	//int posMutate = rnd(0, GENE_LENGTH - 1);
	//int MutatedVal = rnd(MINVAL, MAXVAL);
	//population[idx].gene[posMutate] = MutatedVal;
	std::random_device gen;
	std::uniform_int_distribution<int> genePosDist(0, GENE_LENGTH - 1);
	std::uniform_int_distribution<int> geneValDist(MINVAL, MAXVAL);
	int posMutate = genePosDist(gen);
	int mutatedVal = geneValDist(gen);
	chromosome ret = A;
	ret.gene[posMutate] = mutatedVal;
	return ret;
}

chromosome mutate(chromosome &A) {
	//int posMutate = rnd(0, GENE_LENGTH - 1);
	//int MutatedVal = rnd(MINVAL, MAXVAL);
	//population[idx].gene[posMutate] = MutatedVal;
	std::random_device gen;
	std::uniform_int_distribution<int> genePosDist(0, GENE_LENGTH - 1);
	std::uniform_int_distribution<int> geneValDist(MINVAL, MAXVAL);
	int posMutate = genePosDist(gen);
	while (prototype.gene[posMutate] != 0) posMutate = genePosDist(gen); //KEEP RANDOM UNTIL FOUND UNDEFINED SLOT
	int mutatedVal = geneValDist(gen);
	chromosome ret = A;
	ret.gene[posMutate] = mutatedVal;
	return ret;
}

void printChromosome(int idx) {
	printf("CHROMOSOME %d : \n", idx);

	for (int i = 0; i < SUDOKU_ROW; i++) {
		for (int j = 0; j < SUDOKU_COL; j++) {
			printf("%d ", population[idx].gene[i*SUDOKU_COL + j]);
		}
		printf("\n");
	}
	printf("with fitness : %d\n\n", population[idx].fitness);
}