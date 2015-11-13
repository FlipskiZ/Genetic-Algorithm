#include <iostream>
#include <string>
#include <stdlib.h>
#include <iomanip>
#include <time.h>
#include <sstream>
#include <math.h>
#include <vector>
#include <fstream>

#define RANDOM_NUM  ((double)rand()/(RAND_MAX+1))

using namespace std;

vector<string> chromosome;                      //9 sections of 4 bits each - each section is a number or operator
vector<double> chromosomeFitness;               //The fitness of the chromosomes

int generationCount = 0;
int bestChromosome = 0;

double desiredValue;                            //The desired result
double crossoverRate = 0.7;                     //The chance that the chromosomes will exchange bits
double mutationRate = 0.005;                    //The chance that a random bit gets flipped (1->0 or 0->1)
int geneLenght = 9;                             //The amount of genes. Should be an odd number
int chromosomeLenght;                           //The total chromosome length
int amountChromosomes = 32;                     //The amount of chromosomes. Should be even number
int showGeneration = 1000;                      //The amount of generations to pass before showing the best chromosome

void createChromosomes();                                               //Create new chromosomes if needed
//#pragma optimize("", off)                                               //Fixes a bug in where the compiler optimizes an assignment resulting in displaying the wrong best chromosome
void calculateFitness(int chromosomeId);                                //Calculate the fitness score of the chromosome
string decodeChromosome(int chromosomeId);                              //Decode the chromosome into the expression
string cleanupChromosome(string decodedChromosome);                     //Cleanup the chromosome expression. Uses mostly the same code as the expression parser, just without adding the numbers together.
string calculateAnswer(string decodedChromosome);                       //Calculate the answer of the chromosome
string rouletteChromosome(double totalFitness);                         //Select chromosomes to go through to the next generation. chromosomes that are not selected are empty
void crossoverChromosomes(string &chromosomeId1, string &chromosomeId2);//Chance for some chromosomes to swap bits with other chromosomes
void mutateChromosome(string &chromosome);                              //Chance to mutate some bits in a chromosome

bool done = false;

int main(){
    srand(time(0));

    double totalFitness = 0;

    string temp = "";

    ofstream GAInfo ("GAInfo.txt");

    GAInfo << "Generation" << "|" << "Best Fitness" << "|" << "Average Fitness" << endl;

    cout << "Please input a desired number to solve" << endl;
    getline(cin, temp);
    if(temp.empty()){
        desiredValue = 50;
    }else{
        desiredValue = stod(temp);
    }

    cout << "Please input a crossover rate, enter for the default which is 0.7" << endl;
    getline(cin, temp);
    if(temp.empty()){
        crossoverRate = 0.7;
    }else{
        crossoverRate = stod(temp);
    }

    cout << "Please input a mutation rate, enter for the default which is 0.005" << endl;
    getline(cin, temp);
    if(temp.empty()){
        mutationRate = 0.005;
    }else{
        mutationRate = stod(temp);
    }

    cout << "Please input the gene length, enter for the default which is 9" << endl;
    getline(cin, temp);
    if(temp.empty()){
        geneLenght = 9;
    }else{
        geneLenght = stod(temp);
    }

    cout << "Please input the size of the population, enter for the default which is 32" << endl;
    getline(cin, temp);
    if(temp.empty()){
        amountChromosomes = 32;
    }else{
        amountChromosomes = stod(temp);
    }

    chromosome.resize(amountChromosomes);
    chromosomeFitness.resize(amountChromosomes);

    cout << "Please input every how many generations the best chromosome is shown and written to file\nEnter for the default which is 1000" << endl;
    getline(cin, temp);
    if(temp.empty()){
        showGeneration = 1000;
    }else{
        showGeneration = stod(temp);
    }

    chromosomeLenght = geneLenght*4;

    createChromosomes();

    while(!done){
        totalFitness = 0;

        for(int i = 0; i < amountChromosomes; i++){
            calculateFitness(i);
            totalFitness += chromosomeFitness[i];
        }

        for(int i = 0; i < amountChromosomes; i++){ //Checks if a solution is found
            if(chromosomeFitness[i] == 1){
            cout << "\nSolution found in " << generationCount << " generations!" << endl << endl;
            cout << "Chromosome: " << chromosome[i] << endl;
            string decoded = decodeChromosome(i);
            cout << "Decoded: " << decoded << endl;
            cout << "Cleaned: " << cleanupChromosome(decoded) << endl;
            cout << "Answer: " << calculateAnswer(decoded) << endl;
            cout << "Average Fitness: " << totalFitness/amountChromosomes << endl;

            GAInfo << generationCount << "|" << "1" << "|" << totalFitness/amountChromosomes << endl;

            done = true;
            break;
            }
        }

        if(generationCount % showGeneration == 0){ //Show the generations best chromosone
            bestChromosome = 0;

            for(int i = 0; i < amountChromosomes; i++){
                if(chromosomeFitness[i] > chromosomeFitness[bestChromosome]){
                    bestChromosome = i;
                }
            }
            cout << "\nBest chromosome so far: " << chromosome[bestChromosome] << endl;
            cout << "Decoded: " << cleanupChromosome(decodeChromosome(bestChromosome)) << endl;
            cout << "Answer: " << calculateAnswer(decodeChromosome(bestChromosome)) << endl;
            cout << "With fitness: " << chromosomeFitness[bestChromosome] << endl;
            cout << "Average fitness: " << totalFitness/amountChromosomes << endl;

            if(chromosomeFitness[bestChromosome] != 1){
                GAInfo << generationCount << "|" << chromosomeFitness[bestChromosome] << "|" << totalFitness/amountChromosomes << endl;
            }
        }

        //define some temporary storage for the new population we are about to create
        string temp[amountChromosomes];
        int chromosomeCount = 0;
        //loop until we have created POP_SIZE new chromosomes
        while(chromosomeCount < amountChromosomes){
            // we are going to create the new population by grabbing members of the old population
            // two at a time via roulette wheel selection.
            string offspring1 = rouletteChromosome(totalFitness);
            string offspring2 = rouletteChromosome(totalFitness);

            //add crossover dependent on the crossover rate
            crossoverChromosomes(offspring1, offspring2);

            //now mutate dependent on the mutation rate
            mutateChromosome(offspring1);
            mutateChromosome(offspring2);

            //add these offspring to the new population. (assigning zero as their
            //fitness scores)
            temp[chromosomeCount++] = offspring1;
            temp[chromosomeCount++] = offspring2;

        }//end loop

        //copy temp population into main population array
        for(int i = 0; i < amountChromosomes; i++){
            chromosome[i] = temp[i];
        }

        generationCount++;

        //cin.get();
    }
    GAInfo.close();
    cout << "Press enter to continue..." << endl;
    cin.get();
}

