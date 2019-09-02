#define cimg_use_png
#define cimg_use_jpeg
#include "CImg.h"
#include <iostream>
#include <vector>
#include <math.h>
#include <ostream>
#include <fstream>
#include <sstream>
const double PI = 3.14159265358979323846;

using namespace cimg_library;
using namespace std;

vector<float> linspace(float a, float b, int n)
{
    vector<float> array;
    float epsilon = 0.0001;
    float step = (b - a) / (n - 1);
    if (a == b) {
        for (int i = 0; i < n; i++) {
            array.push_back(a);
        }
    }
    else if (step >= 0) {
        while (a <= b + epsilon) {
            array.push_back(a);
            a += step;
        }
    }
    else {
        while (a + epsilon >= b) {
            array.push_back(a);
            a += step;
        }
    }
    return array;
}

// works for even and odd dimensions
CImg<float> fftshift2D(CImg<float> data, int xdim, int ydim)
{
    int xshift = xdim / 2;
    int yshift = ydim / 2;
    CImg<float> out(xdim, ydim, 1, 1, 0);
    cimg_forXY(out, x, y)
    {
        int outX = (x + xshift) % xdim;
        int outY = (y + yshift) % ydim;
        out(outX + xdim * outY) = data[x + xdim * y];
    }
    return out;
}

CImg<double> PSTFilter(CImg<double> image){
    float S, W, MaxThr, MinThr, LowPF, Sigma;
    int Morph;
    S = 100;//0.48;
    W = 0.01;//12.14;
    LowPF = 0.21;//0.2;
    Sigma = pow(LowPF, 2) / log(2);
    MaxThr = 0.0019;
    MinThr = -1;
    Morph = 1;

    CImg<double> gray(image.width(), image.height(), 1, 1, 0); 
    gray = image;

    float max_gray_value = gray(0, 0, 0, 0);
    
    cimg_forXY(gray, x, y)
    {
        if (max_gray_value < gray(x, y)) {
            max_gray_value = gray(x, y);
        }
    }

    int resized_width = pow(2, ceil(log(image.width()) / log(2)));
    int resized_height = pow(2, ceil(log(image.height()) / log(2)));
    gray = gray.resize(resized_width, resized_height);
    CImg<double> Edge(resized_width, resized_height, 1, 1, 0), X_g(resized_width, resized_height, 1, 1, 0), Y_g(resized_width, resized_height, 1, 1, 0),
        Rho(resized_width, resized_height, 1, 1, 0),
        PST_kernel(resized_width, resized_height, 1, 1, 0);

    float l = 0.5;
    vector<float> X_vector = linspace(-l, l, resized_width);
    vector<float> Y_vector = linspace(-l, l, resized_height);

    // New Method
    cimg_forXY(X_g, x, y)
    {
        float* X_g_p = &X_vector[x];
        X_g(x, y) = *X_g_p;
    }

    cimg_forXY(Y_g, x, y)
    {
        float* Y_g_p = &Y_vector[y];
        Y_g(x, y) = *Y_g_p;
    }

    cimg_forXY(Rho, x, y)
    {
        Rho(x, y) = sqrt(pow(X_g(x, y), 2) + pow(Y_g(x, y), 2));
    }

    cimg_forXY(PST_kernel, x, y)
    {
        PST_kernel(x, y) = W * Rho(x, y) * atan(W * Rho(x, y)) - 0.5 * log(1 + pow(W * Rho(x, y), 2));
    }

    float max_PST_kernel_value = PST_kernel(0, 0);
    cimg_forXY(PST_kernel, x, y)
    {
        if (max_PST_kernel_value < PST_kernel(x, y)) {
            max_PST_kernel_value = PST_kernel(x, y);
        }
    }

    cimg_forXY(PST_kernel, x, y)
    {
        PST_kernel(x, y) = (S * PST_kernel(x, y)) / max_PST_kernel_value; //cout << PST_kernel(x,y) << "\n";
    }

    // Fourier transform
    CImg<double> fft_in(resized_width, resized_height, 1, 1, 0),
        real(resized_width, resized_height, 1, 1, 0),
        imag(resized_width, resized_height, 1, 1, 0),
        mag_img(resized_width, resized_height, 1, 1, 0),
        phase_img(resized_width, resized_height, 1, 1, 0);

    fft_in = gray;

    CImgList<double> fft_out = fft_in.get_FFT();
    real = fft_out[0];
    imag = fft_out[1];
    mag_img = sqrt(fft_out[0].get_pow(2) + fft_out[1].get_pow(2));
    phase_img = fft_out[1].get_atan2(fft_out[0]);

    cimg_forXY(phase_img, x, y)
    {
        if (phase_img(x, y) > PI / 2) {
            phase_img(x, y) = phase_img(x, y) - PI;
        }
        else if (phase_img(x, y) < -PI / 2) {
            phase_img(x, y) = PI + phase_img(x, y);
        }
    }

    CImg<float> expo(resized_width, resized_height, 1, 1, 0), temp(resized_width, resized_height, 1, 1, 0);

    cimg_forXY(expo, x, y)
    {
        expo(x, y) = exp(-pow((Rho(x, y) / sqrt(Sigma)), 2));
    }

    expo = fftshift2D(expo, resized_width, resized_height);
    CImg<float> mag_expo(resized_width, resized_height, 1, 1, 0);

    cimg_forXY(mag_expo, x, y)
    {
        mag_expo(x, y) = expo(x, y) * mag_img(x, y);
    }
    CImg<float> imag_expo(resized_width, resized_height, 1, 1, 0);
    CImg<float> real_expo(resized_width, resized_height, 1, 1, 0);

    CImg<float> real_into_expo(resized_width, resized_height, 1, 1, 0);
    CImg<float> imag_into_expo(resized_width, resized_height, 1, 1, 0);

    cimg_forXY(real_into_expo, x, y)
    {
        real_into_expo(x, y) = expo(x, y) * real(x, y);
    }
    cimg_forXY(imag_into_expo, x, y)
    {
        imag_into_expo(x, y) = expo(x, y) * imag(x, y);
    }

    CImgList<float> tmp = CImgList<float>(real_into_expo, imag_into_expo);
    CImgList<float> Image_filtered = tmp.get_FFT(true);
    CImg<double> real_image_filtered = Image_filtered[0];

    CImgList<double> Image_orig_filtered = real_image_filtered.get_FFT();
    CImg<double> PST_kernel_sin_expo(resized_width, resized_height, 1, 1, 0);
    cimg_forXY(PST_kernel_sin_expo, x, y)
    {
        PST_kernel_sin_expo(x, y) = sin(-PST_kernel(x, y));
    }

    PST_kernel_sin_expo = fftshift2D(PST_kernel_sin_expo, resized_width, resized_height);
    CImg<double> PST_kernel_cos_expo(resized_width, resized_height, 1, 1, 0);
    cimg_forXY(PST_kernel_cos_expo, x, y)
    {
        PST_kernel_cos_expo(x, y) = cos(-PST_kernel(x, y));
    }

    PST_kernel_cos_expo = fftshift2D(PST_kernel_cos_expo, resized_width, resized_height);
    CImg<double> temp_real_filtered(resized_width, resized_height, 1, 1, 0),
        temp_imag_filtered(resized_width, resized_height, 1, 1, 0);

    cimg_forXY(temp_real_filtered, x, y)
    {
        temp_real_filtered(x, y) = Image_orig_filtered(0, x, y) * PST_kernel_cos_expo(x, y) - Image_orig_filtered(1, x, y) * PST_kernel_sin_expo(x, y);
    }

    cimg_forXY(temp_imag_filtered, x, y)
    {
        temp_imag_filtered(x, y) = Image_orig_filtered(0, x, y) * PST_kernel_sin_expo(x, y) + Image_orig_filtered(1, x, y) * PST_kernel_cos_expo(x, y);
    }

    CImgList<float> output_PST_filtered = CImgList<float>(temp_real_filtered, temp_imag_filtered);

    CImgList<float> output_PST_filtered_xy = output_PST_filtered.get_FFT(true);
    CImg<double> PHI_features(resized_width, resized_height, 1, 1, 0),
        out(resized_width, resized_height, 1, 1, 0),
        features(resized_width, resized_height, 1, 1, 0),
        PST_Edge(resized_width, resized_height, 1, 1, 0);

    PHI_features = output_PST_filtered_xy[1].get_atan2(output_PST_filtered_xy[0]);

	string outputFilename = "phi.jpg";
	PHI_features.save(outputFilename.c_str());


    cimg_forXY(features, x, y)
    {
        if (PHI_features(x, y) > MaxThr) {
            features(x, y) = 1;
        }
        else if (PHI_features(x, y) < MinThr) {
            features(x, y) = 1;
        }
        else if (gray(x, y) < (max_gray_value / 20)) {
            features(x, y) = 0;
        }
        else {
            features(x, y) = 0;
        }
    }
   
    out = features;

    float max_out_value = out(0, 0, 0, 0);
    cimg_forXY(out, x, y)
    {
        if (max_out_value < out(x, y)) {
            max_out_value = out(x, y);
        }
    }
    cimg_forXY(PST_Edge, x, y)
    {
        if ((Morph == 0)) {
            PST_Edge(x, y) = (out(x, y)/max_out_value)*3;
        }
        else {
            PST_Edge(x, y) = out(x, y);
        }
    }

    PST_Edge.resize(image.width(), image.height());
    return PST_Edge.normalize(0, 255);
}


