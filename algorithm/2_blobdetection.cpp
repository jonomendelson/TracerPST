#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>

using namespace cv;

int MIN_AREA = 40;
int MAX_AREA = 600;

class Cell{
	public:
		double x;
		double y;
		int area;
		double intensity;
		Cell(double, double, int, double);
};

Cell::Cell(double xx, double yy, int aa, double ii){
	x = xx;
	y = yy;
	area = aa;
	intensity = ii;
}

std::vector<Cell> findHoles(std::string PSTPath, std::string imagePath){
	std::vector<Cell> cells;

	Mat rawImage = imread(imagePath, CV_LOAD_IMAGE_GRAYSCALE);
	Mat im_in = imread(PSTPath, CV_LOAD_IMAGE_GRAYSCALE);

	Mat im_th;
    threshold(im_in, im_th, 80, 255, 0);
	
	Mat im_floodfill = im_th.clone();
	floodFill(im_floodfill, cv::Point(0, 0), Scalar(255));

	for(int x = 0; x < im_in.size().width; x++){
		for(int y = 0; y < im_in.size().height; y++){
			if(im_floodfill.at<uchar>(y, x) == 0){
				Mat im_floodfill_small = im_floodfill.clone();

				floodFill(im_floodfill_small, cv::Point(x, y), Scalar(128));
				floodFill(im_floodfill, cv::Point(x, y), Scalar(255)); //remove from main image
			
				std::vector<std::pair<int, int> > cell;
				for(int m = 0; m < im_in.size().width; m++){
					for(int n = 0; n < im_in.size().height; n++){
						if(im_floodfill_small.at<uchar>(n, m) == 128){
							cell.push_back(std::make_pair(m, n));
						}
					}
				}

				double totalX = 0;
				double totalY = 0;
				int pixelCount = 0;
				double totalIntensity = 0;
				for(int i = 0; i < cell.size(); i++){ //iterate through pixels
					totalX += cell[i].first;
					totalY += cell[i].second;
					pixelCount++;
					Scalar pixelData = rawImage.at<uchar>(cell[i].second, cell[i].first);
					totalIntensity += pixelData.val[0];
				}
				if((pixelCount >= MIN_AREA) && (pixelCount <= MAX_AREA)){
					totalX = totalX / (double)(pixelCount);
					totalY = totalY / (double)(pixelCount);
					totalIntensity = totalIntensity / (double)(pixelCount);
					cells.push_back(Cell(totalX, totalY, pixelCount, totalIntensity));
				}

				

			}
		}
	}
	return cells;
}

int main(){
	for(int frame = 0; frame < 2300; frame++){
		std::cout << frame << "\n";

		std::string PSTFilename = "Datasets/FinalEdges/Edge" + std::to_string(frame) + ".jpg";
		std::string rawFilename = "Datasets/FinalCroppedFrames/" + std::to_string(frame) + ".jpg";

		std::vector<Cell> cells = findHoles(PSTFilename, rawFilename);

		std::ofstream myfile;
		
		//Frame,X,Y,Radius
		myfile.open("blobData.csv", std::ofstream::out | std::ofstream::app);

		for(int i = 0; i < cells.size(); i++){
			myfile << frame << "," << cells[i].x << "," << cells[i].y << "," << cells[i].area << "," << cells[i].intensity << "\n";
		}
	}
}
