/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//// This prog is to read and write initial conditions N-body galaxy data in Gadget2-Format. 
//// The code is modified from read_snapshot.c provided by Volker Springel with the Gadget2  
//// source code and dice_io.c by Valentin Perret with the DICE source code. 
/////////////////////////////////////////////////////////////////////////////////////////////

#include "Gadget2FormatData_io.h"

struct GlobalVars AllVars;

int RW_Snapshot::allocate_memory(void){

    printf("allocating memory...\n");
    if(!(P = (struct particle_data *) malloc(NumPart * sizeof(struct particle_data)))){
        fprintf(stderr, "failed to allocate memory.\n");
        exit(0);
    }
    P--;				/* start with offset 1 */

    if(!(Id = (int*) malloc(NumPart * sizeof(int)))){
        fprintf(stderr, "failed to allocate memory.\n");
        exit(0);
    }
    Id--;				/* start with offset 1 */

    printf("allocating memory...done\n");
    return 0;
}

int RW_Snapshot::load_snapshot(char *fname, int files){

    FILE *fd;
    char buf[MaxCharactersInString];
    int i, j, k, dummy, ntot_withmasses;
    int t, n, off, pc, pc_new, pc_sph;

    #define SKIP fread(&dummy, sizeof(dummy), 1, fd);

    for(i = 0, pc = 1; i < files; i++, pc = pc_new)
    {
        //header
        if(files > 1)
	        sprintf(buf, "%s.%d", fname, i);
        else
	        sprintf(buf, "%s", fname);

        if(!(fd = fopen(buf, "r")))
	    {
	        printf("Can't open file `%s`.\n", buf);
	        exit(0); //gjy add
            return 1;
	    }

        printf("reading `%s' ...\n", buf);
        fflush(stdout);

        fread(&dummy, sizeof(dummy), 1, fd);
        fread(&header1, sizeof(header1), 1, fd);
// cout<<fd<<" "<<sizeof(header1)<<"\n\n\n\n\n\n";
        fread(&dummy, sizeof(dummy), 1, fd);

        if(files == 1)
	    {
	        for(k = 0, NumPart = 0, ntot_withmasses = 0; k < 6; k++)
	            NumPart += header1.npart[k];
	        Ngas = header1.npart[0];
	    }
        else
	    {
	        for(k = 0, NumPart = 0, ntot_withmasses = 0; k < 6; k++)
	            NumPart += header1.npartTotal[k];
	        Ngas = header1.npartTotal[0];
	    }

        for(k = 0, ntot_withmasses = 0; k < 6; k++)
	    {
	        if(header1.mass[k] == 0)
	            ntot_withmasses += header1.npart[k];
	    }

        if(i == 0)
	        allocate_memory();


        ////particle data
        //positions
        //gjy note: P[].Pos begin
        SKIP;
        // printf("%ld %ld\n", sizeof(int), sizeof(float));
        // printf("pos0 dummy = %d\n", dummy);
        for(k = 0, pc_new = pc; k < 6; k++)
	    {
	        for(n = 0; n < header1.npart[k]; n++)
	        {
	            fread(&P[pc_new].Pos[0], sizeof(float), 3, fd); //gjy note: P[].Pos
	            pc_new++;
	        }
	    }
        SKIP;
        // printf("pos1 dummy = %d\n", dummy);
        //gjy note: P[].Pos end

        //velocities
        SKIP;
        // printf("vel0 dummy = %d\n", dummy);
        for(k = 0, pc_new = pc; k < 6; k++)
	    {
	        for(n = 0; n < header1.npart[k]; n++)
	        {
	            fread(&P[pc_new].Vel[0], sizeof(float), 3, fd); //gjy note: P[].Vel
	            pc_new++;
	        }
	        }
        SKIP;
        // printf("vel1 dummy = %d\n", dummy);

        //ID
        SKIP;
        // printf("id0  dummy = %d\n", dummy);
        for(k = 0, pc_new = pc; k < 6; k++)
	    {
	        for(n = 0; n < header1.npart[k]; n++)
	        {
	            fread(&Id[pc_new], sizeof(int), 1, fd);
	            pc_new++;
	        }
	    }
        SKIP;
        // printf("id1  dummy = %d\n", dummy);

        //mass //IO_MASS
        if(ntot_withmasses > 0)
	    SKIP;
        // printf("mas0 dummy = %d\n", dummy);
        for(k = 0, pc_new = pc; k < 6; k++)
	    {
	        for(n = 0; n < header1.npart[k]; n++)
	        {
	            P[pc_new].Type = k;

	            if(header1.mass[k] == 0)
		            fread(&P[pc_new].Mass, sizeof(float), 1, fd); //gjy note: P[].Mass if has mass, but type is ok before
	            else
		            P[pc_new].Mass = header1.mass[k];
	            pc_new++;
	        }
	    }
        if(ntot_withmasses > 0)
	    SKIP;
        // printf("mas1 dummy = %d\n", dummy);

        ////gas type
        //energy, density, Ne //or IO_U, IO_RHO, IO_HSML??
        if(header1.npart[0] > 0) //gjy note: It has been judged by this if() that whether gas and U,RHO,HSML exist.
	    {
	        SKIP;
            // printf("u0   dummy = %d\n", dummy); //4000 particles * 4 sizeof(int) = 16000 dummy, in data_20210314
	        for(n = 0, pc_sph = pc; n < header1.npart[0]; n++)
	        {
	            fread(&P[pc_sph].U, sizeof(float), 1, fd);
	            pc_sph++;
	        }
	        SKIP;
            // printf("u1   dummy = %d\n", dummy);

	        SKIP;
            // printf("rho0 dummy = %d\n", dummy);
	        for(n = 0, pc_sph = pc; n < header1.npart[0]; n++)
	        {
	            fread(&P[pc_sph].Rho, sizeof(float), 1, fd);
	            pc_sph++;
	        }
	        SKIP;
            // printf("rho1 dummy = %d\n", dummy);

	        if(header1.flag_cooling)
	        {
	            SKIP;
                // printf("hsm0 dummy = %d\n", dummy);
	            for(n = 0, pc_sph = pc; n < header1.npart[0]; n++)
		        {
		            fread(&P[pc_sph].Hsml, sizeof(float), 1, fd);
		            pc_sph++;
		        }
	            SKIP;
                // printf("hsm1 dummy = %d\n", dummy);
	        }
	        else //not by read
	            for(n = 0, pc_sph = pc; n < header1.npart[0]; n++)
	            {
		            P[pc_sph].Hsml = 1.0;
		            pc_sph++;
	            }

            //Ne??
	        SKIP;
            // printf("ne0  dummy = %d\n", dummy);
	        for(n = 0, pc_sph = pc; n < header1.npart[0]; n++)
	        {
	            fread(&P[pc_sph].Ne, sizeof(float), 1, fd);
	            pc_sph++;
	        }
	        SKIP;
            // printf("ne1  dummy = %d\n", dummy);
	    }

        //// all output particle data
        //potential
        SKIP;
        // printf("pot0 dummy = %d\n", dummy);
        for(k = 0, pc_new = pc; k < 6; k++)
	    {
	        for(n = 0; n < header1.npart[k]; n++)
	        {
	            fread(&P[pc_new].Pot, sizeof(float), 1, fd);
	            pc_new++;
	        }
	    }
        SKIP;
        // printf("pot1 dummy = %d\n", dummy);

        //acc
        SKIP;
        // printf("acc0 dummy = %d\n", dummy);
        for(k = 0, pc_new = pc; k < 6; k++)
	    {
	        for(n = 0; n < header1.npart[k]; n++)
	        {
	            fread(&P[pc_new].Acc[0], sizeof(float), 3, fd); //gjy note: P[].Vel
	            pc_new++;
	        }
	    }
        SKIP;
        // printf("acc1 dummy = %d\n", dummy);


        //IO_DTENTR?? dA/dt, A is the entropy of gas
	    SKIP;
        // printf("ent0 dummy = %d\n", dummy);
	    for(n = 0, pc_sph = pc; n < header1.npart[0]; n++)
	    {
	        fread(&P[pc_sph].dAdt, sizeof(float), 1, fd);
	        pc_sph++;
	    }
	    SKIP;
        // printf("ent1 dummy = %d\n", dummy);


        //IO_TSTP
        //gjy note: are these Metal and Age??
        //the order is inverted??
        SKIP;
        // printf("age1 dummy = %d\n", dummy);
        for(k = 0, pc_new = pc; k < 6; k++)
	    {
	        for(n = 0; n < header1.npart[k]; n++)
	        {
	            fread(&P[pc_new].Age, sizeof(float), 1, fd);
	            pc_new++;
	        }
	    }
        SKIP;
        // printf("age1 dummy = %d\n", dummy);

        //Metal??
        SKIP;
        // printf("met0 dummy = %d\n", dummy);
        for(k = 0, pc_new = pc; k < 6; k++)
	    {
	        for(n = 0; n < header1.npart[k]; n++)
	        {
	            fread(&P[pc_new].Metal, sizeof(float), 1, fd);
	            pc_new++;
	        }
	    }
        SKIP;
        // printf("met1 dummy = %d\n", dummy);
    }

    fclose(fd);

    Time = header1.time;
    Redshift = header1.redshift;
    return 0;
}

