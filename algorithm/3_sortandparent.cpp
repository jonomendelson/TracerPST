#include <stdio.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <climits>
#include <unordered_set>

int SPLIT_DISP = 100;
int FRAMES_BACK = 25;
int MIN_LIFETIME = 30;
int FINAL_FRAME = 999;

int min_conf = 2;
int nextCellId = 500; 
int assign[2000] = { 0 }; 
int parentAssignments[2000] = { 0 };

std::unordered_set<int> blacklist;
std::vector<std::pair<int, int> > deadTimings;

class Cell{
	public:
		int ObjectNum;
		double X;
		double Y;
		int Area;
		int ParentId;
		double Speed;
		double Intensity;

		double SplitConf;
		bool Pregnant;
		Cell(int, double, double, int, double);
		Cell();
};

Cell::Cell(int o, double nx, double ny, int a, double ii){
	ObjectNum = o;
	X = nx;
	Y = ny;
	Area = a;
	Pregnant = false;
	ParentId = 0;
	Speed = 0;
	Intensity = ii;
}

Cell::Cell(){
	Pregnant = false;
	ParentId = 0;
	Speed = 0;
}

class CPL{
	public:
		int cellId;
		int parentId;
		int lifetime;
		int birthtime;
		CPL(int, int, int, int);
};

CPL::CPL(int c, int p, int l, int b){
	cellId = c;
	parentId = p;
	lifetime = l;
	birthtime = b;
}

std::vector<std::vector<Cell> > data, xytData;
std::vector<CPL> cplData;

class Guess{
	public: 
		int ObjectNum;
		int PosDisp;
		int AreaDisp;
		int CellIndex;
		int FramesBack;
		double nA; //exponent for area
		double nP; //exponent for position

		double Confidence;
		Guess(int, int, int, int, int);
		Guess(int, int, double);
};

Guess::Guess(int c, int o, int p, int a, int f){
	CellIndex = c;
	ObjectNum = o;
	AreaDisp = a;
	PosDisp = p;
	FramesBack = f;

	nP = 2;
	nA = 0;

	Confidence =(10000.0)/(f*pow((PosDisp+10.0), nP));
}

Guess::Guess(int c, int o, double conf){
	CellIndex = c;
	ObjectNum = o;
	Confidence = conf;
	AreaDisp = 0;
	PosDisp = 0;
	FramesBack = 0;
}

bool PosComp(Guess const & guessOne, Guess const & guessTwo){
	return guessOne.PosDisp < guessTwo.PosDisp;
}

bool AreaComp(Guess const & guessOne, Guess const & guessTwo){
	return guessOne.AreaDisp < guessTwo.AreaDisp;
}

bool ConfComp(Guess const & guessOne, Guess const & guessTwo){
	return guessOne.Confidence > guessTwo.Confidence;
}

void readData(){
	std::string line;
  	std::ifstream myfile ("blobData.csv");
  	if (myfile.is_open()){
		int Areax;
		double X, Y, Intensity;
		int frame = 1;
        	std::vector<Cell> currFrame;
    		while (getline(myfile,line)){
			int dataIndex = 0;
			std::stringstream strstr(line);
        		std::string word = "";
			bool lastLineOfFrame = false;
			while (getline(strstr,word, ',')){
				if(dataIndex == 0){
					if(stoi(word) != frame){
						lastLineOfFrame = true;
						frame = stoi(word);
					}
				}
				if(dataIndex == 1) X = stod(word);
				if(dataIndex == 2) Y = stod(word);
				if(dataIndex == 3) Areax = stoi(word);
				if(dataIndex == 4) Intensity = stod(word);
				dataIndex++;
			}
			if(lastLineOfFrame){
				data.push_back(currFrame);
				currFrame.clear();
			}
			currFrame.push_back(Cell(-1, X, Y, Areax, Intensity));
		}
    	myfile.close();
  	}
}

