// ============================================================================
/// \file src/stackel_aa.cpp
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
/// \brief Action finding in Staeckel potentials and Staeckel fudges
///
/// Four classes are implemented:
/// 1. Actions_AxisymmetricStackel: Action finding in axisymmetric Staeckel
///    potentials (currently accepts perfect ellipsoid potential)
/// 2. Actions_TriaxialStackel: Action finding in triaxial Staeckel potential
/// 3. Actions_AxisymmetricStackel_Fudge: Action estimation in axisymmetric
///    potential using Staeckel fudge (as in Binney (2012))
/// 4. Actions_TriaxialStackel_Fudge :Action estimation in triaxial potential
///    using Staeckel fudge (as in Sanders & Binney (2014))
///
//============================================================================

/*======================================*/
/*Actions, Angles, Frequencies & Hessian*/
/*======================================*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include "GSLInterface/GSLInterface.h"
#include "utils.h"
#include "potential.h"
#include "orbit.h"
#include "stackel_aa.h"
#include "debug.h"

// ============================================================================
// Prolate Stackel Angle-action calculator
// ============================================================================

double ptau2ROOT_AxiSym(double tau, void *params){
	/* for finding roots of p_tau^2*2.0*(tau+Alpha)  */
	root_struct_axi *RS = (root_struct_axi *) params;
	return (RS->Ints[0]-RS->Ints[1]/(tau+RS->P->alpha())-RS->Ints[2]/(tau+RS->P->gamma())+RS->P->G(tau));
	}

VecDoub Actions_AxisymmetricStackel::find_limits(const VecDoub& tau, const VecDoub& ints){
	double tiny_number=SMALL;
	double lambda = tau[0], nu = tau[2];
	root_find RF(TINY,100);
	VecDoub limits;
	// create a structure to store parameters for ptau2ROOT
	root_struct_axi RS(Pot,ints);
	// find roots of p^2(lam)
	double laminner=lambda, lamouter=lambda;
	if(ptau2ROOT_AxiSym(lambda, &RS)>=0.0){
		while(ptau2ROOT_AxiSym(laminner, &RS)>=0.0
		      and (laminner+Pot->alpha())>tiny_number)
			laminner-=.1*(laminner+Pot->alpha());
		while(ptau2ROOT_AxiSym(lamouter, &RS)>=0.)	lamouter*=1.1;
		if((laminner+Pot->alpha())>tiny_number)
			limits.push_back(
			    RF.findroot(&ptau2ROOT_AxiSym,laminner,lambda,&RS));
		else limits.push_back(-Pot->alpha());
		limits.push_back(RF.findroot(&ptau2ROOT_AxiSym,lambda,lamouter,&RS));
		if(fabs(limits[0]-limits[1])<TINY){
		    if(ptau2ROOT_AxiSym(lambda-5*TINY, &RS)>0.0)
			limits[0]=RF.findroot(&ptau2ROOT_AxiSym,laminner,lambda-5*TINY,&RS);
	            else
			limits[1]=RF.findroot(&ptau2ROOT_AxiSym,lambda+5*TINY,lamouter,&RS);
		}
	}
	else{
		limits.push_back(lambda-tiny_number);
		limits.push_back(lambda+tiny_number);
	}
	limits.push_back(-Pot->gamma()+TINY);
	// find root of p^2(nu)
	double nuouter=nu;
	RS = root_struct_axi(Pot, {ints[0],ints[1],ints[3]}); //ints[3]!?
	if(ptau2ROOT_AxiSym(nu, &RS)<=0.0){
        while(ptau2ROOT_AxiSym(nuouter, &RS)<=0.
		    and -(nuouter+Pot->alpha())>tiny_number)
			nuouter+=0.1*(-Pot->alpha()-nuouter);
		if(-(nuouter+Pot->alpha())>tiny_number)
			limits.push_back(RF.findroot(&ptau2ROOT_AxiSym,nu+TINY,nuouter,&RS));
		else limits.push_back(-Pot->alpha());
	}
	else limits.push_back(nu+tiny_number);
    return limits;
}

double ptau2_AxiSym(double tau, void *params){
	//p^2(tau) using global integrals
	root_struct_axi *RS = (root_struct_axi *) params;
	return (RS->Ints[0]-RS->Ints[1]/(tau+RS->P->alpha())-RS->Ints[2]/(tau+RS->P->gamma())+RS->P->G(tau))
			/(2.0*(tau+RS->P->alpha()));
}

