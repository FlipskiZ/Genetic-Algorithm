#include <iostream>
#include <string>
#include <stdlib.h>
#include <iomanip>
#include <time.h>
#include <sstream>
#include <math.h>

#define RANDOM_NUM  ((double)rand()/(RAND_MAX+1))

using namespace std;

const int amountchromosomes = 32;               //The amount of chromosomes. Should be even number
string chromosome[amountchromosomes];           //9 sections of 4 bits each - each section is a number or operator
double chromosomeFitness[amountchromosomes];    //The fitness of the chromosomes

int generationCount = 0;

double desiredValue;                                //The desired result
const double crossoverRate = 0.7;                   //The chance that the chromosomes will exchange bits
const double mutationRate = 0.01;                   //The chance that a random bit gets flipped (1->0 or 0->1)
const int geneLenght = 9;                           //The amount of genes. Should be an odd number
const int chromosomeLenght = geneLenght*4;          //The total chromosome lenght

void createchromosomes();                                               //Create new chromosomes if needed
void calculateFitness(int chromosomeId);                                //Calculate the fitness score of the chromosome
string decodechromosome(int chromosomeId);                              //Decode the chromosome into the expression
string cleanupchromosome(string decodedchromosome);                     //Cleanup the chromosome expression. Uses mostly the same code as the expression parser, just without adding the numbers together.
string roulettechromosome(double totalFitness);                         //Select chromosomes to go through to the next generation. chromosomes that are not selected are empty
void crossoverchromosomes(string &chromosomeId1, string &chromosomeId2);//Chance for some chromosomes to swap bits with other chromosomes
void mutatechromosome(string &chromosome);                              //Chance to mutate some bits in a chromosome

bool done = false;

int main(){
    srand(time(0));

    double totalFitness = 0;

    cout << "Please input a desired number to solve" << endl;
    cin >> desiredValue;

    createchromosomes();

    while(!done){
        totalFitness = 0;

        for(int i = 0; i < amountchromosomes; i++){
            calculateFitness(i);
            totalFitness += chromosomeFitness[i];
        }

        for(int i = 0; i < amountchromosomes; i++){
            if(chromosomeFitness[i] == 1){
            cout << "\nSolution found in " << generationCount << " generations!" << endl << endl;
            cout << "chromosome: " << chromosome[i] << endl;
            string decoded = decodechromosome(i);
            cout << "Decoded: " << decoded << endl;
            cout << "Cleaned: " << cleanupchromosome(decoded) << endl;

            done = true;
            break;
            }
        }

        //define some temporary storage for the new population we are about to create
        string temp[amountchromosomes];
        int chromosomeCount = 0;
        //loop until we have created POP_SIZE new chromosomes
        while (chromosomeCount < amountchromosomes){
            // we are going to create the new population by grabbing members of the old population
            // two at a time via roulette wheel selection.
            string offspring1 = roulettechromosome(totalFitness);
            string offspring2 = roulettechromosome(totalFitness);

            //add crossover dependent on the crossover rate
            crossoverchromosomes(offspring1, offspring2);

            //now mutate dependent on the mutation rate
            mutatechromosome(offspring1);
            mutatechromosome(offspring2);

            //add these offspring to the new population. (assigning zero as their
            //fitness scores)
            temp[chromosomeCount++] = offspring1;
            temp[chromosomeCount++] = offspring2;

        }//end loop

        //copy temp population into main population array
        for(int i = 0; i < amountchromosomes; i++){
            chromosome[i] = temp[i];
        }

        int bestchromosome = 0;

        /*for(int i = 0; i < amountchromosomes; i++){
            if(chromosomeFitness[i] > chromosomeFitness[bestchromosome]){
                bestchromosome = i;
            }
        }

        cout << "\nBest chromosome so far: " << chromosome[bestchromosome] << endl;
        cout << "With fitness: " << chromosomeFitness[bestchromosome] << endl;*/

        generationCount++;

        //cin.get();
    }
    cout << "Press enter to continue..." << endl;
    cin.get(); cin.get();
}

void createchromosomes(){
    for(int x = 0; x < amountchromosomes; x++){
        if(chromosome[x].empty()){
            for(int y = 0; y < chromosomeLenght; y++){
                chromosome[x].append(rand()%2 ? "1" : "0");
            }
        }
    }
}

