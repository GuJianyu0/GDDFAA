/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//// This prog is to read and write initial conditions N-body galaxy data in Gadget2-Format. 
//// The code is modified from read_snapshot.c provided by Volker Springel with the Gadget2  
//// source code and dice_io.c by Valentin Perret with the DICE source code. 
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _GGDATAIO_
#define _GGDATAIO_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <math.h>
// #include "/home/darkgaia/0prog/gadget/dice/src/dice.h"
using namespace std;



#define NotCalculatedParticleInfo 0

//// from DICE/src/dice_io.c
struct io_header_1 {
    int npart[6];                           /*!< number of particles of each type in this file */
    double mass[6];                         /*!< mass of particles of each type. If 0, then the masses are explicitly
                                                stored in the mass-block of the snapshot file, otherwise they are omitted */
    double time;                            /*!< time of snapshot file */
    double redshift;                        /*!< redshift of snapshot file */
    int flag_sfr;                           /*!< flags whether the simulation was including star formation */
    int flag_feedback;                      /*!< flags whether feedback was included (obsolete) */
    // unsigned int npartTotal[6];
    int npartTotal[6];
                                            /*!< total number of particles of each type in this snapshot. This can be
                                                different from npart if one is dealing with a multi-file snapshot. */
    int flag_cooling;                       /*!< flags whether cooling was included  */
    int num_files;                          /*!< number of files in multi-file snapshot */
    double BoxSize;                         /*!< box-size of simulation in case periodic boundaries were used */
    double Omega0;                          /*!< matter density in units of critical density */
    double OmegaLambda;                     /*!< cosmological constant parameter */
    double HubbleParam;                     /*!< Hubble parameter in units of 100 km/sec/Mpc */
    
    // char fill[256 -6*4 -6*8 -2*8 -2*4 -6*4 -2*4 -4*8];	/* fills to 256 Bytes (64-bit system) */
    
    //:: Gadget do not have something below. These will make bad reading results.
    int flag_stellarage;                 /*!< flags whether the file contains formation times of star particles */
    int flag_metals;                     /*!< flags whether the file contains metallicity values for gas and star particles */
    unsigned int npartTotalHighWord[6];  /*!< High word of the total number of particles of each type */
    int flag_entropy_instead_u;          /*!< flags that IC-file contains entropy instead of u */
    
    char fill[256 -6*4 -6*8 -2*8 -2*4 -6*4 -2*4 -4*8     -2*4 -6*4 -1*4];	/* fills to 256 Bytes (64-bit system) */
}; //set_header()

struct particle_data{
    float Pos[3];
    float Vel[3];
    int Id; //gjy note: ID is in space [1,N_total] instead of [0,N_total-1]; no use if reodered
    int Type; //gjy note: NOTE, what read donot have this.
    float Mass;
    float Rho, U, Hsml, Ne; //gjy note: Ne should exsist in default Gadget2 while it may be none in DICE
    float Pot; //gjy note: when make with potential
    float Acc[3]; //gjy note: when make with accelerations
    float dAdt, Age, Metal; //gjy note: Metalicity's position is unknown; the data posited in Ne and dAdt are unknown??
    float Time; //gjy add: system time
}; //gjy note: 这样不仅定义了结构, 还定义里了这个结构的指针实体, 并用于下面载入某个snapshot的数据



#define MaxCharactersInString 200

class RW_Snapshot{
public:
	// double time; //this data
    int snap; //this data
    char path[MaxCharactersInString]; //the loaded snapshots path
    
    int* Id;
    float GCP[3], GCV[3];
    int NumPart = 1000000, NumPartTot = 0;
    int Ngas, num = 0, tnum = 0;
    double Tmass = 0;
	double Time, Redshift;
    int Left = 0;
    char *left;
    float sig2[3] = {0, 0, 0};
    bool is_reorder = 0;

    struct io_header_1 header1;
	struct particle_data* P; // = new particle_data;

	RW_Snapshot(){}
	~RW_Snapshot(){
        cout<<"~RW_Snapshot() called.\n";
        if(P!=nullptr){
            cout<<"unload_snapshot() called.\n"; //??
		    unload_snapshot();
        }
	}

    void set_count_of_particles(int np){
        // this->NumPartTot = np;
        this->NumPart = np;
    }