double J_integrand_AxiSym(double theta, void *params){
	action_struct_axi * AS = (action_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	// std::cout<<tau<<" "<<ptau2_AxiSym(tau,&(AS->RS))<<std::endl;
	return sqrt(MAX(0.,ptau2_AxiSym(tau,&(AS->RS))))*cos(theta);
}

double dJdEint_AxiSym(double theta, void *params){
	action_struct_axi * AS = (action_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double P = 0.25*cos(theta)/((tau+AS->RS.P->alpha())*sqrt(MAX(AS->tiny_number,ptau2_AxiSym(tau,&(AS->RS)))));
	return P;
}

double dJdI2int_AxiSym(double theta, void *params){
	action_struct_axi * AS = (action_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	return -0.25*cos(theta)/((tau+AS->RS.P->alpha())*(tau+AS->RS.P->alpha())*sqrt(MAX(AS->tiny_number,ptau2_AxiSym(tau,&(AS->RS)))));
}

double dJdI3int_AxiSym(double theta, void *params){
	action_struct_axi * AS = (action_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	return -0.25*cos(theta)/((tau+AS->RS.P->alpha())*(tau+AS->RS.P->gamma())*sqrt(MAX(AS->tiny_number,ptau2_AxiSym(tau,&(AS->RS)))));
}

double d2Jlamd2Eint_AxiSym(double theta, void *params){
	hess_struct_axi * AS = (hess_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double dtaudE = AS->ASS->dtaudint(AS->limits,0,0,theta);
	double PTAU = sqrt(MAX(TINY,ptau2_AxiSym(tau,&(AS->RS))));
	double Alpha = AS->RS.P->alpha();
	double PTILDE=PTAU/(AS->Deltagl*cos(theta));
	double dptildedE = ((0.5/(tau+Alpha)+AS->ASS->dp2dtau(tau,AS->ints)*dtaudE)/(AS->Deltagl*cos(theta)*AS->Deltagl*cos(theta))
						-(PTILDE*PTILDE/AS->Deltagl)*AS->ASS->dDeltaGLdint(AS->limits,0, 0))/(2.*PTILDE);

	return -0.25/((tau+Alpha)*PTILDE)*(dptildedE/PTILDE+dtaudE/(tau+Alpha));
}

double d2Jnud2Eint_AxiSym(double theta, void *params){
	hess_struct_axi * AS = (hess_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double dtaudE = AS->ASS->dtaudint(AS->limits,1,0,theta);
	double PTAU = sqrt(MAX(TINY,ptau2_AxiSym(tau,&(AS->RS))));
	double PTILDE=PTAU/(AS->Deltagl*cos(theta));
	double Alpha = AS->RS.P->alpha();

	double dptildedE = ((0.5/(tau+Alpha)+AS->ASS->dp2dtau(tau,AS->ints)*dtaudE)/(AS->Deltagl*cos(theta)*AS->Deltagl*cos(theta))
						-(PTILDE*PTILDE/AS->Deltagl)*AS->ASS->dDeltaGLdint(AS->limits,1,0))/(2.0*PTILDE);

	return -0.25/((tau+Alpha)*PTILDE)*(dptildedE/PTILDE+dtaudE/(tau+Alpha));
}
//=================================================================================================
// d2J_τ/dEdI2
double d2JlamdEdI2int_AxiSym(double theta, void *params){
	hess_struct_axi * AS = (hess_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double dtaudI2 = AS->ASS->dtaudint(AS->limits,0,1,theta);
	double PTAU = sqrt(MAX(TINY,ptau2_AxiSym(tau,&(AS->RS))));
	double PTILDE=PTAU/(AS->Deltagl*cos(theta));
	double Alpha = AS->RS.P->alpha();

	double dptildedI2 = ((-0.5/((tau+Alpha)*(tau+Alpha))+AS->ASS->dp2dtau(tau,AS->ints)*dtaudI2)/(AS->Deltagl*cos(theta)*AS->Deltagl*cos(theta))
						-(PTILDE*PTILDE/AS->Deltagl)*AS->ASS->dDeltaGLdint(AS->limits,0,1))/(2.0*PTILDE);

	return -0.25/((tau+Alpha)*PTILDE)*(dptildedI2/PTILDE+dtaudI2/(tau+Alpha));
}

double d2JnudEdI2int_AxiSym(double theta, void *params){
	hess_struct_axi * AS = (hess_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);	double dtaudI2 = AS->ASS->dtaudint(AS->limits,1,1,theta);
	double PTAU = sqrt(MAX(TINY,ptau2_AxiSym(tau,&(AS->RS))));
	double PTILDE=PTAU/(AS->Deltagl*cos(theta));
	double Alpha = AS->RS.P->alpha();

	double dptildedI2 = ((-0.5/((tau+Alpha)*(tau+Alpha))+AS->ASS->dp2dtau(tau,AS->ints)*dtaudI2)/(AS->Deltagl*cos(theta)*AS->Deltagl*cos(theta))
						-(PTILDE*PTILDE/AS->Deltagl)*AS->ASS->dDeltaGLdint(AS->limits,1,1))/(2.0*PTILDE);

	return -0.25/((tau+Alpha)*PTILDE)*(dptildedI2/PTILDE+dtaudI2/(tau+Alpha));
}
//=================================================================================================
// d2J_τ/dEdI3
double d2JlamdEdI3int_AxiSym(double theta, void *params){
	hess_struct_axi * AS = (hess_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double dtaudI3 = AS->ASS->dtaudint(AS->limits,0,2,theta);
	double PTAU = sqrt(MAX(TINY,ptau2_AxiSym(tau,&(AS->RS))));
	double PTILDE=PTAU/(AS->Deltagl*cos(theta));
	double Alpha = AS->RS.P->alpha(), Gamma = AS->RS.P->gamma();

	double dptildedI3 = ((-0.5/((tau+Alpha)*(tau+Gamma))+AS->ASS->dp2dtau(tau,AS->ints)*dtaudI3)/(AS->Deltagl*cos(theta)*AS->Deltagl*cos(theta))
						-(PTILDE*PTILDE/AS->Deltagl)*AS->ASS->dDeltaGLdint(AS->limits,0,2))/(2.0*PTILDE);

	return -0.25/((tau+Alpha)*PTILDE)*(dptildedI3/PTILDE+dtaudI3/(tau+Alpha));
}

double d2JnudEdI3int_AxiSym(double theta, void *params){
	hess_struct_axi * AS = (hess_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double dtaudI3 = AS->ASS->dtaudint(AS->limits,1,2,theta);
	double PTAU = sqrt(MAX(TINY,ptau2_AxiSym(tau,&(AS->RS))));
	double PTILDE=PTAU/(AS->Deltagl*cos(theta));
	double Alpha = AS->RS.P->alpha(), Gamma = AS->RS.P->gamma();

	double dptildedI3 = ((-0.5/((tau+Alpha)*(tau+Gamma))+AS->ASS->dp2dtau(tau,AS->ints)*dtaudI3)/(AS->Deltagl*cos(theta)*AS->Deltagl*cos(theta))
						-(PTILDE*PTILDE/AS->Deltagl)*AS->ASS->dDeltaGLdint(AS->limits,1,2))/(2.0*PTILDE);

	return -0.25/((tau+Alpha)*PTILDE)*(dptildedI3/PTILDE+dtaudI3/(tau+Alpha));
}
//=================================================================================================
// d2J_τ/d2I2
double d2Jlamd2I2int_AxiSym(double theta, void *params){
	hess_struct_axi * AS = (hess_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double dtaudI2 = AS->ASS->dtaudint(AS->limits,0,1,theta);
	double PTAU = sqrt(MAX(TINY,ptau2_AxiSym(tau,&(AS->RS))));
	double PTILDE=PTAU/(AS->Deltagl*cos(theta));
	double Alpha = AS->RS.P->alpha();

	double dptildedI2 = ((-0.5/((tau+Alpha)*(tau+Alpha))+AS->ASS->dp2dtau(tau,AS->ints)*dtaudI2)/(AS->Deltagl*cos(theta)*AS->Deltagl*cos(theta))
						-(PTILDE*PTILDE/AS->Deltagl)*AS->ASS->dDeltaGLdint(AS->limits,0,1))/(2.0*PTILDE);

	return 0.25/((tau+Alpha)*(tau+Alpha)*PTILDE)*(dptildedI2/PTILDE+2.0*dtaudI2/(tau+Alpha));
}

double d2Jnud2I2int_AxiSym(double theta, void *params){
	hess_struct_axi * AS = (hess_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double dtaudI2 = AS->ASS->dtaudint(AS->limits,1,1,theta);
	double PTAU = sqrt(MAX(TINY,ptau2_AxiSym(tau,&(AS->RS))));
	double PTILDE=PTAU/(AS->Deltagl*cos(theta));
	double Alpha = AS->RS.P->alpha();

	double dptildedI2 = ((-0.5/((tau+Alpha)*(tau+Alpha))+AS->ASS->dp2dtau(tau,AS->ints)*dtaudI2)/(AS->Deltagl*cos(theta)*AS->Deltagl*cos(theta))
						-(PTILDE*PTILDE/AS->Deltagl)*AS->ASS->dDeltaGLdint(AS->limits,1,1))/(2.0*PTILDE);

	return 0.25/((tau+Alpha)*(tau+Alpha)*PTILDE)*(dptildedI2/PTILDE+2.0*dtaudI2/(tau+Alpha));
}
//=================================================================================================
// d2J_τ/dI2dI3
double d2JlamdI2dI3int_AxiSym(double theta, void *params){
	hess_struct_axi * AS = (hess_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double dtaudI3 = AS->ASS->dtaudint(AS->limits,0,2,theta);
	double PTAU = sqrt(MAX(TINY,ptau2_AxiSym(tau,&(AS->RS))));
	double PTILDE=PTAU/(AS->Deltagl*cos(theta));
	double Alpha = AS->RS.P->alpha(), Gamma = AS->RS.P->gamma();

	double dptildedI3 = ((-0.5/((tau+Gamma)*(tau+Alpha))+AS->ASS->dp2dtau(tau,AS->ints)*dtaudI3)/(AS->Deltagl*cos(theta)*AS->Deltagl*cos(theta))
						-(PTILDE*PTILDE/AS->Deltagl)*AS->ASS->dDeltaGLdint(AS->limits,0,2))/(2.0*PTILDE);

	return 0.25/((tau+Alpha)*(tau+Alpha)*PTILDE)*(dptildedI3/PTILDE+2.0*dtaudI3/(tau+Alpha));
	}

double d2JnudI2dI3int_AxiSym(double theta, void *params){
	hess_struct_axi * AS = (hess_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double dtaudI3 = AS->ASS->dtaudint(AS->limits,1,2,theta);
	double PTAU = sqrt(MAX(TINY,ptau2_AxiSym(tau,&(AS->RS))));
	double PTILDE=PTAU/(AS->Deltagl*cos(theta));
	double Alpha = AS->RS.P->alpha(), Gamma = AS->RS.P->gamma();

	double dptildedI3 = ((-0.5/((tau+Gamma)*(tau+Alpha))+AS->ASS->dp2dtau(tau,AS->ints)*dtaudI3)/(AS->Deltagl*cos(theta)*AS->Deltagl*cos(theta))
						-(PTILDE*PTILDE/AS->Deltagl)*AS->ASS->dDeltaGLdint(AS->limits,1,2))/(2.0*PTILDE);

	return 0.25/((tau+Alpha)*(tau+Alpha)*PTILDE)*(dptildedI3/PTILDE+2.0*dtaudI3/(tau+Alpha));
}
//=================================================================================================
// d2J_τ/d2I3
double d2Jlamd2I3int_AxiSym(double theta, void *params){
	hess_struct_axi * AS = (hess_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double dtaudI3 = AS->ASS->dtaudint(AS->limits,0,2,theta);
	double PTAU = sqrt(MAX(TINY,ptau2_AxiSym(tau,&(AS->RS))));
	double PTILDE=PTAU/(AS->Deltagl*cos(theta));
	double Alpha = AS->RS.P->alpha(), Gamma = AS->RS.P->gamma();

	double dptildedI3 = ((-0.5/((tau+Gamma)*(tau+Alpha))+AS->ASS->dp2dtau(tau,AS->ints)*dtaudI3)/(AS->Deltagl*cos(theta)*AS->Deltagl*cos(theta))
						-(PTILDE*PTILDE/AS->Deltagl)*AS->ASS->dDeltaGLdint(AS->limits,0,2))/(2.0*PTILDE);

	return 0.25/((tau+Gamma)*(tau+Alpha)*PTILDE)*(dptildedI3/PTILDE+dtaudI3*(1./(tau+Alpha)+1./(tau+Gamma)));
}

double d2Jnud2I3int_AxiSym(double theta, void *params){
	hess_struct_axi * AS = (hess_struct_axi *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double dtaudI3 = AS->ASS->dtaudint(AS->limits,1,2,theta);
	double PTAU = sqrt(MAX(TINY,ptau2_AxiSym(tau,&(AS->RS))));
	double PTILDE=PTAU/(AS->Deltagl*cos(theta));
	double Alpha = AS->RS.P->alpha(), Gamma = AS->RS.P->gamma();
	double dptildedI3 = ((-0.5/((tau+Gamma)*(tau+Alpha))+AS->ASS->dp2dtau(tau,AS->ints)*dtaudI3)/(AS->Deltagl*cos(theta)*AS->Deltagl*cos(theta))
						-(PTILDE*PTILDE/AS->Deltagl)*AS->ASS->dDeltaGLdint(AS->limits,1,2))/(2.0*PTILDE);

	return 0.25/((tau+Gamma)*(tau+Alpha)*PTILDE)*(dptildedI3/PTILDE+dtaudI3*(1./(tau+Alpha)+1./(tau+Gamma)));
}

void Actions_AxisymmetricStackel::dtaudint(const VecDoub& limits, const VecDoub& ints){

	if(fabs(limits[0]+Pot->alpha())>SMALL)
		dtau01dint[0][0] = -(limits[0]+Pot->alpha())*(limits[0]+Pot->gamma())/((2.0*limits[0]+Pot->gamma()+Pot->alpha())*ints[0]-ints[1]-ints[2]+BigFPrime(limits[0]));
	else dtau01dint[0][0]=0.;

	if(fabs(limits[1]+Pot->gamma())>SMALL)
		dtau01dint[0][1] = -(limits[1]+Pot->alpha())*(limits[1]+Pot->gamma())/((2.0*limits[1]+Pot->gamma()+Pot->alpha())*ints[0]-ints[1]-ints[2]+BigFPrime(limits[1]));
	else dtau01dint[0][1]=0.;

	if(limits[2]+Pot->gamma()>SMALL)
		dtau01dint[1][0] = -(limits[2]+Pot->alpha())*(limits[2]+Pot->gamma())/((2.0*limits[2]+Pot->gamma()+Pot->alpha())*ints[0]-ints[1]-ints[2]+BigFPrime(limits[2]));
	else dtau01dint[1][0]=0.;

	dtau01dint[1][1] = -(limits[3]+Pot->alpha())*(limits[3]+Pot->gamma())/((2.0*limits[3]+Pot->gamma()+Pot->alpha())*ints[0]-ints[1]-ints[2]+BigFPrime(limits[3]));

	return;
}

double Actions_AxisymmetricStackel::dtaudint(const VecDoub& limits, int i, int j, double theta){
	if(j==0)
		return 0.5*(dtau01dint[i][1]-dtau01dint[i][0])*sin(theta)+0.5*(dtau01dint[i][1]+dtau01dint[i][0]);
	else if(j==1)
		return 0.5*(-dtau01dint[i][1]/(limits[i*2+1]+Pot->alpha())+dtau01dint[i][0]/(limits[i*2+0]+Pot->alpha()))*sin(theta)
						+0.5*(-dtau01dint[i][1]/(limits[i*2+1]+Pot->alpha())-dtau01dint[i][0]/(limits[i*2+0]+Pot->alpha()));
	else if(j==2)return 0.5*(-dtau01dint[i][1]/(limits[i*2+1]+Pot->gamma())+dtau01dint[i][0]/(limits[i*2+0]+Pot->gamma()))*sin(theta)
						+0.5*(-dtau01dint[i][1]/(limits[i*2+1]+Pot->gamma())-dtau01dint[i][0]/(limits[i*2+0]+Pot->gamma()));
	else{std::cout<<"j out of range"<<std::endl;return 0;}
	}

double Actions_AxisymmetricStackel::dDeltaGLdint(const VecDoub& limits, int i, int j){
	if(j==0)return (dtau01dint[i][1]-dtau01dint[i][0]);
	else if(j==1)return (-dtau01dint[i][1]/(limits[i*2+1]+Pot->alpha())+dtau01dint[i][0]/(limits[i*2+0]+Pot->alpha()));
	else if(j==2)return (-dtau01dint[i][1]/(limits[i*2+1]+Pot->gamma())+dtau01dint[i][0]/(limits[i*2+0]+Pot->gamma()));
	else{std::cout<<"j out of range"<<std::endl;return 0;}
	}

double Actions_AxisymmetricStackel::dp2dtau(double tau, const VecDoub& ints){
	double left =-(ints[0]-ints[1]/(Pot->alpha()+tau)-ints[2]
	               /(Pot->gamma()+tau)+Pot->G(tau))
					/(2.*(Pot->alpha()+tau)*(Pot->alpha()+tau));
	double right=(ints[1]/((Pot->alpha()+tau)*(Pot->alpha()+tau))+ints[2]
	               /((Pot->gamma()+tau)*(Pot->gamma()+tau))+Pot->GPrime(tau))
					/(2.*(Pot->alpha()+tau));
	return (left+right);
}

VecDoub Actions_AxisymmetricStackel::actions(const VecDoub& x, void *params){

	VecDoub tau = Pot->xv2tau(x);
	if(tau[2]==-Pot->gamma())tau[2]+=TINY;
	// If exactly on the plane we integrate a bit
	// if(tau[2]==-Pot->gamma() and tau[5]==0.)
	//     tau = Pot->xv2tau(integrate_a_bit(x,Pot));
	VecDoub integrals = Pot->x2ints(x,&tau);
	VecDoub limits = find_limits(tau,integrals); //先integrals再limits!!
	VecDoub actions(3,0.);
	// JR
	double taubar = 0.5*(limits[0]+limits[1]);
	double Delta = 0.5*(limits[1]-limits[0]);
	action_struct_axi AS(Pot,integrals,taubar,Delta,0.);
	actions[0]=Delta*GaussLegendreQuad(&J_integrand_AxiSym,-.5*PI,.5*PI,&AS)/PI;
	// Lz
	actions[1]=Pot->Lz(x);
	// Jz
	taubar = 0.5*(limits[2]+limits[3]);
	Delta = 0.5*(limits[3]-limits[2]);
	AS = action_struct_axi (Pot,{integrals[0],integrals[1],integrals[3]},taubar,Delta,0.); //这时候实际进的就是I3[3]即关于(f(nv))
	actions[2]=2.*Delta*GaussLegendreQuad(&J_integrand_AxiSym,-.5*PI,.5*PI,&AS)/PI;
	return actions;
}

VecDoub Actions_AxisymmetricStackel::angles(const VecDoub& x, void*params){
// calculates angles, also freqs at end.

	bool with_hess=false;
	if(params)with_hess=true;
	VecDoub tau = Pot->xv2tau(x);
	if(tau[2]==-Pot->gamma())tau[2]+=TINY;
	double tn = (x[3]*x[3]+x[4]*x[4]+x[5]*x[5])/5.e11;
	VecDoub integrals = Pot->x2ints(x,&tau);
	VecDoub limits = find_limits(tau,integrals);

	VecDoub angles(6,0);

	// dIdJ ==================================================================
	double dJdI[3][3]; double dIdJ[3][3];
	double taubar = 0.5*(limits[0]+limits[1]);
	double Delta = 0.5*(limits[1]-limits[0]);
	action_struct_axi AS(Pot,integrals,taubar,Delta, tn);

	double lamFactor = 1.;
	if(Pot->alpha()>Pot->gamma()) lamFactor = 2.;

	dJdI[0][0] = lamFactor*Delta*GaussLegendreQuad(&dJdEint_AxiSym,-.5*PI,.5*PI,&AS)/PI;
	dJdI[0][1] = lamFactor*Delta*GaussLegendreQuad(&dJdI2int_AxiSym,-.5*PI,.5*PI,&AS)/PI;
	dJdI[0][2] = lamFactor*Delta*GaussLegendreQuad(&dJdI3int_AxiSym,-.5*PI,.5*PI,&AS)/PI;

	// dSdI ===================================================================
	double dSdE=0.0,dSdI2=0.0,dSdI3=0.0;
	double thetaLimit = asin(MAX(-1.0,MIN(1.0,(tau[0]-taubar)/Delta)));
	double lsign = SIGN(tau[3]);
	dSdE  += lsign*Delta*GaussLegendreQuad(&dJdEint_AxiSym,-.5*PI,thetaLimit,&AS);
	dSdI2 += lsign*Delta*GaussLegendreQuad(&dJdI2int_AxiSym,-.5*PI,thetaLimit,&AS);
	dSdI3 += lsign*Delta*GaussLegendreQuad(&dJdI3int_AxiSym,-.5*PI,thetaLimit,&AS);

	dJdI[1][0] = 0.0;
	dJdI[1][1] = 1.0/(sqrt(2.0*integrals[1]));
	dJdI[1][2] = 0.0;

	dSdI2+=SIGN(tau[4])*tau[1]/(sqrt(2.0*integrals[1]));

	//Nu
	taubar = 0.5*(limits[2]+limits[3]);
	Delta=0.5*(limits[3]-limits[2]);
	AS = action_struct_axi(Pot,{integrals[0],integrals[1],integrals[3]},taubar,Delta,tn);
	double nuFactor = 1.;
	if(limits[2]==-Pot->gamma()+TINY and Pot->alpha()<Pot->gamma()) nuFactor = 2.;

	dJdI[2][0] = nuFactor*Delta*GaussLegendreQuad(&dJdEint_AxiSym,-.5*PI,.5*PI,&AS)/PI;
	dJdI[2][1] = nuFactor*Delta*GaussLegendreQuad(&dJdI2int_AxiSym,-.5*PI,.5*PI,&AS)/PI;
	dJdI[2][2] = nuFactor*Delta*GaussLegendreQuad(&dJdI3int_AxiSym,-.5*PI,.5*PI,&AS)/PI;
	//Nu
	thetaLimit = asin(MAX(-1.0,MIN(1.0,(tau[2]-taubar)/Delta)));
	double nsign = SIGN(tau[5]);//*SIGN(tau[2]);
	dSdE  += nsign*Delta*GaussLegendreQuad(&dJdEint_AxiSym,-.5*PI,thetaLimit,&AS);
	dSdI2 += nsign*Delta*GaussLegendreQuad(&dJdI2int_AxiSym,-.5*PI,thetaLimit,&AS);
	dSdI3 += nsign*Delta*GaussLegendreQuad(&dJdI3int_AxiSym,-.5*PI,thetaLimit,&AS);

	double Determinant = dJdI[0][0]*det2(dJdI[1][1],dJdI[1][2],dJdI[2][1],dJdI[2][2])
						- dJdI[0][1]*det2(dJdI[1][0],dJdI[1][2],dJdI[2][0],dJdI[2][2])
						+ dJdI[0][2]*det2(dJdI[1][0],dJdI[1][1],dJdI[2][0],dJdI[2][1]);

	dIdJ[0][0] = det2(dJdI[1][1],dJdI[1][2],dJdI[2][1],dJdI[2][2])/Determinant;
	dIdJ[0][1] = -det2(dJdI[0][1],dJdI[0][2],dJdI[2][1],dJdI[2][2])/Determinant;
	dIdJ[0][2] = det2(dJdI[0][1],dJdI[0][2],dJdI[1][1],dJdI[1][2])/Determinant;
	dIdJ[1][0] = -det2(dJdI[1][0],dJdI[1][2],dJdI[2][0],dJdI[2][2])/Determinant;
	dIdJ[1][1] = det2(dJdI[0][0],dJdI[0][2],dJdI[2][0],dJdI[2][2])/Determinant;
	dIdJ[1][2] = -det2(dJdI[0][0],dJdI[0][2],dJdI[1][0],dJdI[1][2])/Determinant;
	dIdJ[2][0] = det2(dJdI[1][0],dJdI[1][1],dJdI[2][0],dJdI[2][1])/Determinant;
	dIdJ[2][1] = -det2(dJdI[0][0],dJdI[0][1],dJdI[2][0],dJdI[2][1])/Determinant;
	dIdJ[2][2] = det2(dJdI[0][0],dJdI[0][1],dJdI[1][0],dJdI[1][1])/Determinant;

	angles[0] = dSdE*dIdJ[0][0]+dSdI2*dIdJ[1][0]+dSdI3*dIdJ[2][0];
	angles[1] = dSdE*dIdJ[0][1]+dSdI2*dIdJ[1][1]+dSdI3*dIdJ[2][1];
	angles[2] = dSdE*dIdJ[0][2]+dSdI2*dIdJ[1][2]+dSdI3*dIdJ[2][2];


	if(tau[5]<0.0 and nuFactor==2.){angles[2]+=PI;}
	if(x[2]<0.0 and Pot->alpha()<Pot->gamma()){angles[2]+=PI;}
	if(Pot->alpha()>Pot->gamma()){angles[0]+=SIGN(x[2])*PI/2.;}
	if(x[2]<0.0 and lamFactor==1. and Pot->alpha()>Pot->gamma()){angles[0]+=PI;}
	if(angles[2]>2.*PI)angles[2]-=2.*PI;
	if(angles[0]<0.0){angles[0]+=2.0*PI;}
	if(angles[1]<0.0){angles[1]+=2.0*PI;}
	if(angles[2]<0.0)angles[2]+=2.*PI;

	angles[3]=det2(dJdI[1][1],dJdI[1][2],dJdI[2][1],dJdI[2][2])/Determinant;
	angles[4]=det2(dJdI[2][1],dJdI[2][2],dJdI[0][1],dJdI[0][2])/Determinant;
	angles[4]*=SIGN(Pot->Lz(x));
	angles[5]=det2(dJdI[0][1],dJdI[0][2],dJdI[1][1],dJdI[1][2])/Determinant;
	// FLAG data which doesn't make sense

	// if(star.freq[0]<0.0 || star.freq[1]<0.0 || star.freq[2]<0.0 ||
	// 	star.Angs[0]<0.0 || star.Angs[0]>2.*PI ||
	// 	star.Angs[1]<0.0 || star.Angs[1]>2.*PI ||
	// 	star.Angs[2]<0.0 || star.Angs[2]>2.*PI ||
	// 	star.Angs[0]!=star.Angs[0] ||
	// 	star.Angs[1]!=star.Angs[1] ||
	// 	star.Angs[2]!=star.Angs[2] ) star.FLAG=1;

	// if(alpha()>Pot->gamma()){
	// 	double tmp = star.Angs[0];star.Angs[0]=star.Angs[2];star.Angs[2]=tmp;
	// 	tmp = star.freq[0];star.freq[0]=star.freq[2];star.freq[2]=tmp;
	// }

	// WithHessian ============================================================

	if(with_hess){
	dtau01dint.push_back({0.,0.});	dtau01dint.push_back({0.,0.});
	double Dij[3][3];
	dtaudint(limits,integrals);
	// d2JlamdI2
	double d2JlamdI2[3][3];
	taubar = 0.5*(limits[1]+limits[0]);
	Delta=0.5*(limits[1]-limits[0]);
	hess_struct_axi AS(this,Pot,{integrals[0],integrals[1],integrals[2]},limits,taubar,Delta);
	int order = 4;
	d2JlamdI2[0][0] = GaussLegendreQuad(&d2Jlamd2Eint_AxiSym,-.5*PI,.5*PI,&AS,order)/PI;
	d2JlamdI2[0][1] = d2JlamdI2[1][0] = GaussLegendreQuad(&d2JlamdEdI2int_AxiSym,-.5*PI,.5*PI,&AS,order)/PI;
	d2JlamdI2[0][2] = d2JlamdI2[2][0] = GaussLegendreQuad(&d2JlamdEdI3int_AxiSym,-.5*PI,.5*PI,&AS,order)/PI;
	d2JlamdI2[1][1] = GaussLegendreQuad(&d2Jlamd2I2int_AxiSym,-.5*PI,.5*PI,&AS,order)/PI;
	d2JlamdI2[1][2] = d2JlamdI2[2][1] = GaussLegendreQuad(&d2JlamdI2dI3int_AxiSym,-.5*PI,.5*PI,&AS,order)/PI;
	d2JlamdI2[2][2] = GaussLegendreQuad(&d2Jlamd2I3int_AxiSym,-.5*PI,.5*PI,&AS,order)/PI;

	// d2JnudI2
	double d2JnudI2[3][3];
	dtaudint(limits,{integrals[0],integrals[1],integrals[3]});
	taubar = 0.5*(limits[3]+limits[2]);
	Delta=0.5*(limits[3]-limits[2]);
	AS = hess_struct_axi(this,Pot,{integrals[0],integrals[1],integrals[3]},limits,taubar,Delta);
	d2JnudI2[0][0] = GaussLegendreQuad(&d2Jnud2Eint_AxiSym,-.5*PI,.5*PI,&AS,order)/PI;
	d2JnudI2[0][1] = d2JnudI2[1][0] = GaussLegendreQuad(&d2JnudEdI2int_AxiSym,-.5*PI,.5*PI,&AS,order)/PI;
	d2JnudI2[0][2] = d2JnudI2[2][0] = GaussLegendreQuad(&d2JnudEdI3int_AxiSym,-.5*PI,.5*PI,&AS,order)/PI;
	d2JnudI2[1][1] = GaussLegendreQuad(&d2Jnud2I2int_AxiSym,-.5*PI,.5*PI,&AS,order)/PI;
	d2JnudI2[1][2] = d2JnudI2[2][1] = GaussLegendreQuad(&d2JnudI2dI3int_AxiSym,-.5*PI,.5*PI,&AS,order)/PI;
	d2JnudI2[2][2] = GaussLegendreQuad(&d2Jnud2I3int_AxiSym,-.5*PI,.5*PI,&AS,order)/PI;

	for(int i=0;i<3;i++)for(int j=0;j<3;j++){d2JlamdI2[i][j]*=lamFactor;d2JnudI2[i][j]*=nuFactor;}
	// d2JphidI2
	double d2JphidI2[3][3];
	d2JphidI2[0][0] = 0.0;
	d2JphidI2[0][1] = d2JphidI2[1][0] = 0.0;
	d2JphidI2[0][2] = d2JphidI2[2][0] = 0.0;
	d2JphidI2[1][1] = -0.5/(sqrt(2.0*integrals[1]*integrals[1]*integrals[1]));
	d2JphidI2[1][2] = d2JphidI2[2][1] = 0.0;
	d2JphidI2[2][2] = 0.0;

	// std::cout<<d2JphidI2[1][1]<<std::endl;

	double DeterminantSq = Determinant*Determinant;
	double dDeterminant[3];
	dDeterminant[0]=(d2JlamdI2[0][0]*dJdI[2][2]+d2JnudI2[2][0]*dJdI[0][0]
				-d2JlamdI2[0][2]*dJdI[2][0]-d2JnudI2[0][0]*dJdI[0][2])*dJdI[1][1];

	dDeterminant[1]=(d2JlamdI2[0][1]*dJdI[2][2]+d2JnudI2[2][1]*dJdI[0][0]
				-d2JlamdI2[1][2]*dJdI[2][0]-d2JnudI2[0][1]*dJdI[0][2])*dJdI[1][1]
				+d2JphidI2[1][1]*(dJdI[0][0]*dJdI[2][2]-dJdI[2][0]*dJdI[0][2]);

	dDeterminant[2]=(d2JlamdI2[0][2]*dJdI[2][2]+d2JnudI2[2][2]*dJdI[0][0]
				-d2JlamdI2[2][2]*dJdI[2][0]-d2JnudI2[0][2]*dJdI[0][2])*dJdI[1][1];
	double dFreqdI[3][3];
	for(int j=0;j<3;j++){
	dFreqdI[0][j]=-(1./(DeterminantSq))*(dDeterminant[j])*det2(dJdI[1][1],dJdI[1][2],dJdI[2][1],dJdI[2][2])
			+(1./Determinant)*(dJdI[2][2]*d2JphidI2[j][1]+dJdI[1][1]*d2JnudI2[j][2]-dJdI[2][1]*d2JphidI2[j][2]
			-dJdI[1][2]*d2JnudI2[j][1]);
	dFreqdI[1][j]=-(1./(DeterminantSq))*(dDeterminant[j])*det2(dJdI[2][1],dJdI[2][2],dJdI[0][1],dJdI[0][2])
			+(1./Determinant)*(dJdI[0][2]*d2JnudI2[j][1]+dJdI[2][1]*d2JlamdI2[j][2]-dJdI[0][1]*d2JnudI2[j][2]
			-dJdI[2][2]*d2JlamdI2[j][1]);
	dFreqdI[2][j]=-(1./(DeterminantSq))*(dDeterminant[j])*det2(dJdI[0][1],dJdI[0][2],dJdI[1][1],dJdI[1][2])
			+(1./Determinant)*(dJdI[1][2]*d2JlamdI2[j][1]+dJdI[0][1]*d2JphidI2[j][2]-dJdI[1][1]*d2JlamdI2[j][2]
			-dJdI[0][2]*d2JphidI2[j][1]);
	}
	for(int i=0;i<3;i++){
	for(int j=0;j<3;j++){
	Dij[i][j]=dFreqdI[i][0]*dIdJ[0][j]+dFreqdI[i][1]*dIdJ[1][j]+dFreqdI[i][2]*dIdJ[2][j];
	angles.push_back(Dij[i][j]);
	}}
	}

	return angles;
}

// ============================================================================
// Prolate Stackel Angle-action Fudge
// ============================================================================

double ptau2ROOT_AxiSym_Fudge(double tau, void *params){
	/* for finding roots of p_tau^2*2.0*(tau+Alpha)*(tau+Gamma)  */
	root_struct_axi_fudge *RS = (root_struct_axi_fudge *) params;
	double phi=0.;

	// //TACT: mu=0.
	// if(RS->swit==0)			phi = RS->ASF->chi_lam({tau,0.,RS->tau_i[2]});
	// else if(RS->swit==1)	phi = RS->ASF->chi_nu({RS->tau_i[0],0.,tau});
	//gjy changed: mu=mu0
	if(RS->swit==0)			phi = RS->ASF->chi_lam({tau,RS->tau_i[1],RS->tau_i[2]});
	else if(RS->swit==1)	phi = RS->ASF->chi_nu({RS->tau_i[0],RS->tau_i[1],tau});

	double Alpha = RS->ASF->CS->alpha(), Gamma = RS->ASF->CS->gamma();
	double p_= ( RS->Ints[0]*(tau+Gamma) -RS->Ints[1]*(tau+Gamma)/(tau+Alpha) -RS->Ints[2] +phi ); //gjy note: SB15, F(34)

	// //gjy note: double tau is the input tau(l or n), while tau_i[] is the initial tau0_i; for the first judge, it is just all tau0_i
	// printf("ptau2ROOT_AxiSym_Fudge(%d): p_ = %f, kp = %f, ptau2 = %f\n", RS->swit, p_, 2.*(RS->tau_i[0]+Alpha)*(RS->tau_i[0]+Gamma), 
	// p_/(2.*(RS->tau_i[0]+Alpha)*(RS->tau_i[0]+Gamma)) ); //gjy add
	// printf("RS->tau_i: "); print_vec(RS->tau_i); //gjy add
	// printf("a = %f, c = %f, tau = %f\n", Alpha, Gamma, tau); //gjy add
	// printf("E = %f, I1 = %f, I2 = %f\n", RS->Ints[0], RS->Ints[1], RS->Ints[2]); //gjy add
	// printf("each term when lambda: ptau2 = %f; E'(=RS->Ints[0]*(tau+Gamma)) = %f, -I1'(=RS->Ints[1]*(tau+Gamma)/(tau+Alpha)) = %f, +X'(=chi_lam(tau)) = %f, -K'(=RS->Ints[2]) = %f\n\n\n", 
	// p_/(2.*(RS->tau_i[0]+Alpha)*(RS->tau_i[0]+Gamma)), RS->Ints[0]*(tau+Gamma), -RS->Ints[1]*(tau+Gamma)/(tau+Alpha), phi, -RS->Ints[2] ); //gjy add
	return p_;
}

void Actions_AxisymmetricStackel_Fudge::integrals(const VecDoub& tau){

	double a = CS->alpha(), c = CS->gamma();

	double Pl2 = (tau[0]-tau[2])/((a+tau[0])*(c+tau[0]))/4.;
	double Pn2 = (tau[2]-tau[0])/((a+tau[2])*(c+tau[2]))/4.;

	double pl = tau[3]*Pl2; pl*=pl;
	double pn = tau[5]*Pn2; pn*=pn;
	if(pn!=pn) pn=0.;

	Kt[0] = -2.*(a+tau[0])*(c+tau[0])*pl +(tau[0]+c)*E -(tau[0]+c)/(tau[0]+a)*I2 +chi_lam(tau); //gjy add: 计算第三运动积分属的Rz //??
	Kt[1] = -2.*(a+tau[2])*(c+tau[2])*pn +(tau[2]+c)*E -(tau[2]+c)/(tau[2]+a)*I2 +chi_nu(tau);

	// printf("AASF::integrals(): Kt[0] = %f, Kt[1] = %f\n", Kt[0], Kt[1] ); //gjy add
	// printf("tau: "); print_vec(tau); //gjy add
	// printf("a = %f, c = %f\n", a, c); //gjy add
	// printf("E = %f, I1 = %f, Kt[0] = %f\n", E, I2, Kt[0]); //gjy add
	// printf("pl = %f, pn = %f\n", pl, pn); //gjy add
	// printf("each term when lambda: ptau2(=2.*(a+tau[0])*(c+tau[0])*pl) = %f; +(tau[0]+c)*E = %f, -(tau[0]+c)/(tau[0]+a)*I2 = %f, +chi_lam(tau) = %f, -Kt[0] = %f\n\n\n", 
	// 2.*(a+tau[0])*(c+tau[0])*pl, (tau[0]+c)*E, -(tau[0]+c)/(tau[0]+a)*I2, chi_lam(tau), -Kt[0] ); //gjy add
}

VecDoub Actions_AxisymmetricStackel_Fudge::find_limits(const VecDoub& tau){

	double lambda = tau[0], nu = tau[2];
	root_find RF(TINY,100);
	VecDoub limits;
	// create a structure to store parameters for ptau2ROOT
	VecDoub ints = {E,I2,Kt[0]};
	root_struct_axi_fudge RS_l(this,ints,tau,0);
	// printf("xv -> tau, ints: "); printVector(tau); printVector(ints); printf("\n\n");

	// find roots of p^2(lam)
	double laminner=lambda, lamouter=lambda;
	if(ptau2ROOT_AxiSym_Fudge(lambda, &RS_l)>=0.0){
		while(ptau2ROOT_AxiSym_Fudge(laminner, &RS_l)>=0.0
		      and (laminner+CS->alpha())>tiny_number)
			laminner-=.1*(laminner+CS->alpha());
		while(ptau2ROOT_AxiSym_Fudge(lamouter, &RS_l)>=0.)	lamouter*=1.1;

		if((laminner+CS->alpha())>tiny_number) //gjy note: 这个在最后一次while(){laminner}后
			limits.push_back(
			    RF.findroot(&ptau2ROOT_AxiSym_Fudge,laminner,lambda,&RS_l));
		else limits.push_back(-CS->alpha()); //gjy note: i.e. l<-a+tiny, then pushback -a; choose the bigger

		limits.push_back(RF.findroot(&ptau2ROOT_AxiSym_Fudge,lambda,lamouter,&RS_l));
		if(fabs(limits[0]-limits[1])<TINY){
		    if(ptau2ROOT_AxiSym_Fudge(lambda-5*TINY, &RS_l)>0.0) //gjy note: now pt2(li)??0 and pt2(l-5T)>0, root in the interval //??
				limits[0]=RF.findroot(&ptau2ROOT_AxiSym_Fudge,laminner,lambda-5*TINY,&RS_l);
	        else
				limits[1]=RF.findroot(&ptau2ROOT_AxiSym_Fudge,lambda+5*TINY,lamouter,&RS_l);
		}
	}
	else{ //gjy note: i.e.: if pt2(l)<0 (bad), then limits[0][1] are near l
		std::cout<<"we are in the no use else...\n"; //gjy add
		limits.push_back(lambda-tiny_number);
		limits.push_back(lambda+tiny_number);
	}
	limits.push_back(-CS->gamma()+TINY);

	// find root of p^2(nu)
	double nuouter=nu;
	ints[2]=Kt[1];
	root_struct_axi_fudge RS_n(this,ints,tau,1);

	if(ptau2ROOT_AxiSym_Fudge(nu, &RS_n)<=0.0){
		while(ptau2ROOT_AxiSym_Fudge(nuouter, &RS_n)<=0.
		      and -(nuouter+CS->alpha())>tiny_number)
			nuouter+=0.1*(-CS->alpha()-nuouter);
		if(-(nuouter+CS->alpha())>tiny_number){
			limits.push_back(RF.findroot(&ptau2ROOT_AxiSym_Fudge,nu,nuouter,&RS_n));
		}
		else limits.push_back(-CS->alpha());
	}
	else{ //gjy note: i.e.: if pt2(l)<0 (bad), then limits[0][1] are near l
		std::cout<<"we are in the no use else...\n"; //gjy add
		limits.push_back(nu+tiny_number);
	}
	return limits;
}

double J_integrand_AxiSym_Fudge(double theta, void *params){ //fudge的p_tau(theta')
	// need to set taubargl and Deltagl
	action_struct_axi_fudge * AS = (action_struct_axi_fudge *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double phi=0.;

	// //TACT: mu = 0.
	// if(AS->swit==0)			phi = AS->ASF->chi_lam({tau, 0., AS->tau_i[2]});
	// else if(AS->swit==1)		phi = AS->ASF->chi_nu({AS->tau_i[0], 0., tau});
	//gjy changed: mu = mu0
	if(AS->swit==0)			phi = AS->ASF->chi_lam({tau, AS->tau_i[1], AS->tau_i[2]});
	else if(AS->swit==1)	phi = AS->ASF->chi_nu({AS->tau_i[0], AS->tau_i[1], tau});

	double Alpha = AS->ASF->CS->alpha(), Gamma = AS->ASF->CS->gamma();
	double ptau2 = AS->Ints[0]*(tau+Gamma)-AS->Ints[1]*(tau+Gamma)/(tau+Alpha)-AS->Ints[2]+phi;
	
    // std::cout<<"J_integrand_AxiSym_Fudge: tau = "<<tau<<", Alpha = "<<Alpha<<", Gamma = "<<Gamma<<", (tau+Alpha)*(tau+Gamma)*2. = "<<(tau+Alpha)*(tau+Gamma)*2.<<"; ";
	// std::cout<<"J_: AS->taubargl = "<<AS->taubargl<<", Deltagl = "<<AS->Deltagl<<"\n"; //gjy add
    // std::cout<<"AS->Ints _123 = "<<AS->Ints[0]<<" "<<AS->Ints[1]<<" "<<AS->Ints[2]<<", phi = "<<phi<<"\n"; //gjy add
    // std::cout<<"ptau2 _1 = "<<ptau2<<", "; //gjy add
	ptau2 /= (tau+Alpha)*(tau+Gamma)*2.;
    // std::cout<<"ptau2 _2 = "<<ptau2<<"    "; printVector(AS->tau_i); //gjy add
	// // ptau2 = abs(ptau2); //gjy add
	return sqrt(MAX(0.,ptau2))*cos(theta);
}

double dJdE_integrand_AxiSym_Fudge(double theta, void *params){
	// need to set taubargl and Deltagl
	action_struct_axi_fudge * AS = (action_struct_axi_fudge *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double phi=0.;

	// // //TACT: mu = 0.
	// if(AS->swit==0)			phi = AS->ASF->chi_lam({tau,0.,AS->tau_i[2]});
	// else if(AS->swit==1)	phi = AS->ASF->chi_nu({AS->tau_i[0],0.,tau});
	//gjy changed: mu = mu0
	if(AS->swit==0)			phi = AS->ASF->chi_lam({tau, AS->tau_i[1], AS->tau_i[2]});
	else if(AS->swit==1)	phi = AS->ASF->chi_nu({AS->tau_i[0], AS->tau_i[1], tau});

	double Alpha = AS->ASF->CS->alpha(), Gamma = AS->ASF->CS->gamma();
	double ptau2 = AS->Ints[0]*(tau+Gamma)-AS->Ints[1]*(tau+Gamma)/(tau+Alpha)-AS->Ints[2]+phi;
	ptau2 /= (tau+Alpha)*(tau+Gamma)*2.;
	return 0.25*cos(theta)/(sqrt(MAX(AS->tiny_number,ptau2))*(tau+Alpha));
}

double dJdI2_integrand_AxiSym_Fudge(double theta, void *params){
	// need to set taubargl and Deltagl
	action_struct_axi_fudge * AS = (action_struct_axi_fudge *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double phi=0.;

	// // //TACT: mu = 0.
	// if(AS->swit==0)			phi = AS->ASF->chi_lam({tau,0.,AS->tau_i[2]});
	// else if(AS->swit==1)	phi = AS->ASF->chi_nu({AS->tau_i[0],0.,tau});
	//gjy changed: mu = mu0
	if(AS->swit==0)			phi = AS->ASF->chi_lam({tau, AS->tau_i[1], AS->tau_i[2]});
	else if(AS->swit==1)	phi = AS->ASF->chi_nu({AS->tau_i[0], AS->tau_i[1], tau});

	double Alpha = AS->ASF->CS->alpha(), Gamma = AS->ASF->CS->gamma();
	double ptau2 = AS->Ints[0]*(tau+Gamma)-AS->Ints[1]*(tau+Gamma)/(tau+Alpha)-AS->Ints[2]+phi;
	ptau2 /= (tau+Alpha)*(tau+Gamma)*2.;
	return -0.25*cos(theta)/(sqrt(MAX(AS->tiny_number,ptau2))*(tau+Alpha)*(tau+Alpha));
}

double dJdI3_integrand_AxiSym_Fudge(double theta, void *params){
	// need to set taubargl and Deltagl
	action_struct_axi_fudge * AS = (action_struct_axi_fudge *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double phi=0.;

	// // //TACT: mu = 0.
	// if(AS->swit==0)			phi = AS->ASF->chi_lam({tau,0.,AS->tau_i[2]});
	// else if(AS->swit==1)	phi = AS->ASF->chi_nu({AS->tau_i[0],0.,tau});
	//gjy changed: mu = mu0
	if(AS->swit==0)			phi = AS->ASF->chi_lam({tau, AS->tau_i[1], AS->tau_i[2]});
	else if(AS->swit==1)	phi = AS->ASF->chi_nu({AS->tau_i[0], AS->tau_i[1], tau});

	double Alpha = AS->ASF->CS->alpha(), Gamma = AS->ASF->CS->gamma();
	double ptau2 = AS->Ints[0]*(tau+Gamma)-AS->Ints[1]*(tau+Gamma)/(tau+Alpha)-AS->Ints[2]+phi;
	ptau2 /= (tau+Alpha)*(tau+Gamma)*2.;
	return -0.25*cos(theta)/(sqrt(MAX(AS->tiny_number,ptau2))*(tau+Alpha)*(tau+Gamma));
}

VecDoub Actions_AxisymmetricStackel_Fudge::actions(const VecDoub& x, void *params){

// DEBUG_PRINT_I(7111);
	// printf("ASF actions() called.\n"); //gjy add
    // printf("phi(x) = %f\n", Pot->Phi(x)); //gjy add
    // printf("x[0] = %f\n", x[0]); //gjy add

    VecDoub actions(3,0.);
    if(action_check(x,actions,Pot)){
    	// std::cout<<"here AAF1 actions = "<<actions[0]<<" "<<actions[1]<<" "<<actions[2]<<" "<<actions[3]<<"\n"; //gjy add
		return actions;
	}
    // std::cout<<"here AAF1 actions = "<<actions[0]<<" "<<actions[1]<<" "<<actions[2]<<" "<<actions[3]<<"\n"; //gjy add

// DEBUG_PRINT_I(7112);
	// printf("CS->alpha()1: %f\n", CS->alpha()); //gjy add
    if(params){ //gjy note: if none NULL *params, then determine alpha by it
		double *deltaguess = (double *) params;
		if(*deltaguess>0.) CS->newalpha(-Pot->DeltaGuess(x)+CS->gamma());
		else CS->newalpha(*deltaguess);
		// printf("CS->alpha()2: %f\n", CS->alpha()); //gjy add
	}
	if(CS->alpha()>CS->gamma()) CS->newalpha(CS->gamma()-0.1);
	// printf("CS->alpha()3: %f\n", CS->alpha()); //gjy add

	VecDoub tau = CS->xv2tau(x);
	// If exactly on the plane we integrate a bit
	if(x[2]==0.)
		tau = CS->xv2tau(integrate_a_bit(x,Pot));

	E = Pot->H(x); //gjy note: calculate AASF::E,I2 here
	if(E>0){
		std::cout<<"You have passed an unbound orbit:"<<std::endl;
		std::cout<<"E=Pot->H(x)="<<E<<std::endl; //gjy add
		printVector(x);
		VecDoub actions(4,std::numeric_limits<double>::infinity()); //gjy add: end inf... and go to a new cycle
		return actions; //gjy add
	}

	I2 = 0.5*(x[0]*x[4]-x[1]*x[3])*(x[0]*x[4]-x[1]*x[3]); //Lz->0.5*Lz^2
	integrals(tau);
// DEBUG_PRINT_I(7116);
	VecDoub limits = find_limits(tau);
    // printf("\nASF limits and tau = \n\n"); printVector(limits); printVector(tau); //gjy add
    // std::cout<<"here AAF2 actions = "<<actions[0]<<" "<<actions[1]<<" "<<actions[2]<<"\n"; //gjy add

	// JR
	double taubar = 0.5*(limits[0]+limits[1]);
	double Delta = 0.5*(limits[1]-limits[0]);
	VecDoub ints = {E,I2,Kt[0]};

	action_struct_axi_fudge AS(this,ints,tau,taubar,Delta,0, 0.);
	actions[0]=Delta*GaussLegendreQuad(&J_integrand_AxiSym_Fudge,-.5*PI,.5*PI,&AS)/PI; //??
	
	// Lz
	actions[1]=Pot->Lz(x);
	// Jz
	taubar = 0.5*(limits[2]+limits[3]);
	Delta = 0.5*(limits[3]-limits[2]);
	ints[2]=Kt[1];
    // std::cout<<"here AAF3 actions = "<<actions[0]<<" "<<actions[1]<<" "<<actions[2]<<"\n"; //gjy add

	// printf("AST actions[2]:\n\n"); //gjy add
	AS = action_struct_axi_fudge(this,ints,tau,taubar,Delta,1, 0.);
	actions[2]=2.*Delta*GaussLegendreQuad(&J_integrand_AxiSym_Fudge,-.5*PI,.5*PI,&AS)/PI;
	// printf("AST the last1 actions[2] = %f\n", actions[2]); //gjy add
	if(x[2]==0. and x[5]==0.){
		// printf("AST x25 = %f %f\n", x[2], x[5]); //gjy add
		// printf("AST the last2 actions[2] = %f\n", actions[2]); //gjy add
		actions[2]=0.;
	}
    // std::cout<<"here AAF4 actions = "<<actions[0]<<" "<<actions[1]<<" "<<actions[2]<<"\n"; //gjy add
	return actions;
}

VecDoub Actions_AxisymmetricStackel_Fudge::angles(const VecDoub& x, void *params){

    // printf("x[0] = %f\n", x[0]); //gjy add
    VecDoub angles(6,0.);
    if(angle_check(x,angles,Pot)) return angles;

	if(params){
		double *deltaguess = (double *) params;
		if(*deltaguess>0.) CS->newalpha(-Pot->DeltaGuess(x)+CS->gamma());
		else CS->newalpha(*deltaguess);
	}
	if(CS->alpha()>CS->gamma()){
	    if(debug_NegativeDelta)
	    	std::cerr<<"Negative Delta at R="<<sqrt(x[0]*x[0]+x[1]*x[1])<<", z="<<x[2]<<std::endl;
        CS->newalpha(CS->gamma()-0.1);
	}
	VecDoub tau = CS->xv2tau(x);
	// If exactly on the plane we integrate a bit
	if(x[2]==0.)
		tau = CS->xv2tau(integrate_a_bit(x,Pot));
	double tn = (x[3]*x[3]+x[4]*x[4]+x[5]*x[5])/5.e11;
	E = Pot->H(x); I2 = 0.5*(x[0]*x[4]-x[1]*x[3])*(x[0]*x[4]-x[1]*x[3]);
	integrals(tau);
	VecDoub limits = find_limits(tau);

	// thetaR
	double taubar = 0.5*(limits[0]+limits[1]);
	double Delta = 0.5*(limits[1]-limits[0]);
	VecDoub ints = {E,I2,Kt[0]};
	action_struct_axi_fudge AS(this,ints,tau,taubar,Delta,0, tn);
	VecDoub dJdIl(3,0);
	dJdIl[0] = Delta*GaussLegendreQuad(&dJdE_integrand_AxiSym_Fudge,-.5*PI,.5*PI,&AS)/PI;
	dJdIl[1] = Delta*GaussLegendreQuad(&dJdI2_integrand_AxiSym_Fudge,-.5*PI,.5*PI,&AS)/PI;
	dJdIl[2] = Delta*GaussLegendreQuad(&dJdI3_integrand_AxiSym_Fudge,-.5*PI,.5*PI,&AS)/PI;
	VecDoub dSdI(3,0);
	double thetaLim = asin(MAX(-1.,MIN(1.,(tau[0]-taubar)/Delta)));
	double lsign = SIGN(tau[3]);
	dSdI[0] += lsign*Delta*GaussLegendreQuad(&dJdE_integrand_AxiSym_Fudge,-.5*PI,thetaLim,&AS);
	dSdI[1] += lsign*Delta*GaussLegendreQuad(&dJdI2_integrand_AxiSym_Fudge,-.5*PI,thetaLim,&AS);
	dSdI[2] += lsign*Delta*GaussLegendreQuad(&dJdI3_integrand_AxiSym_Fudge,-.5*PI,thetaLim,&AS);
	// Lz
	VecDoub dJdIp = {0.,1./sqrt(2.*I2),0.};
	dSdI[1]+=SIGN(tau[4])*tau[1]/sqrt(2.*I2);
	// Jz
	taubar = 0.5*(limits[2]+limits[3]);
	Delta = 0.5*(limits[3]-limits[2]);
	ints[2]=Kt[1];
	AS = action_struct_axi_fudge(this,ints,tau,taubar,Delta,1, tn);
	VecDoub dJdIn(3,0);
	dJdIn[0] = 2.*Delta*GaussLegendreQuad(&dJdE_integrand_AxiSym_Fudge,-.5*PI,.5*PI,&AS)/PI;
	dJdIn[1] = 2.*Delta*GaussLegendreQuad(&dJdI2_integrand_AxiSym_Fudge,-.5*PI,.5*PI,&AS)/PI;
	dJdIn[2] = 2.*Delta*GaussLegendreQuad(&dJdI3_integrand_AxiSym_Fudge,-.5*PI,.5*PI,&AS)/PI;
	thetaLim = asin(MAX(-1.,MIN(1.,(tau[2]-taubar)/Delta)));
	lsign = SIGN(tau[5]);
	dSdI[0] += lsign*Delta*GaussLegendreQuad(&dJdE_integrand_AxiSym_Fudge,-.5*PI,thetaLim,&AS);
	dSdI[1] += lsign*Delta*GaussLegendreQuad(&dJdI2_integrand_AxiSym_Fudge,-.5*PI,thetaLim,&AS);
	dSdI[2] += lsign*Delta*GaussLegendreQuad(&dJdI3_integrand_AxiSym_Fudge,-.5*PI,thetaLim,&AS);
	double Determinant = dJdIp[1]*(dJdIl[0]*dJdIn[2]-dJdIl[2]*dJdIn[0]);

	double dIdJ[3][3];
	dIdJ[0][0] = det2(dJdIp[1],dJdIp[2],dJdIn[1],dJdIn[2])/Determinant;
	dIdJ[0][1] = -det2(dJdIl[1],dJdIl[2],dJdIn[1],dJdIn[2])/Determinant;
	dIdJ[0][2] = det2(dJdIl[1],dJdIl[2],dJdIp[1],dJdIp[2])/Determinant;
	dIdJ[1][0] = -det2(dJdIp[0],dJdIp[2],dJdIn[0],dJdIn[2])/Determinant;
	dIdJ[1][1] = det2(dJdIl[0],dJdIl[2],dJdIn[0],dJdIn[2])/Determinant;
	dIdJ[1][2] = -det2(dJdIl[0],dJdIl[2],dJdIp[0],dJdIp[2])/Determinant;
	dIdJ[2][0] = det2(dJdIp[0],dJdIp[1],dJdIn[0],dJdIn[1])/Determinant;
	dIdJ[2][1] = -det2(dJdIl[0],dJdIl[1],dJdIn[0],dJdIn[1])/Determinant;
	dIdJ[2][2] = det2(dJdIl[0],dJdIl[1],dJdIp[0],dJdIp[1])/Determinant;

	angles[0]=dSdI[0]*dIdJ[0][0]+dSdI[1]*dIdJ[1][0]+dSdI[2]*dIdJ[2][0];
	angles[1]=dSdI[0]*dIdJ[0][1]+dSdI[1]*dIdJ[1][1]+dSdI[2]*dIdJ[2][1];
	angles[2]=dSdI[0]*dIdJ[0][2]+dSdI[1]*dIdJ[1][2]+dSdI[2]*dIdJ[2][2];
	angles[3]=det2(dJdIp[1],dJdIp[2],dJdIn[1],dJdIn[2])/Determinant;
	angles[4]=det2(dJdIn[1],dJdIn[2],dJdIl[1],dJdIl[2])/Determinant;
	angles[5]=det2(dJdIl[1],dJdIl[2],dJdIp[1],dJdIp[2])/Determinant;

	angles[4]*=SIGN(Pot->Lz(x));

	if(tau[5]<0.0){angles[2]+=PI;}
	if(x[2]<0.0){angles[2]+=PI;}
	if(angles[2]>2.*PI)	angles[2]-=2.0*PI;
	if(angles[0]<0.0)	angles[0]+=2.0*PI;
	if(angles[1]<0.0)	angles[1]+=2.0*PI;
	if(angles[2]<0.0)	angles[2]+=2.0*PI;

	return angles;
}

// ============================================================================
// Triaxial Stackel Angle-action calculator
// ============================================================================

double ptau2ROOT_Triax(double tau, void *params){
	/* for finding roots of p_tau^2*2.0*(tau+Alpha)  */
	//gjy note: Z85, ptau(50), G(23)
	root_struct_triax *RS = (root_struct_triax *) params;
	return (RS->Ints[0]-RS->Ints[1]/(tau+RS->P->alpha())-RS->Ints[2]/(tau+RS->P->gamma())+RS->P->G(tau))
			/(2.0*(tau+RS->P->beta()));
	}

VecDoub Actions_TriaxialStackel::find_limits(const VecDoub& tau,const VecDoub& ints){

	double lambda = tau[0], mu=tau[1], nu = tau[2];
	root_find RF(SMALL,100);
	VecDoub limits;
	// create a structure to store parameters for ptau2ROOT
	root_struct_triax RS(Pot,ints);

	// find roots of p^2(lam)
	double laminner=lambda, lamouter=lambda;
	while(ptau2ROOT_Triax(laminner, &RS)>0.0 and (laminner+Pot->alpha())>SMALL)	laminner-=.1*(laminner+Pot->alpha());
	if((laminner+Pot->alpha())>SMALL) limits.push_back(RF.findroot(&ptau2ROOT_Triax,laminner,lambda,&RS));
	else limits.push_back(-Pot->alpha());
	while(ptau2ROOT_Triax(lamouter, &RS)>0.)	lamouter*=1.1;
	limits.push_back(RF.findroot(&ptau2ROOT_Triax,lambda,lamouter,&RS));

	// find root of p^2(mu)
	double muinner=mu, muouter=mu;
	while(ptau2ROOT_Triax(muinner, &RS)>0. and (muinner+Pot->beta())>SMALL)	muinner-=.1*(muinner+Pot->beta());
	if((muinner+Pot->beta())>SMALL) limits.push_back(RF.findroot(&ptau2ROOT_Triax,muinner,mu,&RS));
	else limits.push_back(-Pot->beta());
	while(ptau2ROOT_Triax(muouter, &RS)>0. and (muouter+Pot->alpha())<-SMALL)	muouter+=0.1*(-Pot->alpha()-muouter);
	if((muouter+Pot->alpha())<-SMALL) limits.push_back(RF.findroot(&ptau2ROOT_Triax,mu,muouter,&RS));
	else limits.push_back(-Pot->alpha());

	// find root of p^2(nu)
	double nuinner=nu, nuouter=nu;
	while(ptau2ROOT_Triax(nuinner, &RS)>0. and (nuinner+Pot->gamma())>SMALL)	nuinner-=.1*(nuinner+Pot->gamma());
	if((nuinner+Pot->gamma())>SMALL) limits.push_back(RF.findroot(&ptau2ROOT_Triax,nuinner,nu,&RS));
	else limits.push_back(-Pot->gamma());
	while(ptau2ROOT_Triax(nuouter, &RS)>0. and (nuouter+Pot->beta())<-SMALL)	nuouter+=0.1*(-Pot->beta()-nuouter);
	if((nuouter+Pot->beta())<-SMALL) limits.push_back(RF.findroot(&ptau2ROOT_Triax,nu,nuouter,&RS));
	else limits.push_back(-Pot->beta());

	return limits;
}

double J_integrand_Triax(double theta, void *params){
	// need to set taubargl and Deltagl
	action_struct_triax * AS = (action_struct_triax *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double ptau = ((tau+AS->P->alpha())*(tau+AS->P->gamma())*AS->Ints[0]
			-AS->Ints[1]*(tau+AS->P->gamma())-AS->Ints[2]*(tau+AS->P->alpha())
			+(tau+AS->P->alpha())*(tau+AS->P->gamma())*AS->P->G(tau))
			/(2.0*(tau+AS->P->alpha())*(tau+AS->P->beta())*(tau+AS->P->gamma()));
	return sqrt(MAX(0.,ptau))*cos(theta);
}

double dJdH_integrand_Triax(double theta, void *params){
	// need to set taubargl and Deltagl
	action_struct_triax * AS = (action_struct_triax *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double ptau = ((tau+AS->P->alpha())*(tau+AS->P->gamma())*AS->Ints[0]
			-AS->Ints[1]*(tau+AS->P->gamma())-AS->Ints[2]*(tau+AS->P->alpha())
			+(tau+AS->P->alpha())*(tau+AS->P->gamma())*AS->P->G(tau))
			/(2.0*(tau+AS->P->alpha())*(tau+AS->P->beta())*(tau+AS->P->gamma()));
	//gjy note: see Zeeuw, 1985, formula (127)
	return sqrt(MAX(0.,1./ptau))*cos(theta)/(tau+AS->P->beta());
}

double dJdI2_integrand_Triax(double theta, void *params){
	// need to set taubargl and Deltagl
	action_struct_triax * AS = (action_struct_triax *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double ptau = ((tau+AS->P->alpha())*(tau+AS->P->gamma())*AS->Ints[0]
			-AS->Ints[1]*(tau+AS->P->gamma())-AS->Ints[2]*(tau+AS->P->alpha())
			+(tau+AS->P->alpha())*(tau+AS->P->gamma())*AS->P->G(tau))
			/(2.0*(tau+AS->P->alpha())*(tau+AS->P->beta())*(tau+AS->P->gamma()));
	return -sqrt(MAX(0.,1./ptau))*cos(theta)/(tau+AS->P->beta())/(tau+AS->P->alpha());
}

double dJdI3_integrand_Triax(double theta, void *params){
	// need to set taubargl and Deltagl
	action_struct_triax * AS = (action_struct_triax *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double ptau = ((tau+AS->P->alpha())*(tau+AS->P->gamma())*AS->Ints[0]
			-AS->Ints[1]*(tau+AS->P->gamma())-AS->Ints[2]*(tau+AS->P->alpha())
			+(tau+AS->P->alpha())*(tau+AS->P->gamma())*AS->P->G(tau))
			/(2.0*(tau+AS->P->alpha())*(tau+AS->P->beta())*(tau+AS->P->gamma()));
	return -sqrt(MAX(0.,1./ptau))*cos(theta)/(tau+AS->P->beta())/(tau+AS->P->gamma());
}

//// gjy notes: actions(), xv t
VecDoub Actions_TriaxialStackel::actions(const VecDoub& x, void *params){ //gjy note: it has not only actions but also angles and frequencies

	VecDoub tau = Pot->xv2tau(x);

	VecDoub integrals = Pot->tau2ints(tau);
	VecDoub limits = find_limits(tau,integrals);
	VecDoub actions, freqs;

	// We need to check which coordinates are oscillating and which are circulating
	// and multiply by the appropriate factor
	VecDoub circ={1.,1.,1.};
	if(limits[0]==-Pot->alpha()) circ[0] = 1.; else circ[0] = 0.5;
	// if(limits[2]==-Pot->beta() and limits[3]==-Pot->alpha()) circ[1] = 1.; else circ[1] = 0.5;
	// if(limits[4]==-Pot->gamma() and limits[5]==-Pot->beta()) circ[2] = 0.5; else circ[2] = 1.;

	// JR
	double taubar = 0.5*(limits[0]+limits[1]);
	double Delta = 0.5*(limits[1]-limits[0]);
	action_struct_triax AS(Pot,integrals,taubar,Delta);
	actions.push_back(2.*circ[0]*Delta*GaussLegendreQuad(&J_integrand_Triax,-.5*PI,.5*PI,&AS)/PI);

	if(params){
		freqs.push_back(0.5*circ[0]*Delta*GaussLegendreQuad(&dJdH_integrand_Triax,-.5*PI,.5*PI,&AS)/PI);
		freqs.push_back(0.5*circ[0]*Delta*GaussLegendreQuad(&dJdI2_integrand_Triax,-.5*PI,.5*PI,&AS)/PI);
		freqs.push_back(0.5*circ[0]*Delta*GaussLegendreQuad(&dJdI3_integrand_Triax,-.5*PI,.5*PI,&AS)/PI);
	}
	// return actions;

	// Jp
	taubar = 0.5*(limits[2]+limits[3]);
	Delta = 0.5*(limits[3]-limits[2]);
	AS = action_struct_triax(Pot,integrals,taubar,Delta);
	actions.push_back(2.*circ[1]*Delta*GaussLegendreQuad(&J_integrand_Triax,-.5*PI,.5*PI,&AS)/PI);

	if(params){
		freqs.push_back(0.5*circ[1]*Delta*GaussLegendreQuad(&dJdH_integrand_Triax,-.5*PI,.5*PI,&AS)/PI);
		freqs.push_back(0.5*circ[1]*Delta*GaussLegendreQuad(&dJdI2_integrand_Triax,-.5*PI,.5*PI,&AS)/PI);
		freqs.push_back(0.5*circ[1]*Delta*GaussLegendreQuad(&dJdI3_integrand_Triax,-.5*PI,.5*PI,&AS)/PI);
	}

	// Jz
	taubar = 0.5*(limits[4]+limits[5]);
	Delta = 0.5*(limits[5]-limits[4]);
	AS = action_struct_triax(Pot,integrals,taubar,Delta);
	actions.push_back(2.*circ[2]*Delta*GaussLegendreQuad(&J_integrand_Triax,-.5*PI,.5*PI,&AS)/PI);

	if(params){
		freqs.push_back(0.5*circ[2]*Delta*GaussLegendreQuad(&dJdH_integrand_Triax,-.5*PI,.5*PI,&AS)/PI);
		freqs.push_back(0.5*circ[2]*Delta*GaussLegendreQuad(&dJdI2_integrand_Triax,-.5*PI,.5*PI,&AS)/PI);
		freqs.push_back(0.5*circ[2]*Delta*GaussLegendreQuad(&dJdI3_integrand_Triax,-.5*PI,.5*PI,&AS)/PI);
		double det = freqs[0]*(freqs[4]*freqs[8]-freqs[5]*freqs[7])
					-freqs[1]*(freqs[3]*freqs[8]-freqs[5]*freqs[6])
					+freqs[2]*(freqs[3]*freqs[7]-freqs[4]*freqs[6]);
		actions.push_back((freqs[4]*freqs[8]-freqs[5]*freqs[7])/det);
		actions.push_back((freqs[7]*freqs[2]-freqs[8]*freqs[1])/det);
		actions.push_back((freqs[1]*freqs[5]-freqs[2]*freqs[4])/det);
	}

	return actions;
}



// ============================================================================
// Triaxial Stackel Fudge
// ============================================================================
//gjy note: here only triaxial Fudge without focus estimation (in lmn_orb.h/cpp)

double ptau2ROOT_Triax_Fudge(double tau, void *params){
	/* for finding roots of p_tau^2*2.0*(tau+Alpha)*(tau+Beta)*(tau+Gamma)  */
	root_struct_triax_fudge *RS = (root_struct_triax_fudge *) params;
	double phi=0.;
	if(RS->swit==0)			phi = RS->ATSF->chi_lam({tau,RS->tau_i[1],RS->tau_i[2]});
	else if(RS->swit==1)	phi = RS->ATSF->chi_mu({RS->tau_i[0],tau,RS->tau_i[2]});
	else if(RS->swit==2)	phi = RS->ATSF->chi_nu({RS->tau_i[0],RS->tau_i[1],tau});
	double p = (RS->Ints[0]*tau*tau-RS->Ints[1]*tau+RS->Ints[2]+phi); //gjy note: SB15, (17)
	// double pttt = p/(2.0*(tau+a)*(tau+b)*(tau+c)); //gjy note
	// std::cout<<"ptau2ROOT_Triax_Fudge: "<<tau<<" "<<phi<<" "<<p<<"\n"; //gjy add
	return p;
}

void Actions_TriaxialStackel_Fudge::integrals(const VecDoub& tau){
	//gjy note: to calculate pseudo integrals in Fudge
	double a = CS->alpha(), b = CS->beta(), c = CS->gamma();

	double lf = (a+tau[0])*(b+tau[0])*(c+tau[0]);
	double mf = (a+tau[1])*(b+tau[1])*(c+tau[1]);
	double nf = (a+tau[2])*(b+tau[2])*(c+tau[2]);

	double Pl2 = (tau[0]-tau[1])*(tau[0]-tau[2])/lf/4.;
	double Pm2 = (tau[1]-tau[2])*(tau[1]-tau[0])/mf/4.;
	double Pn2 = (tau[2]-tau[1])*(tau[2]-tau[0])/nf/4.;

	double pl = tau[3]*Pl2; pl*=pl;
	double pm = tau[4]*Pm2; pm*=pm;
	double pn = tau[5]*Pn2; pn*=pn;

	//gjy note: the A_\tau, B_\tau of in Pater S15
	Jt[0]=(tau[1]+tau[2])*E+0.5*pm/Pm2*(tau[0]-tau[1])+0.5*pn/Pn2*(tau[0]-tau[2]);
	Jt[1]=(tau[0]+tau[2])*E+0.5*pl/Pl2*(tau[1]-tau[0])+0.5*pn/Pn2*(tau[1]-tau[2]);
	Jt[2]=(tau[0]+tau[1])*E+0.5*pl/Pl2*(tau[2]-tau[0])+0.5*pm/Pm2*(tau[2]-tau[1]);
	Kt[0]=2.*lf*pl-tau[0]*tau[0]*E+tau[0]*Jt[0]-chi_lam(tau);
	Kt[1]=2.*mf*pm-tau[1]*tau[1]*E+tau[1]*Jt[1]-chi_mu(tau);
	Kt[2]=2.*nf*pn-tau[2]*tau[2]*E+tau[2]*Jt[2]-chi_nu(tau);
}

VecDoub Actions_TriaxialStackel_Fudge::find_limits(const VecDoub& tau){

	double lambda = tau[0], mu=tau[1], nu = tau[2];
	root_find RF(1e-15,1000);
	VecDoub limits;
	VecDoub ints = {E,Jt[0],Kt[0]};
	root_struct_triax_fudge RS_l(this,ints,tau,0);
	// std::cout<<"\n\n\nTSF find_limits = "<<tau[0]<<" "<<tau[1]<<" "<<tau[2]<<", "<<ints[0]<<" "<<ints[1]<<" "<<ints[2]<<std::endl; //gjy add

	// find roots of p^2(lam)
	double laminner=lambda, lamouter=lambda;
	if(ptau2ROOT_Triax_Fudge(lambda, &RS_l)>0.0){
		while(ptau2ROOT_Triax_Fudge(laminner, &RS_l)>0.0 and (laminner+CS->alpha())>tiny_number)	laminner-=.1*(laminner+CS->alpha()); //gjy note: minus 0.1times variance
		if((laminner+CS->alpha())>tiny_number) limits.push_back(RF.findroot(&ptau2ROOT_Triax_Fudge,laminner,lambda,&RS_l));
		else limits.push_back(-CS->alpha());
		while(ptau2ROOT_Triax_Fudge(lamouter, &RS_l)>0.)	lamouter*=1.1;
		limits.push_back(RF.findroot(&ptau2ROOT_Triax_Fudge,lambda,lamouter,&RS_l));
	}
	else{
		limits.push_back(lambda-tiny_number);limits.push_back(lambda+tiny_number);
		std::cout<<"we are in the no use else...\n"; //gjy add
	}
	// std::cout<<"limits_range of lambda = "<<lamouter-laminner<<std::endl; //gjy add

	// find root of p^2(mu)
	ints[1]=Jt[1];ints[2]=Kt[1];
	root_struct_triax_fudge RS_m(this,ints,tau,1);
	double muinner=mu, muouter=mu;
	if(ptau2ROOT_Triax_Fudge(mu, &RS_m)<0.0){
	while(ptau2ROOT_Triax_Fudge(muinner, &RS_m)<0. and (muinner+CS->beta())>tiny_number)	muinner-=.1*(muinner+CS->beta());
	if((muinner+CS->beta())>tiny_number) limits.push_back(RF.findroot(&ptau2ROOT_Triax_Fudge,muinner,mu,&RS_m));
	else limits.push_back(-CS->beta());
	while(ptau2ROOT_Triax_Fudge(muouter, &RS_m)<0. and (muouter+CS->alpha())<-tiny_number)	muouter+=0.1*(-CS->alpha()-muouter);
	if((muouter+CS->alpha())<-tiny_number) limits.push_back(RF.findroot(&ptau2ROOT_Triax_Fudge,mu,muouter,&RS_m));
	else limits.push_back(-CS->alpha());
	}
	else{ limits.push_back(mu-tiny_number);limits.push_back(mu+tiny_number);}

	// find root of p^2(nu)
	ints[1]=Jt[2];ints[2]=Kt[2];
	root_struct_triax_fudge RS_n(this,ints,tau,2);
	double nuinner=nu, nuouter=nu;
	if(ptau2ROOT_Triax_Fudge(nu, &RS_n)>0.0){
	while(ptau2ROOT_Triax_Fudge(nuinner, &RS_n)>0. and (nuinner+CS->gamma())>tiny_number)	nuinner-=.1*(nuinner+CS->gamma());
	if((nuinner+CS->gamma())>tiny_number) limits.push_back(RF.findroot(&ptau2ROOT_Triax_Fudge,nuinner,nu,&RS_n));
	else limits.push_back(-CS->gamma());
	while(ptau2ROOT_Triax_Fudge(nuouter, &RS_n)>0. and (nuouter+CS->beta())<-tiny_number)	nuouter+=0.1*(-CS->beta()-nuouter);
	if((nuouter+CS->beta())<-tiny_number) limits.push_back(RF.findroot(&ptau2ROOT_Triax_Fudge,nu,nuouter,&RS_n));
	else limits.push_back(-CS->beta());
	}
	else{ limits.push_back(nu-tiny_number);limits.push_back(nu+tiny_number);}

	// std::cout<<"find_limits() end..."<<std::endl; //gjy add
	return limits;
}

double J_integrand_Triax_Fudge(double theta, void *params){
	// need to set taubargl and Deltagl
	action_struct_triax_fudge * AS = (action_struct_triax_fudge *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double a = AS->ATSF->CS->alpha(), b = AS->ATSF->CS->beta(), c = AS->ATSF->CS->gamma();
	double phi=0.;
	if(AS->swit==0)			phi = AS->ATSF->chi_lam({tau,AS->tau_i[1],AS->tau_i[2]});
	else if(AS->swit==1)	phi = AS->ATSF->chi_mu({AS->tau_i[0],tau,AS->tau_i[2]});
	else if(AS->swit==2)	phi = AS->ATSF->chi_nu({AS->tau_i[0],AS->tau_i[1],tau});
	double ptau = (AS->Ints[0]*tau*tau-AS->Ints[1]*tau+AS->Ints[2]+phi)/(2.0*(tau+a)*(tau+b)*(tau+c));
	// DEBUG_PRINT_V0d(1, AS->swit, "J_integrand_Triax_Fudge: AS->swit");
	double ptau_return = sqrt(MAX(0., ptau))*cos(theta); //gjy add
	if(AS->ATSF->is_recording){
		AA_integrating_data AD;
		AD.swit = AS->swit;
		AD.theta = theta;
		AD.alpha = a;
		AD.beta = b;
		AD.gamma = c;
		AD.tau = tau;
		AD.phichi = phi;
		AD.ptau = ptau;
		AD.ptau_return = ptau_return;
// DEBUG_PRINT_I(6612);
		AS->ATSF->PAD[AS->swit].push_back(AD);
// DEBUG_PRINT_I(6613);
	}
	return ptau_return;
}

double dJdE_integrand_Triax_Fudge(double theta, void *params){
	// need to set taubargl and Deltagl
	action_struct_triax_fudge * AS = (action_struct_triax_fudge *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta); //gjy note: samples are 5*2*pi range but only in a circle.
	double a = AS->ATSF->CS->alpha(), b = AS->ATSF->CS->beta(), c = AS->ATSF->CS->gamma();
	double phi=0.;
	if(AS->swit==0)			phi = AS->ATSF->chi_lam({tau,AS->tau_i[1],AS->tau_i[2]});
	else if(AS->swit==1)	phi = AS->ATSF->chi_mu({AS->tau_i[0],tau,AS->tau_i[2]});
	else if(AS->swit==2)	phi = AS->ATSF->chi_nu({AS->tau_i[0],AS->tau_i[1],tau});
	double ptau = (AS->Ints[0]*tau*tau-AS->Ints[1]*tau+AS->Ints[2]+phi)/(2.0*(tau+a)*(tau+b)*(tau+c));
	return 0.25*cos(theta)*tau*tau/(sqrt(MAX(1e-6,ptau))*(tau+a)*(tau+b)*(tau+c));
}

double dJdJ_integrand_Triax_Fudge(double theta, void *params){
	// need to set taubargl and Deltagl
	action_struct_triax_fudge * AS = (action_struct_triax_fudge *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double a = AS->ATSF->CS->alpha(), b = AS->ATSF->CS->beta(), c = AS->ATSF->CS->gamma();
	double phi=0.;
	if(AS->swit==0)			phi = AS->ATSF->chi_lam({tau,AS->tau_i[1],AS->tau_i[2]});
	else if(AS->swit==1)	phi = AS->ATSF->chi_mu({AS->tau_i[0],tau,AS->tau_i[2]});
	else if(AS->swit==2)	phi = AS->ATSF->chi_nu({AS->tau_i[0],AS->tau_i[1],tau});
	double ptau = (AS->Ints[0]*tau*tau-AS->Ints[1]*tau+AS->Ints[2]+phi)/(2.0*(tau+a)*(tau+b)*(tau+c));
	//gjy note: 前面都一样, 其实可以复用
	return -0.25*cos(theta)*tau/(sqrt(MAX(1e-6,ptau))*(tau+a)*(tau+b)*(tau+c));
}

double dJdK_integrand_Triax_Fudge(double theta, void *params){
	// need to set taubargl and Deltagl
	action_struct_triax_fudge * AS = (action_struct_triax_fudge *) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double a = AS->ATSF->CS->alpha(), b = AS->ATSF->CS->beta(), c = AS->ATSF->CS->gamma();
	double phi=0.;
	if(AS->swit==0)			phi = AS->ATSF->chi_lam({tau,AS->tau_i[1],AS->tau_i[2]});
	else if(AS->swit==1)	phi = AS->ATSF->chi_mu({AS->tau_i[0],tau,AS->tau_i[2]});
	else if(AS->swit==2)	phi = AS->ATSF->chi_nu({AS->tau_i[0],AS->tau_i[1],tau});
	double ptau = (AS->Ints[0]*tau*tau-AS->Ints[1]*tau+AS->Ints[2]+phi)/(2.0*(tau+a)*(tau+b)*(tau+c));
	return 0.25*cos(theta)/(sqrt(MAX(1e-6,ptau))*(tau+a)*(tau+b)*(tau+c));
}

VecDoub Actions_TriaxialStackel_Fudge::actions(const VecDoub& x, void *params){
	// Remember that J_\tau = 2/\pi \int_{\tau_-}^{\tau_+} p_\tau d\tau
	// This produces a J_r a factor of two large for the loop orbits
	// if(debug_triaxial_stackel) printVector(x);
	//gjy note: fast method
// DEBUG_PRINT_I("SFudge called");
// DEBUG_PRINT_V1d(1, x, "input x");

	VecDoub tau = CS->xv2tau(x);
	E = Pot->H(x);
	if(E>0){
		std::cout<<"You have passed an unbound orbit:"<<std::endl;
		std::cout<<"E=Pot->H(x)="<<E<<std::endl;
		printVector(x);
		VecDoub actions(4,std::numeric_limits<double>::infinity()); //gjy add: end inf... and go to a new cycle
		return actions;
	}
    // std::cout<<"here1 TSF\n"; //gjy add
	integrals(tau); //gjy note: to obtain the values of Jt, Kt, i.e. At, Bt
	VecDoub limits = find_limits(tau);
	// DEBUG_PRINT_V1d(1, limits, "limits_fudge");

	VecDoub actions, freqs;
	// JR
	VecDoub ints = {E,Jt[0],Kt[0]};
	double taubar = 0.5*(limits[0]+limits[1]);
	double Delta = 0.5*(limits[1]-limits[0]);
	action_struct_triax_fudge AS(this,ints,tau,taubar,Delta,0);
	VecDoub circ={1.,1.,1.};
	// if(limits[0]<-CS->alpha()+SMALL) circ[0] = 1.; else circ[0] = 0.5;

// DEBUG_PRINT_I(11);
	int order = 8;
	// int order = 16; //gjy changed
// DEBUG_PRINT_V0d(1, is_record_action_integrand, "actions: is_record before");
	if(is_record_action_integrand){ //gjy add: NOT safe for much memory, one should set 0 and swap() the vector to free each time after recording
		is_recording = 1;
	}
	actions.push_back(2.*circ[0]*Delta*GaussLegendreQuad(&J_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS,order)/PI);
// DEBUG_PRINT_V0d(1, is_record_action_integrand, "actions: is_record after");
	if(is_record_action_integrand){ //gjy add
		is_recording = 0;
	}
// DEBUG_PRINT_I(12);

	if(freq_yes){
		freqs.push_back(0.5*circ[0]*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS,order)/PI);
		freqs.push_back(0.5*circ[0]*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS,order)/PI);
		freqs.push_back(0.5*circ[0]*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS,order)/PI);
	}
	//gjy note: 注意这句积分里的的参数&AS(有tau和ints)是固定的, 而在整个这个大函数里tau(x)是作为参数(变的)的, 这里只有是边界的范围内theta(tau)
	//作为积分变量是变的, 从而在三轴fudge文中又有积分积没了\tau, 又有当前点\tau_0的ints, 从而有了轨道各点都能算出一个作用量

	// Jp
	ints[1]=Jt[1];ints[2]=Kt[1];
	taubar = 0.5*(limits[2]+limits[3]);
	Delta = 0.5*(limits[3]-limits[2]);
	// if(limits[0]<-CS->alpha()+SMALL and limits[5]>-CS->beta()-SMALL) circ[1] = .5; else circ[1] = 1.;

	AS = action_struct_triax_fudge(this,ints,tau,taubar,Delta,1);
	if(is_record_action_integrand){ //gjy add: NOT safe for much memory, one should set 0 and swap() the vector to free each time after recording
		is_recording = 1;
	}
	actions.push_back(2.*circ[1]*Delta*GaussLegendreQuad(&J_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS,order)/PI);
	if(is_record_action_integrand){ //gjy add
		is_recording = 0;
	}

	if(freq_yes){
		freqs.push_back(0.5*circ[1]*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS,order)/PI);
		freqs.push_back(0.5*circ[1]*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS,order)/PI);
		freqs.push_back(0.5*circ[1]*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS,order)/PI);
	}

	// Jz
	ints[1]=Jt[2];ints[2]=Kt[2];
	taubar = 0.5*(limits[4]+limits[5]);
	Delta = 0.5*(limits[5]-limits[4]);
	AS = action_struct_triax_fudge(this,ints,tau,taubar,Delta,2);
	if(is_record_action_integrand){ //gjy add: NOT safe for much memory, one should set 0 and swap() the vector to free each time after recording
		is_recording = 1;
	}
	actions.push_back(2.*circ[2]*Delta*GaussLegendreQuad(&J_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS,order)/PI);
	if(is_record_action_integrand){ //gjy add
		is_recording = 0;
	}

	if(freq_yes){
		freqs.push_back(0.5*circ[2]*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS,order)/PI);
		freqs.push_back(0.5*circ[2]*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS,order)/PI);
		freqs.push_back(0.5*circ[2]*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS,order)/PI);
	}

	//gjy note: triaxial potential orbits classifying
	// 0 box, 1 short-axis loop, 2 inner long-axis loop, 3 outer long-axis loop
	if(limits[2]<-CS->beta()+tiny_number and limits[3]>-CS->alpha()-tiny_number)
		actions.push_back(1);
	else if(limits[5]>-CS->beta()-tiny_number and limits[0]<-CS->alpha()+tiny_number)actions.push_back(2);
	else if(limits[5]>-CS->beta()-tiny_number and limits[3]>-CS->alpha()-tiny_number)actions.push_back(3);
	else if(limits[0]<-CS->alpha()+tiny_number and limits[2]<-CS->beta()+tiny_number and limits[4]<-CS->gamma()+tiny_number)actions.push_back(0);
	else{
		if(limits[0]<-CS->alpha()+tiny_number) //looks a bit like a box...
			actions.push_back(0);
		else if(limits[3]>-CS->alpha()-tiny_number){
			//looks a bit like a short-axis loop...
			if(actions[1]>actions[2])
				actions.push_back(1);
			else if(actions[1]<actions[2])
				actions.push_back(3);
			else
				actions.push_back(-1);
		}
		else if(limits[5]>-CS->beta()-tiny_number){
			actions.push_back(2);
		}
		else if(actions[0]>actions[1])
			actions.push_back(0);
		else if(actions[1]>actions[0])
			actions.push_back(0);
		else{
			actions.push_back(-1);
			std::cout<<CS->alpha()<<" "<<CS->beta()<<" ";
			for(auto i: limits) std::cout<<i<<" ";
			for(auto i: actions) std::cout<<i<<" ";std::cout<<std::endl;
		}
	}

	for (VecDoub::iterator i = actions.begin(); i != actions.end(); ++i)
		if(std::isinf(*i) or std::isnan(*i) or (*i)!=(*i))
			(*i)=-1e20;

	if(freq_yes){
		double det = freqs[0]*(freqs[4]*freqs[8]-freqs[5]*freqs[7])
					-freqs[1]*(freqs[3]*freqs[8]-freqs[5]*freqs[6])
					+freqs[2]*(freqs[3]*freqs[7]-freqs[4]*freqs[6]);
		actions.push_back((freqs[4]*freqs[8]-freqs[5]*freqs[7])/det);
		actions.push_back((freqs[7]*freqs[2]-freqs[8]*freqs[1])/det);
		actions.push_back((freqs[1]*freqs[5]-freqs[2]*freqs[4])/det);
	}

	if(debug_triaxial_stackel) printVector(actions);

	// if(actions[3]==2){
	// 	double tmp = actions[0];
	// 	actions[0]=actions[1];
	// 	actions[1]=tmp;
	// }

	// for(auto i:actions)std::cout<<i<<" ";
	// std::cout<<E<<" ";
	// for(auto i:Jt)std::cout<<i<<" ";
	// for(auto i:Kt)std::cout<<i<<" ";
	// for(auto i:tau)std::cout<<i<<" ";
	// 	std::cout<<std::endl;

	// for(auto i:x)std::cout<<i<<" ";
	// for(auto i:actions)std::cout<<i<<" ";
	// std::cout<<std::endl;

// DEBUG_PRINT_I(13);
    // std::cout<<"here-1 ATSF actions = "<<actions[0]<<" "<<actions[1]<<" "<<actions[2]<<" "<<actions[3]<<"\n"; //gjy add
	return actions;
}

VecDoub Actions_TriaxialStackel_Fudge::angles(const VecDoub& x, void *params){
	// This function needs some tweaking -- it is currently giving dodgy results for Omega_z

// DEBUG_PRINT_V1d(1, x, "input x");
	VecDoub tau = CS->xv2tau(x);
	E = Pot->H(x);
	if(E>0){
		std::cout<<"You have passed an unbound orbit:"<<std::endl;
		std::cout<<"E=Pot->H(x)="<<E<<std::endl;
		printVector(x);
		VecDoub angles(11,std::numeric_limits<double>::infinity()); //gjy add: end inf... and go to a new cycle
		return angles;
	}

	integrals(tau);
	VecDoub limits = find_limits(tau);
// DEBUG_PRINT_I(651);
	VecDoub angles;

	// lam
	VecDoub ints = {E,Jt[0],Kt[0]};
	double taubar = 0.5*(limits[0]+limits[1]);
	double Delta = 0.5*(limits[1]-limits[0]);
	action_struct_triax_fudge AS(this,ints,tau,taubar,Delta,0);
	double circ = 0.5;
	if(limits[0]==-CS->alpha()) circ = 1.;
	VecDoub dJdIl(3,0);
// DEBUG_PRINT_I(652);
	dJdIl[0] = 2.*circ*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS)/PI;
// DEBUG_PRINT_I(653);
	dJdIl[1] = 2.*circ*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS)/PI;
	dJdIl[2] = 2.*circ*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS)/PI;
	VecDoub dSdI(3,0);
	double thetaLim = asin(MAX(-1.,MIN(1.,(tau[0]-taubar)/Delta)));
	double lsign = SIGN(tau[3]);
	dSdI[0] += lsign*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge,-.5*PI,thetaLim,&AS);
	dSdI[1] += lsign*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge,-.5*PI,thetaLim,&AS);
	dSdI[2] += lsign*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge,-.5*PI,thetaLim,&AS);

	if(x[0]<0 and circ==1.)  for(int i=0;i<3;i++) dSdI[i]+=dJdIl[i]*PI;
	if(lsign<0) for(int i=0;i<3;i++) dSdI[i]+=dJdIl[i]*PI/circ;

	// Lz
	ints[1]=Jt[1];ints[2]=Kt[1];
	taubar = 0.5*(limits[2]+limits[3]);
	Delta = 0.5*(limits[3]-limits[2]);
	AS = action_struct_triax_fudge(this,ints,tau,taubar,Delta,1);
	VecDoub dJdIm(3,0);
	circ = 1;
	if(limits[3]!=-CS->alpha() and limits[2]!=-CS->beta()) circ = 0.5;
	dJdIm[0] = 2.*circ*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS)/PI;
	dJdIm[1] = 2.*circ*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS)/PI;
	dJdIm[2] = 2.*circ*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS)/PI;
	thetaLim = asin(MAX(-1.,MIN(1.,(tau[1]-taubar)/Delta)));
	lsign = SIGN(tau[4]);
	dSdI[0] += lsign*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge,-.5*PI,thetaLim,&AS);
	dSdI[1] += lsign*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge,-.5*PI,thetaLim,&AS);
	dSdI[2] += lsign*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge,-.5*PI,thetaLim,&AS);

	// Short and Box
	if(limits[2]==-CS->beta()){
		if(lsign<0)for(int i=0;i<3;i++) dSdI[i]+=dJdIm[i]*PI;
		if(x[1]<0) for(int i=0;i<3;i++) dSdI[i]+=dJdIm[i]*PI;
	}
	// Outer long-axis loop
	else if(limits[3]==-CS->alpha() and limits[2]!=-CS->beta()){
		for(int i=0;i<3;i++) dSdI[i]=dJdIm[i]*PI/2.+dSdI[i];
		if(x[0]<0) for(int i=0;i<3;i++) dSdI[i]+=dJdIm[i]*PI;
		// if(lsign>0)for(int i=0;i<3;i++) dSdI[i]+=dJdIm[i]*PI;
	}
	// Inner long-axis loop
	else if(limits[3]!=-CS->alpha() and limits[2]!=-CS->beta()){
		if(lsign<0) for(int i=0;i<3;i++) dSdI[i]+=dJdIl[i]*PI/circ;
	}

	// Jz
	ints[1]=Jt[2];ints[2]=Kt[2];
	taubar = 0.5*(limits[4]+limits[5]);
	Delta = 0.5*(limits[5]-limits[4]);
	AS = action_struct_triax_fudge(this,ints,tau,taubar,Delta,2);
	VecDoub dJdIn(3,0);
	dJdIn[0] = 2.*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS)/PI;
	dJdIn[1] = 2.*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS)/PI;
	dJdIn[2] = 2.*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge,-.5*PI,.5*PI,&AS)/PI;
	thetaLim = asin(MAX(-1.,MIN(1.,(tau[2]-taubar)/Delta)));
	lsign = SIGN(tau[5]);
	dSdI[0] += lsign*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge,-.5*PI,thetaLim,&AS);
	dSdI[1] += lsign*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge,-.5*PI,thetaLim,&AS);
	dSdI[2] += lsign*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge,-.5*PI,thetaLim,&AS);

	if(x[2]<0)  for(int i=0;i<3;i++) dSdI[i]+=dJdIn[i]*PI;
	if(lsign<0) for(int i=0;i<3;i++) dSdI[i]+=dJdIn[i]*PI;

	double Determinant = dJdIl[0]*det2(dJdIm[1],dJdIm[2],dJdIn[1],dJdIn[2])
						- dJdIl[1]*det2(dJdIm[0],dJdIm[2],dJdIn[0],dJdIn[2])
						+ dJdIl[2]*det2(dJdIm[0],dJdIm[1],dJdIn[0],dJdIn[1]);

	double dIdJ[3][3];
	dIdJ[0][0] = det2(dJdIm[1],dJdIm[2],dJdIn[1],dJdIn[2])/Determinant;
	dIdJ[0][1] = -det2(dJdIl[1],dJdIl[2],dJdIn[1],dJdIn[2])/Determinant;
	dIdJ[0][2] = det2(dJdIl[1],dJdIl[2],dJdIm[1],dJdIm[2])/Determinant;
	dIdJ[1][0] = -det2(dJdIm[0],dJdIm[2],dJdIn[0],dJdIn[2])/Determinant;
	dIdJ[1][1] = det2(dJdIl[0],dJdIl[2],dJdIn[0],dJdIn[2])/Determinant;
	dIdJ[1][2] = -det2(dJdIl[0],dJdIl[2],dJdIm[0],dJdIm[2])/Determinant;
	dIdJ[2][0] = det2(dJdIm[0],dJdIm[1],dJdIn[0],dJdIn[1])/Determinant;
	dIdJ[2][1] = -det2(dJdIl[0],dJdIl[1],dJdIn[0],dJdIn[1])/Determinant;
	dIdJ[2][2] = det2(dJdIl[0],dJdIl[1],dJdIm[0],dJdIm[1])/Determinant;

	angles.push_back(dSdI[0]*dIdJ[0][0]+dSdI[1]*dIdJ[1][0]+dSdI[2]*dIdJ[2][0]);
	angles.push_back(dSdI[0]*dIdJ[0][1]+dSdI[1]*dIdJ[1][1]+dSdI[2]*dIdJ[2][1]);
	angles.push_back(dSdI[0]*dIdJ[0][2]+dSdI[1]*dIdJ[1][2]+dSdI[2]*dIdJ[2][2]);
	angles.push_back(det2(dJdIm[1],dJdIm[2],dJdIn[1],dJdIn[2])/Determinant);
	angles.push_back(det2(dJdIn[1],dJdIn[2],dJdIl[1],dJdIl[2])/Determinant);
	angles.push_back(det2(dJdIl[1],dJdIl[2],dJdIm[1],dJdIm[2])/Determinant);

	for(int i=0;i<3;i++){
		if(angles[i]<0.)angles[i]+=2.*PI;
		if(angles[i]>2.*PI)angles[i]-=2.*PI;
		angles.push_back(x[i]);
	}
	angles.push_back(tau[1]);
	angles.push_back(SIGN(tau[4]));
	return angles;
}

