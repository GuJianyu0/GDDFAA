// ============================================================================
/// \file src/test_actions.cpp
// ============================================================================
/// \author Jason Sanders
/// \date 2014-2015
/// Institute of Astronomy, University of Cambridge (and University of Oxford)
// ============================================================================

// ============================================================================
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// ============================================================================
/// \brief Computes the actions using most of the available methods for axisymmetric potentials
///
/// Must pass phase-space point x y z vx vy vz, output file and no of orbital
/// times to integrate for.
/// e.g. mains/./test_actions.exe 8.29 0.1 0.1 30.22 211.1 19.22 thin 10.
///
/// Used to produce data for Fig. 2 in Sanders & Binney (2016) using the script orbits.sh
// ============================================================================
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>
#include "GSLInterface/GSLInterface.h"
#include "gnuplot/gnuplot_i.h"
#include <gsl/gsl_poly.h>
#include "/home/darkgaia/0prog/Stackel/tact-master/general/utils.h"
#include "/home/darkgaia/0prog/Stackel/tact-master/general/coordtransforms/inc/coordsys.h"
#include "/home/darkgaia/0prog/Stackel/tact-master/general/coordtransforms/inc/coordtransforms.h"
#include "/home/darkgaia/0prog/Stackel/tact-master/pot/inc/potential.h"
#include "/home/darkgaia/0prog/Stackel/tact-master/pot/inc/orbit.h"
#include "/home/darkgaia/0prog/Stackel/tact-master/aa/inc/aa.h"
#include "/home/darkgaia/0prog/Stackel/tact-master/aa/inc/stackel_aa.h"
#include "/home/darkgaia/0prog/Stackel/tact-master/aa/inc/spherical_aa.h"
#include "/home/darkgaia/0prog/Stackel/tact-master/aa/inc/genfunc_aa.h"
#include "/home/darkgaia/0prog/Stackel/tact-master/aa/inc/adiabatic_aa.h"
#include "/home/darkgaia/0prog/Stackel/tact-master/aa/inc/uv_orb.h"
#include "/home/darkgaia/0prog/Stackel/tact-master/aa/inc/lmn_orb.h"
#include "/home/darkgaia/0prog/Stackel/tact-master/aa/inc/stackel_fit.h"
using namespace std;

#ifdef TORUS
#include "/home/darkgaia/0prog/Stackel/Torus-master/src/pot/falPot.h"
#include "/home/darkgaia/0prog/Stackel/tact-master/aa/inc/it_torus.h"
#include "/home/darkgaia/0prog/Stackel/Torus-master/src/utils/PJM_cline.h"
#endif
//if want to find these in #def, "//" and then right mouse, then "go to def"