    ////----------The below private funcs in this class are adapted from Gadget2 and DICE.------------
    /* this routine allocates the memory for the 
    * particle data.
    */
    int allocate_memory(void);

    ////gjy note; the main func that read snapshot to particle data *P
    /* this routine loads particle data from Gadget's default
    * binary file format. (A snapshot may be distributed
    * into multiple files.
    */
    int load_snapshot(char *fname, int files);

    /* This routine brings the particles back into
    * the order of their ID's.
    * NOTE: The routine only works if the ID's cover
    * the range from 1 to NumPart !
    * In other cases, one has to use more general
    * sorting routines.
    */
    int reordering(void);

    /* here the particle data is at your disposal 
    */
    int do_what_you_want(void){
        return 0;
    }

    /* this template shows how one may convert from Gadget's units
    * to cgs units.
    * In this example, the temperate of the gas is computed.
    * (assuming that the electron density in units of the hydrogen density
    * was computed by the code. This is done if cooling is enabled.)
    */
    int unit_conversion(void){
        return 0;
    }

    // This function frees the memory used when the load_snapshot function is
    // called. In general, users will not know that P has to be incremented
    // because it was decremented and an attempt to free it will result in
    // a segmentation fault. This prevents the problem.
    int unload_snapshot(void){
        P++;
        free(P);
        return 0;
    }

    //// ---------------- The below functions are to read and write. ----------------

    /*
        To print the header info.
    */
    void print_header_info(string discpt=""){
        std::cout
            <<"RW_Snapshot::print_header_info(), "<<discpt<<":\n"
            <<header1.fill<<" \n"
            <<sizeof(header1)<<" \n"
            <<NumPart<<" "<<NumPartTot<<" \n"
            <<header1.BoxSize<<" "<<header1.flag_cooling<<" "
            <<header1.flag_feedback<<" "<<header1.flag_sfr<<" "
            <<header1.Omega0<<" "<<header1.OmegaLambda<<" "
            <<header1.HubbleParam<<" "<<header1.num_files<<" "
            <<header1.redshift<<" "<<header1.time<<" "
        <<"\n";
        for(int i=0;i<6;i++){
            std::cout
                <<header1.mass[i]<<" "
                <<header1.npart[i]<<" "<<header1.npartTotal[i]<<" "
            <<"\n";
        }
        fflush(stdout);
    }

    /*
        To load snapshot g1 file.
    */
	void load(string path_snapshot){ //this func must be called if Data_Potential used

		char input_fname[MaxCharactersInString];
		// if(pathload!=nullptr) sprintf(path, "%s", pathload);
    	// sprintf(input_fname, "%ssnapshot/%s_%03d", path, bname, snap);
    	
        sprintf(input_fname, "%s", path_snapshot.data());
        printf("Loading %s ...\n", input_fname);
        int count_files = 1;
        load_snapshot(input_fname, count_files); //cannot open file: in this func
        if(is_reorder){
            // free(Id); // can not reorder, freed before??
            reordering();			/* call this routine only if your ID's are set properly */
        }
		for(int id=1;id<NumPart+1;id++){
		    // printf("%d: %e %d \n", id, P[id].Pos[0], P[id].Id);
		    // printf("%d: %e %e \n", id, P[id].Pot, P[id].Mass);
			P[id].Id = id; //rewrite id
			P[id].Time = 0.;
			// P[id].Time = this->time; //rewrite time
		}
  		// unit_conversion();		/* optional stuff */
  		// do_what_you_want();
        printf("Loading snapshot g1 file ... Done.\n");
	}