void readDeadTimings(){
	std::string line;
  	std::ifstream myfile ("deadTimings.txt");
  	if (myfile.is_open()){
		int ObjectNum, DeadFrame;
    		while (getline(myfile,line)){
			int dataIndex = 0;
			std::stringstream strstr(line);
        		std::string word = "";
			while (getline(strstr,word, ' ')){
				if(dataIndex == 0) ObjectNum = stoi(word);
				if(dataIndex == 2){
					if(word == "Never"){
						DeadFrame = FINAL_FRAME;
					}else{
						DeadFrame = -1;
					}
				}
				if(dataIndex == 3){
					if(DeadFrame == -1){
						DeadFrame = std::min(stoi(word), FINAL_FRAME);
					}
				}
				dataIndex++;
			}
			deadTimings.push_back(std::make_pair(ObjectNum, DeadFrame));
		}
    	myfile.close();
  	}
}

int cleanData(){
	int cellCount[2000] = { 0 };
	for(int i = 0; i < 2000; i++){
		assign[i] = i;
		cellCount[i] = 0;
		if(parentAssignments[i] != -1){
			cellCount[i] = MIN_LIFETIME;
		}
	}

	for(int frame = 0; frame < xytData.size(); frame++){
		for(int cell = 0; cell < xytData[frame].size(); cell++){
			cellCount[xytData[frame][cell].ObjectNum]++;
		}
	}
	int maxCellId = 0;
	for(int i = 0; i < 2000; i++){
		if(cellCount[i] < MIN_LIFETIME){
			
			assign[i] = -1;
			for(int m = i+1; m < 2000; m++){
				assign[m]--;
			}
		}else{
			maxCellId = std::max(maxCellId, i);
		}
	}
	for(int frame = 0; frame < xytData.size(); frame++){
		for(int cell = 0; cell < xytData[frame].size(); cell++){
			xytData[frame][cell].ObjectNum = assign[xytData[frame][cell].ObjectNum];
		}
	}
	return assign[maxCellId];
}

double distance(Cell cellOne, Cell cellTwo){
	return sqrt(pow((cellOne.X - cellTwo.X), 2) + pow((cellOne.Y - cellTwo.Y), 2));
}

int reverseAssign(int newObj){
	for(int i = 0; i < 2000; i++){
		if(assign[i] == newObj){
			return i;
		}
	}
}

void log(std::string text){
	std::ofstream outfile;
	outfile.open("log.txt", std::ios_base::app);
	outfile << text	<< "\n";
	outfile.close();
}

int indexOfObj(int ObjectNum, int frame){
	for(int cell = 0; cell < xytData[frame].size(); cell++){
		if(xytData[frame][cell].ObjectNum == ObjectNum){
			return cell;
		}
	}
	return -1;
}


void outputTrainingData(){
	std::ofstream myfile("deathTraining.csv");
	myfile << "Id,Frame,Speed,Intensity,Area,Alive\n";
	if (myfile.is_open()){
		for(int obj = 0; obj < deadTimings.size(); obj++){
			for(int frame = deadTimings[obj].second; frame < FINAL_FRAME; frame++){
				if(indexOfObj(deadTimings[obj].first, frame) != -1){ //this cell exists in this frame
					Cell currCell = xytData[frame][indexOfObj(deadTimings[obj].first, frame)];
					myfile << deadTimings[obj].first << "," << frame << "," << currCell.Speed << "," << currCell.Intensity << "," << currCell.Area << "," << "0\n";
				}
			}
		}
		for(int obj = 0; obj < deadTimings.size(); obj++){
			for(int frame = 0; frame < deadTimings[obj].second; frame++){
				if(indexOfObj(deadTimings[obj].first, frame) != -1){ //this cell exists in this frame
					Cell currCell = xytData[frame][indexOfObj(deadTimings[obj].first, frame)];
					myfile << deadTimings[obj].first << "," << frame << "," << currCell.Speed << "," << currCell.Intensity << "," << currCell.Area << "," << "1\n";
				}
			}
		}

	}
	std::cout << "Successfully wrote to deathTraining.csv\n";
}


