
#include "DataInterface.h"
// #include "out.h"



int main(int argc, char* argv[]){

DEBUG_PRINT_I(11);
	////1. input
	//:: in linux shell: 
	// # cd path/of/aa/; make clean && cd .. && make LAPACK=1 TORUS=1 && cd aa;  
	// # mpirun -np 4 mains/./data.exe  
	// # <input arguments below>
	//:: snapshots range
	double t_init 				= atof(argv[1]); 	//0.
    double t_final 				= atof(argv[2]); 	//9.
	double dt_load				= atof(argv[3]); 	//0.01
	double dt_step 				= atof(argv[4]);	//0.001
	double t_aligment			= atof(argv[5]);	//0.

	//:: what to run
	int is_witeSnapshot 		= atoi(argv[6]);	//write snapshots //0
	int is_preprocessed			= atoi(argv[7]); 	//0: not <1>; 1: write and centerize //0
	int onlyRun_Actionmethod	= atoi(argv[8]);	//7 = 111(2)

	int AlgorithmPot 			= atoi(argv[9]); 	//0: direct summation; 5: direct summation 1
	int WhatPotential 			= atoi(argv[10]); 	//0: formula potential; 1: data potential //2
	int WhatSymmetry			= atoi(argv[11]); 	//0: spherical; 1: axisymmetric; 2: triaxial //2
	int WhatActionmethod		= atoi(argv[12]); 	//0: SF; 1: PPOD all; 2: {Jl} by <1> and {Jm,Jn} by <2> //3

	//:: time and particle_ID range to run
	double t_start_run			= atof(argv[13]); 	//5
	double t_end_run			= atof(argv[14]); 	//5.01
	double dt_run 				= atof(argv[15]); 	//1.

	int ID_start 				= atoi(argv[16]);	//1
	int N_ptcs 					= atoi(argv[17]); 	//the max index of particles //N_allPtcs
	
	int is_run_samples 			= atoi(argv[18]); 	//samples for action state density
	int N_action_samples		= atoi(argv[19]);

	int is_DEBUG				= atoi(argv[20]);

	//:: other
	double input_other_1	= atof(argv[21]);
	double input_other_2	= atof(argv[22]);
	double input_other_3	= atof(argv[23]);
	double input_other_4	= atof(argv[24]);
	double input_other_5	= atof(argv[25]);
	double input_other_6	= atof(argv[26]);
	double input_other_7	= atof(argv[27]);
	double input_other_8	= atof(argv[28]);
DEBUG_PRINT_I(12);





	string path_IC = "/home/darkgaia/workroom/0prog/GDDFAA_work/"
		"GDDFAA/step1_galaxy_IC_preprocess/step1_set_IC_DDFA/IC_param_Sersic.txt";
	// string path_IC = "../../../../"
	// 	"GDDFAA/step2_Nbody_simulation/gadget/Gadget-2.0.7/galaxy_general/"
	// 	"IC_param.txt";
	Stage STAGE(path_IC);
	Stage* pSTAGE = &STAGE;

	string path_gm_1 = pSTAGE->path_gm;
	printf("path_gm: %s", path_gm_1.data());
	pSTAGE->load_multi_snapshots(t_init, t_final, dt_load, dt_step, 0, 0);
	double Alpha = TACT_semiaxis_Alpha, Beta = TACT_semiaxis_Beta;

	// Potential_JS DPot(&STAGE);
	// Potential_JS* pDPot = &DPot;
	// //pDPot->Phi(..., Potential_other*=nullptr) //??
	// pDPot->set_algorithm(AlgorithmPot);
	// vector<Potential_JS> vFPot; //??

	for(double t0=t_start_run;t0<t_end_run;t0+=dt_run){ //time
		int s = t_to_loadsnapshot(t0, dt_load, t_init, 0.); //load
		int snapshot = pSTAGE->SS[s]->snap; //snapshot
		DEBUG_PRINT_V0d(1, t_init, "t_init");
		DEBUG_PRINT_V0d(1, s, "s");
		DEBUG_PRINT_V0d(10, snapshot, "snapshot");

		////(1) data potential
		//!!?? debug preprocess: false when multiload, true now
		#ifdef DEBUG_GJY
		pSTAGE->SS[s]->preprocess(true); //to get triaxialize matix of this s~snapshot
		#endif
		// DEBUG_PRINT_V0d(1, pSTAGE->SS[s]->potential_SCF({19.6, 1., 1.}), "potscf");

		VecDoub ar = {1., 0.99, 0.98};
		double rs0 = 19.6;
		pSTAGE->SS[0]->RebuildPotentialSP3(ar, rs0);
		VecDoub xsp3 = {0.01, 2., 30.};
		auto pot = pSTAGE->SS[0]->potential_TRSP3(xsp3);
		DEBUG_PRINT_V0d(10, pot, "pot");
		auto forces = pSTAGE->SS[0]->forces_TRSP3(xsp3);
	}

	cout<<"end\n";
	return 0;
}