void createChromosomes(){
    for(int x = 0; x < amountChromosomes; x++){
        if(chromosome[x].empty()){
            for(int y = 0; y < chromosomeLenght; y++){
                chromosome[x].append(rand()%2 ? "1" : "0");
            }
        }
    }
}

void calculateFitness(int chromosomeId){

    string decodedChromosome = decodeChromosome(chromosomeId);

    decodedChromosome = calculateAnswer(decodedChromosome);

    if(decodedChromosome == "NaN"){
        chromosomeFitness.at(chromosomeId) = 0;
        return;
    }

    double fitness = stod(decodedChromosome);

    fitness -= (double)desiredValue;

    fitness = (double)fabs(fitness);

    fitness += (double)1;

    fitness = (double)1/fitness;

    chromosomeFitness.at(chromosomeId) = fitness; //Gives a fitness value between 0 and 1

    //cout << "Fitness: " << chromosomeFitness[chromosomeId] << endl << endl;
}

string decodeChromosome(int chromosomeId){
    string decodedChromosome = "";
    for(int i = 0; i < geneLenght; i++){
        string chromosomeSection = chromosome[chromosomeId].substr(i*4, 4);
        if(chromosomeSection == "0000"){
            decodedChromosome += "0";
        }else if(chromosomeSection == "0001"){
            decodedChromosome += "1";
        }else if(chromosomeSection == "0010"){
            decodedChromosome += "2";
        }else if(chromosomeSection == "0011"){
            decodedChromosome += "3";
        }else if(chromosomeSection == "0100"){
            decodedChromosome += "4";
        }else if(chromosomeSection == "0101"){
            decodedChromosome += "5";
        }else if(chromosomeSection == "0110"){
            decodedChromosome += "6";
        }else if(chromosomeSection == "0111"){
            decodedChromosome += "7";
        }else if(chromosomeSection == "1000"){
            decodedChromosome += "8";
        }else if(chromosomeSection == "1001"){
            decodedChromosome += "9";
        }else if(chromosomeSection == "1010"){
            decodedChromosome += "+";
        }else if(chromosomeSection == "1011"){
            decodedChromosome += "-";
        }else if(chromosomeSection == "1100"){
            decodedChromosome += "*";
        }else if(chromosomeSection == "1101"){
            decodedChromosome += "/";
        }else{
            decodedChromosome += "#";
        }
    }
    return decodedChromosome;
}

