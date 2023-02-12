///////////////////////////////////////////////////////////
//// Some useful tools.
///////////////////////////////////////////////////////////

#ifndef _UTILITIES_
#define _UTILITIES_

#include<math.h>
#include<stdio.h>
#include<time.h>
#include<iostream>
#include<fstream>
#include<string>
#include"string.h"
#include<vector>
#include<algorithm>
#include"assert.h"
using namespace std;

#include "Eigen/Eigen"
using namespace Eigen;

namespace UTILITIES{

#define Dim 3
#define MaxCharactersInString 200
#define pi_8 3.1415926
#define Err 1.e-18
#define SKIP_UNTILENDL(fp) fscanf((fp), "%*[^\n]%*c");

#define isSign(a) 		( (a)>=0? 1 : 0 )
#define Sign_(a) 		( (a)>0? 1 : ((a)<0? -1 : 0) )
#define selectMin(a,b) 	( (a)<=(b)? (a) : (b) )
#define selectMax(a,b) 	( (a)<=(b)? (b) : (a) )

int write_data_debug(const vector<struct data_debug>& vdd, string filename, bool is_write_pure=true);

struct data_debug{
	vector<double> xv;
	vector<double> value_double;
	vector<double> info;
	// vector<int> value_int;
	data_debug(){
		xv.resize(Dim*2, 0.);
		vector<double>().swap(value_double);
		info.resize(Dim*2, 0.);
		// vector<double>().swap(value_int);
	}
};

int copy_file(const char *SourceFile, const char *NewFile);



template<typename TYPEN>
const vector<TYPEN>* const Substance_of_VecDoub_as_nullptr = nullptr;

template<typename TYPEN>
TYPEN l2norm_real(const vector<TYPEN>& a)
{
	TYPEN sum = (TYPEN)0;
	size_t n = a.size();
	for(int i=0;i<n;i++){
		sum += abs(a[i]*a[i]);
	}
	return sqrt(sum);
}

//overload operator for data class "+" ??
template<typename TYPEN>
// typename std::enable_if<std::is_same<std::nullptr_t, T>::value || std::is_pointer<T>::value>::type
TYPEN lpnorm_real_with_distance_and_coef(const vector<TYPEN>* pa, const vector<TYPEN>* pb=nullptr, 
	const vector<TYPEN>* pc=nullptr, double pp=2)
{
	TYPEN sum = (TYPEN)0;
	vector<TYPEN> a = *pa, b, c;
	size_t n = a.size();
	if(pp==0){ //to return the count of elements that is none zero
		int idx = 0;
		for(auto i:a){
			if(abs(i)<Err){
				idx++;
			}
		}
		return (TYPEN)idx;
	}else{
		if(pp==std::numeric_limits<TYPEN>::infinity()){ //to return the max element
			return *max_element(a.begin(), a.end());
		}
	}

	if(pb==nullptr){
		b.resize(n, (TYPEN)0);
	}else{
		b = *pb;
	}
	if(pc==nullptr){
		c.resize(n, (TYPEN)1);
	}else{
		c = *pc;
	}
	if( !( n>=b.size() && n>=c.size() ) ){
		std::cerr<<"The sizes of vectors are not equal. Exit.\n";
		exit(1);
	}
	for(int i=0;i<a.size();i++){ //to return the usual norm of a real number vector
		sum += abs(pow((a[i]-b[i])*c[i], pp));
	}
	return (TYPEN)pow(sum, 1./pp);
}

template<typename TYPEN, int dim>
vector<TYPEN> calculate_mass_center_unitmass(const vector<vector<TYPEN>>& x){
	assert((x[0]).size()==dim);
	vector<TYPEN> c;
	c.resize(dim, (TYPEN)0.);
	for(auto ia : x){ //"x" should be shape like ["count of points"][dim]
		for(int i=0;i<dim;++i){
			c[i] += ia[i];
		}
	}
	for(int i=0;i<dim;++i){
		c[i] /= x.size();
	}
	return c;
}

template<typename TYPEN, int dim>
void translate_vector(vector<vector<TYPEN>>& x, const vector<TYPEN>& c){
	assert((x[0]).size()==dim);
	assert(c.size()==dim);
	int n = x.size();
	for(int j=0;j<n;++j){
		for(int i=0;i<dim;++i){
			x[j][i] -= c[i];
		}
	}
}

//from euler angle to SO3 matrix element
//ang is Euler angle
//?? only for dim==3
template<typename TYPEN, int dim>
MatrixXd SO3_angle_to_matrix(const vector<TYPEN>& ang){
	assert(ang.size()==dim);
	double c0 = cos(ang[0]), s0 = sin(ang[0]), 
	c1 = cos(ang[1]), s1 = sin(ang[1]), 
	c2 = cos(ang[2]), s2 = sin(ang[2]);
	MatrixXd mat(dim, dim);
	mat(0,0) = c1*c2;
	mat(0,1) = -c0*s2+s0*s1*c2;
	mat(0,2) = s0*s2+c0*s1*c2;
	mat(1,0) = c1*s2;
	mat(1,1) = c0*c2+s0*s1*s2;
	mat(1,2) = -s0*c2+c0*s1*s2;
	mat(2,0) = -s1;
	mat(2,1) = s0*c1;
	mat(2,2) = c0*c1;
	return mat;
}

template<typename TYPEN, int dim>
vector<vector<TYPEN>> SO3_vector(const vector<vector<TYPEN>>& x, const MatrixXd& mat){
	assert((x[0]).size()==dim);
	assert(mat.size()==dim*dim);
	int n = x.size();
	auto x_ = x;
	MatrixXd x1(1, dim), x2; //??
	for(int j=0;j<n;++j){
		for(int i=0;i<dim;++i){
			x1(i) = x[j][i];
		}
		x2 = x1*mat;
		for(int i=0;i<dim;++i){
			x_[j][i] = x2(i);
		}
	}
	return x_;
}

template<typename TYPEN, int dim>
vector<vector<TYPEN>> SO3_vector(const vector<vector<TYPEN>>& x, const vector<TYPEN>& ang){
	auto mat = SO3_angle_to_matrix<TYPEN, dim>(ang);
	return SO3_vector<TYPEN, dim>(x, mat);
}

//from Sanders TACT/general/utils.h
template<class c>
std::vector<std::vector<c>> transpose(const std::vector<std::vector<c>> &a) {
    std::vector<std::vector<c>> result(a[0].size(),std::vector<c>(a.size()));
    for (unsigned int i = 0; i < a[0].size(); i++)
        for (unsigned int j = 0; j < a.size(); j++) {
            result[i][j] = a[j][i];
        }
    return result;
}



}

#endif