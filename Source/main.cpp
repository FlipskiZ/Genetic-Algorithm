#include <iostream>
#include <string>
#include <stdlib.h>
#include <iomanip>
#include <time.h>
#include <sstream>
#include <math.h>

#define RANDOM_NUM  ((double)rand()/(RAND_MAX+1))

using namespace std;

const int amountChromosones = 32;               //The amount of chromosones. Should be even number
string chromosone[amountChromosones];           //9 sections of 4 bits each - each section is a number or operator
double chromosoneFitness[amountChromosones];    //The fitness of the chromosones

int generationCount = 0;

double desiredValue;                                //The desired result
const double crossoverRate = 0.7;                   //The chance that the chromosones will exchange bits
const double mutationRate = 0.01;                   //The chance that a random bit gets flipped (1->0 or 0->1)
const int geneLenght = 9;                           //The amount of genes. Should be an odd number
const int chromosoneLenght = geneLenght*4;          //The total chromosone lenght

void createChromosones();                                               //Create new chromosones if needed
void calculateFitness(int chromosoneId);                                //Calculate the fitness score of the chromosone
string decodeChromosone(int chromosoneId);                              //Decode the chromosone into the expression
string cleanupChromosone(string decodedChromosone);                     //Cleanup the chromosone expression. Uses mostly the same code as the expression parser, just without adding the numbers together.
string rouletteChromosone(double totalFitness);                         //Select chromosones to go through to the next generation. Chromosones that are not selected are empty
void crossoverChromosones(string &chromosoneId1, string &chromosoneId2);//Chance for some chromosones to swap bits with other chromosones
void mutateChromosone(string &chromosone);                              //Chance to mutate some bits in a chromosone

bool done = false;

int main(){
    srand(time(0));

    double totalFitness = 0;

    cout << "Please input a desired number to solve" << endl;
    cin >> desiredValue;

    createChromosones();

    while(!done){
        totalFitness = 0;

        for(int i = 0; i < amountChromosones; i++){
            calculateFitness(i);
            totalFitness += chromosoneFitness[i];
        }

        for(int i = 0; i < amountChromosones; i++){
            if(chromosoneFitness[i] == 1){
            cout << "\nSolution found in " << generationCount << " generations!" << endl << endl;
            cout << "Chromosone: " << chromosone[i] << endl;
            string decoded = decodeChromosone(i);
            cout << "Decoded: " << decoded << endl;
            cout << "Cleaned: " << cleanupChromosone(decoded) << endl;

            done = true;
            break;
            }
        }

        //define some temporary storage for the new population we are about to create
        string temp[amountChromosones];
        int chromosoneCount = 0;
        //loop until we have created POP_SIZE new chromosomes
        while (chromosoneCount < amountChromosones){
            // we are going to create the new population by grabbing members of the old population
            // two at a time via roulette wheel selection.
            string offspring1 = rouletteChromosone(totalFitness);
            string offspring2 = rouletteChromosone(totalFitness);

            //add crossover dependent on the crossover rate
            crossoverChromosones(offspring1, offspring2);

            //now mutate dependent on the mutation rate
            mutateChromosone(offspring1);
            mutateChromosone(offspring2);

            //add these offspring to the new population. (assigning zero as their
            //fitness scores)
            temp[chromosoneCount++] = offspring1;
            temp[chromosoneCount++] = offspring2;

        }//end loop

        //copy temp population into main population array
        for(int i = 0; i < amountChromosones; i++){
            chromosone[i] = temp[i];
        }

        int bestChromosone = 0;

        /*for(int i = 0; i < amountChromosones; i++){
            if(chromosoneFitness[i] > chromosoneFitness[bestChromosone]){
                bestChromosone = i;
            }
        }

        cout << "\nBest chromosone so far: " << chromosone[bestChromosone] << endl;
        cout << "With fitness: " << chromosoneFitness[bestChromosone] << endl;*/

        generationCount++;

        //cin.get();
    }
    cout << "Press enter to continue..." << endl;
    cin.get(); cin.get();
}

void createChromosones(){
    for(int x = 0; x < amountChromosones; x++){
        if(chromosone[x].empty()){
            for(int y = 0; y < chromosoneLenght; y++){
                chromosone[x].append(rand()%2 ? "1" : "0");
            }
        }
    }
}