    /*
        To set header.
    */
    // void set_header(){
    void set_header(int np=-1){ //find print_header_info
        if(np>=0){
            printf("Set total count of particles: %d.\n", np);
            this->NumPart = np;
        }

        NumPart = NumPartTot; //gjy add

        int i;
        for(i=0;i<6;i++) header1.npart[i] = 0;
        header1.npart[1] = NumPart; //??: only set comp1
        
        for(i=0;i<6;i++) header1.mass[i] = 0.0;
        header1.redshift = 0.0; header1.time = 1.; //??
        header1.flag_sfr = header1.flag_feedback = 0;

        for(i=0;i<6;i++) header1.npartTotal[i] = 0;
        header1.npartTotal[1] = NumPart; //??: only set comp1

        header1.flag_cooling = 0;
        header1.num_files = 1;
        header1.BoxSize = 0.0;
        header1.Omega0 = 0.0; header1.OmegaLambda = 0.0;
        header1.HubbleParam = 1.0;

        //:: Some .g1 file might do not have something below. These will make bad reading results.
        header1.flag_stellarage = 0;                        /*!< flags whether the file contains formation times of star particles */
        header1.flag_metals = 0;                            /*!< flags whether the file contains metallicity values for gas and star particles */
        for(i=0;i<6;i++) header1.npartTotalHighWord[i] = 0; /*!< High word of the total number of particles of each type */
        header1.flag_entropy_instead_u = 0;                 /*!< flags that IC-file contains entropy instead of u */
    
    }

	/*
		Function to write initial conditions to file in default Gadget2 format by changing 
		known particle_data without the struct member galaxy* gal. After changing *P of a 
		snapshot, write it to IC file.
		One should change code each time for a certain new request.
	*/
	int write_gadget_ics_known(string path_snapshot);
    int write_gadget_ics_known1(string path_snapshot);
	
    /*
        To write particle_data to txt.
    */
    int write_PD_txt(string path_snapshot, string disctrp=""){

	    char wt_fname[MaxCharactersInString];
	    // sprintf(wt_fname, "%stxt/%s_%03d_%s.txt", path, bname, snap, disctrp.data());
	    sprintf(wt_fname, "%s", (path_snapshot+disctrp).data());
        FILE *fp = fopen(wt_fname, "w");
        if(fp==nullptr) return -1;
        for(int id=1;id<NumPart+1;id++){
            fprintf(fp, "%e %e %e %e %e %e    %d %d %e %e    %e %e %e %e    %e %e %e %e    %e %e %e \n", 
				P[id].Pos[0], P[id].Pos[1], P[id].Pos[2],    P[id].Vel[0], P[id].Vel[1], P[id].Vel[2],    
				P[id].Id, P[id].Type, P[id].Mass, P[id].Time, 
				P[id].U, P[id].Rho, P[id].Hsml, P[id].Ne, 
				P[id].Pot,    P[id].Acc[0], P[id].Acc[1], P[id].Acc[2], 
				P[id].dAdt, P[id].Age, P[id].Metal 
            );
        }
        fclose(fp);
        printf("Write particle_data *P to file %s ... done.\n", wt_fname);
        return 0;
    }