double Actions_TriaxialStackel_Fudge::sos(const VecDoub& x, int comp, const std::string& file){
	std::ofstream outfile; outfile.open(file, std::ios::app);
	outfile<<x[comp]/*sqrt(x[0]*x[0]+x[1]*x[1]/0.95/0.95+x[2]*x[2]/0.85/0.85)*/<<" "<<x[comp+3]<<std::endl;

	VecDoub tau = CS->xv2tau(x);
	E = Pot->H(x);
	integrals(tau);
	VecDoub limits = find_limits(tau);
	double a = CS->alpha(), b = CS->beta(), c = CS->gamma();
	double up = 0., down = 0.; int coord = 0;
	if(comp==0){ down = limits[0]+SMALL; up = limits[1]-SMALL; coord = 0;}
	if(comp==1){ down = limits[0]+SMALL; up = limits[1]-SMALL; coord = 0;}
	if(comp==2){ down = limits[2]+SMALL; up = limits[3]-SMALL; coord = 1;}
	VecDoub ints = {E,Jt[coord],Kt[coord]};

	double l;
	for(int n=0;n<200;n++){
		l = sqrt(down+a)+n*(sqrt(up+a)-sqrt(down+a))/199.;
		l = l*l-a;
		//to let l=down when n=0 or l=up when n=199 //comp is index ijk??
		if(comp==0){
			outfile<<sqrt(MAX(0.,l+a))<<" ";
			if(fabs(l+b)<SMALL or fabs(l+c)<SMALL) outfile<<0.;
			else outfile<<sqrt(MAX(0.,2.*(ints[0]*l*l-ints[1]*l+ints[2]+chi_lam({l,tau[1],tau[2]}))/((l+b)*(l+c))));
		}
		if(comp==1){
			outfile<<sqrt(MAX(0.,l+b))<<" ";
			if(fabs(l+a)<SMALL or fabs(l+c)<SMALL) outfile<<0.;
			else outfile<<sqrt(MAX(0.,2.*(ints[0]*l*l-ints[1]*l+ints[2]+chi_lam({l,tau[1],tau[2]}))/((l+a)*(l+c))));
		}
		if(comp==2){
			outfile<<sqrt(MAX(0.,l+c))<<" ";
			if(fabs(l+a)<SMALL or fabs(l+b)<SMALL) outfile<<0.;
			else outfile<<sqrt(MAX(0.,2.*(ints[0]*l*l-ints[1]*l+ints[2]+chi_mu({tau[0],l,tau[2]}))/((l+a)*(l+b))));
		}
		outfile<<std::endl;
	}
	outfile.close();
	return 0;
}



