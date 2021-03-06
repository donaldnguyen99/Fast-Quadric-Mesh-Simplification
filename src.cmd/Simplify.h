/////////////////////////////////////////////
//
// Mesh Simplification Tutorial
//
// (C) by Sven Forstmann in 2014
//
// License : MIT
// http://opensource.org/licenses/MIT
//
//https://github.com/sp4cerat/Fast-Quadric-Mesh-Simplification
//
// 5/2016: Chris Rorden created minimal version for OSX/Linux/Windows compile

//#include <iostream>
//#include <stddef.h>
//#include <functional>
//#include <sys/stat.h>
//#include <stdbool.h>
#include <string.h>
//#include <ctype.h>
//#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <string>
#include <math.h>
#include <float.h> //FLT_EPSILON, DBL_EPSILON
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <cstdlib>
#include "Functions.h"

// #define loopi(start_l,end_l) for ( int i=start_l;i<end_l;++i )
#define loopi(start_l,end_l) for ( int i=start_l;i<int(end_l);++i )
#define loopj(start_l,end_l) for ( int j=start_l;j<int(end_l);++j )
#define loopk(start_l,end_l) for ( int k=start_l;k<end_l;++k )

struct vector3
{
double x, y, z;
};

struct vec3f
{
    double x, y, z;

    inline vec3f( void ) {}

    //inline vec3f operator =( vector3 a )
	// { vec3f b ; b.x = a.x; b.y = a.y; b.z = a.z; return b;}

    inline vec3f( vector3 a )
	 { x = a.x; y = a.y; z = a.z; }

    inline vec3f( const double X, const double Y, const double Z )
    { x = X; y = Y; z = Z; }

    inline vec3f operator + ( const vec3f& a ) const
    { return vec3f( x + a.x, y + a.y, z + a.z ); }

	inline vec3f operator += ( const vec3f& a ) const
    { return vec3f( x + a.x, y + a.y, z + a.z ); }

    inline vec3f operator * ( const double a ) const
    { return vec3f( x * a, y * a, z * a ); }

    inline vec3f operator * ( const vec3f a ) const
    { return vec3f( x * a.x, y * a.y, z * a.z ); }

    inline vec3f v3 () const
    { return vec3f( x , y, z ); }

    inline vec3f operator = ( const vector3 a )
    { x=a.x;y=a.y;z=a.z;return *this; }

	inline bool operator==(const vec3f a) {
        return ((x == a.x) && (y == a.y) && (z == a.z));
    }
	
    inline vec3f operator = ( const vec3f a )
    { x=a.x;y=a.y;z=a.z;return *this; }

    inline vec3f operator / ( const vec3f a ) const
    { return vec3f( x / a.x, y / a.y, z / a.z ); }

    inline vec3f operator - ( const vec3f& a ) const
    { return vec3f( x - a.x, y - a.y, z - a.z ); }

    inline vec3f operator / ( const double a ) const
    { return vec3f( x / a, y / a, z / a ); }

    inline double dot( const vec3f& a ) const
    { return a.x*x + a.y*y + a.z*z; }

    inline vec3f cross( const vec3f& a , const vec3f& b )
    {
		x = a.y * b.z - a.z * b.y;
		y = a.z * b.x - a.x * b.z;
		z = a.x * b.y - a.y * b.x;
		return *this;
	}

    inline double angle( const vec3f& v )
    {
		vec3f a = v , b = *this;
		double dot = v.x*x + v.y*y + v.z*z;
		double len = a.length() * b.length();
		if(len==0)len=0.00001f;
		double input = dot  / len;
		if (input<-1) input=-1;
		if (input>1) input=1;
		return (double) acos ( input );
	}

    inline double angle2( const vec3f& v , const vec3f& w )
    {
		vec3f a = v , b= *this;
		double dot = a.x*b.x + a.y*b.y + a.z*b.z;
		double len = a.length() * b.length();
		if(len==0)len=1;

		vec3f plane; plane.cross( b,w );

		if ( plane.x * a.x + plane.y * a.y + plane.z * a.z > 0 )
			return (double) -acos ( dot  / len );

		return (double) acos ( dot  / len );
	}

    inline vec3f rot_x( double a )
    {
		double yy = cos ( a ) * y + sin ( a ) * z;
		double zz = cos ( a ) * z - sin ( a ) * y;
		y = yy; z = zz;
		return *this;
	}
    inline vec3f rot_y( double a )
    {
		double xx = cos ( -a ) * x + sin ( -a ) * z;
		double zz = cos ( -a ) * z - sin ( -a ) * x;
		x = xx; z = zz;
		return *this;
	}
    inline void clamp( double min, double max )
    {
		if (x<min) x=min;
		if (y<min) y=min;
		if (z<min) z=min;
		if (x>max) x=max;
		if (y>max) y=max;
		if (z>max) z=max;
	}
    inline vec3f rot_z( double a )
    {
		double yy = cos ( a ) * y + sin ( a ) * x;
		double xx = cos ( a ) * x - sin ( a ) * y;
		y = yy; x = xx;
		return *this;
	}
    inline vec3f invert()
	{
		x=-x;y=-y;z=-z;return *this;
	}
    inline vec3f frac()
	{
		return vec3f(
			x-double(int(x)),
			y-double(int(y)),
			z-double(int(z))
			);
	}

    inline vec3f integer()
	{
		return vec3f(
			double(int(x)),
			double(int(y)),
			double(int(z))
			);
	}

    inline double length() const
    {
		return (double)sqrt(x*x + y*y + z*z);
	}

    inline vec3f normalize( double desired_length = 1 )
    {
		double square = sqrt(x*x + y*y + z*z);
		/*
		if (square <= 0.00001f )
		{
			x=1;y=0;z=0;
			return *this;
		}*/
		//double len = desired_length / square;
		x/=square;y/=square;z/=square;

		return *this;
	}
    static vec3f normalize( vec3f a );

	static void random_init();
	static double random_double();
	static vec3f random();

	static int random_number;

	double random_double_01(double a){
		double rnf=a*14.434252+a*364.2343+a*4213.45352+a*2341.43255+a*254341.43535+a*223454341.3523534245+23453.423412;
		int rni=((int)rnf)%100000;
		return double(rni)/(100000.0f-1.0f);
	}

	vec3f random01_fxyz(){
		x=(double)random_double_01(x);
		y=(double)random_double_01(y);
		z=(double)random_double_01(z);
		return *this;
	}

};

vec3f barycentric(const vec3f &p, const vec3f &a, const vec3f &b, const vec3f &c){
	vec3f v0 = b-a;
	vec3f v1 = c-a;
	vec3f v2 = p-a;
	double d00 = v0.dot(v0);
	double d01 = v0.dot(v1);
	double d11 = v1.dot(v1);
	double d20 = v2.dot(v0);
	double d21 = v2.dot(v1);
	double denom = d00*d11-d01*d01;
	double v = (d11 * d20 - d01 * d21) / denom;
	double w = (d00 * d21 - d01 * d20) / denom;
	double u = 1.0 - v - w;
	return vec3f(u,v,w);
}

vec3f interpolate(const vec3f &p, const vec3f &a, const vec3f &b, const vec3f &c, const vec3f attrs[3])
{
	vec3f bary = barycentric(p,a,b,c);
	vec3f out = vec3f(0,0,0);
	out = out + attrs[0] * bary.x;
	out = out + attrs[1] * bary.y;
	out = out + attrs[2] * bary.z;
	return out;
}