void outputCSV(){
	std::ofstream myfile("cpl.csv");
	if (myfile.is_open()){
		myfile << "CellId,ParentId,Lifetime,Birthtime\n";
		for(int obj = 0; obj < cplData.size(); obj++){
			myfile << cplData[obj].cellId << "," << cplData[obj].parentId << "," << cplData[obj].lifetime << "," << cplData[obj].birthtime << "\n";
		}
	}
	std::cout << "Successfully wrote to cpl.csv\n";
}

void outputJS(){
	std::ofstream myfile("output.js");
	if (myfile.is_open()){
		myfile << "var data=[";
		for(int frame = 0; frame < xytData.size(); frame++){
			myfile << "[";

			for(int cell = 0; cell < xytData[frame].size(); cell++){
				myfile << "[" << xytData[frame][cell].ObjectNum << "," << xytData[frame][cell].X << "," << xytData[frame][cell].Y << "," << xytData[frame][cell].ParentId << "," << xytData[frame][cell].SplitConf << "," << xytData[frame][cell].Speed << "," << xytData[frame][cell].Intensity << "]";
				if(cell != xytData[frame].size()-1){
					myfile << ",";
				}
			}

			if(frame == xytData.size()-1){
				myfile << "]";
			}else{
				myfile << "],";
			}
		}
		myfile << "]; var tree = [";
		for(int obj = 0; obj < cplData.size(); obj++){
			myfile << "[" << cplData[obj].cellId << "," << cplData[obj].parentId << "," << cplData[obj].lifetime << "," << cplData[obj].birthtime;
			if(obj == cplData.size()-1){
				myfile << "]";
			}else{
				myfile << "],";
			}
		}
		myfile << "];";
	}
	std::cout << "Successfully wrote to output.js\n";
	
}