void calculateFitness(int chromosoneId){

    string decodedChromosone = decodeChromosone(chromosoneId);

    //cout << "Encoded: " << chromosone[chromosoneId] << endl;

    //cout << "Decoded: " << decodedChromosone << endl;

    bool noNumbers = true;

    for(int i = 0; i < decodedChromosone.length(); i++){
        if(isdigit(decodedChromosone[i])){
            noNumbers = false;
        }
    }

    if(noNumbers){
        chromosoneFitness[chromosoneId] = 0;
        return;
    }

    for(int i = 0; i < decodedChromosone.length(); i++){
        if(decodedChromosone[i] == '#'){
            decodedChromosone.erase(i, 1);
            i = -1;
        }
    }

    double a = 0, b = 0;
    int start = 0, secondStart = 0;
    string toNumber = "";
    bool afterOperator = false, multiplication = false, addition = false, divisionByZero = false;

    for(int i = 0; i < decodedChromosone.length()+1 && !divisionByZero; i++){
        if(!afterOperator){
            if(isdigit(decodedChromosone[i]) || (decodedChromosone[i] == '.' && toNumber.length() > 0 && toNumber != "-") || (decodedChromosone[i] == '-' && i == start)){
                toNumber += decodedChromosone[i];
            }else if((decodedChromosone[i] == '*' || decodedChromosone[i] == '/') && (!toNumber.empty() && toNumber != "-")){
                multiplication = decodedChromosone[i] == '*' ? true : false;
                afterOperator = true;
                secondStart = i+1;
                a = stod(toNumber.c_str());
                toNumber = "";
            }else if((decodedChromosone[i] == '*' || decodedChromosone[i] == '/') && (toNumber.empty() || toNumber == "-")){
                decodedChromosone.erase(i, 1);
                i = -1;
                toNumber = "";
            }else{
                start = i+1;
                a = 0;
                b = 0;
                toNumber = "";
            }
        }else{
            if(isdigit(decodedChromosone[i]) || decodedChromosone[i] == '.' || (decodedChromosone[i] == '-' && i == secondStart)){
                toNumber += decodedChromosone[i];
            }else{
                if(toNumber.empty()){
                    decodedChromosone.erase(i-1, 1);
                }else if(toNumber == "-"){
                    decodedChromosone.erase(i-2, 2);
                }else{
                    b = stod(toNumber.c_str());
                    if(!multiplication && b == 0){
                        divisionByZero = true;
                    }
                    decodedChromosone.replace(start, i-start, to_string(multiplication ? a*b : a/b));
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
        //cout << "Chromosone tries to divide by zero" << endl;
        chromosoneFitness[chromosoneId] = 0;
    }else{
        start = 0;

        for(int i = 0; i < decodedChromosone.length()+1; i++){
            if(!afterOperator){
                if(isdigit(decodedChromosone[i]) || (decodedChromosone[i] == '.' && toNumber.length() > 0 && toNumber != "-") || (decodedChromosone[i] == '-' && i == start)){
                    toNumber += decodedChromosone[i];
                }else if((decodedChromosone[i] == '+' || decodedChromosone[i] == '-') && !toNumber.empty() && toNumber != "-"){
                    addition = decodedChromosone[i] == '+' ? true : false;
                    afterOperator = true;
                    secondStart = i+1;
                    a = stod(toNumber.c_str());
                    toNumber = "";
                }else if((decodedChromosone[i] == '+' || decodedChromosone[i] == '-') && (toNumber.empty() || toNumber == "-")){
                    decodedChromosone.erase(i, 1);
                    i = -1;
                    toNumber = "";
                }else{
                    start = i+1;
                    a = 0;
                    b = 0;
                    toNumber = "";
                }
            }else{
                if(isdigit(decodedChromosone[i]) || decodedChromosone[i] == '.' || (decodedChromosone[i] == '-' && i == secondStart)){
                    toNumber += decodedChromosone[i];
                }else{
                    if(toNumber.empty()){
                        decodedChromosone.erase(i-1, 1);
                    }else if(toNumber == "-"){
                        decodedChromosone.erase(i-2, 2);
                    }else{
                        b = stod(toNumber.c_str());
                        decodedChromosone.replace(start, i-start, to_string(addition ? a+b : a-b));
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
        //cout << "Result: " << decodedChromosone << endl;
        chromosoneFitness[chromosoneId] = (double)1/(fabs(desiredValue-stod(decodedChromosone))+1);
    }
    //cout << "Fitness: " << chromosoneFitness[chromosoneId] << endl << endl;
}

string decodeChromosone(int chromosoneId){
    string decodedChromosone = "";
    for(int i = 0; i < geneLenght; i++){
        string chromosoneSection = chromosone[chromosoneId].substr(i*4, 4);
        if(chromosoneSection == "0000"){
            decodedChromosone += "0";
        }else if(chromosoneSection == "0001"){
            decodedChromosone += "1";
        }else if(chromosoneSection == "0010"){
            decodedChromosone += "2";
        }else if(chromosoneSection == "0011"){
            decodedChromosone += "3";
        }else if(chromosoneSection == "0100"){
            decodedChromosone += "4";
        }else if(chromosoneSection == "0101"){
            decodedChromosone += "5";
        }else if(chromosoneSection == "0110"){
            decodedChromosone += "6";
        }else if(chromosoneSection == "0111"){
            decodedChromosone += "7";
        }else if(chromosoneSection == "1000"){
            decodedChromosone += "8";
        }else if(chromosoneSection == "1001"){
            decodedChromosone += "9";
        }else if(chromosoneSection == "1010"){
            decodedChromosone += "+";
        }else if(chromosoneSection == "1011"){
            decodedChromosone += "-";
        }else if(chromosoneSection == "1100"){
            decodedChromosone += "*";
        }else if(chromosoneSection == "1101"){
            decodedChromosone += "/";
        }else{
            decodedChromosone += "#";
        }
    }
    return decodedChromosone;
}

string cleanupChromosone(string decodedChromosone){

    for(int i = 0; i < decodedChromosone.length(); i++){
        if(decodedChromosone[i] == '#'){
            decodedChromosone.erase(i, 1);
            i = -1;
        }
    }

    int start = 0, secondStart = 0;
    string toNumber = "";
    bool afterOperator = false;

    for(int i = 0; i < decodedChromosone.length()+1; i++){
        if(!afterOperator){
            if(isdigit(decodedChromosone[i]) || (decodedChromosone[i] == '.' && toNumber.length() > 0 && toNumber != "-") || (decodedChromosone[i] == '-' && i == start)){
                toNumber += decodedChromosone[i];
            }else if((decodedChromosone[i] == '*' || decodedChromosone[i] == '/') && (!toNumber.empty() && toNumber != "-")){
                afterOperator = true;
                secondStart = i+1;
                toNumber = "";
            }else if((decodedChromosone[i] == '*' || decodedChromosone[i] == '/') && (toNumber.empty() || toNumber == "-")){
                decodedChromosone.erase(i, 1);
                i = -1;
                toNumber = "";
            }else{
                start = i+1;
                toNumber = "";
            }
        }else{
            if(isdigit(decodedChromosone[i]) || decodedChromosone[i] == '.' || (decodedChromosone[i] == '-' && i == secondStart)){
                toNumber += decodedChromosone[i];
            }else{
                if(toNumber.empty()){
                    decodedChromosone.erase(i-1, 1);
                    i = -1;
                }else if(toNumber == "-"){
                    decodedChromosone.erase(i-2, 2);
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

    for(int i = 0; i < decodedChromosone.length()+1; i++){
        if(!afterOperator){
            if(isdigit(decodedChromosone[i]) || (decodedChromosone[i] == '.' && toNumber.length() > 0 && toNumber != "-") || (decodedChromosone[i] == '-' && i == start)){
                toNumber += decodedChromosone[i];
            }else if((decodedChromosone[i] == '+' || decodedChromosone[i] == '-') && (!toNumber.empty() && toNumber != "-")){
                afterOperator = true;
                secondStart = i+1;
                toNumber = "";
            }else if((decodedChromosone[i] == '+' || decodedChromosone[i] == '-') && (toNumber.empty() || toNumber == "-")){
                decodedChromosone.erase(i, 1);
                i = -1;
                toNumber = "";
            }else{
                start = i+1;
                toNumber = "";
            }
        }else{
            if(isdigit(decodedChromosone[i]) || decodedChromosone[i] == '.' || (decodedChromosone[i] == '-' && i == secondStart)){
                toNumber += decodedChromosone[i];
            }else{
                if(toNumber.empty()){
                    decodedChromosone.erase(i-1, 1);
                    i = -1;
                }else if(toNumber == "-"){
                    decodedChromosone.erase(i-2, 2);
                    i = -1;
                }
                toNumber = "";
                afterOperator = false;
                start = 0;
                secondStart = 0;
            }
        }
    }

    return decodedChromosone;
}

string rouletteChromosone(double totalFitness){
	//generate a random number between 0 & total fitness count
	double slice = (double)(RANDOM_NUM * totalFitness);
	//go through the chromosones adding up the fitness so far
	double fitnessSoFar = 0.0;

	for(int i = 0; i < amountChromosones; i++){
		fitnessSoFar += chromosoneFitness[i];

		//if the fitness so far > random number return the chromo at this point
		if(fitnessSoFar >= slice)

			return chromosone[i];
	}

	return "";
}

void crossoverChromosones(string &offspring1, string &offspring2){
      if(RANDOM_NUM < crossoverRate){
        //create a random crossover point
        int crossover = RANDOM_NUM * chromosoneLenght;

        string t1 = offspring1.substr(0, crossover) + offspring2.substr(crossover, chromosoneLenght);
        string t2 = offspring2.substr(0, crossover) + offspring1.substr(crossover, chromosoneLenght);

        offspring1 = t1; offspring2 = t2;
      }
}

void mutateChromosone(string &chromosone){
    for(int i = 0; i < chromosoneLenght; i++){
        if (RANDOM_NUM < mutationRate){
            if(chromosone[i] == '1'){
				chromosone[i] = '0';
			}else{
				chromosone[i] = '1';
			}
		}
	}
    return;
}