double min(double v1, double v2) {
	return fmin(v1,v2);
}


class SymetricMatrix {

	public:

	// Constructor

	SymetricMatrix(double c=0) { loopi(0,10) m[i] = c;  }

	SymetricMatrix(	double m11, double m12, double m13, double m14,
			            double m22, double m23, double m24,
			                        double m33, double m34,
			                                    double m44) {
			 m[0] = m11;  m[1] = m12;  m[2] = m13;  m[3] = m14;
			              m[4] = m22;  m[5] = m23;  m[6] = m24;
			                           m[7] = m33;  m[8] = m34;
			                                        m[9] = m44;
	}

	// Make plane

	SymetricMatrix(double a,double b,double c,double d)
	{
		m[0] = a*a;  m[1] = a*b;  m[2] = a*c;  m[3] = a*d;
		             m[4] = b*b;  m[5] = b*c;  m[6] = b*d;
		                          m[7 ] =c*c; m[8 ] = c*d;
		                                       m[9 ] = d*d;
	}

	double operator[](int c) const { return m[c]; }

	// Determinant

	double det(	int a11, int a12, int a13,
				int a21, int a22, int a23,
				int a31, int a32, int a33)
	{
		double det =  m[a11]*m[a22]*m[a33] + m[a13]*m[a21]*m[a32] + m[a12]*m[a23]*m[a31]
					- m[a13]*m[a22]*m[a31] - m[a11]*m[a23]*m[a32]- m[a12]*m[a21]*m[a33];
		return det;
	}

	const SymetricMatrix operator+(const SymetricMatrix& n) const
	{
		return SymetricMatrix( m[0]+n[0],   m[1]+n[1],   m[2]+n[2],   m[3]+n[3],
						                    m[4]+n[4],   m[5]+n[5],   m[6]+n[6],
						                                 m[ 7]+n[ 7], m[ 8]+n[8 ],
						                                              m[ 9]+n[9 ]);
	}

	SymetricMatrix& operator+=(const SymetricMatrix& n)
	{
		 m[0]+=n[0];   m[1]+=n[1];   m[2]+=n[2];   m[3]+=n[3];
		 m[4]+=n[4];   m[5]+=n[5];   m[6]+=n[6];   m[7]+=n[7];
		 m[8]+=n[8];   m[9]+=n[9];
		return *this;
	}

	double m[10];
};
///////////////////////////////////////////

namespace Simplify
{
	// Global Variables & Strctures
	enum Attributes {
		NONE,
		NORMAL = 2,
		TEXCOORD = 4,
		COLOR = 8
	};
	struct Triangle { int v[3];double err[4];int deleted,dirty,attr;vec3f n;vec3f uvs[3];int material; };
	struct Vertex { vec3f p;int tstart,tcount;SymetricMatrix q;int border;};
	struct Ref { int tid,tvertex; };
	struct Region {
		double regionTarget;
		bool regionBound;
		double (*func)(double, double, double, double, double, double, double, double, bool);
		vec3f coord;
		double radius;
		double scale;
		double power;
		bool isNegative;
		bool done;
		int startCount;
		int endCount;
	};
	std::vector<Region> regions;
	std::vector<Triangle> triangles;
	std::vector<Vertex> vertices;
	std::vector<Ref> refs;
    std::string mtllib;
    std::vector<std::string> materials;

	/*class AdjList {
	public:
		// map of start vertex id (int) to set of end vertex ids (int)
		// should use hash key instead of start vertex id (int)
		unordered_map<int, unordered_set<int> > edgeList;

		AdjList() {};
		AdjList(const vector<Triangle>& triangles) {
			for (size_t i = 0; i < triangles.size(); i++) {
				for (int j = 0; j < 3; j++) {
					unordered_map<int, unordered_set<int> >::iterator edgeIt = edgeList.find(triangles[i].v[j]);
					if (edgeIt != edgeList.end()) { // if key is found
						for (int k = 0; k < 3; k++) { // v[k] will be other v
							if (k != j) {
								(edgeIt->second).insert(triangles[i].v[k]); // insert(triangles[i].v[k]);
							}
						}
					} else { // if key is NOT found
					
						// insert a new mapping to edgeList
						vector<int> tempV;
						for (int k = 0; k < 3; k++) { // v[k] will be other v
							if (k != j) {
								tempV.push_back(triangles[i].v[k]); // insert(triangles[i].v[k]);
							}
						}
						unordered_set<int> tempSet(tempV.begin(), tempV.end());
						edgeList.emplace_hint(edgeIt, triangles[i].v[j], tempSet);
						;
					}
				}
			}
		}
		~AdjList() {};

		void print_edges() {
			for (auto it = edgeList.begin(); it != edgeList.end(); it++) {
				cout << "Key: " << it->first << " => " << "Set: ";
				for (auto itSet = (it->second).begin(); itSet != (it->second).end(); itSet++) {
					cout << *itSet << " ";
				}
				cout << endl;
			}
		}
		void write_edges(const char* filename, bool verbose=false, int verboselines=10000) {
			FILE *file=fopen(filename, "w");

			if (!file) {
				printf("write_edges: can't write data file \"%s\".\n", filename);
				exit(0);
			}
			int ind = 1;
			for (auto it = edgeList.begin(); it != edgeList.end(); it++, ind++) {
				unordered_set<int> * p_currentMap = &(edgeList[ind].second);
				for (auto it = edgeList[ind].second.begin(); it != edgeList[ind])
				fprintf(file, "%d", edgeList[ind].second); // Update here, below is for example (from write_obj)
			}
			double totalvertices = double(vertices.size());
			loopi(0,vertices.size())
			{
				if(verbose && (i%verboselines==0)) printf("Vertices written: %d, %.2lf%% of vertices\n", i, double(i)/totalvertices*100);
				fprintf(file, "v %lf %lf %lf\n", vertices[i].p.x,vertices[i].p.y,vertices[i].p.z);
			}
			if (has_uv)
			{
				loopi(0,triangles.size()) if(!triangles[i].deleted)
				{
					fprintf(file, "vt %lf %lf\n", triangles[i].uvs[0].x, triangles[i].uvs[0].y);
					fprintf(file, "vt %lf %lf\n", triangles[i].uvs[1].x, triangles[i].uvs[1].y);
					fprintf(file, "vt %lf %lf\n", triangles[i].uvs[2].x, triangles[i].uvs[2].y);
				}
			}
			int uv = 1;
			double totaltriangles = double(triangles.size());
			loopi(0,triangles.size()) if(!triangles[i].deleted)
			{
				if(verbose && (i%verboselines==0)) printf("Triangles written: %d, %.2lf%% of triangles\n", i, double(i)/totaltriangles*100);
				if (triangles[i].material != cur_material)
				{
					cur_material = triangles[i].material;
					fprintf(file, "usemtl %s\n", materials[triangles[i].material].c_str());
				}
				if (has_uv)
				{
					fprintf(file, "f %d/%d %d/%d %d/%d\n", triangles[i].v[0]+1, uv, triangles[i].v[1]+1, uv+1, triangles[i].v[2]+1, uv+2);
					uv += 3;
				}
				else
				{
					fprintf(file, "f %d %d %d\n", triangles[i].v[0]+1, triangles[i].v[1]+1, triangles[i].v[2]+1);
				}
				//fprintf(file, "f %d// %d// %d//\n", triangles[i].v[0]+1, triangles[i].v[1]+1, triangles[i].v[2]+1); //more compact: remove trailing zeros
			}
			fclose(file);
		}
	};*/