// ============================================================================
// Triaxial
// gjy add: direct integrating by orbit data
// ============================================================================

double ptau2_orbdata(double theta, void *params){
	action_struct_triax_fudge_orbdata* AS = (action_struct_triax_fudge_orbdata*) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double a = AS->ATSF->CS->alpha(), b = AS->ATSF->CS->beta(), c = AS->ATSF->CS->gamma(); //before
	// double phi=0.; phi = AS->ATSF->chi_lam({tau,AS->tau_i[1],AS->tau_i[2]});
	double ptau = AS->ATSF->Pot->pSTAGE->ptau_interp(tau, AS->swit, AS->leaf); //donot need t
	ptau = ptau*ptau;
	return ptau;
}

double J_integrand_Triax_Fudge_orbdata(double theta, void *params){
	action_struct_triax_fudge_orbdata* AS = (action_struct_triax_fudge_orbdata*) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double a = AS->ATSF->CS->alpha(), b = AS->ATSF->CS->beta(), c = AS->ATSF->CS->gamma(); //before
	// double phi=0.; phi = AS->ATSF->chi_lam({tau,AS->tau_i[1],AS->tau_i[2]});
	double ptau = AS->ATSF->Pot->pSTAGE->ptau_interp(tau, AS->swit, AS->leaf); //donot need t
	ptau = ptau*ptau;
	// DEBUG_PRINT_V0d(1, tau, "J_integrand_Triax_Fudge_orbdata: tau");
	// DEBUG_PRINT_V0d(2, ptau, "ptau");
	return sqrt(MAX(0.,ptau))*cos(theta);
}

