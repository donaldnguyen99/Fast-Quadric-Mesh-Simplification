// tri10 format
/*
<polygon_count> <factor>
    <v0x>   <v0y>   <v0z>   <v1x>   <v1y>   <v1z>   <v2x>   <v2y>   <v2z>   <ele>
    ...
*/

// obj format
/*
v <x> <y> <z>
...
f <i0> <i1> <i2>
...
*/

#include "Simplify.h"
#include <stdio.h>
#include <time.h>  // clock_t, clock, CLOCKS_PER_SEC
#include <unistd.h>
// #include <boost/functional/hash.hpp>
using namespace Simplify;

const char *filename;
bool process_uv = false;

void loadfromtri10(const char *filename, bool verbose = false, int verboselines = 10000) {
    printf("Loading %s ...\n", filename);
    FILE *fn;
    if ((filename == NULL) || ((char)filename[0] == 0) || ((fn = fopen(filename, "rb")) == NULL)) {
        printf("File %s not found!\n", filename);
        exit(EXIT_FAILURE);
    }
    char line[1000];
    memset(line, 0, 1000);
    if (fgets(line, 1000, fn) != NULL) {
        double totallines, magnification, moreArgs;
        if (verbose && (sscanf(line, "%lf %lf %lf", &totallines, &magnification, &moreArgs) == 2)) {
            printf("tri file header:\n Polygons: %lf, Magnification (ignored): %lf\n", totallines, magnification);
        } else {
            rewind(fn);
        }
    }
    vertices.clear();
    triangles.clear();
    int line_index = 0;
    const bool useNsquared = false; // user decide
    const bool useNlogN = false; // user decide
    const bool useN = true; // user decide
    while (fgets(line, 1000, fn) != NULL) {
        Triangle t;
        Vertex v0, v1, v2;
        double quality; // quality is the 10th number and is ignored
        if (sscanf(line, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
        &v0.p.x, &v0.p.y, &v0.p.z,
        &v1.p.x, &v1.p.y, &v1.p.z,
        &v2.p.x, &v2.p.y, &v2.p.z, &quality) >= 9) {
            if (useNsquared) {
                // Find vertex from container of vertices, and use existing vertex if found
                bool v0Missing = true, v1Missing = true, v2Missing = true;
                for (std::vector<Vertex>::reverse_iterator it = vertices.rbegin(); it != vertices.rend(); ++it) {
                    if (v0Missing && ((*it).p == v0.p)) {
                        t.v[0] = std::distance(vertices.begin(), it.base()) - 1;
                        v0Missing = false;
                    } else if (v1Missing && ((*it).p == v1.p)) {
                        t.v[1] = std::distance(vertices.begin(), it.base()) - 1;
                        v1Missing = false;
                    } else if (v2Missing && ((*it).p == v2.p)) {
                        t.v[2] = std::distance(vertices.begin(), it.base()) - 1;
                        v2Missing = false;
                    }
                    if ((!v0Missing) && (!v1Missing) && (!v2Missing)) break;
                }
                // After search, push_back vertex to container if necessary
                if (v0Missing) {
                    vertices.push_back(v0);
                    t.v[0] = int(vertices.size()) - 1;
                }
                if (v1Missing) {
                    vertices.push_back(v1);
                    t.v[1] = int(vertices.size()) - 1;
                }
                if (v2Missing) {
                    vertices.push_back(v2);
                    t.v[2] = int(vertices.size()) - 1;
                }
                triangles.push_back(t);
            } else { // use NlogN or N
                vertices.push_back(v0);
                vertices.push_back(v1);
                vertices.push_back(v2);
                t.v[0] = (3*line_index);
                t.v[1] = (3*line_index)+1;
                t.v[2] = (3*line_index)+2;
                triangles.push_back(t);
            }
            line_index++;
            if (verbose && (line_index % verboselines == 0)) printf("  tri lines read: %d\n", line_index);
        }
    }
    printf("Total lines (triangles) read: %d\n", line_index);
    // Done reading lines from file

    // Removing duplicates if necessary
    if (useNsquared && !useNlogN) {
        printf("Duplicates already removed by O(n^2) method.\n");
    } else if (useNlogN) {
        // Using sorting method, O(NlogN) time-complexity
        printf("Removing duplicate vertices by O(nlogn) sorting method...\n");

        // Need to provide algorithm


    } else if (useN) {
        // Use hashtable, O(N) time-complexity
        // std::unordered_map may encounter key collision when trying to insert new Vertex, so far so good
        printf("Removing duplicate vertices by O(n) hashtable method...\n");

        struct hashFunc{
            size_t operator()(const Vertex &k) const{
                size_t h1 = std::hash<double>()(k.p.x);
                size_t h2 = std::hash<double>()(k.p.y);
                size_t h3 = std::hash<double>()(k.p.z);
                return (h1 ^ (h2 << 1)) ^ h3;
            }
        };
        struct equalsFunc{
            bool operator()( const Vertex& lhs, const Vertex& rhs ) const{
                static const double EPS = 0.0000001;
                return (fabs(lhs.p.x - rhs.p.x) <= EPS) && (fabs(lhs.p.y - rhs.p.y) <= EPS) && (fabs(lhs.p.z - rhs.p.z) <= EPS);
            }
        };
        
        std::vector<Vertex> original_vertices;
        original_vertices = vertices;

        // Insertion of vertices into hashtable
        std::unordered_map<Vertex, size_t, hashFunc, equalsFunc> setVertices;
        std::vector<Vertex>::iterator vitr = vertices.begin();
        for (size_t i = 0; i < vertices.size(); i++) {
            if(setVertices.insert(std::make_pair(vertices[i], std::distance(vertices.begin(), vitr))).second) {
                *vitr = vertices[i]; vitr++;
            }
            if(verbose && ((i+1)%verboselines==0)) printf("  Checking vertex v #%d, %.2lf%% of vertices\n", int(i+1), double(i+1)/(vertices.size()+1)*100);
        }
        vertices.erase(vitr, vertices.end());
        
        // Updating of triangles vertex ids
        int count = 0;
        int triwithdups = 0;
        size_t startTrianglesSize = triangles.size();
        for (int i = 0; i < int(startTrianglesSize); i++) {
            for (int j = 0; j < 3; j++) {
                if (equalsFunc()(vertices[setVertices[original_vertices[triangles[i].v[j]]]], original_vertices[triangles[i].v[j]]))
                    triangles[i].v[j] = int(setVertices[original_vertices[triangles[i].v[j]]]);
                else {
                    count++;
                    // We try to fix the failed vertices...

                    bool foundInVertices = false;
                    for(int k = 0; k < int(vertices.size()); k++) {
                        if (equalsFunc()(vertices[k], original_vertices[triangles[i].v[j]])) {
                            triangles[i].v[j] = k;
                            foundInVertices = true;
                            break;
                        }
                    }
                    if (!foundInVertices) {
                        vertices.push_back(original_vertices[triangles[i].v[j]]);
                        triangles[i].v[j] = int(vertices.size()-1);
                    }
                }
            }
            // Checks if any 2 of 3 vertices of a triangle are duplicates
            if((equalsFunc()(vertices[triangles[i].v[0]], vertices[triangles[i].v[1]])) || (equalsFunc()(vertices[triangles[i].v[1]],vertices[triangles[i].v[2]])) || (equalsFunc()(vertices[triangles[i].v[0]],vertices[triangles[i].v[2]]))) {
                triwithdups++;
            }
            if(verbose && ((i+1)%verboselines==0)) printf("  Updating triangle f #%d, %.2lf%% of triangles\n", int(i+1), double(i+1)/(startTrianglesSize+1)*100);
        }
        if (count > 0) printf("  Failed vertices by hashtable: %d\n", count); else printf("  No failed vertices by hashtable.\n");
        if (triwithdups > 0) printf("  Triangles with duplicate vertices: %d\n", triwithdups); else printf("  No triangles with duplicate vertices.\n");
        printf("  Number of original vertices:  %d\n  Number of removed duplicates: %d\n  Number of finished vertices:  %d\n", int(original_vertices.size()), int(original_vertices.size())-int(vertices.size()), int(vertices.size()));
    }
    fclose(fn);
}