	// Helper functions

	double vertex_error(SymetricMatrix q, double x, double y, double z);
	double calculate_error(int , int , vec3f &);
	bool flipped(vec3f p,int i0,int i1,Vertex &v0,Vertex &v1,std::vector<int> &deleted);
	void update_uvs(int i0,const Vertex &v,const vec3f &p,std::vector<int> &deleted);
	void update_triangles(int i0,Vertex &v,std::vector<int> &deleted,int &deleted_triangles);
	void update_mesh(int iteration);
	void compact_mesh();
	int consecutiveNoDeletionThreshold = 10000;
	int initialTotalCount = 0;
	int initialRegionCount = 0;
	int currentRegionCount = 0;
	bool regionDone = false;
	bool anyRegionsBound = false;
	double currentRegionRatio;
	double currentOutsideRatio;
	double target_region_ratio;
	double target_outside_ratio;
	double magnification;
	int load_txt(const char* filename, bool verbose);
	bool inRegion(Triangle &t, double coord[], double radius);
	double thresholdRegionsBound(Triangle &t, double &threshold0);
	double thresholdAllCenters(Triangle &t, double &threshold0);
	int currentCountInRegion(Region &region0);
	bool allRegionsDone();
	//
	// Main simplification function
	//
	// target_count  : target nr. of triangles
	// agressiveness : sharpness to increase the threshold.
	//                 5..8 are good numbers
	//                 more iterations yield higher quality
	//

	void simplify_mesh(double coord[3], int target_count, double agressiveness=7, bool verbose=false,
		double (*func)(double, double, double, double, double, double, double, double, bool)=constantFunc,
		double radius=def_radius, double scale=def_scale, double power=1, bool isneg=false,
		bool doRegionSimplification=false, bool doMultipleCenters=false)
	{
		// init
		loopi(0,triangles.size())
        {
            triangles[i].deleted=0;
        }

		// main iteration loop
		int deleted_triangles=0;
		int deleted_triangles_before=-1;
		int deleted_triangles_after=-1;
		std::vector<int> deleted0,deleted1;
		int triangle_count=triangles.size();
		int consecutiveNoDeletion = 0;
		double threshold0;
		double threshold;
		if (doRegionSimplification && !doMultipleCenters) {
			regions.clear();
			Region singleRegion = {target_region_ratio, false, func, 
			vec3f(coord[0], coord[1], coord[2]), radius, scale, power, isneg,
			regionDone, initialTotalCount, 0};
			regions.push_back(singleRegion);
			currentRegionCount = currentCountInRegion(regions[0]);
			currentRegionRatio = double(currentRegionCount)/double(initialRegionCount);
			if (currentRegionRatio <= target_region_ratio) {
				// printf("Inside Region Ratio: %f\n", currentRegionRatio);
				regionDone = true;
			}
		} else if (doMultipleCenters) {
			doRegionSimplification = false;
			// regions will have been assigned in main
			loopi(0, int(regions.size())) {
				regions[i].endCount = currentCountInRegion(regions[i]);
				regions[i].startCount = regions[i].endCount;
			}
		}
		//int iteration = 0;
		//loop(iteration,0,100)
		//double initial_agressiveness = agressiveness;
		bool printRegionDone = false;
		int printwidth = int(log10(triangle_count)+1);
		for (int iteration = 0; iteration < triangle_count; iteration ++)
		{
			deleted_triangles_before = deleted_triangles;
			if(triangle_count-deleted_triangles<=target_count)break;
			currentRegionCount = 0;
			if (doRegionSimplification) {
				for (int i = 0; i < (int)(triangles.size()); i++) {
            		if (inRegion(triangles[i], coord, radius)) currentRegionCount++;
        		}
				currentRegionRatio = double(currentRegionCount)/double(initialRegionCount);
				if(currentRegionRatio <= target_region_ratio) {
					// printf("Inside Region Ratio: %f\n", currentRegionRatio);
					regionDone = true;
				}
			}
			//if(regionDone) agressiveness=initial_agressiveness + 3;
			// update mesh once in a while
			if(iteration%5==0)
			{
				update_mesh(iteration);
			}

			// clear dirty flag
			loopi(0,triangles.size()) triangles[i].dirty=0;

			//
			// All triangles with edges below the threshold will be removed
			//
			// The following numbers works well for most models.
			// If it does not, try to adjust the 3 parameters
			//
			threshold0 = 0.000000001*pow(double(iteration+3),agressiveness);
			// target number of triangles reached ? Then break
			if ((verbose) && (iteration%5==0)) {
				printf("iteration %*d - total triangles %*d, iter's threshold = (iter+3)^aggr*e-09 = %g\n",printwidth, iteration, printwidth, triangle_count-deleted_triangles, threshold0);
				// printf(" triangle.size(): %f\n", double(triangles.size()));
				// printf(" Count: %lli, Region inside radius reduced to %f\n", currentRegionCount, double(currentRegionCount)/double(initialRegionCount));
			}
			bool breakIteration = false;
			// remove vertices & mark deleted triangles
			loopi(0,triangles.size())
			{
				Triangle &t=triangles[i];
				if(t.deleted) continue;
				if(t.dirty) continue;
				//bool multipleCentersDone = true;
				
				if (doMultipleCenters) {
					// 
					// Doing Multiple Bound/Unbound Centers (option -L)
					// 
					if (anyRegionsBound) { // Fix here
						threshold = thresholdRegionsBound(t, threshold0);
					} else {
						threshold = thresholdAllCenters(t, threshold0);
					}
				} else if (doRegionSimplification && regionDone) {
					// 
					// Doing Single Bound Region (option -T)
					// 
					if(inRegion(t, coord, radius)) {
						threshold = 0;
					} else {
					double squareFactor = square(vertices[t.v[0]].p.x, vertices[t.v[0]].p.y, vertices[t.v[0]].p.z,
					coord[0], coord[1], coord[2], radius, 1.0, false);
					if (func != constantFunc) {
						if ((fabs(squareFactor - 0)) <= 0.000000001) {
							threshold = 0;
						} else {
							threshold = threshold0*pow(func(
							vertices[t.v[0]].p.x, vertices[t.v[0]].p.y, vertices[t.v[0]].p.z,
							coord[0], coord[1], coord[2],
							radius, scale, isneg), power)*squareFactor;
						}
					} else {
						threshold = threshold0*squareFactor;
					}
					}
				} else {
					// 
					// Doing Whole Mesh (default option)
					// 
					if (func != constantFunc) {
						threshold = threshold0*pow(func(
						vertices[t.v[0]].p.x, vertices[t.v[0]].p.y, vertices[t.v[0]].p.z,
						coord[0], coord[1], coord[2],
						radius, scale, isneg), power);
					} else {
						threshold = threshold0;
					}
					//printf("vertices.size() = %d, triangles.size() = %d\n", int(vertices.size()), int(triangles.size()));
				}
				
				
				if(t.err[3]>threshold) continue;

				loopj(0,3)if(t.err[j]<threshold)
				{

					int i0=t.v[ j     ]; Vertex &v0 = vertices[i0];
					int i1=t.v[(j+1)%3]; Vertex &v1 = vertices[i1];
					// Border check
					if(v0.border != v1.border)  continue;

					// Compute vertex to collapse to
					vec3f p;
					calculate_error(i0,i1,p);
					deleted0.resize(v0.tcount); // normals temporarily
					deleted1.resize(v1.tcount); // normals temporarily
					// don't remove if flipped
					if( flipped(p,i0,i1,v0,v1,deleted0) ) continue;

					if( flipped(p,i1,i0,v1,v0,deleted1) ) continue;

					if ( (t.attr & TEXCOORD) == TEXCOORD  )
					{
						update_uvs(i0,v0,p,deleted0);
						update_uvs(i0,v1,p,deleted1);
					}

					// not flipped, so remove edge
					v0.p=p;
					v0.q=v1.q+v0.q;
					int tstart=refs.size();

					update_triangles(i0,v0,deleted0,deleted_triangles);
					update_triangles(i0,v1,deleted1,deleted_triangles);

					int tcount=refs.size()-tstart;

					if(tcount<=v0.tcount)
					{
						// save ram
						if(tcount)memcpy(&refs[v0.tstart],&refs[tstart],tcount*sizeof(Ref));
					}
					else
						// append
						v0.tstart=tstart;

					v0.tcount=tcount;
					break;
				}
				// done?
				//if(triangle_count-deleted_triangles<=target_count) break;
				if(!breakIteration && (triangle_count-deleted_triangles<=target_count)) breakIteration = true; // may delete entire mesh
			}
			// 
			// Here is at the end of iteration
			// 
			deleted_triangles_after = deleted_triangles;
			if(deleted_triangles_before == deleted_triangles_after) {
				consecutiveNoDeletion++;
				if(verbose && (iteration%5==0)) printf("No triangles deleted yet.\n");
				if(consecutiveNoDeletion >= consecutiveNoDeletionThreshold) {
					consecutiveNoDeletion = 0;
					break;
				}
			}
			if(doRegionSimplification && regionDone) {
				//currentRegionCount = currentCountInRegion(regions[0]);
				currentRegionCount = 0;
				for (int i = 0; i < (int)(triangles.size()); i++) {
            		if (inRegion(triangles[i], coord, radius)) currentRegionCount++;
        		}
				currentRegionRatio = double(currentRegionCount)/double(initialRegionCount);
				int initialOutsideCount = initialTotalCount - initialRegionCount;
				int currentOutsideCount = (int)(triangles.size()) - currentRegionCount;
				currentOutsideRatio = double(currentOutsideCount)/double(initialOutsideCount);
				if (currentOutsideRatio <= target_outside_ratio) break;
			} else if (doMultipleCenters) {
				if (allRegionsDone()) {
					//currentCountInAllRegions(); // Fix here
					break;
				}
			}
			if(!printRegionDone && regionDone) {
				printf("  Region is done. Region reduction: %.8lf (%d triangles)\n", currentRegionRatio, currentRegionCount);
				printRegionDone = true;
			}
			if(breakIteration) break;
		}
		// clean up mesh
		// if (doRegionSimplification && regionDone) printf("Inside Region Reduction: %.8lf (%d triangles), Outside Region Reduction: %.8lf (%d triangles)\n",
		// 		 currentRegionRatio, currentRegionCount, currentOutsideRatio, (int)(triangles.size()) - currentRegionCount);
		// printf("Final tri count: %lli, Region inside radius reduced to %f\n", currentRegionCount, double(currentRegionCount)/double(initialRegionCount));
		compact_mesh();
	} //simplify_mesh()