// int main(int argc, char* argv[]){

// 	////1. input
// 	//:: in linux shell: 
// 	// # cd ~/workroom/0prog/Stackel/tact-master/aa; make clean && cd .. && make LAPACK=1 TORUS=1 && cd aa;  
// 	// # mpirun -np 4 mains/./data.exe  
// 	// # <input arguments below>
// 	//:: snapshots range
// 	double t_init 				= atof(argv[1]); 	//0.
//     double t_final 				= atof(argv[2]); 	//9.
// 	double dt_load				= atof(argv[3]); 	//0.01
// 	double dt_step 				= atof(argv[4]);	//0.001
// 	double t_aligment			= atof(argv[5]);	//0.

// 	//:: what to run
// 	int is_witeSnapshot 		= atoi(argv[6]);	//write snapshots //0
// 	int is_preprocessed			= atoi(argv[7]); 	//0: not <1>; 1: write and centerize //0
// 	int onlyRun_Actionmethod	= atoi(argv[8]);	//7 = 111(2)

// 	int AlgorithmPot 			= atoi(argv[9]); 	//0: direct summation; 5: direct summation 1
// 	int WhatPotential 			= atoi(argv[10]); 	//0: formula potential; 1: data potential //2
// 	int WhatSymmetry			= atoi(argv[11]); 	//0: spherical; 1: axisymmetric; 2: triaxial //2
// 	int WhatActionmethod		= atoi(argv[12]); 	//0: SF; 1: PPOD all; 2: {Jl} by <1> and {Jm,Jn} by <2> //3

// 	//:: time and particle_ID range to run
// 	double t_start_run			= atof(argv[13]); 	//5
// 	double t_end_run			= atof(argv[14]); 	//5.01
// 	double dt_run 				= atof(argv[15]); 	//1.

// 	int ID_start 				= atoi(argv[16]);	//1
// 	int N_ptcs 					= atoi(argv[17]); 	//the max index of particles //10000: N_all
	
// 	int is_run_samples 			= atoi(argv[18]); 	//samples for action state density
// 	int N_action_samples		= atoi(argv[19]);

// 	int is_DEBUG				= atoi(argv[20]);

// 	//:: other
// 	// double changedParticle_mass	= atof(argv[21]);	//1. //0.1



// 	////3. snapshot settings
// 	string path_work = get_workpath();
// 	string path_base = path_work+"0prog/gadget/Gadget-2.0.7/galaxy_general/";
// 	string path_IC = "IC_param.txt";
// 	Stage STAGE(path_base, path_IC);
// 	Stage* pSTAGE = &STAGE;

// 	////calculate
// 	pSTAGE->load_multi_snapshots(t_init, t_final, dt_load, dt_step, 0, 0);
// 	double Alpha = TACT_semiaxis_Alpha, Beta = TACT_semiaxis_Beta;

// 	for(double t0=t_start_run;t0<t_end_run;t0+=dt_run){ //time
// 		int s = t_to_loadsnapshot(t0, dt_load, t_init, 0.); //load
// 		int snapshot = pSTAGE->SS[s]->snap; //snapshot
// 		// int calculateIndex = (int)((t0-t_start_run)/dt_run); //calculate
// 		DEBUG_PRINT_V0d(1, t0, "t0");
// 		DEBUG_PRINT_V0d(1, s, "s");
// 		DEBUG_PRINT_V0d(1, snapshot, "snapshot");