    /*
        To read particle_data from txt.
    */
    int read_PD_txt(string path_snapshot, bool is_from_snapshot_txt=true){

	    char wt_fname[MaxCharactersInString];
	    // sprintf(wt_fname, "%stxt/%s_%03d.txt", path, bname, snap);
	    sprintf(wt_fname, "%s", path_snapshot.data());
        FILE *fp = fopen(wt_fname, "r");
        if(fp==nullptr) return -1;

        int id = 1; //??
		float x[3], v[3], mass, pot, acc[3], time; int ID, type;
        if(!is_from_snapshot_txt){
            while(
                fscanf(fp, "%e %e %e %e %e %e    %e%*[^\n]%*c",     // note: no quad
                    &(x[0]), &(x[1]), &(x[2]),    &v[0], &v[1], &v[2],    
                    &mass 
                ) != EOF
            ){
                P[id].Id = NotCalculatedParticleInfo, P[id].Type = NotCalculatedParticleInfo, P[id].Time = NotCalculatedParticleInfo, 
                P[id].U = NotCalculatedParticleInfo, P[id].Rho = NotCalculatedParticleInfo, P[id].Hsml = NotCalculatedParticleInfo, P[id].Ne = NotCalculatedParticleInfo,    
                P[id].Pot = NotCalculatedParticleInfo,    P[id].Acc[0] = NotCalculatedParticleInfo, P[id].Acc[1] = NotCalculatedParticleInfo, P[id].Acc[2] = NotCalculatedParticleInfo,    
                P[id].dAdt = NotCalculatedParticleInfo, P[id].Age = NotCalculatedParticleInfo, P[id].Metal = NotCalculatedParticleInfo;
                P[id].Pos[0] = x[0],P[id].Pos[1] = x[1],P[id].Pos[2] = x[2],    
                P[id].Vel[0] = v[0],P[id].Vel[1] = v[1],P[id].Vel[2] = v[2],    
                P[id].Mass = mass;
                id++;
            }
        }else{
            while(
                fscanf(fp, "%e %e %e %e %e %e    %d %d %e %e    %*e %*e %*e %*e    %e %e %e %e    %*e %*e %*e%*[^\n]%*c",    
            	&(x[0]), &(x[1]), &(x[2]),    &v[0], &v[1], &v[2],    
            	&ID, &type,    &mass,     &time,    
            	&pot,    &acc[0], &acc[1], &acc[2] 
                ) != EOF
            ){
                P[id].Id = NotCalculatedParticleInfo, P[id].Type = NotCalculatedParticleInfo, P[id].Time = NotCalculatedParticleInfo, 
                P[id].U = NotCalculatedParticleInfo, P[id].Rho = NotCalculatedParticleInfo, P[id].Hsml = NotCalculatedParticleInfo, P[id].Ne = NotCalculatedParticleInfo,    
                P[id].dAdt = NotCalculatedParticleInfo, P[id].Age = NotCalculatedParticleInfo, P[id].Metal = NotCalculatedParticleInfo;
                P[id].Pos[0] = x[0],P[id].Pos[1] = x[1],P[id].Pos[2] = x[2],    
                P[id].Vel[0] = v[0],P[id].Vel[1] = v[1],P[id].Vel[2] = v[2];   
                P[id].Id = ID, P[id].Type = type,    P[id].Mass = mass,     P[id].Time = time; 
                P[id].Pot = pot,    P[id].Acc[0] = acc[0],P[id].Acc[1] = acc[1],P[id].Acc[2] = acc[2];
                id++;
            }
        }
        this->NumPartTot = id-1;
        printf("NumPartTot = %d\n", NumPartTot);

        fclose(fp);
        printf("Read particle_data *P from file %s ... done.\n", wt_fname);
        return 0;
    }

    int write_PD_toSCF(string path_snapshot, string discrpt=""){
        // this->snap = snap_id;
	    char wt_fname[MaxCharactersInString];
	    sprintf(wt_fname, "%s", (path_snapshot+discrpt).data());
	    // sprintf(wt_fname, "%stxt/%s_%03d.SCF.txt", path, bname, snap);
        FILE *fp = fopen(wt_fname, "w");
        if(fp==nullptr) return 1;
        fprintf(fp, "%d \n%d \n%le \n", 100, NumPart, 0.01); //the first line
        for(int id=1;id<NumPart+1;id++){
            fprintf(fp, "%d %e    %e %e %e %e %e %e\n",    
            P[id].Id, P[id].Mass,   
            P[id].Pos[0], P[id].Pos[1], P[id].Pos[2], P[id].Vel[0], P[id].Vel[1], P[id].Vel[2]  
            );
        }
        fclose(fp);
        printf("Write particle_data *P to file %s ... done.\n", wt_fname);
        return 0;
    }

    void read_ic_ascii(string path_snapshot){
        const char* fname = path_snapshot.data();
        FILE* finp;
        int i, j, itmp;
        static int ptr=0, FileCnt=0;
        float rtmp, kpc, kms;
        
        //  sprintf(fname,"data.inp");
        
        FileCnt++;
        printf("reading file %d : %s\n", FileCnt, fname);
        
        if(!(finp = fopen(fname,"r")))
        {
            printf("read_ic_ascii: Cannot open file %s\n", fname);
            exit(1);
        }
        
        kpc = 3.09E+18 * 1000.0;
        kms = 1.0E+03 * 1.0E+02;

        i = ptr;
        while( fscanf(finp,"%E %E %E %E %E %E%*[^\n]%*c",
                &P[i+1].Pos[0], &P[i+1].Pos[1], &P[i+1].Pos[2],
                &P[i+1].Vel[0], &P[i+1].Vel[1], &P[i+1].Vel[2] ) != EOF)
        {
            P[i+1].Mass = 1.37e-2;
            P[i+1].Id = i+1;
            P[i+1].Type = 1;  // set to halo type
            i++;
        }
        NumPartTot = i;
        printf("NumPartTot = %d\n", NumPartTot);

        /*
        NumPartTot = 10000;
        
        for(i=ptr;i<NumPartTot;i++)
        { 
            fscanf(finp,"%E %E %E %E %E %E\n",
                &P[i+1].Pos[0], &P[i+1].Pos[1], &P[i+1].Pos[2],
                &P[i+1].Vel[0], &P[i+1].Vel[1], &P[i+1].Vel[2] );
            
            P[i+1].Mass = 137.0/NumPartTot;
            P[i+1].id = i+1;
            P[i+1].Type = 1;  // set to halo type
        
        }
        */
        fclose(finp);
    }

};





