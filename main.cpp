//////////////////////////////////////////////////////
//
// Grayscale square grid texture generator
//
// Author: Nick Stones-Havas
// Date:   29 Oct 2015
//

#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstdint>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define SQUARE_SIZE 50    // Width/Height of each square
#define GAP_SIZE 4        // Size of the gap between squares

#define NUM_SQUARES_X 16  // Number of squares in the x direction
#define NUM_SQUARES_Y 11  // Number of squares in the y direction
#define BORDER_SIZE 10    // Size of border surrounding the entire texture

const int PAGE_WIDTH  = (BORDER_SIZE * 2) + int(NUM_SQUARES_X * SQUARE_SIZE) + ((NUM_SQUARES_X + 1) * GAP_SIZE);
const int PAGE_HEIGHT = (BORDER_SIZE * 2) + int(NUM_SQUARES_Y * SQUARE_SIZE) + ((NUM_SQUARES_Y + 1) * GAP_SIZE);

#define NUM_INTENSITIES 5  // The total number of different intensities

// The 8-bit value of each intensity
const uint8_t intensities[] = {
    8,
    16,
    32,
    64,
    128
};

// The probability intervals for each intensity (Spcified number is the upper bound of the interval. The lower bound is the previous upper-bound)
const float prob_intervals[] = {
    0.05f,
    0.1f,
    0.3f,
    0.7f,
    1.0f
};


// Note about noise: The noise applied is a intensity bias, where the bias is picked from a number in a range centered at 0, but not including 0.
// Noise is applied in a sequence of rounds, where each round has double the range and half the probability of the last.

#define APPLY_NOISE              // Comment this out to skip adding noise
#define NOISE_START_RANGE  2     // Starting noise bias interval
#define NOISE_START_PROB   2.0f  // Starting probability that a given pixel will have noise applied to it
#define NOISE_ROUNDS       8     // Number of noise rounds to do


// Note about the filter: The filter is a weighted mean filter. This is open to experimentation.

#define APPLY_FILTER             // Comment this out to skip the filtering step
#define FILTER_INPLACE           // Comment this out to perform an out-of-place filter
#define FILTER_KERNEL_WIDTH 5    // Width/height of the filter

float KERNEL[FILTER_KERNEL_WIDTH][FILTER_KERNEL_WIDTH] = {  // The arbitrary blur kernel
    {0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 0.0f, 2.0f, 0.0f},
    {0.0f, 0.0f, 8.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
};


// Find the sum of values in the kernel matrix
inline float sumMatrix(float matrix[FILTER_KERNEL_WIDTH][FILTER_KERNEL_WIDTH], int width, int height) {
    float sum = 0;

    for(int row = 0; row < width; row++) {
        for(int col = 0; col < height; col++) {
            sum += matrix[row][col];
        }
    }

    return sum;
}

// Draw a sqaure to the page
void fillSquare(Mat& image, int x, int y, int size, uint8_t shade) {
    for(int row = y; row < y+size; row++) {
        for(int col = x; col < x+size; col++) {
            image.ptr<uint8_t>(row)[col] = shade;
        }
    }
}


// Main routine
int run(int argc, char** argv) {
    Mat page(Size(PAGE_WIDTH, PAGE_HEIGHT), CV_8UC1);
    page = Scalar(255); // Set page to be initially white

    srand(time(NULL));

    // Generate squares
    for(int x = 0; x < NUM_SQUARES_X; x++) {
        for(int y = 0; y < NUM_SQUARES_Y; y++) {
            int squarex = BORDER_SIZE + x * (GAP_SIZE + SQUARE_SIZE) + GAP_SIZE;
            int squarey = BORDER_SIZE + y * (GAP_SIZE + SQUARE_SIZE) + GAP_SIZE;

            float random = float(rand() % 101) / 100.0f;

            int i;
            for(i = 0; i < NUM_INTENSITIES; i++)
                if(random <= prob_intervals[i]) break;

            fillSquare(page, squarex, squarey, SQUARE_SIZE, intensities[i]);

        }
    }

    // Apply noise
#ifdef APPLY_NOISE
    int noiseRange = NOISE_START_RANGE;
    float noiseProb = NOISE_START_PROB;

    for(int i = 0; i < NOISE_ROUNDS; i++) {
        for(int row = BORDER_SIZE; row < PAGE_HEIGHT - BORDER_SIZE; row++) {
            for(int col = BORDER_SIZE; col < PAGE_WIDTH - BORDER_SIZE; col++) {
                int shade = page.ptr<uint8_t>(row)[col];
                float random = float(rand() % 101) / 100.0f;

                if(random <= noiseProb) {
                    int noise = (rand() % noiseRange) - noiseRange/2;
                    if(noise == 0) noise = noiseRange/2;

                    shade += noise;

                    // Clamp shade if required
                    if(shade < 0 || shade > 255)  shade -= 2 * noise;

                    page.ptr<uint8_t>(row)[col] = (char)shade;
                }
            }
        }
        noiseRange *= 2;
        noiseProb /= 2.0f;
    }
#endif

    // Filter the page
#ifdef APPLY_FILTER
    // Initialize kernel
    float kernelSum = sumMatrix(KERNEL, FILTER_KERNEL_WIDTH, FILTER_KERNEL_WIDTH);

    for(int i = 0; i < FILTER_KERNEL_WIDTH; i++)
        for(int j = 0; j < FILTER_KERNEL_WIDTH; j++)
            KERNEL[i][j] /= kernelSum;


    // Apply the filter
    Mat filteredPage;
    page.copyTo(filteredPage);
    for(int row = BORDER_SIZE; row < PAGE_HEIGHT - BORDER_SIZE; row++) {
        for(int col = BORDER_SIZE; col < PAGE_WIDTH - BORDER_SIZE; col++) {
            float shade = 0.0f;

            for(int krow = 0; krow < FILTER_KERNEL_WIDTH; krow++) {
                for(int kcol = 0; kcol < FILTER_KERNEL_WIDTH; kcol++) {
                    int x = col - int(FILTER_KERNEL_WIDTH/2) + kcol;
                    int y = row - int(FILTER_KERNEL_WIDTH/2) + krow;
#ifdef FILTER_INPLACE
                    shade += KERNEL[krow][kcol] * (float)filteredPage.ptr<uint8_t>(y)[x];
#else
                    shade += KERNEL[krow][kcol] * (float)page.ptr<uint8_t>(y)[x];
#endif
                }
            }

            filteredPage.ptr<uint8_t>(row)[col] = shade;
        }
    }
#endif

    // Write the images to the current folder
    imwrite("texture.png", page);
#ifdef APPLY_FILTER
    imwrite("filtered_texture.png", filteredPage);
#endif

    return EXIT_SUCCESS;
}


// Program entry point
int main(int argc, char** argv) {
    try {
        return run(argc, argv);
    } catch(exception e) {
        cerr << "ERROR: " << e.what();
    }

    return EXIT_FAILURE;
}