string cleanupChromosome(string decodedChromosome){

    for(int i = 0; i < decodedChromosome.length(); i++){
        if(decodedChromosome[i] == '#'){
            decodedChromosome.erase(i, 1);
            i = -1;
        }
    }

    int start = 0, secondStart = 0;
    string toNumber = "";
    bool afterOperator = false;

    for(int i = 0; i < decodedChromosome.length()+1; i++){
        if(!afterOperator){
            if(isdigit(decodedChromosome[i]) || (decodedChromosome[i] == '.' && toNumber.length() > 0 && toNumber != "-") || (decodedChromosome[i] == '-' && i == start)){
                toNumber += decodedChromosome[i];
            }else if((decodedChromosome[i] == '*' || decodedChromosome[i] == '/') && (!toNumber.empty() && toNumber != "-")){
                afterOperator = true;
                secondStart = i+1;
                toNumber = "";
            }else if((decodedChromosome[i] == '*' || decodedChromosome[i] == '/') && (toNumber.empty() || toNumber == "-")){
                decodedChromosome.erase(i, 1);
                i = -1;
                toNumber = "";
            }else{
                start = i+1;
                toNumber = "";
            }
        }else{
            if(isdigit(decodedChromosome[i]) || decodedChromosome[i] == '.' || (decodedChromosome[i] == '-' && i == secondStart)){
                toNumber += decodedChromosome[i];
            }else{
                if(toNumber.empty()){
                    decodedChromosome.erase(i-1, 1);
                    i = -1;
                }else if(toNumber == "-"){
                    decodedChromosome.erase(i-2, 2);
                    i = -1;
                }
                toNumber = "";
                afterOperator = false;
                start = 0;
                secondStart = 0;
                //cout << "hiloop1" << endl;
            }
        }
    }

    start = 0;

    for(int i = 0; i < decodedChromosome.length()+1; i++){
        if(!afterOperator){
            if(isdigit(decodedChromosome[i]) || (decodedChromosome[i] == '.' && toNumber.length() > 0 && toNumber != "-") || (decodedChromosome[i] == '-' && i == start)){
                toNumber += decodedChromosome[i];
            }else if((decodedChromosome[i] == '+' || decodedChromosome[i] == '-') && (!toNumber.empty() && toNumber != "-")){
                afterOperator = true;
                secondStart = i+1;
                toNumber = "";
            }else if((decodedChromosome[i] == '+' || decodedChromosome[i] == '-') && (toNumber.empty() || toNumber == "-")){
                decodedChromosome.erase(i, 1);
                i = -1;
                toNumber = "";
            }else{
                start = i+1;
                toNumber = "";
            }
        }else{
            if(isdigit(decodedChromosome[i]) || decodedChromosome[i] == '.' || (decodedChromosome[i] == '-' && i == secondStart)){
                toNumber += decodedChromosome[i];
            }else{
                if(toNumber.empty()){
                    decodedChromosome.erase(i-1, 1);
                    i = -1;
                }else if(toNumber == "-"){
                    decodedChromosome.erase(i-2, 2);
                    i = -1;
                }
                toNumber = "";
                afterOperator = false;
                start = 0;
                secondStart = 0;
            }
        }
    }

    int temp = decodedChromosome.length()-1;

    if(decodedChromosome[temp] == '+' || decodedChromosome[temp] == '-' || decodedChromosome[temp] == '*' || decodedChromosome[temp] == '/'){
        decodedChromosome.erase(temp);
    }

    return decodedChromosome;
}