double dJdE_integrand_Triax_Fudge_orbdata(double theta, void *params){
	action_struct_triax_fudge_orbdata* AS = (action_struct_triax_fudge_orbdata*) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double a = AS->ATSF->CS->alpha(), b = AS->ATSF->CS->beta(), c = AS->ATSF->CS->gamma(); //before
	// double phi=0.; phi = AS->ATSF->chi_lam({tau,AS->tau_i[1],AS->tau_i[2]});
	double ptau = AS->ATSF->Pot->pSTAGE->ptau_interp(tau, AS->swit, AS->leaf); //donot need t
	ptau = ptau*ptau;
	return 0.25*cos(theta)*tau*tau/(sqrt(MAX(1e-6,ptau))*(tau+a)*(tau+b)*(tau+c));
}

double dJdJ_integrand_Triax_Fudge_orbdata(double theta, void *params){
	action_struct_triax_fudge_orbdata* AS = (action_struct_triax_fudge_orbdata*) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double a = AS->ATSF->CS->alpha(), b = AS->ATSF->CS->beta(), c = AS->ATSF->CS->gamma(); //before
	// double phi=0.; phi = AS->ATSF->chi_lam({tau,AS->tau_i[1],AS->tau_i[2]});
	double ptau = AS->ATSF->Pot->pSTAGE->ptau_interp(tau, AS->swit, AS->leaf); //donot need t
	ptau = ptau*ptau;
	return -0.25*cos(theta)*tau/(sqrt(MAX(1e-6,ptau))*(tau+a)*(tau+b)*(tau+c));
}

