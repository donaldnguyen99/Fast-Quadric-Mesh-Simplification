// Simple wrapper for Sven Forstmann's mesh simplification tool
//
// Loads a OBJ format mesh, decimates mesh, saves decimated mesh as OBJ format
// http://voxels.blogspot.com/2014/05/quadric-mesh-simplification-with-source.html
// https://github.com/sp4cerat/Fast-Quadric-Mesh-Simplification
//To compile for Linux/OSX (GCC/LLVM)
//  g++ Main.cpp -O3 -o simplify
//To compile for Windows (Visual Studio)
// vcvarsall amd64
// cl /EHsc Main.cpp /osimplify
//To execute
//  ./simplify wall.obj out.obj 0.04
//
// Pascal Version by Chris Roden:
// https://github.com/neurolabusc/Fast-Quadric-Mesh-Simplification-Pascal-
//

#include "Simplify.h"
#include <stdio.h>
#include <time.h>  // clock_t, clock, CLOCKS_PER_SEC
#include <unistd.h>

void showHelp(char *const argv[]) {
    const char *cstr = (argv[0]);
    printf("Usage: %s <input> <output> <ratio> <agressiveness> <function> <indexOfInput> <radius> <scale> <power> <negative>)\n", cstr);
    printf(" Input: name of existing OBJ format mesh\n");
    printf(" Output: name for decimated OBJ format mesh\n");
    printf(" Ratio: (default = 0.5) for example 0.2 will decimate 80%% of triangles\n");
    printf(" Agressiveness: (default = 7.0) higher -> faster, lower -> better decimation\n");
	printf("   Previous options must be given to use <function>\n");
	printf(" Function: (default = constantFunc) gaussian, triangular, or square\n");
	printf("   IndexOfInput: (default = 0) pick index of vertex from list of vertices\n");
	printf("   Radius: (default = 1) radius for function\n");
	printf("   Scale: specific for each function\n");
	printf("     (gaussian's default = 2 if scale <= 1) scales gaussian curve by 1/scale");
	printf("     (triangular's default = 1) scales down radius/width by 1/scale\n");
	printf("     (square's default = 1) decimal from -1.0 and 1.0, with 1.0 -> no simplification inside radius\n");
	printf("   Power: (default = 1.0) Exponent which the function is raised to, higher -> function more visibly conveyed\n");
	printf("   Negative: (default = true) inverts function mapping\n");
    printf("Examples :\n");
#if defined(_WIN64) || defined(_WIN32)
    printf("  %s c:\\dir\\in.obj c:\\dir\\out.obj 0.2\n", cstr);
#else
    printf("  %s ~/dir/in.obj ~/dir/out.obj 0.2\n", cstr);
	printf("  %s ~/dir/in.obj ~/dir/out.obj 0.2 7 gaussian\n", cstr);
	printf("  %s ~/dir/in.obj ~/dir/out.obj 0.2 7 triangular 0 1 1\n", cstr);
#endif
} //showHelp()

int getopt(int argc, char *const argv[], const char *optstring);
// extern char *optarg;
// extern int optind, opterr, optopt;

int main(int argc, char *const argv[]) {
    printf("Mesh Simplification (C)2014 by Sven Forstmann in 2014, MIT License (%zu-bit)\n", sizeof(size_t)*8);
    
    double reduceFraction = 1.0;
    double aggressiveness = 7.0;
    double (*func)(double, double, double, double, double, double, double, double, bool) = constantFunc;
    double coord[] = {0, 0, 0};
    double radius = 1.0;
    double scale = 1.0;
    double power = 1.0;
    bool isVerbose = false, isNegative = false;

    int c;
	char *pcoord;
    const char *optstring = "t:a:f:c:r:s:p:vnh";
    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch (c) {
        case 't':
            reduceFraction = atof(optarg);
            if (reduceFraction > 1.0)
                reduceFraction = 1.0; //lossless only
            if (reduceFraction <= 0.0) {
                printf("Ratio must be BETWEEN zero and one.\n");
                return EXIT_FAILURE;
            }
            break;
        case 'a':
            aggressiveness = atof(optarg);
            break;
        case 'f':
            printf("Checking function: ");
            if (strcmp(optarg, "gaussian") == 0) {
                printf("gaussian\n");
                func = gaussian;
            } else if (strcmp(optarg, "triangular") == 0) {
                printf("triangular\n");
                func = triangular;
            } else if (strcmp(optarg, "square") == 0) {
                printf("square\n");
                func = square;
            } else {
                printf("constant function (uniform)\n");
            }
            break;
        case 'c':
            pcoord = strtok(optarg, "{[( ,)]}");
            coord[0] = atof(pcoord);
            for (int i = 1; i < 3; i++) {
                pcoord = strtok(NULL, "{[( ,)]}");
                coord[i] = atof(pcoord);
            }
            break;
        case 'r':
            radius = atof(optarg);
            break;
        case 's':
            scale = atof(optarg);
            break;
        case 'p':
            power = atof(optarg);
            break;
        case 'v':
            isVerbose = true;
            break;
        case 'n':
            isNegative = true;
            break;
        case '?':
        case 'h':
            showHelp(argv);
            return EXIT_SUCCESS;
        default:
            return EXIT_FAILURE;
        }
    }
    if ((func == gaussian) && (scale <= 1))
		printf("  Warning: cannot use scale = %f for gaussian, will use default = 2\n", scale);
    if (argc - optind < 2) {
        showHelp(argv);
        return EXIT_SUCCESS;
    }
	Simplify::load_obj(argv[optind]);
	if ((Simplify::triangles.size() < 3) || (Simplify::vertices.size() < 3))
		return EXIT_FAILURE;
	int target_count = round((float)Simplify::triangles.size() * reduceFraction);
    if (target_count < 4) {
		printf("Object will not survive such extreme decimation\n");
    	return EXIT_FAILURE;
    }
	clock_t start = clock();
	printf("Input: %zu vertices, %zu triangles (target %d)\n", Simplify::vertices.size(), Simplify::triangles.size(), target_count);
	int startSize = Simplify::triangles.size();
    Simplify::simplify_mesh(target_count, aggressiveness, isVerbose, func, coord[0], radius, scale, power, isNegative);
	//Simplify::simplify_mesh_lossless( false);
	if ( Simplify::triangles.size() >= (size_t) startSize) {
		printf("Unable to reduce mesh.\n");
    	return EXIT_FAILURE;
	}
	Simplify::write_obj(argv[optind+1]);
	printf("Output: %zu vertices, %zu triangles (%f reduction; %.4f sec)\n",Simplify::vertices.size(), Simplify::triangles.size()
		, (float)Simplify::triangles.size()/ (float) startSize  , ((float)(clock()-start))/CLOCKS_PER_SEC );
	return EXIT_SUCCESS;
}