	void simplify_mesh_lossless(bool verbose=false)
	{
		// init
		loopi(0,triangles.size()) triangles[i].deleted=0;

		// main iteration loop
		int deleted_triangles=0;
		std::vector<int> deleted0,deleted1;
		//int triangle_count=triangles.size(); // unused variable in function
		//int iteration = 0;
		//loop(iteration,0,100)
		for (int iteration = 0; iteration < 9999; iteration ++)
		{
			// update mesh constantly
			update_mesh(iteration);
			// clear dirty flag
			loopi(0,triangles.size()) triangles[i].dirty=0;
			//
			// All triangles with edges below the threshold will be removed
			//
			// The following numbers works well for most models.
			// If it does not, try to adjust the 3 parameters
			//
			double threshold = DBL_EPSILON; //1.0E-3 EPS;
			if (verbose) {
				printf("lossless iteration %d\n", iteration);
			}

			// remove vertices & mark deleted triangles
			loopi(0,triangles.size())
			{
				Triangle &t=triangles[i];
				if(t.err[3]>threshold) continue;
				if(t.deleted) continue;
				if(t.dirty) continue;

				loopj(0,3)if(t.err[j]<threshold)
				{
					int i0=t.v[ j     ]; Vertex &v0 = vertices[i0];
					int i1=t.v[(j+1)%3]; Vertex &v1 = vertices[i1];

					// Border check
					if(v0.border != v1.border)  continue;

					// Compute vertex to collapse to
					vec3f p;
					calculate_error(i0,i1,p);

					deleted0.resize(v0.tcount); // normals temporarily
					deleted1.resize(v1.tcount); // normals temporarily

					// don't remove if flipped
					if( flipped(p,i0,i1,v0,v1,deleted0) ) continue;
					if( flipped(p,i1,i0,v1,v0,deleted1) ) continue;

					if ( (t.attr & TEXCOORD) == TEXCOORD )
					{
						update_uvs(i0,v0,p,deleted0);
						update_uvs(i0,v1,p,deleted1);
					}

					// not flipped, so remove edge
					v0.p=p;
					v0.q=v1.q+v0.q;
					int tstart=refs.size();

					update_triangles(i0,v0,deleted0,deleted_triangles);
					update_triangles(i0,v1,deleted1,deleted_triangles);

					int tcount=refs.size()-tstart;

					if(tcount<=v0.tcount)
					{
						// save ram
						if(tcount)memcpy(&refs[v0.tstart],&refs[tstart],tcount*sizeof(Ref));
					}
					else
						// append
						v0.tstart=tstart;

					v0.tcount=tcount;
					break;
				}
			}
			if(deleted_triangles<=0)break;
			deleted_triangles=0;
		} //for each iteration
		// clean up mesh
		compact_mesh();
	} //simplify_mesh_lossless()

	// Check if a triangle flips when this edge is removed

	bool flipped(vec3f p,int i0,int i1,Vertex &v0,Vertex &v1,std::vector<int> &deleted)
	{

		loopk(0,v0.tcount)
		{
			Triangle &t=triangles[refs[v0.tstart+k].tid];
			if(t.deleted)continue;

			int s=refs[v0.tstart+k].tvertex;
			int id1=t.v[(s+1)%3];
			int id2=t.v[(s+2)%3];

			if(id1==i1 || id2==i1) // delete ?
			{

				deleted[k]=1;
				continue;
			}
			vec3f d1 = vertices[id1].p-p; d1.normalize();
			vec3f d2 = vertices[id2].p-p; d2.normalize();
			if(fabs(d1.dot(d2))>0.999) return true;
			vec3f n;
			n.cross(d1,d2);
			n.normalize();
			deleted[k]=0;
			if(n.dot(t.n)<0.2) return true;
		}
		return false;
	}