////not used
#define MAXLEN_FILENAME     100
#define MAXLEN_FILELINE     500
#define MAX_GAL             64
#define MAX_STREAM          64
#define GSL_WORKSPACE_SIZE  100000
#define NSTORAGE            8

// Global variables such as the parameters of the Keplerian system, or the number of galaxies to generate.
extern struct GlobalVars {
    char ParameterFile[MAXLEN_FILENAME];
    char GalaxyFiles[MAX_GAL][MAXLEN_FILENAME];
    char StreamFiles[MAX_STREAM][MAXLEN_FILENAME];
    char Filename[MAXLEN_FILENAME];
    char ICformat[MAXLEN_FILENAME];
    char UnitMassName[20];
    char UnitVelocityName[20];
    char UnitLengthName[20];
    int Kepler_Gal1[MAX_GAL];
    int Kepler_Gal2[MAX_GAL];
    int Kepler_GalCenter[MAX_GAL];
    int Circular_Gal1[MAX_GAL];
    int Circular_Gal2[MAX_GAL];
    int Circular_GalCenter[MAX_GAL];
    int GalId[MAX_GAL];
    int StreamId[MAX_STREAM];
    int Ngal;
    int Nstream;
    int Nthreads;
    int MeanPartDist;
    int OutputRc;
    int OutputGasRc;
    int OutputPot;
    int OutputRho;
    int OutputSigma;
    int OutputToomre;
    int MaxCompNumber;
    int MaxNlevel;
    int GaussianRejectIter;
    int CurrentGalaxy;
    int NormMassFact;
    int GslIntegrationScheme;
    unsigned long int GalStart[MAX_GAL];
    unsigned long int GalNpart[MAX_GAL];
    unsigned long int StreamStart[MAX_STREAM];
    unsigned long int StreamNpart[MAX_STREAM];
    unsigned long int GslWorkspaceSize;
    double StreamSpin[MAX_STREAM];
    double StreamIncl[MAX_STREAM];
    double StreamPos[MAX_STREAM][3];
    double GalSpin[MAX_GAL];
    double GalIncl[MAX_GAL];
    double GalMass[MAX_GAL];
    double GalPos[MAX_GAL][3];
    double GalVel[MAX_GAL][3];
    double Kepler_Ecc[MAX_GAL];
    double Kepler_Rinit[MAX_GAL];
    double Kepler_Rperi[MAX_GAL];
    double Kepler_OrbitPlaneTheta[MAX_GAL];
    double Kepler_OrbitPlanePhi[MAX_GAL];
    double Circular_Rinit[MAX_GAL];
    double Circular_OrbitPlaneTheta[MAX_GAL];
    double Circular_OrbitPlanePhi[MAX_GAL];
    double Circular_Vc[MAX_GAL];
    double redshift;
    double H0;
    double H;
    double h;
    double Omega_m;
    double Omega_l;
    double Omega_k;
    double UnitMass;
    double UnitVelocity;
    double UnitLength;
} AllVars;