int X_MIN_ONE = 152;
int X_MAX_ONE = 1048;
int Y_MIN_ONE = 0;
int Y_MAX_ONE = 899;

int X_MIN_TWO = 311;
int X_MAX_TWO = 805;
int Y_MIN_TWO = 604;
int Y_MAX_TWO = 1096;

int main(){
	for(int frame = 0; frame <= 2300; frame++){
		std::cout << frame << "\n";
		CImg<double> croppedImage(X_MAX_ONE-X_MIN_ONE, Y_MAX_ONE-Y_MIN_ONE, 1, 1, 0);

		CImg<double> rawImage; 
		string filename = "Datasets/Edges/Edge" + std::to_string(frame) + ".jpg";
		rawImage.load(filename.c_str());

		cimg_forXY(rawImage, x, y){
			bool inBoxOne = (x > X_MIN_ONE) && (x < X_MAX_ONE) && (y > Y_MIN_ONE) && (y < Y_MAX_ONE);
			bool inBoxTwo = false;
			if(inBoxOne){
				croppedImage(x-X_MIN_ONE, y-Y_MIN_ONE, 0, 0) = rawImage(x, y, 0, 0);
			}else if(inBoxTwo){
				croppedImage(x-X_MIN_TWO, y-Y_MIN_TWO+Y_MAX_ONE-Y_MIN_ONE-1, 0, 0) = rawImage(x, y, 0, 0);				
			}
		}

		croppedImage = croppedImage.get_resize(498, 500);

		string outputFilename = "Datasets/FinalEdges/Edge" + std::to_string(frame) + ".jpg";
		croppedImage.save(outputFilename.c_str());
	}
}