void write2obj(const char *filename, bool verbose, int verboselines = 10000) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("write_obj: can't write data file \"%s\".\n", filename);
        exit(0);
    }
    printf("Writing to %s ...\n", filename);

    double totalvertices = double(vertices.size());
    loopi(0, vertices.size()) {
        if(verbose && (i%verboselines==0)) printf("obj vertices written: %d, %.2lf%% of vertices\n", i, double(i)/totalvertices*100);
        fprintf(file, "v %lf %lf %lf\n", vertices[i].p.x,vertices[i].p.y,vertices[i].p.z);
    }

    double totaltriangles = double(triangles.size());
    loopi(0, triangles.size()) {
        if(verbose && (i%verboselines==0)) printf("obj triangles written: %d, %.2lf%% of triangles\n", i, double(i)/totaltriangles*100);
        fprintf(file, "f %d %d %d\n", triangles[i].v[0] + 1, triangles[i].v[1] + 1, triangles[i].v[2] + 1);
        //fprintf(file, "f %d// %d// %d//\n", triangles[i].v[0]+1, triangles[i].v[1]+1, triangles[i].v[2]+1);
    }
    fclose(file);
}

// int getopt(int argc, char *const argv[], const char *optstring);

int main(int argc, char *const argv[]) {
    const char *cstr = (argv[0]);
    bool isVerbose = false;
    int verboselines = 100000;
    int tempverboselines = verboselines;
    bool helpshown = false;

    int c;
    const char *optstring = "vV:h";
    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch (c) {
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
        case 'h':
        default:
            helpshown = true;
            printf("Usage: %s [-v|-h|-?|[-V <verbose_interval>]] inputfile.tri10 outputfile.obj\n", cstr);
            break;
        }
    }
    if (argc - optind < 2) {
        if(!helpshown) printf("Usage: %s [-v|-h|-?|[-V <verbose_interval>]] inputfile.tri10 outputfile.obj\n", cstr);
        return EXIT_SUCCESS;
    }
    clock_t load_start = clock();
    std::string filenameIn(argv[optind]);
    std::string filenameOut(argv[optind+1]);
    std::string::size_type idx;
    std::string::size_type outidx;
    idx = filenameIn.rfind('.');
    outidx = filenameOut.rfind('.');
    bool doload = false, dowrite = false;
    if (idx != std::string::npos) {
        std::string extensionIn = filenameIn.substr(idx+1);
        if ((extensionIn == "tri10") | (extensionIn == "tri9")) doload = true;
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
        if (extensionOut == "obj") dowrite = true;
        else {
            printf("Cannot write to file with extension .%s\n", extensionOut.c_str());
            return EXIT_FAILURE;
        }
    } else {
        printf("Output file's extension not found.\n");
        return EXIT_FAILURE;
    }
    if(doload) loadfromtri10(argv[optind], isVerbose, verboselines);
    if(dowrite) write2obj(argv[optind+1], isVerbose, verboselines);
    if(isVerbose) printf("Finished %s in %.4f sec)\n", cstr, ((float)(clock()-load_start))/CLOCKS_PER_SEC );
	return EXIT_SUCCESS;
}