// 		////(1) data potential
// 		#ifdef DEBUG_GJY
// 		pSTAGE->SS[s]->preprocess(); //to get triaxialize matix of this s~snapshot
// 		#endif
		
// 		DEBUG_PRINT_V0d(10, s, "before xtree");
// 		std::array<std::array<double, Dim>, N_total> xdata; //dynamic tree?? //x-tree and J-tree
// 		for(int i=0;i<N_total;i++){
// 			int iP = i+1;
// 			xdata[i][0] = pSTAGE->SS[s]->P[iP].Pos[0];
// 			xdata[i][1] = pSTAGE->SS[s]->P[iP].Pos[1];
// 			xdata[i][2] = pSTAGE->SS[s]->P[iP].Pos[2];
// 		}
// 		KDtree<double, N_total, Dimension> kdt(&xdata);
// 		pSTAGE->SS[s]->loadtree(&kdt);
// 		DEBUG_PRINT_V0d(10, s, "after xtree");

// 		// potential and actions





// 		////small main
// 		// write kernel density of mass and actions
// 		if(1){
// 			//another file, or in python instead of this slow-debug-snale CC++ //??
// 			std::array<std::array<double, Dim>, N_total> xdata0, xdata1; //dynamic tree?? //x-tree and J-tree
// 			wtfh = (struct write_firsthand *) malloc(sizeof(struct write_firsthand)*(N_total));
// 			wtsh = (struct write_secondhand *) malloc(sizeof(struct write_secondhand)*(N_total));
// 			int WhatCannonical;

// 			WhatCannonical = 2;
// 			pSTAGE->SS[s]->load_to_firsthand_from_PD(); //??
// 			for(int i=0;i<N_total;i++){
// 				int iP = i+1;
// 				xdata0[i][0] = pSTAGE->SS[s]->P[iP].Pos[0];
// 				xdata0[i][1] = pSTAGE->SS[s]->P[iP].Pos[1];
// 				xdata0[i][2] = pSTAGE->SS[s]->P[iP].Pos[2];
// 			}
// 			KDtree<double, N_total, Dim> kdt0(&xdata0);
// 			pSTAGE->SS[s]->loadtree(&kdt0);
// 			// string suffix = "DF_CartesianMass";
// 			pSTAGE->SS[s]->write_secondhand_all_txt(WhatCannonical, 0, 0, 0);
// 			// pSTAGE->SS[s]->remove_tree(); //then after {}, the array will be release
// 			free(wtfh);
// 			free(wtsh);

// 			WhatCannonical = 5; //WhatCannonical
// 			for(int WP=0;WP<2;WP++){ //WhatPotential {0,1}
// 				for(int WS=2;WS<3;WS++){ //WhatSymmetry, {0,1,2}
// 					for(int WA=0;WA<3;WA++){ //WhatActionmethod {0,1,2}
// 						wtfh = (struct write_firsthand *) malloc(sizeof(struct write_firsthand)*(N_total));
// 						wtsh = (struct write_secondhand *) malloc(sizeof(struct write_secondhand)*(N_total));
// 						int isExistFile = pSTAGE->SS[s]->read_firsthand_all_txt(WhatCannonical, WP, WS, WA);
// 						if(isExistFile!=0){
// 							continue;
// 						}
// 						for(int i=0;i<N_total;i++){
// 							xdata1[i][0] = wtfh[i].QP[3]; 
// 							xdata1[i][1] = wtfh[i].QP[4]; 
// 							xdata1[i][2] = wtfh[i].QP[5];
// 						}
// 						KDtree<double, N_total, Dim> kdt1(&xdata1);
// 						pSTAGE->SS[s]->loadtree1(&kdt1);
// 						// suffix = "DF_actionUnit";
// 						pSTAGE->SS[s]->write_secondhand_all_txt(WhatCannonical, WP, WS, WA);
// 						// pSTAGE->SS[s]->remove_tree();
// 						free(wtfh);
// 						free(wtsh);
// 					}
// 				}
// 			}
// 		}

// 		printf("snapshot_%d done.\n", snapshot);
// 	}
// 	printf("Done.\n");
// }
