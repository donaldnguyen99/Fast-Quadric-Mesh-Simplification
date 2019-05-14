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
    printf("Usage: %s [option...] inputfile outputfile\n", cstr);
    printf("%s simplifies an triangular mesh .obj, .tri9, or .tri10 file using a quadric\n", cstr);
    printf("error metrics; Input file can be either obj, tri9, or tri10; output file is only\n");
    printf("in obj. trii9 and tri10 files will take longer to load because duplicate\n");
    printf("vertices references need to be merged for every triangle. Suggested to use obj\n");
    printf("with duplicate vertices already removed.\n");
    printf(" Examples:\n");
#if defined(_WIN64) || defined(_WIN32)
    printf("  %s -t 0.2 c:\\dir\\in.obj c:\\dir\\out.obj\n", cstr);
#else
    printf("  %s -t 0.2 ~/dir/in.obj ~/dir/out.obj\n", cstr);
	printf("  %s -vn -t 0.1 -f gaussian -c 10,-20,0.5 -r 10 ~/dir/in.obj ~/dir/out.obj\n", cstr);
#endif
    printf(" Common Options:\n");
    printf("  -h|?      Show help\n");
    printf("  -v        Be verbose.\n");
    printf("  -V <arg>  Be verbose with details at every <arg> iterations (default: 10000)\n");
    printf("  -t <arg>  Total ratio of target's polygon count to source's (default: 0.5)\n");
    printf("  -T <arg1>,<arg2>  Region INSIDE radius will be reduced by ratio arg1. Region\n");
    printf("            OUTSIDE radius by arg2. -t option will be ignored.\n");
    printf("            Example: 0.8,0.1   \"( 0.1, 0.01 )\" (default: 0.5,0.5)\n");
    printf("  -a <arg>  Aggressiveness; higher=faster lower=better decimation (default: 7.0)\n");
    printf(" Function options for a spacially non-uniform reduction:\n");
    printf("  -f <arg>  Function name\n");
    printf("                ARG: square|triangular|gaussian (default: constFunc)\n");
    printf("  -c <arg>  Comma-separated coordinate for center of function (default: 0,0,0)\n");
    printf("            Use quotes if including spaces: \"(-1, 0, 100)\"   \"[ 0.1, 4, 2 ]\"\n");
    printf("  -r <arg>  Radius or boundary of function (default: 1.0)\n");
    printf("  -s <arg>  Scale down specifically for each function (default: 1.0)\n");
    printf("            square: region is retained by s, outside region is simplified fully\n");
    printf("                square's ARG: -1 to 1; negative ARG equivalent to -n flag\n");
    printf("            triangular: change radius by factor of 1/s\n");
    printf("            gaussian: attenuate curve by factor of 1/s at radius;\n");
    printf("                (default: 2.0); if s =< 1, default is used\n");
    printf("                For reference, gaussian STD_DEVIATION = r/sqrt(2*ln(s))\n");
    printf("  -p <arg>  Power to which the function is raised (default: 1.0)\n");
    printf("                higher means function is more visibly conveyed\n");
    printf("  -n        Negative form of function used.\n");
    printf("  -b <arg>  Breaking all iterations if selected number of consecutive iterations\n");
    printf("            failed to delete triangles. (default: 1000)\n");
} //showHelp()

// int getopt(int argc, char *const argv[], const char *optstring);
// extern char *optarg;
// extern int optind, opterr, optopt;

