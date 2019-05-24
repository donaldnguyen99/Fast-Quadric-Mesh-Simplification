#include "Simplify.h"
#include <unistd.h>
#include <iostream>

int main(int argc, char *const argv[]) {

    double binCount = 100; // Number of lines for number of bins
    int maxHeight = 200; // Number of characters for bin height

    int c;
	bool isVerbose = false;
    const char *optstring = "b:c:vh";
    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch (c) {
        case 'v':
            isVerbose = true;
            break;
        case 'b':
            binCount = atof(optarg);
            break;
        case 'c':
            maxHeight = atof(optarg);
            break;
        case '?':
        case 'h':
            printf("Usage: %s [-v|-b [int]|-c [int]|-h] input.obj", argv[0]);
            return EXIT_SUCCESS;
        default:
            return EXIT_FAILURE;
        }
    }

    Simplify::load_obj(argv[optind], isVerbose);
    std::vector<double> lengths;
    for (int i = 0; i < int(Simplify::triangles.size()); i++) {
        lengths.push_back((Simplify::vertices[Simplify::triangles[i].v[1]].p
                         - Simplify::vertices[Simplify::triangles[i].v[0]].p).length());
        lengths.push_back((Simplify::vertices[Simplify::triangles[i].v[2]].p
                         - Simplify::vertices[Simplify::triangles[i].v[1]].p).length());
        lengths.push_back((Simplify::vertices[Simplify::triangles[i].v[0]].p
                         - Simplify::vertices[Simplify::triangles[i].v[2]].p).length());
    }
    //for (std::vector<double>::iterator it = lengths.begin(); it != lengths.end(); ++it) {
    //    printf("%g\n",*it);
    //}
    
    int totalLengths = int(lengths.size());
    printf("Lengths vector size = %d\n", totalLengths);

    // Find min and max of lengths vector
    double maxVal = lengths.at(0);
    double minVal = lengths.at(0);
    for (std::vector<double>::iterator it = lengths.begin(); it != lengths.end(); ++it) {
        if (*it > maxVal) 
            maxVal = *it;
        if (*it < minVal)
            minVal = *it;
    }

    // Create frequency vector with a specific bin size
    double lengthsRange = (maxVal - minVal);
    double binSize = lengthsRange/binCount;
    int frequencySize = int(binCount+1);
    std::vector<int> frequency(frequencySize, 0);

    // Loop over lengths vector and count elements for frequency vector
    int frequencyIndex;
    for (std::vector<double>::iterator it = lengths.begin(); it != lengths.end(); ++it) {
        frequencyIndex = int(round((*it - minVal)/lengthsRange*binCount));
        frequency.at(frequencyIndex)++;
    }
    
    // Find min and max of frequency
    int maxFrequency = frequency.at(0);
    int minFrequency = frequency.at(0);
    for (std::vector<int>::iterator it = frequency.begin(); it != frequency.end(); ++it) {
        if (*it > maxFrequency) 
            maxFrequency = *it;
        if (*it < minFrequency)
            minFrequency = *it;
    }
    int frequencyRange = maxFrequency - minFrequency;
    
    // Create histogram
    printf("  %8d", minFrequency);
    for (int i = 0; i < maxHeight-8; i++) {
        printf("_");
    }
    printf("%d\n", maxFrequency);
    {
        int i = 0;
        for (std::vector<int>::iterator it = frequency.begin(); it != frequency.end(); ++it) {
            printf("%*f |", 8, i*binSize+minVal);
            int numSpaces = int(ceil((*it - minFrequency)/double(frequencyRange)*(maxHeight-1)));
            for (int j = 0; j < numSpaces; j++) {
                printf(" ");
            }
            printf(".\n");
            i++;
        }
    }
    return 0;
}