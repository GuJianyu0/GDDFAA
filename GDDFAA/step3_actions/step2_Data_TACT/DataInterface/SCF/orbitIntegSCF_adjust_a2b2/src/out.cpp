//// main
extern "C"{
#include"F_to_C.h"
}
#include"adjust_foci.h"
using namespace std;
using namespace UTILITIES;

const double r_range_min = 1e-1; //1e1 nan while 1e1+0.1 not
const double r_range_max = 2.5e2;
// const double r_range_min = 9e0+0.1;
// const double r_range_max = 1.5e2+0.1;
// const int N_grid = 3;
const int N_grid = 32; //3;

int main(int argc, char* argv[])
{
    SCFORB::get_parameter_(); //note: to get parameters about SCF potential
    double poti, ai[Dim], adoti[Dim];

    double xcar_target[] = {10., 1., -2.};
    double vcar_target[] = {20., 20., 20.};
    SCFORB::get_pot_(xcar_target, &poti);
    SCFORB::get_force_(xcar_target, vcar_target, ai, adoti);
    std::cout<<"poti    : "<<poti<<"\n";
    std::cout<<"ai      : "<<ai[0]<<" "<<ai[1]<<" "<<ai[2]<<"\n";
    std::cout<<"adoti   : "<<adoti[0]<<" "<<adoti[1]<<" "<<adoti[2]<<"\n";
    
    double xcar_target1[] = {0., 0., 1.}; //nan for poti
    // double xcar_target1[] = {1e-1, 1e-1, 1e0};
    // double xcar_target1[] = {1e-56, 1e-56, 1e0};
    // double xcar_target1[] = {1e-8, 1e-8, 1.}; //nan for ai and adoti
    double vcar_target1[] = {20., 20., 20.};
    SCFORB::get_pot_(xcar_target1, &poti);
    SCFORB::get_force_(xcar_target1, vcar_target1, ai, adoti);
    std::cout<<"poti    : "<<poti<<"\n";
    std::cout<<"ai      : "<<ai[0]<<" "<<ai[1]<<" "<<ai[2]<<"\n";
    std::cout<<"adoti   : "<<adoti[0]<<" "<<adoti[1]<<" "<<adoti[2]<<"\n";
    // exit(0);



    string filename = "some_lmn_foci_Pot.txt";
    //\ note: [filename] is the output file of the foci~energy table for actions, 
    //\ each line is "#energy alpha beta delta1 delta2 vy yy "
    
    double delta_y = (r_range_max-r_range_min)/(N_grid);
    vector<data_debug> vdd;
	vdd.resize(N_grid);

    for(int i=0;i<N_grid;i++){
        string a2b2c2 = "a2b2c2.txt";
        string orbitdata = "Orbit.dat"; //note: orbit data (it will updated when a new energy)
        string orbitdata_num_b2 = "orbit/orbit_"+to_string(i)+"_b2.dat";
        string orbitdata_num_a2 = "orbit/orbit_"+to_string(i)+"_a2.dat";
        double D1 = 0., D2 = 0.;

        FILE* fp1 = fopen(a2b2c2.data(), "w");
        if(fp1==nullptr){
            printf("Cannot open file %s. Now exit.\n", a2b2c2.data());
            exit(0);
        }
        fprintf(fp1, "%le\n%le\n%le\n", a2_init, b2_init, c2_init);
        fclose(fp1);

        //[step]1: get closed orbit and search the min-max axis for foci
        SCFAEF<3> SCFAEF_O; //note: wrapper class to get foci~energy table
        double ef[N_var];
        double yy = r_range_min+delta_y*i; //note: y cut of each time (corresponded to an energy)
        cout<<"\nAt grid "<<i<<", yy = "<<yy<<":\n";

        remove(orbitdata.data());
        SCFORB::__orbintegz_MOD_main_for_orbit_integrating(&yy); //note: wrapper to get the closed orbit, from Fortran
        SCFAEF_O.load_orbit_data(); //note: load orbit data
        // SCFAEF_O.main_1_b2(ef);
        // SCFORB::writetofile_release_();
        D2 = SCFAEF_O.foci_by_shape(1); //note: search the min-max axis for foci
        copy_file(orbitdata.data(), orbitdata_num_b2.data()); //cp Orbit.dat Orbit_grid_${i}_a.dat

        remove(orbitdata.data()); //??
        SCFORB::__orbintegx_MOD_main_for_orbit_integrating(&yy);
        SCFAEF_O.load_orbit_data();
        // SCFAEF_O.main_2_a2(ef);
        // SCFORB::writetofile_release_();
        D1 = SCFAEF_O.foci_by_shape(0);
        copy_file(orbitdata.data(), orbitdata_num_a2.data());

        //[step]2: get energy of that y cut and prepare to write
        double x0 = SCFAEF_O.xyz_grid[0][0], 
        x1 = SCFAEF_O.xyz_grid[0][1],
        x2 = SCFAEF_O.xyz_grid[0][2];
        double vx0 = SCFAEF_O.vvv_grid[0][0], 
        vx1 = SCFAEF_O.vvv_grid[0][1], 
        vx2 = SCFAEF_O.vvv_grid[0][2];
        double pot;
        SCFORB::get_pot_xyz_(&x0, &x1, &x2, &pot);
        double energy = 0.5*l2norm_real<double>({vx0,vx1,vx2})+pot; //??
        double b2m = -D1-1., a2m = -D2+b2m;
        printf("%le %le %le; %le %le \n", energy, b2m, a2m, yy, pot);
        printf("%le %le %le; %le %le %le \n", x0, x1, x2, vx0, vx1, vx2);

        vdd[i].value_double.resize(8);
        vdd[i].value_double[0] = energy;    //energy of best foci orbit
        vdd[i].value_double[1] = b2m;       //-b^2 of ...
        vdd[i].value_double[2] = a2m;       //-a^2
        vdd[i].value_double[3] = -1.;       //-c^2
        vdd[i].value_double[4] = yy;        //y_cut
    }
    write_data_debug(vdd, filename);

    return 0;
}



//// old
// double dd(double *a){
//     return a[1];
// }
// void main(){
//     double a[] = {0, 1.1, 2.2};
//     double dd_value = dd(a); //ok
//     std::cout<<"dd: "<<dd_value<<"\n";
// }



// int main(int argc, char* argv[])
// {
//     const int ni = 4;
//     double x_samples[ni], y_samples[ni];
//     for(int i=0;i<ni;++i){
//         x_samples[i] = 1.*(i-1.);
//         y_samples[i] = sin(x_samples[i]);
//     }
//     double x_target = x_samples[1]+0.01, y_target, ydx_target;
//     int ni_tem = ni, m = 1; //[learn code] can not ocnvert "int const *" to "int *"; *a_form -- &a_conctent 
//     double tmp;
//     tmp = x_samples[1]; x_samples[1] = x_samples[2]; x_samples[2] = tmp;
//     tmp = y_samples[1]; y_samples[1] = y_samples[2]; y_samples[2] = tmp;
//     SCFORB::interpolate_3o_spline_1d_(x_samples, y_samples, &ni_tem, 
//         &x_target, &y_target, &ydx_target, &m);
//     std::cout<<"xt: "<<x_target<<"\n";
//     std::cout<<"yf: "<<sin(x_target)<<"\n";
//     std::cout<<"yt: "<<y_target<<"\n";
//     exit(0);
// }