int RW_Snapshot::reordering(void){

    int i, j;
    int idsource, idsave, dest;
    struct particle_data psave, psource;
    
    printf("reordering...\n");
    for(i = 1; i <= NumPart; i++)
    {
        if(Id[i] != i)
	    {
	        psource = P[i];
	        idsource = Id[i];
	        dest = Id[i];

	        do
	        {
	            psave = P[dest];
	            idsave = Id[dest];

	            P[dest] = psource;
	            Id[dest] = idsource;

	            if(dest == i)
		            break;

	            psource = psave;
	            idsource = idsave;

	            dest = idsource;
	        }
	        while(1);
	    }
    }
    printf("reordering...done\n");

    Id++;
    free(Id);
    Id = nullptr;

    printf("space for particle ID freed\n");
    return 0;
}



int RW_Snapshot::write_gadget_ics_known1(string path_snapshot)
{
    const char* ofname = (path_snapshot+".g1").data();
    FILE *fp;
    int temp;
    int i, j, n, pc_sph;

    if (!(fp = fopen(ofname, "w"))){
        printf("cannot create init file\n");
        exit(1);
    }

    temp = sizeof(struct io_header_1);
    fwrite(&temp, sizeof(temp), 1, fp);
    fwrite(&header1, sizeof(struct io_header_1), 1, fp);
    fwrite(&temp, sizeof(temp), 1, fp);

    //position block
    temp = sizeof(float) * 3 * NumPart;
    fwrite(&temp, sizeof(temp), 1, fp);
    for (i = 1; i <= NumPart; i++)
    {
        for (j = 0; j < 3; j++)
        {
            fwrite(&(P[i].Pos[j]), sizeof(float), 1, fp);
        }
    }
    fwrite(&temp, sizeof(temp), 1, fp);

    //velocity block
    temp = sizeof(float) * 3 * NumPart;
    fwrite(&temp, sizeof(temp), 1, fp);
    for (i = 1; i <= NumPart; i++)
    {
        for (j = 0; j < 3; j++)
        {
            fwrite(&(P[i].Vel[j]), sizeof(float), 1, fp);
        }
    }
    fwrite(&temp, sizeof(temp), 1, fp);

    //ID block
    temp = sizeof(unsigned int) * NumPart;
    fwrite(&temp, sizeof(temp), 1, fp);
    for (i = 1; i <= NumPart; i++)
    {
        fwrite(&(P[i].Id), sizeof(int), 1, fp);
    }
    fwrite(&temp, sizeof(temp), 1, fp);

    //Mass block
    temp = sizeof(float) * NumPart;
    fwrite(&temp, sizeof(temp), 1, fp);
    for (i = 1; i <= NumPart; i++)
    {
        fwrite(&(P[i].Mass), sizeof(float), 1, fp);
    }
    fwrite(&temp, sizeof(temp), 1, fp);

    if (header1.npart[0] > 0)
    {
        temp = sizeof(float) * Ngas;
        fwrite(&temp, sizeof(temp), 1, fp);
        for (n = 0, pc_sph = 1; n < header1.npart[0]; n++)
        {
            fwrite(&P[pc_sph].U, sizeof(float), 1, fp);
            pc_sph++;
        }
        fwrite(&temp, sizeof(temp), 1, fp);

        temp = sizeof(float) * Ngas;
        fwrite(&temp, sizeof(temp), 1, fp);
        for (n = 0, pc_sph = 1; n < header1.npart[0]; n++)
        {
            fwrite(&P[pc_sph].Rho, sizeof(float), 1, fp);
            pc_sph++;
        }
        fwrite(&temp, sizeof(temp), 1, fp);

        if (header1.flag_cooling)
        {
            temp = sizeof(float) * Ngas;
            fwrite(&temp, sizeof(temp), 1, fp);
            for (n = 0, pc_sph = 1; n < header1.npart[0]; n++)
            {
                fwrite(&P[pc_sph].Ne, sizeof(float), 1, fp);
                pc_sph++;
            }
            fwrite(&temp, sizeof(temp), 1, fp);
        }
    }
    fwrite(left, Left, 1, fp);
    fclose(fp);
}// end of write_snapshot()

