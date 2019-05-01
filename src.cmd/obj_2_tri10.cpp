// obj format
/*
v <x> <y> <z>
...
f <i0> <i1> <i2>
...
*/

// tri10 format
/*
<polygon_count> <factor>
    <v0x>   <v0y>   <v0z>   <v1x>   <v1y>   <v1z>   <v2x>   <v2y>   <v2z>   <ele>
    ...
*/

#include "Fast-Quadric-Mesh-Simplification/src.cmd/Simplify.h"
#include <stdio.h>
#include <time.h>  // clock_t, clock, CLOCKS_PER_SEC
#include <unistd.h>
using namespace Simplify;

const char *filename;
bool process_uv = false;

void loadfromobj(const char *filename, bool verbose = false, int verboselines = 10000, bool process_uv = false) {
    vertices.clear();
    triangles.clear();
    //printf ( "Loading Objects %s ... \n",filename);
    FILE *fn;
    if (filename == NULL)
        return;
    if ((char)filename[0] == 0)
        return;
    if ((fn = fopen(filename, "rb")) == NULL) {
        printf("File %s not found!\n", filename);
        return;
    }
    char line[1000];
    memset(line, 0, 1000);
    int vertex_cnt = 0;
    int material = -1;
    std::map<std::string, int> material_map;
    std::vector<vec3f> uvs;
    std::vector<std::vector<int> > uvMap;
    int line_index = 0;
    while (fgets(line, 1000, fn) != NULL) {
        Vertex v;
        vec3f uv;

        if (strncmp(line, "mtllib", 6) == 0) {
            mtllib = trimwhitespace(&line[7]);
        }
        if (strncmp(line, "usemtl", 6) == 0) {
            std::string usemtl = trimwhitespace(&line[7]);
            if (material_map.find(usemtl) == material_map.end()) {
                material_map[usemtl] = materials.size();
                materials.push_back(usemtl);
            }
            material = material_map[usemtl];
        }

        if (line[0] == 'v' && line[1] == 't') {
            if (line[2] == ' ') {
                if (sscanf(line, "vt %lf %lf",
                           &uv.x, &uv.y) == 2) {
                    uv.z = 0;
                    uvs.push_back(uv);
                }
            } else {
                if (sscanf(line, "vt %lf %lf %lf",
                           &uv.x, &uv.y, &uv.z) == 3) {
                    uvs.push_back(uv);
                }
            }
        } else if (line[0] == 'v') {
            if (line[1] == ' ')
                if (sscanf(line, "v %lf %lf %lf",
                           &v.p.x, &v.p.y, &v.p.z) == 3) {
                    vertices.push_back(v);
                }
        }
        int integers[9];
        if (line[0] == 'f') {
            Triangle t;
            bool tri_ok = false;
            bool has_uv = false;

            if (sscanf(line, "f %d %d %d",
                       &integers[0], &integers[1], &integers[2]) == 3) {
                tri_ok = true;
            } else if (sscanf(line, "f %d// %d// %d//",
                              &integers[0], &integers[1], &integers[2]) == 3) {
                tri_ok = true;
            } else if (sscanf(line, "f %d//%d %d//%d %d//%d",
                              &integers[0], &integers[3],
                              &integers[1], &integers[4],
                              &integers[2], &integers[5]) == 6) {
                tri_ok = true;
            } else if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                              &integers[0], &integers[6], &integers[3],
                              &integers[1], &integers[7], &integers[4],
                              &integers[2], &integers[8], &integers[5]) == 9) {
                tri_ok = true;
                has_uv = true;
            } else {
                printf("unrecognized sequence\n");
                printf("%s\n", line);
                while (1)
                    ;
            }
            if (tri_ok) {
                t.v[0] = integers[0] - 1 - vertex_cnt;
                t.v[1] = integers[1] - 1 - vertex_cnt;
                t.v[2] = integers[2] - 1 - vertex_cnt;
                t.attr = 0;

                if (process_uv && has_uv) {
                    std::vector<int> indices;
                    indices.push_back(integers[6] - 1 - vertex_cnt);
                    indices.push_back(integers[7] - 1 - vertex_cnt);
                    indices.push_back(integers[8] - 1 - vertex_cnt);
                    uvMap.push_back(indices);
                    t.attr |= TEXCOORD;
                }

                t.material = material;
                //geo.triangles.push_back ( tri );
                triangles.push_back(t);
                //state_before = state;
                //state ='f';
            }
        }
        ++line_index;
        if (verbose && (line_index % verboselines == 0))
            printf("Lines read: %d\n", line_index);
    }

    if (process_uv && uvs.size()) {
        loopi(0, triangles.size()) {
            loopj(0, 3)
                triangles[i]
                    .uvs[j] = uvs[uvMap[i][j]];
        }
    }

    fclose(fn);

    //printf("load_obj: vertices = %lu, triangles = %lu, uvs = %lu\n", vertices.size(), triangles.size(), uvs.size() );
}

void write2tri10(const char *filename, bool verbose = false, int verboselines = 10000) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("write_obj: can't write data file \"%s\".\n", filename);
        exit(0);
    }

    double quality = 0.0;
    double totalsize = double(triangles.size());
    loopi(0, triangles.size()) {
        fprintf(file, " %15g %15g %15g %15g %15g %15g %15g %15g %15g %15g\n",
        vertices[triangles[i].v[0]].p.x, vertices[triangles[i].v[0]].p.y, vertices[triangles[i].v[0]].p.z,
        vertices[triangles[i].v[1]].p.x, vertices[triangles[i].v[1]].p.y, vertices[triangles[i].v[1]].p.z,
        vertices[triangles[i].v[2]].p.x, vertices[triangles[i].v[2]].p.y, vertices[triangles[i].v[2]].p.z, quality);
        if (verbose && (i%verboselines==0)) printf("tri10 lines written: %d, %.2lf%%\n", i, double(i)/totalsize*100);
        //fprintf(file, "f %d// %d// %d//\n", triangles[i].v[0]+1, triangles[i].v[1]+1, triangles[i].v[2]+1); //more compact: remove trailing zeros
    }
    fclose(file);
}

int getopt(int argc, char *const argv[], const char *optstring);

int main(int argc, char *const argv[]) {
    const char *cstr = (argv[0]);
    bool isVerbose = false;
    int verboselines = 10000;
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
            verboselines = atof(optarg);
            if (verboselines <= 0) verboselines = 10000;
            break;
        case 'h':
        default:
            helpshown = true;
            printf("Usage: %s [-v|-h|-?|[-V <verbose_interval>]] inputfile.obj outputfile.tri10\n", cstr);
            break;
        }
    }
    if (argc - optind < 2) {
        if(!helpshown) printf("Usage: %s [-v|-h|-?|[-V <verbose_interval>]] inputfile.obj outputfile.tri10\n", cstr);
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
        if (extensionIn == "obj") doload = true;
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
        if (extensionOut == "tri10") dowrite = true;
        else {
            printf("Cannot write to file with extension .%s\n", extensionOut.c_str());
            return EXIT_FAILURE;
        }
    } else {
        printf("Output file's extension not found.\n");
        return EXIT_FAILURE;
    }
    if(isVerbose) printf("Loading .obj file\n");
    if(doload) loadfromobj(argv[optind], isVerbose, verboselines);
    if(dowrite) write2tri10(argv[optind+1],isVerbose, verboselines);
    if(isVerbose) printf("Finished %s in %.4f sec)\n", cstr, ((float)(clock()-load_start))/CLOCKS_PER_SEC );
	return EXIT_SUCCESS;
}