double dJdK_integrand_Triax_Fudge_orbdata(double theta, void *params){
	action_struct_triax_fudge_orbdata* AS = (action_struct_triax_fudge_orbdata*) params;
	double tau=AS->taubargl+AS->Deltagl*sin(theta);
	double a = AS->ATSF->CS->alpha(), b = AS->ATSF->CS->beta(), c = AS->ATSF->CS->gamma(); //before
	// double phi=0.; phi = AS->ATSF->chi_lam({tau,AS->tau_i[1],AS->tau_i[2]});
	double ptau = AS->ATSF->Pot->pSTAGE->ptau_interp(tau, AS->swit, AS->leaf); //donot need t
	ptau = ptau*ptau;
	return 0.25*cos(theta)/(sqrt(MAX(1e-6,ptau))*(tau+a)*(tau+b)*(tau+c));
}

VecDoub Actions_TriaxialStackel_Fudge::actions(const int& ID, const double& t0, void *params){
	// Remember that J_\tau = 2/\pi \int_{\tau_-}^{\tau_+} p_\tau d\tau
	// This produces a J_r a factor of two large for the loop orbits
	// integration method, algm = 0: GaussLegendreQuad; algm = 1: trapezoidal; 2: others...
// DEBUG_PRINT_I("D-TACT TEPPOD called");
// DEBUG_PRINT_V0d(10, ID, "ID");

	// int s0 = t_to_loadsnapshot(t0, this->Pot->pSTAGE->dt_load, this->Pot->pSTAGE->t_init, 0.);
	// // if(debug_triaxial_stackel) printVector(x);
	// VecDoub x = {this->Pot->pSTAGE->SS[s0]->P[ID].Pos[0], this->Pot->pSTAGE->SS[s0]->P[ID].Pos[1], this->Pot->pSTAGE->SS[s0]->P[ID].Pos[2], 
	// 	this->Pot->pSTAGE->SS[s0]->P[ID].Vel[0], this->Pot->pSTAGE->SS[s0]->P[ID].Vel[1], this->Pot->pSTAGE->SS[s0]->P[ID].Vel[2] };
	// DEBUG_PRINT_V1d(1, x, "x_orbdata");
	// VecDoub tau = CS->xv2tau(x);
	// E = Pot->H(x);
	// if(E>0){
	// 	std::cout<<"You have passed an unbound orbit:"<<std::endl;
	// 	std::cout<<"E=Pot->H(x)="<<E<<std::endl;
	// 	printVector(x);
	// 	// VecDoub actions(4, std::numeric_limits<double>::infinity()); //gjy add: end inf... and go to a new cycle
	// 	VecDoub actions(26, std::numeric_limits<double>::infinity()); //gjy add: end inf... and go to a new cycle
	// 	return actions;
	// }

	int s0 = t_to_loadsnapshot(t0, this->Pot->pSTAGE->dt_load, this->Pot->pSTAGE->t_init, 0.);
	//:: integrals() may not be used when donot use integrals in this actionmethod
	// integrals(tau); //gjy note: to obtain the values of Jt, Kt, i.e. At, Bt
	// VecDoub limits = find_limits(tau);
	int bot;
	VecDoub limits(6), timelimits(6);
	vector<int> counts(3);

	//gjy changed:
	bool is_doubleleaf = false;
	// double Alpha = TACT_semiaxis_Alpha, Beta = TACT_semiaxis_Beta, Gamma = -1.;
	// this->Pot->pSTAGE->set_ConfocalEllipsoidalCoordSys_focus(Alpha, Beta);
	// this->Pot->pSTAGE->set_ConfocalEllipsoidalCoordSys_focus(CS->alpha(), CS->beta()); //??
	double Alpha = this->Pot->pSTAGE->CS->alpha(), Beta = this->Pot->pSTAGE->CS->beta(), Gamma = this->Pot->pSTAGE->CS->gamma();
	// this->Pot->pSTAGE->set_info_particle(ID);
DEBUG_PRINT_I(623);
	this->Pot->pSTAGE->search_orbitPesudoPeriod(ID, t0, Alpha, Beta, 10000);
DEBUG_PRINT_I(624);
	auto AA0 = this->Pot->pSTAGE->integrate_actions(is_doubleleaf, 0);
	auto BOT = this->Pot->pSTAGE->get_badOrbitType();
	auto CS = this->Pot->pSTAGE->get_countSamples_orbitApproxPeriod();
	auto TAUL = this->Pot->pSTAGE->get_taulimits_estimatedfrom_orbitApproxPeriod();
	auto TIMEL = this->Pot->pSTAGE->get_timelimits_estimatedfrom_orbitApproxPeriod();
	//->reset() and ->write() should be in out of this function
	//put results in leaf0 to variables in this function
	int leaf = 0;
	bot = BOT;
	limits[0] = TAUL[0][leaf][0], limits[1] = TAUL[0][leaf][1]; //lambda-+
	limits[2] = TAUL[1][leaf][0], limits[3] = TAUL[1][leaf][1]; //mu-+
	limits[4] = TAUL[2][leaf][0], limits[5] = TAUL[2][leaf][1]; //nu-+
	timelimits[0] = TIMEL[0][leaf][0], timelimits[1] = TIMEL[0][leaf][1]; //lambda-+
	timelimits[2] = TIMEL[1][leaf][0], timelimits[3] = TIMEL[1][leaf][1]; //mu-+
	timelimits[4] = TIMEL[2][leaf][0], timelimits[5] = TIMEL[2][leaf][1]; //nu-+
	counts[0] = CS[0][leaf]; //lambda-+
	counts[1] = CS[1][leaf]; //mu-+
	counts[2] = CS[2][leaf]; //nu-+
// DEBUG_PRINT_I(625);

	/*
	auto COEFS = this->Pot->pSTAGE->calculate_Coefs_spline3();
	VecDoub actions, freqs;
	// JR
	VecDoub ints = {E,Jt[0],Kt[0]};
	double taubar = 0.5*(limits[0]+limits[1]);
	double Delta = 0.5*(limits[1]-limits[0]);
	action_struct_triax_fudge_orbdata AS(this,tau,taubar,Delta,ID,0,leaf); //gjy changed
	VecDoub circ={1.,1.,1.};
	// if(limits[0]<-Alpha+SMALL) circ[0] = 1.; else circ[0] = 0.5;

	int order = 8;
	// int order = 16; //gjy changed
	actions.push_back(2.*circ[0]*Delta*GaussLegendreQuad(&J_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS,order)/PI);
	// double a0 = 2.*circ[0]*Delta*GaussLegendreQuad(&J_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS,order)/PI;
	// DEBUG_PRINT_V0d(0, a0, "a0");

	if(freq_yes){
		freqs.push_back(0.5*circ[0]*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS,order)/PI);
		freqs.push_back(0.5*circ[0]*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS,order)/PI);
		freqs.push_back(0.5*circ[0]*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS,order)/PI);
	}

	// Jp
	ints[1]=Jt[1];ints[2]=Kt[1];
	taubar = 0.5*(limits[2]+limits[3]);
	Delta = 0.5*(limits[3]-limits[2]);
	// if(limits[0]<-Alpha+SMALL and limits[5]>-Beta-SMALL) circ[1] = .5; else circ[1] = 1.;

	AS = action_struct_triax_fudge_orbdata(this,tau,taubar,Delta,ID,1,leaf);
	actions.push_back(2.*circ[1]*Delta*GaussLegendreQuad(&J_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS,order)/PI);

	if(freq_yes){
		freqs.push_back(0.5*circ[1]*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS,order)/PI);
		freqs.push_back(0.5*circ[1]*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS,order)/PI);
		freqs.push_back(0.5*circ[1]*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS,order)/PI);
	}

	// Jz
	ints[1]=Jt[2];ints[2]=Kt[2];
	taubar = 0.5*(limits[4]+limits[5]);
	Delta = 0.5*(limits[5]-limits[4]);
	AS = action_struct_triax_fudge_orbdata(this,tau,taubar,Delta,ID,2,leaf);
	actions.push_back(2.*circ[2]*Delta*GaussLegendreQuad(&J_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS,order)/PI);

	if(freq_yes){
		freqs.push_back(0.5*circ[2]*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS,order)/PI);
		freqs.push_back(0.5*circ[2]*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS,order)/PI);
		freqs.push_back(0.5*circ[2]*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS,order)/PI);
	}

	//gjy note: triaxial potential orbits classifying
	// 0 box, 1 short-axis loop, 2 inner long-axis loop, 3 outer long-axis loop
	if(limits[2]<-Beta+tiny_number and limits[3]>Alpha-tiny_number)
		actions.push_back(1);
	else if(limits[5]>-Beta-tiny_number and limits[0]<-Alpha+tiny_number) actions.push_back(2);
	else if(limits[5]>-Beta-tiny_number and limits[3]>-Alpha-tiny_number) actions.push_back(3);
	else if(limits[0]<-Alpha+tiny_number and limits[2]<-Beta+tiny_number and limits[4]<-Gamma+tiny_number) actions.push_back(0);
	else{
		if(limits[0]<-Alpha+tiny_number) //looks a bit like a box...
			actions.push_back(0);
		else if(limits[3]>-Alpha-tiny_number){
			//looks a bit like a short-axis loop...
			if(actions[1]>actions[2])
				actions.push_back(1);
			else if(actions[1]<actions[2])
				actions.push_back(3);
			else
				actions.push_back(-1);
		}
		else if(limits[5]>-Beta-tiny_number){
			actions.push_back(2);
		}
		else if(actions[0]>actions[1])
			actions.push_back(0);
		else if(actions[1]>actions[0])
			actions.push_back(0);
		else{
			actions.push_back(-1);
			std::cout<<Alpha<<" "<<Beta<<" ";
			for(auto i: limits) std::cout<<i<<" ";
			for(auto i: actions) std::cout<<i<<" ";std::cout<<std::endl;
		}
	}

	for (VecDoub::iterator i = actions.begin(); i != actions.end(); ++i)
		if(std::isinf(*i) or std::isnan(*i) or (*i)!=(*i))
			(*i)=-1e20;

	if(freq_yes){
		double det = freqs[0]*(freqs[4]*freqs[8]-freqs[5]*freqs[7])
					-freqs[1]*(freqs[3]*freqs[8]-freqs[5]*freqs[6])
					+freqs[2]*(freqs[3]*freqs[7]-freqs[4]*freqs[6]);
		actions.push_back((freqs[4]*freqs[8]-freqs[5]*freqs[7])/det);
		actions.push_back((freqs[7]*freqs[2]-freqs[8]*freqs[1])/det);
		actions.push_back((freqs[1]*freqs[5]-freqs[2]*freqs[4])/det);
	}

	if(debug_triaxial_stackel) printVector(actions);
	// if(actions[3]==2){
	// 	double tmp = actions[0];
	// 	actions[0]=actions[1];
	// 	actions[1]=tmp;
	// }
	// for(auto i:x)std::cout<<i<<" ";
	// for(auto i:actions)std::cout<<i<<" ";
	// std::cout<<E<<" ";
	// for(auto i:Jt)std::cout<<i<<" ";
	// for(auto i:Kt)std::cout<<i<<" ";
	// for(auto i:tau)std::cout<<i<<" ";
	// 	std::cout<<std::endl;
	*/
	VecDoub actions;
	actions.resize(4, 1.);

	VecDoub AARETURN;
	for(auto a:{AA0[0],AA0[1],AA0[2]}){AARETURN.push_back(a);} //index: 0:6
	AARETURN.push_back((double)bot); //18
	for(auto a:{AA0[3],AA0[4],AA0[5]}){AARETURN.push_back(a);} //index: 0:6
// DEBUG_PRINT_I(626);
	for(auto a:limits){AARETURN.push_back(a);} //6:12
	for(auto a:timelimits){AARETURN.push_back(a);} //12:18
	for(auto a:counts){AARETURN.push_back((double)a);} //19:22
	for(auto a:actions){AARETURN.push_back(a);} //22:26 //the last 4 variables are for reference
	// return actions;
// DEBUG_PRINT_I(627);
	return AARETURN;
}