int main(int argc, char *const argv[]) {
    printf("Mesh Simplification (C)2014 by Sven Forstmann in 2014, MIT License (%zu-bit)\n", sizeof(size_t)*8);
    
    double reduceFraction = 0.5;
    double aggressiveness = 2.0;
    double (*func)(double, double, double, double, double, double, double, double, bool) = constantFunc;
    double coord[] = {0, 0, 0};
    double radius = 1.0;
    double scale = 1.0;
    double power = 1.0;
    bool doRegionSimplification = false;
    bool isVerbose = false, isNegative = false;
    int tempverboselines, verboselines = 10000;
    int tempConsecutiveNoDeletionThreshold;

    int c;
	char *pcoord;
    const char *optstring = "t:a:f:c:r:s:p:T:V:b:vnh";
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
        case 'T':
            Simplify::target_region_ratio = atof(strtok(optarg, "{[( ,)]}"));
            Simplify::target_outside_ratio = atof(strtok(NULL, "{[( ,)]}"));
            if (Simplify::target_region_ratio > 1) {
                Simplify::target_region_ratio = 0.5;
                printf("Warning: Region's ratio will use default: 0.5\n");
            }
            if (Simplify::target_outside_ratio > 1) {
                Simplify::target_outside_ratio = 0.5;
                printf("Warning: Outside Region's ratio will use default: 0.5\n");
            }
            reduceFraction = min(double(Simplify::target_region_ratio), double(Simplify::target_outside_ratio));
            doRegionSimplification = true;
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
            for (int i = 0; i < 3; i++) {
                coord[i] = atof(pcoord);
                pcoord = strtok(NULL, "{[( ,)]}");
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
        case 'V':
            isVerbose = true;
            tempverboselines = atoi(optarg);
            if (tempverboselines <= 0) {
                printf("-V needs an valid argument, using default: %d\n", verboselines);
                tempverboselines = verboselines;
            }
            verboselines = tempverboselines;
            break;
        case 'b':
            tempConsecutiveNoDeletionThreshold = atoi(optarg);
            if (tempConsecutiveNoDeletionThreshold <= 0) {
                printf("-b needs a positive integer, using default: %d\n", Simplify::consecutiveNoDeletionThreshold);
                tempConsecutiveNoDeletionThreshold = Simplify::consecutiveNoDeletionThreshold;
            }
            Simplify::consecutiveNoDeletionThreshold = tempConsecutiveNoDeletionThreshold;
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
		printf("  Warning: cannot use scale = %g for gaussian, will use default = 2\n", scale);
    if (argc - optind < 2) {
        showHelp(argv);
        return EXIT_SUCCESS;
    }
    clock_t load_start = clock();
    std::string filenameIn(argv[optind]);
    std::string filenameOut(argv[optind+1]);
    std::string::size_type idx;
    std::string::size_type outidx;
    idx = filenameIn.rfind('.');
    outidx = filenameOut.rfind('.');
    bool doloadobj = false, doloadtri10 = false, dowriteobj = false, dowritetri10 = false, dowritetri9 = false;
    if (idx != std::string::npos) {
        std::string extensionIn = filenameIn.substr(idx+1);
        if (extensionIn == "obj") doloadobj = true;
        else if (extensionIn == "tri10") doloadtri10 = true;
        else if (extensionIn == "tri9") doloadtri10 = true; // load_tri10 will read tri9 the same
        else {
            printf("Cannot load file with extension .%s\n", extensionIn.c_str());
            return EXIT_FAILURE;
        }
    } else {
        printf("Input file's extension not found.\n");
        return EXIT_FAILURE;
    }
    if (outidx != std::string::npos) {
        std::string extensionOut = filenameOut.substr(outidx+1);
        if (extensionOut == "obj") dowriteobj = true;
        else if (extensionOut == "tri10") dowritetri10 = true;
        else if (extensionOut == "tri9") dowritetri9 = true;
        else {
            printf("Cannot write to file with extension .%s\n", extensionOut.c_str());
            return EXIT_FAILURE;
        }
    } else {
        printf("Output file's extension not found.\n");
        return EXIT_FAILURE;
    }
    if (doloadobj) Simplify::load_obj(argv[optind], isVerbose, verboselines);
    else if (doloadtri10) Simplify::load_tri10(argv[optind], isVerbose, verboselines);
    printf("File loaded in %.4f sec\n", ((float)(clock()-load_start))/CLOCKS_PER_SEC);
	if ((Simplify::triangles.size() < 3) || (Simplify::vertices.size() < 3))
		return EXIT_FAILURE;
    if (doRegionSimplification) {
        Simplify::initialRegionCount = 0;
        for (int i = 0; i < (int)(Simplify::triangles.size()); i++) {
            if (Simplify::inRegion(Simplify::triangles[i], coord, radius)) {
                Simplify::initialRegionCount++;
            }
        }
    }
	int target_count = round((float)Simplify::triangles.size() * reduceFraction);
    if (target_count < 4) {
		printf("Object will not survive such extreme decimation\n");
    	return EXIT_FAILURE;
    }
	clock_t start = clock();
	printf("Input: %zu vertices, %zu triangles (target %d)\n", Simplify::vertices.size(), Simplify::triangles.size(), target_count);
	int startSize = int(Simplify::triangles.size());
    Simplify::initialTotalCount = startSize;
    Simplify::simplify_mesh(coord, target_count, aggressiveness, isVerbose, func, radius, scale, power, isNegative, doRegionSimplification);
	//Simplify::simplify_mesh_lossless( false);
	if (int(Simplify::triangles.size()) >= startSize) {
		printf("Unable to reduce mesh.\n");
    	return EXIT_FAILURE;
	}
	if (dowriteobj) Simplify::write_obj(argv[optind+1], isVerbose, verboselines);
    else if (dowritetri10) Simplify::write_tri10(argv[optind+1], isVerbose, verboselines);
    else if (dowritetri9) Simplify::write_tri9(argv[optind+1], isVerbose, verboselines);
	printf("Output: %zu vertices, %zu triangles (%f reduction; %.4f sec)\n",Simplify::vertices.size(), Simplify::triangles.size()
		, (float)Simplify::triangles.size()/ (float) startSize  , ((float)(clock()-start))/CLOCKS_PER_SEC );
	return EXIT_SUCCESS;
}