    // update_uvs

	void update_uvs(int i0,const Vertex &v,const vec3f &p,std::vector<int> &deleted)
	{
		loopk(0,v.tcount)
		{
			Ref &r=refs[v.tstart+k];
			Triangle &t=triangles[r.tid];
			if(t.deleted)continue;
			if(deleted[k])continue;
			vec3f p1=vertices[t.v[0]].p;
			vec3f p2=vertices[t.v[1]].p;
			vec3f p3=vertices[t.v[2]].p;
			t.uvs[r.tvertex] = interpolate(p,p1,p2,p3,t.uvs);
		}
	}

	// Update triangle connections and edge error after a edge is collapsed

	void update_triangles(int i0,Vertex &v,std::vector<int> &deleted,int &deleted_triangles)
	{
		vec3f p;
		loopk(0,v.tcount)
		{
			Ref &r=refs[v.tstart+k];
			Triangle &t=triangles[r.tid];
			if(t.deleted)continue;
			if(deleted[k])
			{
				t.deleted=1;
				deleted_triangles++;
				continue;
			}
			t.v[r.tvertex]=i0;
			t.dirty=1;
			t.err[0]=calculate_error(t.v[0],t.v[1],p);
			t.err[1]=calculate_error(t.v[1],t.v[2],p);
			t.err[2]=calculate_error(t.v[2],t.v[0],p);
			t.err[3]=min(t.err[0],min(t.err[1],t.err[2]));
			refs.push_back(r);
		}
	}

	// compact triangles, compute edge error and build reference list

	void update_mesh(int iteration)
	{
		if(iteration>0) // compact triangles
		{
			int dst=0;
			loopi(0,triangles.size())
			if(!triangles[i].deleted)
			{
				triangles[dst++]=triangles[i];
			}
			triangles.resize(dst);
		}
		//
		// Init Quadrics by Plane & Edge Errors
		//
		// required at the beginning ( iteration == 0 )
		// recomputing during the simplification is not required,
		// but mostly improves the result for closed meshes
		//
		if( iteration == 0 )
		{
			loopi(0,vertices.size())
			vertices[i].q=SymetricMatrix(0.0);

			loopi(0,triangles.size())
			{
				Triangle &t=triangles[i];
				vec3f n,p[3];
				loopj(0,3) p[j]=vertices[t.v[j]].p;
				n.cross(p[1]-p[0],p[2]-p[0]);
				n.normalize();
				t.n=n;
				loopj(0,3) vertices[t.v[j]].q =
					vertices[t.v[j]].q+SymetricMatrix(n.x,n.y,n.z,-n.dot(p[0]));
			}
			loopi(0,triangles.size())
			{
				// Calc Edge Error
				Triangle &t=triangles[i];vec3f p;
				loopj(0,3) t.err[j]=calculate_error(t.v[j],t.v[(j+1)%3],p);
				t.err[3]=min(t.err[0],min(t.err[1],t.err[2]));
			}
		}

		// Init Reference ID list
		loopi(0,vertices.size())
		{
			vertices[i].tstart=0;
			vertices[i].tcount=0;
		}
		loopi(0,triangles.size())
		{
			Triangle &t=triangles[i];
			loopj(0,3) vertices[t.v[j]].tcount++;
		}
		int tstart=0;
		loopi(0,vertices.size())
		{
			Vertex &v=vertices[i];
			v.tstart=tstart;
			tstart+=v.tcount;
			v.tcount=0;
		}

		// Write References
		refs.resize(triangles.size()*3);
		loopi(0,triangles.size())
		{
			Triangle &t=triangles[i];
			loopj(0,3)
			{
				Vertex &v=vertices[t.v[j]];
				refs[v.tstart+v.tcount].tid=i;
				refs[v.tstart+v.tcount].tvertex=j;
				v.tcount++;
			}
		}

		// Identify boundary : vertices[].border=0,1
		if( iteration == 0 )
		{
			std::vector<int> vcount,vids;

			loopi(0,vertices.size())
				vertices[i].border=0;

			loopi(0,vertices.size())
			{
				Vertex &v=vertices[i];
				vcount.clear();
				vids.clear();
				loopj(0,v.tcount)
				{
					int k=refs[v.tstart+j].tid;
					Triangle &t=triangles[k];
					loopk(0,3)
					{
						int ofs=0,id=t.v[k];
						while((size_t)ofs<vcount.size())
						{
							if(vids[ofs]==id)break;
							ofs++;
						}
						if((size_t)ofs==vcount.size())
						{
							vcount.push_back(1);
							vids.push_back(id);
						}
						else
							vcount[ofs]++;
					}
				}
				loopj(0,vcount.size()) if(vcount[j]==1)
					vertices[vids[j]].border=1;
			}
		}
	}

	// Finally compact mesh before exiting

	void compact_mesh()
	{
		int dst=0;
		loopi(0,vertices.size())
		{
			vertices[i].tcount=0;
		}
		loopi(0,triangles.size())
		if(!triangles[i].deleted)
		{
			Triangle &t=triangles[i];
			triangles[dst++]=t;
			loopj(0,3)vertices[t.v[j]].tcount=1;
		}
		triangles.resize(dst);
		dst=0;
		loopi(0,vertices.size())
		if(vertices[i].tcount)
		{
			vertices[i].tstart=dst;
			vertices[dst].p=vertices[i].p;
			dst++;
		}
		loopi(0,triangles.size())
		{
			Triangle &t=triangles[i];
			loopj(0,3)t.v[j]=vertices[t.v[j]].tstart;
		}
		vertices.resize(dst);
	}

	// Error between vertex and Quadric

	double vertex_error(SymetricMatrix q, double x, double y, double z)
	{
 		return   q[0]*x*x + 2*q[1]*x*y + 2*q[2]*x*z + 2*q[3]*x + q[4]*y*y
 		     + 2*q[5]*y*z + 2*q[6]*y + q[7]*z*z + 2*q[8]*z + q[9];
	}

	// Error for one edge

	double calculate_error(int id_v1, int id_v2, vec3f &p_result)
	{
		// compute interpolated vertex

		SymetricMatrix q = vertices[id_v1].q + vertices[id_v2].q;
		bool   border = vertices[id_v1].border & vertices[id_v2].border;
		double error=0;
		double det = q.det(0, 1, 2, 1, 4, 5, 2, 5, 7);
		if ( det != 0 && !border )
		{

			// q_delta is invertible
			p_result.x = -1/det*(q.det(1, 2, 3, 4, 5, 6, 5, 7 , 8));	// vx = A41/det(q_delta)
			p_result.y =  1/det*(q.det(0, 2, 3, 1, 5, 6, 2, 7 , 8));	// vy = A42/det(q_delta)
			p_result.z = -1/det*(q.det(0, 1, 3, 1, 4, 6, 2, 5,  8));	// vz = A43/det(q_delta)

			error = vertex_error(q, p_result.x, p_result.y, p_result.z);
		}
		else
		{
			// det = 0 -> try to find best result
			vec3f p1=vertices[id_v1].p;
			vec3f p2=vertices[id_v2].p;
			vec3f p3=(p1+p2)/2;
			double error1 = vertex_error(q, p1.x,p1.y,p1.z);
			double error2 = vertex_error(q, p2.x,p2.y,p2.z);
			double error3 = vertex_error(q, p3.x,p3.y,p3.z);
			error = min(error1, min(error2, error3));
			if (error1 == error) p_result=p1;
			if (error2 == error) p_result=p2;
			if (error3 == error) p_result=p3;
		}
		return error;
	}