// This is a type definition of a galaxy. The thought here is to create galaxies
// as objects and, hopefully, make the code extremely clean. It is essentially a
// a collection of arrays and constants.
typedef struct {
    // Spin parameter of the galaxy
    double lambda;
    // Baryonic mass fraction of the disk
    double m_d;
    // Total mass of the galactic model
    double total_mass;
    // Total mass of the galactic model within R200
    double total_mass_r200;
    // Density fluctuation dispersion
    double dens_fluct_sigma;
    // Density fluctuation injection scale
    double dens_fluct_scale_inj;
    // Density fluctuation injection scale
    double dens_fluct_scale_diss;
    // Density fluctuations spectral power index
    double dens_fluct_nspec;
    // Density fluctuation seed
    long dens_fluct_seed;
    // Total number of components
    int n_component;
    int                 *selected_comp;
    int hydro_eq_niter;
    int hydro_eq;
    double maxrad;
    double maxrad_gas;
    // Component quantities
    char                **comp_profile_name;
    char                **comp_imf_name;
    unsigned long int   *comp_npart;
    unsigned long int   *comp_npart_pot;
    unsigned long int   *comp_start_part;
    double              *comp_mass_frac;
    double              *comp_mass;
    int                 *comp_model;
    double              *comp_scale_length;
    double              *comp_concentration;
    double              *comp_scale_height;
    double              *comp_cut;
    double              *comp_sigma_cut;
    double              *comp_sigma_cut_in;
    double              *comp_flatx;
    double              *comp_flaty;
    double              *comp_flatz;
    double              *comp_flatx_cut;
    double              *comp_flaty_cut;
    double              *comp_flatz_cut;
    double              *comp_flatx_out;
    double              *comp_flaty_out;
    double              *comp_flatz_out;
    double              *comp_flatx_rt;
    double              *comp_flaty_rt;
    double              *comp_flatz_rt;
    double              *comp_flatx_st;
    double              *comp_flaty_st;
    double              *comp_flatz_st;
    char                *comp_flatx_var;
    char                *comp_flaty_var;
    char                *comp_flatz_var;
    double              *comp_mcmc_step;
    double              *comp_mcmc_step_slope;
    double              *comp_mcmc_step_hydro;
    double              *comp_vmax_esc;
    double              *comp_vmax_circ;
    int                 *comp_type;
    int                 *comp_bool;
    double              *comp_stream_frac;
    double              *comp_cut_dens;
    double              *comp_theta_sph;
    double              *comp_phi_sph;
    double              *comp_metal;
    double              *comp_metal_sigma;
    double              *comp_metal_scale;
    long                *comp_metal_seed;
    double              *comp_t_init;
    double              *comp_u_init;
    double              *comp_cs_init;
    int                 *comp_turb_gradient;
    double              *comp_turb_sigma;
    double              *comp_turb_frac;
    double              *comp_turb_scale;
    double              *comp_turb_scale_inj;
    double              *comp_turb_scale_diss;
    double              *comp_turb_nspec;
    long                *comp_turb_seed;
    double              *comp_age_sigma;
    double              *comp_age_scale;
    long                *comp_age_seed;
    double              *comp_mean_age;
    double              *comp_min_age;
    double              *comp_alpha_struct;
    double              *comp_beta_struct;
    double              *comp_scale_dens;
    double              *comp_radius_nfw;
    double              *comp_Q_lim;
    double              *comp_Q_min;
    double              *comp_Q_fixed;
    double              *comp_Q_boost;
    double              *comp_Q_bar;
    double              *comp_t_min;
    int                 *comp_compute_vel;
    int                 *comp_hydro_eq;
    int                 *comp_hydro_eq_mode;
    int                 *comp_spherical_hydro_eq;
    int                 *comp_dens_fluct;
    double              *comp_cut_in;
    int                 *comp_thermal_eq;
    double              *comp_part_mass;
    double              *comp_part_mass_pot;
    int                 *comp_jeans_mass_cut;
    int                 *comp_jeans_anisotropy_model;
    int                 *comp_epicycle;
    int                 *comp_metal_gradient;
    int                 *comp_excavate;
    double              *comp_sfr;
    double              *comp_spiral_theta_out;
    double              *comp_spiral_r_in;
    double              *comp_spiral_r_out;
    double              *comp_spiral_alpha;
    double              *comp_warp_scale;
    int                 *comp_warp_mode;
    int                 *comp_jeans_dim;
    int                 *comp_sigmar_model;
    int                 *comp_sigmaz_model;
    double              *comp_sigmar;
    double              *comp_sigmaz;
    double              *comp_sigmar_radius;
    double              *comp_sigmaz_radius;
    double              *comp_sigmar_scale;
    double              *comp_sigmaz_scale;
    double              *comp_jeans_f_sigma;
    double              *comp_k_stream;
    int         *comp_delete;
    double              *comp_stream_scale;
    int                 *comp_stream_method;
    double              *comp_angmom_frac;
    double              *comp_dens_min;
    double              *comp_dens_max;
    double              *comp_gamma_poly;
    double              *comp_k_poly;
    double              *comp_dens_init;
    double              *comp_accept_min;
    double              *comp_accept_max;
    double              *comp_rcore;
    double      *comp_ggd_beta;
    double      *comp_softening;
    double      *comp_rc_entropy;
    double      *comp_alpha_entropy;
    double      *comp_cut_hydro_eq;
    int                 *comp_symmetry;
    int                 *comp_imf_model;
    double              *comp_mstar_min;
    double              *comp_mstar_max;
    double              *comp_mcmc_step_mass;
    // Virial quantities
    double v200;
    double r200;
    double m200;
    double s200;
    // Total angular momentum within R200
    double J200;
    // Coordinates vectors
    double              *x;
    double              *y;
    double              *z;
    double              *r_cyl;
    double              *theta_cyl;
    double              *r_sph;
    double              *phi_sph;
    // Velocities vectors
    double              *vel_x;
    double              *vel_y;
    double              *vel_z;
    // Mass vector
    double              *mass;
    // Density vector
    double              *rho;
    // Internal energy vector
    double              *u;
    // Age vector
    double              *age;
    // Metallicity vector
    double              *metal;
    // Particle Mesh potential grid
    double              ****potential;
    double              ****potential_ext;
    double      **vr2_tilted_mixed;
    double      **vz2_tilted_mixed;
    double      **vtheta2_mixed;
    // Particle Mesh gaussian field grid
    double              ***gaussian_field;
    // Gas midplane density grid
    double              **midplane_dens;
    // Potential grid cell size vector [kpc]
    double *dx;
    // Gas midplane density grid cell size vector [kpc]
    double dx_dens;
    double dx_jeans;
    // Gaussian field grid cell size vector [kpc]
    double dx_gauss;
    // Number of particles per particle type
    unsigned long int num_part[4];
    unsigned long int num_part_pot[4];
    // Total potential grid size [kpc]
    double *boxsize;
    double *boxsize_flatx;
    double *boxsize_flaty;
    double *boxsize_flatz;
    // Total gas midplane density grid size [kpc]
    double boxsize_dens;
    double boxsize_jeans;
    // Level of refinement of the potential grid
    int level_coarse;
    int nlevel;
    // Level of refinement of the gas density grid
    int level_grid_dens;
    int level_grid_jeans_3D;
    // Level of refinement of the gas turbulence grid
    int level_grid_turb;
    // Level of refinement of the stars age grid
    int level_grid_age;
    // Level of refinement of the metal grid
    int level_grid_metal;
    // Level of refinement of the density gaussian fluctuations
    int level_grid_dens_fluct;
    // Number of cells in the potential grid
    int **ngrid;
    // Number of cells in the midplane density grid
    int ngrid_dens[2];
    int ngrid_jeans[2];
    // Number of cells in the turbulence grid
    int ngrid_gauss[3];
    // Storage array
    double              **storage;
    // Identifier of particle
    unsigned long int   *index;
    unsigned long int   *id;
    // Total number of particles
    unsigned long int ntot_part;
    unsigned long int ntot_part_stars;
    unsigned long int ntot_part_pot;
    // Boolean variable checking the computation of the potential
    int potential_defined;
    // Boolean variable checking the computation of the midplane gas density
    int midplane_dens_defined;
    // Boolean variable checking the computation of the gaussian field
    int gaussian_field_defined;
    int jeans_3D_defined;
    // Seed for random number generator
    long seed;
    // Pseudo density boolean
    int *pseudo;
    // Gravitational softening
    double softening;
    // MCMC multiple try parameter
    int mcmc_ntry;
    // Main halo component index
    int index_halo;
    // Main disk component index
    int index_disk;
    // Main gas disk component index
    int index_gasdisk;
    // First valid component index
    int index_first;
    // Copy tag
    int copy;
    // Masses per type
    double gas_mass;
    double halo_mass;
    double disk_mass;
    double bulge_mass;
    double stellar_mass;
} galaxy;



#endif