void calculateFitness(int chromosomeId){

    string decodedchromosome = decodechromosome(chromosomeId);

    //cout << "Encoded: " << chromosome[chromosomeId] << endl;

    //cout << "Decoded: " << decodedchromosome << endl;

    bool noNumbers = true;

    for(int i = 0; i < decodedchromosome.length(); i++){
        if(isdigit(decodedchromosome[i])){
            noNumbers = false;
        }
    }

    if(noNumbers){
        chromosomeFitness[chromosomeId] = 0;
        return;
    }

    for(int i = 0; i < decodedchromosome.length(); i++){
        if(decodedchromosome[i] == '#'){
            decodedchromosome.erase(i, 1);
            i = -1;
        }
    }

    double a = 0, b = 0;
    int start = 0, secondStart = 0;
    string toNumber = "";
    bool afterOperator = false, multiplication = false, addition = false, divisionByZero = false;

    for(int i = 0; i < decodedchromosome.length()+1 && !divisionByZero; i++){
        if(!afterOperator){
            if(isdigit(decodedchromosome[i]) || (decodedchromosome[i] == '.' && toNumber.length() > 0 && toNumber != "-") || (decodedchromosome[i] == '-' && i == start)){
                toNumber += decodedchromosome[i];
            }else if((decodedchromosome[i] == '*' || decodedchromosome[i] == '/') && (!toNumber.empty() && toNumber != "-")){
                multiplication = decodedchromosome[i] == '*' ? true : false;
                afterOperator = true;
                secondStart = i+1;
                a = stod(toNumber.c_str());
                toNumber = "";
            }else if((decodedchromosome[i] == '*' || decodedchromosome[i] == '/') && (toNumber.empty() || toNumber == "-")){
                decodedchromosome.erase(i, 1);
                i = -1;
                toNumber = "";
            }else{
                start = i+1;
                a = 0;
                b = 0;
                toNumber = "";
            }
        }else{
            if(isdigit(decodedchromosome[i]) || decodedchromosome[i] == '.' || (decodedchromosome[i] == '-' && i == secondStart)){
                toNumber += decodedchromosome[i];
            }else{
                if(toNumber.empty()){
                    decodedchromosome.erase(i-1, 1);
                }else if(toNumber == "-"){
                    decodedchromosome.erase(i-2, 2);
                }else{
                    b = stod(toNumber.c_str());
                    if(!multiplication && b == 0){
                        divisionByZero = true;
                    }
                    decodedchromosome.replace(start, i-start, to_string(multiplication ? a*b : a/b));
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
        //cout << "chromosome tries to divide by zero" << endl;
        chromosomeFitness[chromosomeId] = 0;
    }else{
        start = 0;

        for(int i = 0; i < decodedchromosome.length()+1; i++){
            if(!afterOperator){
                if(isdigit(decodedchromosome[i]) || (decodedchromosome[i] == '.' && toNumber.length() > 0 && toNumber != "-") || (decodedchromosome[i] == '-' && i == start)){
                    toNumber += decodedchromosome[i];
                }else if((decodedchromosome[i] == '+' || decodedchromosome[i] == '-') && !toNumber.empty() && toNumber != "-"){
                    addition = decodedchromosome[i] == '+' ? true : false;
                    afterOperator = true;
                    secondStart = i+1;
                    a = stod(toNumber.c_str());
                    toNumber = "";
                }else if((decodedchromosome[i] == '+' || decodedchromosome[i] == '-') && (toNumber.empty() || toNumber == "-")){
                    decodedchromosome.erase(i, 1);
                    i = -1;
                    toNumber = "";
                }else{
                    start = i+1;
                    a = 0;
                    b = 0;
                    toNumber = "";
                }
            }else{
                if(isdigit(decodedchromosome[i]) || decodedchromosome[i] == '.' || (decodedchromosome[i] == '-' && i == secondStart)){
                    toNumber += decodedchromosome[i];
                }else{
                    if(toNumber.empty()){
                        decodedchromosome.erase(i-1, 1);
                    }else if(toNumber == "-"){
                        decodedchromosome.erase(i-2, 2);
                    }else{
                        b = stod(toNumber.c_str());
                        decodedchromosome.replace(start, i-start, to_string(addition ? a+b : a-b));
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
        //cout << "Result: " << decodedchromosome << endl;
        chromosomeFitness[chromosomeId] = (double)1/(fabs(desiredValue-stod(decodedchromosome))+1);
    }
    //cout << "Fitness: " << chromosomeFitness[chromosomeId] << endl << endl;
}

string decodechromosome(int chromosomeId){
    string decodedchromosome = "";
    for(int i = 0; i < geneLenght; i++){
        string chromosomeSection = chromosome[chromosomeId].substr(i*4, 4);
        if(chromosomeSection == "0000"){
            decodedchromosome += "0";
        }else if(chromosomeSection == "0001"){
            decodedchromosome += "1";
        }else if(chromosomeSection == "0010"){
            decodedchromosome += "2";
        }else if(chromosomeSection == "0011"){
            decodedchromosome += "3";
        }else if(chromosomeSection == "0100"){
            decodedchromosome += "4";
        }else if(chromosomeSection == "0101"){
            decodedchromosome += "5";
        }else if(chromosomeSection == "0110"){
            decodedchromosome += "6";
        }else if(chromosomeSection == "0111"){
            decodedchromosome += "7";
        }else if(chromosomeSection == "1000"){
            decodedchromosome += "8";
        }else if(chromosomeSection == "1001"){
            decodedchromosome += "9";
        }else if(chromosomeSection == "1010"){
            decodedchromosome += "+";
        }else if(chromosomeSection == "1011"){
            decodedchromosome += "-";
        }else if(chromosomeSection == "1100"){
            decodedchromosome += "*";
        }else if(chromosomeSection == "1101"){
            decodedchromosome += "/";
        }else{
            decodedchromosome += "#";
        }
    }
    return decodedchromosome;
}

string cleanupchromosome(string decodedchromosome){

    for(int i = 0; i < decodedchromosome.length(); i++){
        if(decodedchromosome[i] == '#'){
            decodedchromosome.erase(i, 1);
            i = -1;
        }
    }

    int start = 0, secondStart = 0;
    string toNumber = "";
    bool afterOperator = false;

    for(int i = 0; i < decodedchromosome.length()+1; i++){
        if(!afterOperator){
            if(isdigit(decodedchromosome[i]) || (decodedchromosome[i] == '.' && toNumber.length() > 0 && toNumber != "-") || (decodedchromosome[i] == '-' && i == start)){
                toNumber += decodedchromosome[i];
            }else if((decodedchromosome[i] == '*' || decodedchromosome[i] == '/') && (!toNumber.empty() && toNumber != "-")){
                afterOperator = true;
                secondStart = i+1;
                toNumber = "";
            }else if((decodedchromosome[i] == '*' || decodedchromosome[i] == '/') && (toNumber.empty() || toNumber == "-")){
                decodedchromosome.erase(i, 1);
                i = -1;
                toNumber = "";
            }else{
                start = i+1;
                toNumber = "";
            }
        }else{
            if(isdigit(decodedchromosome[i]) || decodedchromosome[i] == '.' || (decodedchromosome[i] == '-' && i == secondStart)){
                toNumber += decodedchromosome[i];
            }else{
                if(toNumber.empty()){
                    decodedchromosome.erase(i-1, 1);
                    i = -1;
                }else if(toNumber == "-"){
                    decodedchromosome.erase(i-2, 2);
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

    for(int i = 0; i < decodedchromosome.length()+1; i++){
        if(!afterOperator){
            if(isdigit(decodedchromosome[i]) || (decodedchromosome[i] == '.' && toNumber.length() > 0 && toNumber != "-") || (decodedchromosome[i] == '-' && i == start)){
                toNumber += decodedchromosome[i];
            }else if((decodedchromosome[i] == '+' || decodedchromosome[i] == '-') && (!toNumber.empty() && toNumber != "-")){
                afterOperator = true;
                secondStart = i+1;
                toNumber = "";
            }else if((decodedchromosome[i] == '+' || decodedchromosome[i] == '-') && (toNumber.empty() || toNumber == "-")){
                decodedchromosome.erase(i, 1);
                i = -1;
                toNumber = "";
            }else{
                start = i+1;
                toNumber = "";
            }
        }else{
            if(isdigit(decodedchromosome[i]) || decodedchromosome[i] == '.' || (decodedchromosome[i] == '-' && i == secondStart)){
                toNumber += decodedchromosome[i];
            }else{
                if(toNumber.empty()){
                    decodedchromosome.erase(i-1, 1);
                    i = -1;
                }else if(toNumber == "-"){
                    decodedchromosome.erase(i-2, 2);
                    i = -1;
                }
                toNumber = "";
                afterOperator = false;
                start = 0;
                secondStart = 0;
            }
        }
    }

    return decodedchromosome;
}

string roulettechromosome(double totalFitness){
	//generate a random number between 0 & total fitness count
	double slice = (double)(RANDOM_NUM * totalFitness);
	//go through the chromosomes adding up the fitness so far
	double fitnessSoFar = 0.0;

	for(int i = 0; i < amountchromosomes; i++){
		fitnessSoFar += chromosomeFitness[i];

		//if the fitness so far > random number return the chromo at this point
		if(fitnessSoFar >= slice)

			return chromosome[i];
	}

	return "";
}

void crossoverchromosomes(string &offspring1, string &offspring2){
      if(RANDOM_NUM < crossoverRate){
        //create a random crossover point
        int crossover = RANDOM_NUM * chromosomeLenght;

        string t1 = offspring1.substr(0, crossover) + offspring2.substr(crossover, chromosomeLenght);
        string t2 = offspring2.substr(0, crossover) + offspring1.substr(crossover, chromosomeLenght);

        offspring1 = t1; offspring2 = t2;
      }
}

void mutatechromosome(string &chromosome){
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