int main(){
	readData();

	for(int i = 0; i < 2000; i++){
		parentAssignments[i] = -1;
	}
	
	xytData = data;
	xytData.erase(xytData.begin()+FINAL_FRAME, xytData.end());

	for(int frame = 0; frame < xytData.size(); frame++){
		std::cout << std::to_string(frame) << "\n";
		for(int cell = 0; cell < xytData[frame].size(); cell++){
			xytData[frame][cell].ObjectNum = -1;
		}

		std::vector<Guess> guessQueue;
		for(int cell = 0; cell < xytData[frame].size(); cell++){
			for(int framesBack = 1; framesBack <= std::min(FRAMES_BACK, frame); framesBack++){
				for(int oldCell = 0; oldCell < xytData[frame-framesBack].size(); oldCell++){
					double currDistance = distance(xytData[frame][cell], xytData[frame-framesBack][oldCell]);
					double currArea = abs(xytData[frame][cell].Area - xytData[frame-framesBack][oldCell].Area);
					guessQueue.push_back(Guess(cell, xytData[frame-framesBack][oldCell].ObjectNum, currDistance, currArea, framesBack));
				}
			}
		}
		std::sort(guessQueue.begin(), guessQueue.end(), ConfComp);

		if(frame==965){
			std::ofstream myfile("guessQueue.txt");
			for(int i = 0; i < guessQueue.size(); i++){
				myfile << guessQueue[i].CellIndex << " (random index) could be " <<  guessQueue[i].ObjectNum << " with conf " << guessQueue[i].Confidence << "\n";
			
			}
		}


		std::unordered_set<int> usedNumbers;
		for(int guess = 0; guess < guessQueue.size(); guess++){
			if(usedNumbers.find(guessQueue[guess].ObjectNum) == usedNumbers.end()){ //unused object num!
				if(xytData[frame][guessQueue[guess].CellIndex].ObjectNum == -1){
					if(guessQueue[guess].Confidence > min_conf){
						if(blacklist.find(guessQueue[guess].ObjectNum) == blacklist.end()){ 
							xytData[frame][guessQueue[guess].CellIndex].ObjectNum = guessQueue[guess].ObjectNum;
							usedNumbers.insert(guessQueue[guess].ObjectNum);
						}
					}
				}
			}
		}

		for(int cell = 0; cell < xytData[frame].size(); cell++){
			xytData[frame][cell].SplitConf = xytData[frame][cell].Area;
		}

		for(int cell = 0; cell < xytData[frame].size(); cell++){
			int oldIndex = indexOfObj(xytData[frame][cell].ObjectNum, frame-std::min(frame, 1));
			if(frame == 0){oldIndex = -1;}
			if(oldIndex != -1){ //cell existed in past frame, continue
				if((xytData[frame-1][oldIndex].Area - xytData[frame][cell].Area) > SPLIT_DISP){ //sharp dip in area generally means split
					bool boundary_error = false;
					if(!boundary_error){
						//FIRST: find closest non-assigned cell within radius
						double min_distance = 50; //TODO: add nice limit
						int min_index = -1;

						for(int cellTwo = 0; cellTwo < xytData[frame].size(); cellTwo++){
							if(cellTwo != cell){
								if(distance(xytData[frame][cellTwo], xytData[frame][cell]) < min_distance){
									if(xytData[frame][cellTwo].ObjectNum == -1){ //do we want the closest cell to the split or the closest non-assigned cell? (right now is closest unassigned cell)
										min_distance = distance(xytData[frame][cellTwo], xytData[frame][cell]);
										min_index = cellTwo;
									}
								}
							}
						}

						if(min_index != -1){ //we found a valid daughter cell
							xytData[frame-1][oldIndex].Pregnant = true;

							int parentId = xytData[frame][cell].ObjectNum; //this is the same as xytData[frame-1][oldIndex].ObjectNum
							blacklist.insert(parentId); // like a death of the splitting cell - it can never reappear
							
							xytData[frame][cell].ObjectNum = nextCellId;
							parentAssignments[nextCellId] = parentId;
							nextCellId++;

							xytData[frame][min_index].ObjectNum = nextCellId;
							parentAssignments[nextCellId] = parentId;
							nextCellId++;
						}
					}
				} 
			}
		}




		for(int cell = 0; cell < xytData[frame].size(); cell++){
			if(xytData[frame][cell].ObjectNum == -1){
				xytData[frame][cell].ObjectNum = nextCellId;
				nextCellId++;
			}
		}

		for(int cell = 0; cell < xytData[frame].size(); cell++){
			if(frame != 0){
				if(indexOfObj(xytData[frame][cell].ObjectNum,frame-1) != -1){
					xytData[frame][cell].Speed = distance(xytData[frame-1][indexOfObj(xytData[frame][cell].ObjectNum,frame-1)], xytData[frame][cell]);
				}else{
					xytData[frame][cell].Speed = 0; //cell was not found in previous frame 
				}
			}
		}

		min_conf = (4/82) * xytData[frame].size() + 1.12;
	}

	int maxCellId = cleanData();

	for(int obj = 0; obj <= maxCellId; obj++){
		int startFrame = INT_MAX;
		int endFrame = -1;
		for(int frame = 0; frame < xytData.size(); frame++){
			if(indexOfObj(obj, frame) != -1){ //this object is in this frame
				startFrame = std::min(startFrame, frame);
				endFrame = frame;
			} 
		}
		if(parentAssignments[reverseAssign(obj)] == -1){
			cplData.push_back(CPL(obj, -1, endFrame - startFrame + 1, startFrame));
		}else{
			int parentId = assign[parentAssignments[reverseAssign(obj)]];
			cplData.push_back(CPL(obj, parentId, endFrame - startFrame + 1, startFrame));
		}
	}

	outputJS();
	outputCSV();
}