int main(int argc, char*argv[]){

	clock_t time_start0=clock();
	#ifdef TORUS
	GalPot Pot("pot/Piffl14.Tpot");
	WrapperTorusPotential TPot(&Pot); //引用, 直接改
	std::cerr<<"Using Piffl14 GalPot"<<std::endl;
	#else
	Logarithmic Pot(220.,1.,0.9);
	std::cerr<<"Using logarithmic potential -- to use GalPot need TORUS flag to make"<<std::endl;
	#endif
	if(argc<8)
		std::cerr<<"Need to pass phase-space point and filename\n";
	VecDoub X(6,0.);
	for(unsigned i=0;i<6;++i){
		X[i]=atof(argv[i+1]);
		std::cout<<"这些输入是"<<argc<<' '<<argv[i]<<' '<<X[i]<<std::endl;
	}
	Orbit O(&Pot); //结合stackel_fit.cpp的轨道积分前进, 所受到的真实势就是这个*Pot(即此Piffle势)
	// Fudge
	Actions_AxisymmetricStackel_Fudge AA(&Pot,100.); //在stackel_aa.h, 估算运动积分; stackel_fit.h的步骤在里面已经用了?

	//其他方法
	// Iterative Torus
	#ifdef TORUS
	IterativeTorusMachine Tor(&AA,&Pot,1e-8,5,1e-3);
	#endif
	// Generating Function
	Actions_Genfunc AG(&Pot,"axisymmetric"); //AG是直接建好的(依靠给定真实势)
	VecDoub acts = AG.actions(X);
	double Rmin=1.,Rmax=40.,zmax=30.;
	#ifdef TORUS
	Actions J;J[0]=acts[0]/conv::kpcMyr2kms;
	J[2]=acts[1]/conv::kpcMyr2kms;J[1]=acts[2]/conv::kpcMyr2kms;
	Torus T; T.AutoFit(J,&TPot,1e-5);
	Rmin = .9*T.minR();
	Rmax = 1.1*T.maxR();
	zmax = 1.1*T.maxz();
	#endif


	//打印
	double tt = 10.; //默认的周期数
	double tstep = 0.01*Pot.torb(X);
	if(argc>8) tt=atof(argv[8]);
	O.integrate(X,tt*Pot.torb(X),tstep); //在外面独立画了一个轨道, 10/0.01个点?
	//O.plot(0,1);
	int ir=0;
	for(auto i:O.results()){
		ir++;
	}
	std::cout<<"周期?? "<<tt<<'\n';
	std::cout<<"Pot.torb(X) "<<Pot.torb(X)<<'\n';
	std::cout<<"次数 "<<ir<<'\n';
	cout<<(double)CLOCKS_PER_SEC<<endl<<endl;
	clock_t time_end0=clock();
	cout<<"time use: "<<(time_end0-time_start0)/(double)CLOCKS_PER_SEC<<"s"<<endl;


	//fudge1
	std::ofstream outfile;
	clock_t time_start1=clock();
	VecDoub Fudge, ITorus, Genfunc;

	outfile.open(argv[7]);
	outfile<<"# Fudgev1 ";

	int guess_alpha=1; int N=0;
	for(auto i:O.results()){
		Fudge = AA.actions(i,&guess_alpha); //执行用fudge估算角作用量的函数, 用偏势式估算得到的, 不是Stackel拟合
		outfile << tstep*N<<" "
				<<Fudge[0]<<" "<<Fudge[2]<<" " //打印这次的[轨道积分前进的这点的]AA.acions(...)
				;
		for(auto j:i) outfile<<j<<" ";
		outfile <<std::endl;
	++N;
	}
	outfile.close();
	
	clock_t time_end1=clock();
	cout<<"time use: "<<(time_end1-time_start1)/(double)CLOCKS_PER_SEC<<"s"<<endl;


	//ItTC
	clock_t time_start2=clock();
	outfile.open(argv[7]);
	#ifdef TORUS
	outfile<<"ItTC ";
	#endif

	for(auto i:O.results()){
		#ifdef TORUS
		ITorus = Tor.actions(i);
		#endif
		outfile << tstep*N<<" "
				#ifdef TORUS
				<<ITorus[0]<<" "<<ITorus[2]<<" "
				#endif
				;
		for(auto j:i) outfile<<j<<" ";
		outfile <<std::endl;
	++N;
	}
	outfile.close();
	
	clock_t time_end2=clock();
	cout<<"time use: "<<(time_end2-time_start2)/(double)CLOCKS_PER_SEC<<"s"<<endl;


	//O2GF
	clock_t time_start3=clock();
	outfile.open(argv[7]);
	outfile<<"O2GF ...\n";

	for(auto i:O.results()){
		Fudge = AA.actions(i,&guess_alpha); //执行用fudge估算角作用量的函数, 用偏势式估算得到的, 不是Stackel拟合
		#ifdef TORUS
		ITorus = Tor.actions(i);
		#endif
		Genfunc = AG.actions(i); //但各个要求点是独立画周期圈计算的?
		outfile << tstep*N<<" "
				<<Genfunc[0]<<" "<<Genfunc[2]<<" "
				;
		for(auto j:i) outfile<<j<<" ";
		outfile <<std::endl;
	++N;
	}
	outfile.close();
	
	clock_t time_end3=clock();
	cout<<"time use: "<<(time_end3-time_start3)/(double)CLOCKS_PER_SEC<<"s"<<endl;
}

// ============================================================================