VecDoub Actions_TriaxialStackel_Fudge::angles(const double& t0, const int& ID, void *params){
	// This function needs some tweaking -- it is currently giving dodgy results for Omega_z
	int s0 = t_to_loadsnapshot(t0, this->Pot->pSTAGE->dt_load, this->Pot->pSTAGE->t_init, 0.);
	VecDoub x = {this->Pot->pSTAGE->SS[s0]->P[ID].Pos[0], this->Pot->pSTAGE->SS[s0]->P[ID].Pos[1], this->Pot->pSTAGE->SS[s0]->P[ID].Pos[2], 
		this->Pot->pSTAGE->SS[s0]->P[ID].Vel[0], this->Pot->pSTAGE->SS[s0]->P[ID].Vel[1], this->Pot->pSTAGE->SS[s0]->P[ID].Vel[2] };
	VecDoub tau = CS->xv2tau(x);
	E = Pot->H(x);
	if(E>0){
		std::cout<<"You have passed an unbound orbit:"<<std::endl;
		std::cout<<"E=Pot->H(x)="<<E<<std::endl;
		printVector(x);
		VecDoub angles(11,std::numeric_limits<double>::infinity()); //gjy add: end inf... and go to a new cycle
		return angles;
	}

	integrals(tau);
	VecDoub limits = find_limits(tau);
	VecDoub angles;
	// lam
	VecDoub ints = {E,Jt[0],Kt[0]};
	double taubar = 0.5*(limits[0]+limits[1]);
	double Delta = 0.5*(limits[1]-limits[0]);
	action_struct_triax_fudge AS(this,ints,tau,taubar,Delta,0);
	double circ = 0.5;
	if(limits[0]==-CS->alpha()) circ = 1.;
	VecDoub dJdIl(3,0);
	dJdIl[0] = 2.*circ*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS)/PI;
	dJdIl[1] = 2.*circ*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS)/PI;
	dJdIl[2] = 2.*circ*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS)/PI;
	VecDoub dSdI(3,0);
	double thetaLim = asin(MAX(-1.,MIN(1.,(tau[0]-taubar)/Delta)));
	double lsign = SIGN(tau[3]);
	dSdI[0] += lsign*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge_orbdata,-.5*PI,thetaLim,&AS);
	dSdI[1] += lsign*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge_orbdata,-.5*PI,thetaLim,&AS);
	dSdI[2] += lsign*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge_orbdata,-.5*PI,thetaLim,&AS);

	if(x[0]<0 and circ==1.)  for(int i=0;i<3;i++) dSdI[i]+=dJdIl[i]*PI;
	if(lsign<0) for(int i=0;i<3;i++) dSdI[i]+=dJdIl[i]*PI/circ;

	// Lz
	ints[1]=Jt[1];ints[2]=Kt[1];
	taubar = 0.5*(limits[2]+limits[3]);
	Delta = 0.5*(limits[3]-limits[2]);
	AS = action_struct_triax_fudge(this,ints,tau,taubar,Delta,1);
	VecDoub dJdIm(3,0);
	circ = 1;
	if(limits[3]!=-CS->alpha() and limits[2]!=-CS->beta()) circ = 0.5;
	dJdIm[0] = 2.*circ*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS)/PI;
	dJdIm[1] = 2.*circ*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS)/PI;
	dJdIm[2] = 2.*circ*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS)/PI;
	thetaLim = asin(MAX(-1.,MIN(1.,(tau[1]-taubar)/Delta)));
	lsign = SIGN(tau[4]);
	dSdI[0] += lsign*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge_orbdata,-.5*PI,thetaLim,&AS);
	dSdI[1] += lsign*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge_orbdata,-.5*PI,thetaLim,&AS);
	dSdI[2] += lsign*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge_orbdata,-.5*PI,thetaLim,&AS);

	// Short and Box
	if(limits[2]==-CS->beta()){
		if(lsign<0)for(int i=0;i<3;i++) dSdI[i]+=dJdIm[i]*PI;
		if(x[1]<0) for(int i=0;i<3;i++) dSdI[i]+=dJdIm[i]*PI;
	}
	// Outer long-axis loop
	else if(limits[3]==-CS->alpha() and limits[2]!=-CS->beta()){
		for(int i=0;i<3;i++) dSdI[i]=dJdIm[i]*PI/2.+dSdI[i];
		if(x[0]<0) for(int i=0;i<3;i++) dSdI[i]+=dJdIm[i]*PI;
		// if(lsign>0)for(int i=0;i<3;i++) dSdI[i]+=dJdIm[i]*PI;
	}
	// Inner long-axis loop
	else if(limits[3]!=-CS->alpha() and limits[2]!=-CS->beta()){
		if(lsign<0) for(int i=0;i<3;i++) dSdI[i]+=dJdIl[i]*PI/circ;
	}

	// Jz
	ints[1]=Jt[2];ints[2]=Kt[2];
	taubar = 0.5*(limits[4]+limits[5]);
	Delta = 0.5*(limits[5]-limits[4]);
	AS = action_struct_triax_fudge(this,ints,tau,taubar,Delta,2);
	VecDoub dJdIn(3,0);
	dJdIn[0] = 2.*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS)/PI;
	dJdIn[1] = 2.*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS)/PI;
	dJdIn[2] = 2.*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge_orbdata,-.5*PI,.5*PI,&AS)/PI;
	thetaLim = asin(MAX(-1.,MIN(1.,(tau[2]-taubar)/Delta)));
	lsign = SIGN(tau[5]);
	dSdI[0] += lsign*Delta*GaussLegendreQuad(&dJdE_integrand_Triax_Fudge_orbdata,-.5*PI,thetaLim,&AS);
	dSdI[1] += lsign*Delta*GaussLegendreQuad(&dJdJ_integrand_Triax_Fudge_orbdata,-.5*PI,thetaLim,&AS);
	dSdI[2] += lsign*Delta*GaussLegendreQuad(&dJdK_integrand_Triax_Fudge_orbdata,-.5*PI,thetaLim,&AS);

	if(x[2]<0)  for(int i=0;i<3;i++) dSdI[i]+=dJdIn[i]*PI;
	if(lsign<0) for(int i=0;i<3;i++) dSdI[i]+=dJdIn[i]*PI;

	double Determinant = dJdIl[0]*det2(dJdIm[1],dJdIm[2],dJdIn[1],dJdIn[2])
						- dJdIl[1]*det2(dJdIm[0],dJdIm[2],dJdIn[0],dJdIn[2])
						+ dJdIl[2]*det2(dJdIm[0],dJdIm[1],dJdIn[0],dJdIn[1]);

	double dIdJ[3][3];
	dIdJ[0][0] = det2(dJdIm[1],dJdIm[2],dJdIn[1],dJdIn[2])/Determinant;
	dIdJ[0][1] = -det2(dJdIl[1],dJdIl[2],dJdIn[1],dJdIn[2])/Determinant;
	dIdJ[0][2] = det2(dJdIl[1],dJdIl[2],dJdIm[1],dJdIm[2])/Determinant;
	dIdJ[1][0] = -det2(dJdIm[0],dJdIm[2],dJdIn[0],dJdIn[2])/Determinant;
	dIdJ[1][1] = det2(dJdIl[0],dJdIl[2],dJdIn[0],dJdIn[2])/Determinant;
	dIdJ[1][2] = -det2(dJdIl[0],dJdIl[2],dJdIm[0],dJdIm[2])/Determinant;
	dIdJ[2][0] = det2(dJdIm[0],dJdIm[1],dJdIn[0],dJdIn[1])/Determinant;
	dIdJ[2][1] = -det2(dJdIl[0],dJdIl[1],dJdIn[0],dJdIn[1])/Determinant;
	dIdJ[2][2] = det2(dJdIl[0],dJdIl[1],dJdIm[0],dJdIm[1])/Determinant;

	angles.push_back(dSdI[0]*dIdJ[0][0]+dSdI[1]*dIdJ[1][0]+dSdI[2]*dIdJ[2][0]);
	angles.push_back(dSdI[0]*dIdJ[0][1]+dSdI[1]*dIdJ[1][1]+dSdI[2]*dIdJ[2][1]);
	angles.push_back(dSdI[0]*dIdJ[0][2]+dSdI[1]*dIdJ[1][2]+dSdI[2]*dIdJ[2][2]);
	angles.push_back(det2(dJdIm[1],dJdIm[2],dJdIn[1],dJdIn[2])/Determinant);
	angles.push_back(det2(dJdIn[1],dJdIn[2],dJdIl[1],dJdIl[2])/Determinant);
	angles.push_back(det2(dJdIl[1],dJdIl[2],dJdIm[1],dJdIm[2])/Determinant);

	for(int i=0;i<3;i++){
		if(angles[i]<0.)angles[i]+=2.*PI;
		if(angles[i]>2.*PI)angles[i]-=2.*PI;
		angles.push_back(x[i]);
	}
	angles.push_back(tau[1]);
	angles.push_back(SIGN(tau[4]));
	return angles;
}

// ============================================================================
// stackel_aa.cpp