int RW_Snapshot::write_gadget_ics_known(string path_snapshot){

    char fname[MaxCharactersInString];
    // if(pathload!=nullptr) sprintf(fname, "%s%s_%03d", pathload, bname, snap);
    // else sprintf(fname, "%s", this->path, bname, snap);
    sprintf(fname, "%s", path_snapshot.data());

    FILE *fp1;
    int dummy, ntot_withmasses, ptype;
    unsigned long int i, j, k;
    int t, n, off, pc, pc_new, pc_sph;
    int files = 1;
    char buf[MaxCharactersInString];
    #define SKIP2 fwrite(&dummy, sizeof(dummy), 1, fp1); //record

    /* hearder */
    //// We have read data to hearder and *P.
    //// change particle data, set galaxy* only for number, write.
    // int ID_change_one = ID_change;
    // // DEBUG_PRINT_V0d(1, this->P[ID_change_one].Mass, "");
    // // DEBUG_PRINT_V0d(1, rate, "");
    // this->P[ID_change_one].Mass *= rate;
    // // DEBUG_PRINT_V0d(1, this->P[ID_change_one].Mass, "");

    ////write all pd to IC file
    int N_tot_part = this->NumPartTot;
    // int N_tot_part = N_all;
    int N_tot_part_stars = 0;
    // for(int i_comp=0;i_comp<components;i_comp++){
    //     if(type_comp[i_comp]>=2){ //particle with type (>=2) are stars
    //         N_tot_part_stars += N_comp[i_comp];
    //     }
    // }

    for (i = 0, pc = 1; i < files; i++, pc = pc_new) {
        if (files > 1) //gjy changed
            sprintf(buf, "%s.g1_file%d", fname, (int)i);
        else
            sprintf(buf, "%s.g1", fname);

        if (!(fp1 = fopen(buf, "w"))) {
            fprintf(stderr, "Can't open file `%s`.\n", buf);
            exit(0);
        }

        fflush(stdout);
        // Header
        dummy = sizeof(header1);
// std::cout<<dummy<<"\n";
// exit(0);
        fwrite(&dummy, sizeof(dummy), 1, fp1);
        fwrite(&header1, sizeof(header1), 1, fp1);
        fwrite(&dummy, sizeof(dummy), 1, fp1);

        for (k = 0, ntot_withmasses = 0; k < 6; k++) {
            if (header1.mass[k] == 0)
                ntot_withmasses += header1.npart[k];
        }

        // Positions
        dummy = 3 * sizeof(float) * N_tot_part;
        SKIP2;
        for (k = 0, pc_new = pc; k < 6; k++) {
            for (n = 0; n < header1.npart[k]; n++) {
                fwrite(&P[pc_new].Pos[0], sizeof(float), 3, fp1);
                pc_new++;
            }
        }
        SKIP2;

        // Velocities
        SKIP2;
        for (k = 0, pc_new = pc; k < 6; k++) {
            for (n = 0; n < header1.npart[k]; n++) {
                fwrite(&P[pc_new].Vel[0], sizeof(float), 3, fp1);
                pc_new++;
            }
        }
        SKIP2;

        // Identifiers
        dummy = sizeof(int) * N_tot_part;
        SKIP2;
        for (k = 0, pc_new = pc; k < 6; k++) {
            for (n = 0; n < header1.npart[k]; n++) {
                fwrite(&P[pc_new].Id, sizeof(int), 1, fp1);
                pc_new++;
            }
        }
        SKIP2;

        // Mass
        if (ntot_withmasses > 0) {
            dummy = sizeof(float) * N_tot_part;
            SKIP2;
        }
        for (k = 0, pc_new = pc; k < 6; k++) {
            for (n = 0; n < header1.npart[k]; n++) {
                if (header1.mass[k] == 0)
                    fwrite(&P[pc_new].Mass, sizeof(float), 1, fp1);
                else
                    P[pc_new].Mass = header1.mass[k];
                pc_new++;
            }
        }
        if (ntot_withmasses > 0) {
            SKIP2;
        }

        //// Gas specific datablocks
        if (header1.npart[0] > 0) {
            // Internal energy
            dummy = sizeof(float) * header1.npart[0];
            SKIP2;
            for (n = 0, pc_sph = pc; n < header1.npart[0]; n++) {
                fwrite(&P[pc_sph].U, sizeof(float), 1, fp1);
                pc_sph++;
            }
            SKIP2;
            // Density
            SKIP2;
            for (n = 0, pc_sph = pc; n < header1.npart[0]; n++) {
                fwrite(&P[pc_sph].Rho, sizeof(float), 1, fp1);
                pc_sph++;
            }
            SKIP2;
        }

        //// added, two
        //Potential
        dummy = sizeof(float) * N_tot_part;
        SKIP2;
        for (k = 0, pc_new = pc; k < 6; k++) {
            for (n = 0; n < header1.npart[k]; n++) {
                fwrite(&P[pc_new].Pot, sizeof(float), 1, fp1);
                pc_new++;
            }
        }
        SKIP2;

        //Accelerations
        dummy = 3 * sizeof(float) * N_tot_part;
        SKIP2;
        for (k = 0, pc_new = pc; k < 6; k++) {
            for (n = 0; n < header1.npart[k]; n++) {
                fwrite(&P[pc_new].Acc[0], sizeof(float), 3, fp1);
                pc_new++;
            }
        }
        SKIP2;

        ////others no use
        // Metallicity
        dummy = sizeof(int) * N_tot_part; //gjy note: int??
        SKIP2;
        for (k = 0, pc_new = pc; k < 6; k++) {
            for (n = 0; n < header1.npart[k]; n++) {
                fwrite(&P[pc_new].Metal, sizeof(float), 1, fp1);
                pc_new++;
            }
        }
        SKIP2;

        // Age for star particles
        if (N_tot_part_stars > 0) {
            dummy = sizeof(int) * (N_tot_part_stars);
            SKIP2;
            for (k = 2, pc_new = header1.npart[0] + header1.npart[1] + 1; k < 6; k++) {
                for (n = 0; n < header1.npart[k]; n++) {
                    fwrite(&P[pc_new].Age, sizeof(float), 1, fp1);
                    pc_new++;
                }
            }
            SKIP2;
        }

    }

    fclose(fp1);
    printf("Wrote %s ... done.\n", buf);
    return 0;
}

////to debug
int main0001(){

    abort();
    return 0;
}