	char *trimwhitespace(char *str)
	{
		char *end;

		// Trim leading space
		while(isspace((unsigned char)*str)) str++;

		if(*str == 0) { // All spaces?
			return str;
		}

		// Trim trailing space
		end = str + strlen(str) - 1;
		while(end > str && isspace((unsigned char)*end)) end--;

		// Write new null terminator
		*(end+1) = 0;

		return str;
	}

	//Option : Load OBJ
	void load_obj(const char* filename, bool verbose=false, int verboselines=10000, bool process_uv=false) {
		vertices.clear();
		triangles.clear();
		//printf ( "Loading Objects %s ... \n",filename);
		FILE* fn;
		if(filename==NULL)		return ;
		if((char)filename[0]==0)	return ;
		if ((fn = fopen(filename, "rb")) == NULL)
		{
			printf ( "File %s not found!\n" ,filename );
			return;
		}
		char line[1000];
		memset ( line,0,1000 );
		int vertex_cnt = 0;
		int material = -1;
		std::map<std::string, int> material_map;
		std::vector<vec3f> uvs;
		std::vector<std::vector<int> > uvMap;
		int line_index = 0;
		while(fgets( line, 1000, fn ) != NULL)
		{
			Vertex v;
			vec3f uv;

			if (strncmp(line, "mtllib", 6) == 0)
			{
				mtllib = trimwhitespace(&line[7]);
			}
			if (strncmp(line, "usemtl", 6) == 0)
			{
				std::string usemtl = trimwhitespace(&line[7]);
				if (material_map.find(usemtl) == material_map.end())
				{
					material_map[usemtl] = materials.size();
					materials.push_back(usemtl);
				}
				material = material_map[usemtl];
			}

			if ( line[0] == 'v' && line[1] == 't' )
			{
				if ( line[2] == ' ' ) {
				if(sscanf(line,"vt %lf %lf",
					&uv.x,&uv.y)==2)
				{
					uv.z = 0;
					uvs.push_back(uv);
				}
				} else {
				if(sscanf(line,"vt %lf %lf %lf",
					&uv.x,&uv.y,&uv.z)==3)
				{
					uvs.push_back(uv);
				}
				}
			}
			else if ( line[0] == 'v' )
			{
				if ( line[1] == ' ' )
				if(sscanf(line,"v %lf %lf %lf",
					&v.p.x,	&v.p.y,	&v.p.z)==3)
				{
					vertices.push_back(v);
				}
			}
			int integers[9];
			if ( line[0] == 'f' )
			{
				Triangle t;
				bool tri_ok = false;
                bool has_uv = false;

				if(sscanf(line,"f %d %d %d",
					&integers[0],&integers[1],&integers[2])==3)
				{
					tri_ok = true;
				}else
				if(sscanf(line,"f %d// %d// %d//",
					&integers[0],&integers[1],&integers[2])==3)
				{
					tri_ok = true;
				}else
				if(sscanf(line,"f %d//%d %d//%d %d//%d",
					&integers[0],&integers[3],
					&integers[1],&integers[4],
					&integers[2],&integers[5])==6)
				{
					tri_ok = true;
				}else
				if(sscanf(line,"f %d/%d/%d %d/%d/%d %d/%d/%d",
					&integers[0],&integers[6],&integers[3],
					&integers[1],&integers[7],&integers[4],
					&integers[2],&integers[8],&integers[5])==9)
				{
					tri_ok = true;
					has_uv = true;
				}
				else
				{
					printf("unrecognized sequence\n");
					printf("%s\n",line);
					while(1);
				}
				if ( tri_ok )
				{
					t.v[0] = integers[0]-1-vertex_cnt;
					t.v[1] = integers[1]-1-vertex_cnt;
					t.v[2] = integers[2]-1-vertex_cnt;
					t.attr = 0;

					if ( process_uv && has_uv )
					{
						std::vector<int> indices;
						indices.push_back(integers[6]-1-vertex_cnt);
						indices.push_back(integers[7]-1-vertex_cnt);
						indices.push_back(integers[8]-1-vertex_cnt);
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
            	printf("obj lines read: %d\n", line_index);
		}

		if ( process_uv && uvs.size() )
		{
			loopi(0,triangles.size())
			{
				loopj(0,3)
				triangles[i].uvs[j] = uvs[uvMap[i][j]];
			}
		}

		fclose(fn);

		//printf("load_obj: vertices = %lu, triangles = %lu, uvs = %lu\n", vertices.size(), triangles.size(), uvs.size() );
	} // load_obj()

	// Option: Load Tri10	ex.	v0x	v0y	v0z	v1x v1y v1z v2x v2y v2z q
	void load_tri10(const char* filename, bool verbose=false, int verboselines=10000) {
		printf("Loading %s ...\n", filename);
		FILE *fn;
		if ((filename == NULL) || ((char)filename[0] == 0) || ((fn = fopen(filename, "rb")) == NULL)) {
			printf("File %s not found!\n", filename);
			exit(EXIT_FAILURE);
		}
		char line[1000];
		memset(line, 0, 1000);
		bool is12char = false;
		if (fgets(line, 1000, fn) != NULL) {
			double totallines, magnification, moreArgs;
			double arrArgs[10];
			if (verbose && (sscanf(line, "%lf %lf %lf", &totallines, &magnification, &moreArgs) == 2)) {
				printf("tri file header:\n Polygons: %lf, Magnification (ignored): %lf\n", totallines, magnification);
			} else if (sscanf(line, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &(arrArgs[0]), &(arrArgs[1]), &(arrArgs[2]), 
					&(arrArgs[3]), &(arrArgs[4]), &(arrArgs[5]), &(arrArgs[6]), &(arrArgs[7]), &(arrArgs[8]), &(arrArgs[9])) >= 9) {
				is12char = false;
				rewind(fn);
			} else if (sscanf(line, "%12lf%12lf%12lf%12lf%12lf%12lf%12lf%12lf%12lf%12lf", &(arrArgs[0]), &(arrArgs[1]), &(arrArgs[2]), 
					&(arrArgs[3]), &(arrArgs[4]), &(arrArgs[5]), &(arrArgs[6]), &(arrArgs[7]), &(arrArgs[8]), &(arrArgs[9])) >= 9) {
				is12char = true;
				rewind(fn);
			} else {
				int rewindcount = 1;
				int maxlinestocheckformat = 100;
				for(int i = 0; (i < maxlinestocheckformat) && (fgets(line, 1000, fn) != NULL); i++) {
					rewindcount++;
					if (sscanf(line, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &(arrArgs[0]), &(arrArgs[1]), &(arrArgs[2]), 
							&(arrArgs[3]), &(arrArgs[4]), &(arrArgs[5]), &(arrArgs[6]), &(arrArgs[7]), &(arrArgs[8]), &(arrArgs[9])) >= 9) {
						is12char = false;
						break;
					} else if (sscanf(line, "%12lf%12lf%12lf%12lf%12lf%12lf%12lf%12lf%12lf%12lf", &(arrArgs[0]), &(arrArgs[1]), &(arrArgs[2]), 
							&(arrArgs[3]), &(arrArgs[4]), &(arrArgs[5]), &(arrArgs[6]), &(arrArgs[7]), &(arrArgs[8]), &(arrArgs[9])) >= 9) {
						is12char = true;
						break;
					}
					if (i == maxlinestocheckformat-1) printf("Format cannot be interpreted (using top %d lines).\n", maxlinestocheckformat);
				}
				for(int i = 0; i < rewindcount; i++) {rewind(fn);}
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
			bool sscanfcondition;
			if (is12char) {
				sscanfcondition = (sscanf(line, "%12lf%12lf%12lf%12lf%12lf%12lf%12lf%12lf%12lf%12lf", 
					&v0.p.x, &v0.p.y, &v0.p.z,
					&v1.p.x, &v1.p.y, &v1.p.z,
					&v2.p.x, &v2.p.y, &v2.p.z, &quality) >= 9);
			} else {
				sscanfcondition = (sscanf(line, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
					&v0.p.x, &v0.p.y, &v0.p.z,
					&v1.p.x, &v1.p.y, &v1.p.z,
					&v2.p.x, &v2.p.y, &v2.p.z, &quality) >= 9);
			}
			// if (sscanf(line, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", // originally
			//         &v0.p.x, &v0.p.y, &v0.p.z,
			//         &v1.p.x, &v1.p.y, &v1.p.z,
			//         &v2.p.x, &v2.p.y, &v2.p.z, &quality) >= 9) {
			if (sscanfcondition) {
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
		if(line_index < 1) {
			printf("Could not read file. Should be whitespace delimited or 12 character columns.\n");
			exit(EXIT_FAILURE);
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

	// Optional : Store as OBJ
	void write_obj(const char* filename, bool verbose=false, int verboselines=10000) {
		FILE *file=fopen(filename, "w");
		int cur_material = -1;
		bool has_uv = (triangles.size() && (triangles[0].attr & TEXCOORD) == TEXCOORD);

		if (!file)
		{
			printf("write_obj: can't write data file \"%s\".\n", filename);
			exit(0);
		}
		if (!mtllib.empty())
		{
			fprintf(file, "mtllib %s\n", mtllib.c_str());
		}
    	double totalvertices = double(vertices.size());
		loopi(0,vertices.size())
		{
			if(verbose && (i%(int(totalvertices)/5)==0)) printf("Vertices written: %d, %.4lf%% of vertices\n", i, double(i)/totalvertices*100);
			fprintf(file, "v %lf %lf %lf\n", vertices[i].p.x,vertices[i].p.y,vertices[i].p.z);
		}
		if (has_uv)
		{
			loopi(0,triangles.size()) if(!triangles[i].deleted)
			{
				fprintf(file, "vt %lf %lf\n", triangles[i].uvs[0].x, triangles[i].uvs[0].y);
				fprintf(file, "vt %lf %lf\n", triangles[i].uvs[1].x, triangles[i].uvs[1].y);
				fprintf(file, "vt %lf %lf\n", triangles[i].uvs[2].x, triangles[i].uvs[2].y);
			}
		}
		int uv = 1;
		double totaltriangles = double(triangles.size());
		loopi(0,triangles.size()) if(!triangles[i].deleted)
		{
			if(verbose && (i%(int(totaltriangles)/5)==0)) printf("Triangles written: %d, %.4lf%% of triangles\n", i, double(i)/totaltriangles*100);
			if (triangles[i].material != cur_material)
			{
				cur_material = triangles[i].material;
				fprintf(file, "usemtl %s\n", materials[triangles[i].material].c_str());
			}
			if (has_uv)
			{
				fprintf(file, "f %d/%d %d/%d %d/%d\n", triangles[i].v[0]+1, uv, triangles[i].v[1]+1, uv+1, triangles[i].v[2]+1, uv+2);
				uv += 3;
			}
			else
			{
				fprintf(file, "f %d %d %d\n", triangles[i].v[0]+1, triangles[i].v[1]+1, triangles[i].v[2]+1);
			}
			//fprintf(file, "f %d// %d// %d//\n", triangles[i].v[0]+1, triangles[i].v[1]+1, triangles[i].v[2]+1); //more compact: remove trailing zeros
		}
		fclose(file);
	}

	// Option: Write Tri10 
	void write_tri10(const char *filename, bool verbose = false, int verboselines = 10000) {
		FILE *file = fopen(filename, "w");
		if (!file) {
			printf("write_obj: can't write data file \"%s\".\n", filename);
			exit(0);
		}

		double quality = 0.0;
		double totalsize = double(triangles.size());
		loopi(0, triangles.size()) {
			fprintf(file, " %15lf %15lf %15lf %15lf %15lf %15lf %15lf %15lf %15lf %15lf\n",
			vertices[triangles[i].v[0]].p.x, vertices[triangles[i].v[0]].p.y, vertices[triangles[i].v[0]].p.z,
			vertices[triangles[i].v[1]].p.x, vertices[triangles[i].v[1]].p.y, vertices[triangles[i].v[1]].p.z,
			vertices[triangles[i].v[2]].p.x, vertices[triangles[i].v[2]].p.y, vertices[triangles[i].v[2]].p.z, quality);
			if (verbose && (i%(int(totalsize)/10)==0)) printf("tri10 lines written: %d, %.4lf%%\n", i, double(i)/totalsize*100);
		}
		fclose(file);
	}

	void write_tri9(const char *filename, bool verbose = false, int verboselines = 10000) {
		FILE *file = fopen(filename, "w");
		if (!file) {
			printf("write_obj: can't write data file \"%s\".\n", filename);
			exit(0);
		}
		double totalsize = double(triangles.size());
		loopi(0, triangles.size()) {
			fprintf(file, " %15lf %15lf %15lf %15lf %15lf %15lf %15lf %15lf %15lf\n",
			vertices[triangles[i].v[0]].p.x, vertices[triangles[i].v[0]].p.y, vertices[triangles[i].v[0]].p.z,
			vertices[triangles[i].v[1]].p.x, vertices[triangles[i].v[1]].p.y, vertices[triangles[i].v[1]].p.z,
			vertices[triangles[i].v[2]].p.x, vertices[triangles[i].v[2]].p.y, vertices[triangles[i].v[2]].p.z);
			if (verbose && (i%(int(totalsize)/10)==0)) printf("tri9 lines written: %d, %.4lf%%\n", i, double(i)/totalsize*100);
			//fprintf(file, "f %d// %d// %d//\n", triangles[i].v[0]+1, triangles[i].v[1]+1, triangles[i].v[2]+1); //more compact: remove trailing zeros
		}
		fclose(file);
	}

	// Need to add line or option for outside regions target ratio
	// regionTarget 	function        center          radius      scale   power   negative?
	// -q 0.2       	-f gaussian     -c 1.2,3.4,10   -r 20.0     -s 2.0  -p 3.0  -n false
	int load_txt(const char* filename, bool verbose=false) {
		FILE *fn;
		if ((filename == NULL) | ((char)filename[0] == 0) | ((fn = fopen(filename, "rb")) == NULL)) {
			printf("File %s not found!\n", filename);
			return EXIT_FAILURE;
		}
		Region currentRegion;
		Region def_region;
		regions.clear();
		def_region.regionTarget = 0.5;
		def_region.regionBound = false;
		def_region.func = constantFunc;
		def_region.coord = vec3f(0.0, 0.0, 0.0);
		def_region.radius = 1.0;
		def_region.scale = 1.0;
		def_region.power = 1.0;
		def_region.isNegative = false;
		def_region.done = false;
		def_region.startCount = 0;
		def_region.endCount = 0;
		char line[1000];
		memset(line, 0, 1000);
		std::vector<char*> line_argv;
		int line_count = 0;
		while (fgets(line, 1000, fn) != NULL) {
			line_count++;
			char opt_exists;
			char arg_exists;
			currentRegion = def_region;
			line_argv.clear();
			char* pch = strtok(line, " ");
			if ((pch[0] == '-') && (sscanf(line," %s %s ", &opt_exists, &arg_exists)==2)) {
				while (pch != NULL) {
					line_argv.push_back(pch);
					pch = strtok(NULL, " ");
				}
				for (std::vector<char*>::iterator it = line_argv.begin(); it != line_argv.end(); ++it) {
					if (strncmp(*it, "-q", 2) == 0) {
						if ((atof(*(it+1)) <= 1) && (atof(*(it+1)) > 0))
							currentRegion.regionTarget = atof(*(it+1));
					} else if (strncmp(*it, "-Q", 2) == 0) {
						if ((atof(*(it+1)) <= 1) && (atof(*(it+1)) > 0))
							currentRegion.regionTarget = atof(*(it+1));
						currentRegion.regionBound = true;
						anyRegionsBound = true;
					} else if (strncmp(*it, "-f", 2) == 0) {
						if (strcmp(*(it+1), "gaussian") == 0) {
							currentRegion.func = gaussian;
						} else if (strcmp(*(it+1), "triangular") == 0) {
							currentRegion.func = triangular;
						} else if (strcmp(*(it+1), "square") == 0) {
							currentRegion.func = square;
						}
					} else if (strncmp(*it, "-c", 2) == 0) {
						double coord_arr[3] = {0, 0, 0};
						char* pcoord = strtok(*(it+1), "{[( ,)]}");
						for (int i = 0; i < 3; i++) {
							coord_arr[i] = atof(pcoord);
							pcoord = strtok(NULL, "{[( ,)]}");
						}
						currentRegion.coord = vec3f(coord_arr[0], coord_arr[1], coord_arr[2]);
					} else if (strncmp(*it, "-r", 2) == 0) {
						currentRegion.radius = atof(*(it+1));
					} else if (strncmp(*it, "-s", 2) == 0) {
						currentRegion.scale = atof(*(it+1));
					} else if (strncmp(*it, "-p", 2) == 0) {
						currentRegion.power = atof(*(it+1));
					} else if (strncmp(*it, "-n", 2) == 0) {
						if (strncmp(*(it+1), "true", 4) == 0) {
							currentRegion.isNegative = true;
						}
					}
				}
				regions.push_back(currentRegion);
				
				// printf final regions if verbose
				if (verbose) {
					std::string function_str;
					std::string isNegative_str;
					if (currentRegion.func == gaussian) {
						function_str = "gaussian";
					} else if (currentRegion.func == triangular) {
						function_str = "triangular";
					} else if (currentRegion.func == square) {
						function_str = "square";
					} else {
						function_str = "constantFunc";
					}
					if (currentRegion.isNegative) isNegative_str = "true"; else isNegative_str = "false";
					printf("line interpreted: -T %lf -f %s -c %lf,%lf,%lf -r %lf -s %lf -p %lf -n %s",
					currentRegion.regionTarget, function_str.c_str(), currentRegion.coord.x,
					currentRegion.coord.y, currentRegion.coord.z, currentRegion.radius, currentRegion.scale,
					currentRegion.power, isNegative_str.c_str());
				}
			} else if (verbose) {
				printf("line #%d not loaded: \"%.*s ...\"", line_count, 150, line);
			}
		}
		return EXIT_SUCCESS;
	} //load_txt
	
	// Is triangle in region specified by center coordinate and radius?
	bool inRegion(Triangle &t, double coord[], double radius) {
		bool allin = true;
		for (int i = 0; i < 3; i++) {
    		allin = allin && (pow(vertices[t.v[i]].p.x-coord[0], 2.0) + pow(vertices[t.v[i]].p.y-coord[1], 2.0) + pow(vertices[t.v[i]].p.z-coord[2], 2.0) <= radius*radius);
		}
		return allin;
    }

	// Computes the factor to threshold locked to a region
	double thresholdRegionsBound(Triangle &t, double &threshold0) {
		double squareFactor;
		for (std::vector<Region>::iterator it = regions.begin(); it != regions.end(); it++) {
			squareFactor = square(vertices[t.v[0]].p.x, vertices[t.v[0]].p.y, 
			vertices[t.v[0]].p.z, (*it).coord.x, (*it).coord.y, (*it).coord.z,
			(*it).radius, (*it).scale, (*it).isNegative);
			if (squareFactor == 0) {
				return 0;
			} else {
				threshold0 = threshold0 * pow((*it).func(vertices[t.v[0]].p.x, vertices[t.v[0]].p.y, 
				vertices[t.v[0]].p.z, (*it).coord.x, (*it).coord.y, (*it).coord.z,
				(*it).radius, (*it).scale, (*it).isNegative), (*it).power) * squareFactor;
			}
		}
		return threshold0;
	}

	// Computes the factor to threshold of all vertices corresponding to the position of triangle (1st vertex)
	double thresholdAllCenters(Triangle &t, double &threshold0) {
		for (std::vector<Region>::iterator it = regions.begin(); it != regions.end(); it++) {
			threshold0 = threshold0 * pow((*it).func(vertices[t.v[0]].p.x, vertices[t.v[0]].p.y, 
			vertices[t.v[0]].p.z, (*it).coord.x, (*it).coord.y, (*it).coord.z,
			(*it).radius, (*it).scale, (*it).isNegative), (*it).power);
		}
		return threshold0;
	}

	int currentCountInRegion(Region &region0) {
		int tempCount = 0;
		double tempcoord[3] = {region0.coord.x, region0.coord.y, region0.coord.z};
		loopi(0, int(triangles.size())) if (!(triangles[i].deleted) && inRegion(triangles[i], tempcoord, region0.radius)) {
			tempCount++;
		}
		return tempCount;
	}

	int currentCountInAllRegions() {
		int count = 0;
		loopi(0, int(regions.size())) {
			count = count + regions[i].endCount;
		}
		return count;
	}

	int currentCountOutsideAllRegions() {
		return Simplify::initialTotalCount - Simplify::initialRegionCount - currentCountInAllRegions();
	}

	bool allRegionsDone() {
		bool allDone = true;
		loopi(0, int(regions.size())) {
			if (!regions[i].done) {
				regions[i].endCount = currentCountInRegion(regions[i]);
				regions[i].done = ((double(regions[i].endCount)/regions[i].startCount) <= regions[i].regionTarget);
			}
			allDone = (allDone && regions[i].done);
			if (!allDone) break;
		}
		return allDone;
    }
};
///////////////////////////////////////////
