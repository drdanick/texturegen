#ifndef CONFIG_H
#define CONFIG_H


//////////////////////////////
// Grid settings
//

#define SQUARE_SIZE 50    // Width/Height of each square
#define GAP_SIZE 4        // Size of the gap between squares

#define NUM_SQUARES_X 48  // Number of squares in the x direction
#define NUM_SQUARES_Y 33  // Number of squares in the y direction
#define BORDER_SIZE 10    // Size of border surrounding the entire texture


//////////////////////////////
// Noise settings
//

// Note about noise: The noise applied is a intensity bias, where the bias is picked from a number in a range centered at 0, but not including 0.
// Noise is applied in a sequence of rounds, where each round has double the range and half the probability of the last.
#define APPLY_NOISE              // Comment this out to skip adding noise
#define NOISE_START_RANGE  2     // Starting noise bias interval
#define NOISE_START_PROB   2.0f  // Starting probability that a given pixel will have noise applied to it
#define NOISE_ROUNDS       8     // Number of noise rounds to do


//////////////////////////////
// Filter settings
//

// Note about the filter: The filter is a weighted mean filter. This is open to experimentation.
#define APPLY_FILTER             // Comment this out to skip the filtering step
#define FILTER_INPLACE           // Comment this out to perform an out-of-place filter

#endif /* CONFIG_H */