string calculateAnswer(string decodedChromosome){
    cleanupChromosome(decodedChromosome);

    bool noNumbers = true;

    for(int i = 0; i < decodedChromosome.length(); i++){
        if(isdigit(decodedChromosome[i])){
            noNumbers = false;
        }
    }

    if(noNumbers){
        return "NaN";
    }

    for(int i = 0; i < decodedChromosome.length(); i++){
        if(decodedChromosome[i] == '#'){
            decodedChromosome.erase(i, 1);
            i = -1;
        }
    }

    double a = 0, b = 0;
    int start = 0, secondStart = 0;
    string toNumber = "";
    bool afterOperator = false, multiplication = false, addition = false, divisionByZero = false;

    for(int i = 0; i < decodedChromosome.length()+1 && !divisionByZero; i++){
        if(!afterOperator){
            if(isdigit(decodedChromosome[i]) || (decodedChromosome[i] == '.' && toNumber.length() > 0 && toNumber != "-") || (decodedChromosome[i] == '-' && i == start)){
                toNumber += decodedChromosome[i];
            }else if((decodedChromosome[i] == '*' || decodedChromosome[i] == '/') && (!toNumber.empty() && toNumber != "-")){
                multiplication = decodedChromosome[i] == '*' ? true : false;
                afterOperator = true;
                secondStart = i+1;
                a = stod(toNumber.c_str());
                toNumber = "";
            }else if((decodedChromosome[i] == '*' || decodedChromosome[i] == '/') && (toNumber.empty() || toNumber == "-")){
                decodedChromosome.erase(i, 1);
                i = -1;
                toNumber = "";
            }else{
                start = i+1;
                a = 0;
                b = 0;
                toNumber = "";
            }
        }else{
            if(isdigit(decodedChromosome[i]) || decodedChromosome[i] == '.' || (decodedChromosome[i] == '-' && i == secondStart)){
                toNumber += decodedChromosome[i];
            }else{
                if(toNumber.empty()){
                    decodedChromosome.erase(i-1, 1);
                }else if(toNumber == "-"){
                    decodedChromosome.erase(i-2, 2);
                }else{
                    b = stod(toNumber.c_str());
                    if(!multiplication && b == 0){
                        divisionByZero = true;
                    }
                    decodedChromosome.replace(start, i-start, to_string(multiplication ? a*b : a/b));
                }
                toNumber = "";
                a = 0;
                b = 0;
                multiplication = false;
                afterOperator = false;
                start = 0;
                secondStart = 0;
                i = -1;
            }
        }
    }
    if(divisionByZero){
        return "NaN";
    }else{
        start = 0;

        for(int i = 0; i < decodedChromosome.length()+1; i++){
            if(!afterOperator){
                if(isdigit(decodedChromosome[i]) || (decodedChromosome[i] == '.' && toNumber.length() > 0 && toNumber != "-") || (decodedChromosome[i] == '-' && i == start)){
                    toNumber += decodedChromosome[i];
                }else if((decodedChromosome[i] == '+' || decodedChromosome[i] == '-') && !toNumber.empty() && toNumber != "-"){
                    addition = decodedChromosome[i] == '+' ? true : false;
                    afterOperator = true;
                    secondStart = i+1;
                    a = stod(toNumber.c_str());
                    toNumber = "";
                }else if((decodedChromosome[i] == '+' || decodedChromosome[i] == '-') && (toNumber.empty() || toNumber == "-")){
                    decodedChromosome.erase(i, 1);
                    i = -1;
                    toNumber = "";
                }else{
                    start = i+1;
                    a = 0;
                    b = 0;
                    toNumber = "";
                }
            }else{
                if(isdigit(decodedChromosome[i]) || decodedChromosome[i] == '.' || (decodedChromosome[i] == '-' && i == secondStart)){
                    toNumber += decodedChromosome[i];
                }else{
                    if(toNumber.empty()){
                        decodedChromosome.erase(i-1, 1);
                    }else if(toNumber == "-"){
                        decodedChromosome.erase(i-2, 2);
                    }else{
                        b = stod(toNumber.c_str());
                        decodedChromosome.replace(start, i-start, to_string(addition ? a+b : a-b));
                    }
                    toNumber = "";
                    a = 0;
                    b = 0;
                    addition = false;
                    afterOperator = false;
                    start = 0;
                    secondStart = 0;
                    i = -1;
                }
            }
        }
    }
    return decodedChromosome;
}

string rouletteChromosome(double totalFitness){
	//generate a random number between 0 & total fitness count
	double slice = (double)(RANDOM_NUM * totalFitness);
	//go through the chromosomes adding up the fitness so far
	double fitnessSoFar = 0.0;

	for(int i = 0; i < amountChromosomes; i++){
		fitnessSoFar += chromosomeFitness[i];

		//if the fitness so far > random number return the chromo at this point
		if(fitnessSoFar >= slice)

			return chromosome[i];
	}

	return "";
}

void crossoverChromosomes(string &offspring1, string &offspring2){
      if(RANDOM_NUM < crossoverRate){
        //create a random crossover point
        int crossover = RANDOM_NUM * chromosomeLenght;

        string t1 = offspring1.substr(0, crossover) + offspring2.substr(crossover, chromosomeLenght);
        string t2 = offspring2.substr(0, crossover) + offspring1.substr(crossover, chromosomeLenght);

        offspring1 = t1; offspring2 = t2;
      }
}

void mutateChromosome(string &chromosome){
    for(int i = 0; i < chromosomeLenght; i++){
        if (RANDOM_NUM < mutationRate){
            if(chromosome[i] == '1'){
				chromosome[i] = '0';
			}else{
				chromosome[i] = '1';
			}
		}
	}
    return;
}

