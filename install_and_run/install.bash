#!/usr/bin/env bash
#### prepare and install
echo -e "Begin to install ... \n"
set -e -u

####[steps]: step0. preparing
# folder_main=../ #[learn code] wrong because all the next will use "../"
folder_main=${HOME}/workroom/0prog/GDDFAA_work/
folder_packages=${folder_main}packages/
folder_dependencies=${folder_main}GDDFAA/dependencies/
folder_thisFile=${folder_main}install_and_run/ #it is the path of this the file
folder_make_DICE=${folder_main}GDDFAA/step1_galaxy_IC_preprocess/step2_select_generate_IC_DICE/
folder_simulation=${folder_main}GDDFAA/step2_Nbody_simulation/gadget/
folder_dependencies=${folder_main}GDDFAA/dependencies/
folder_actions=${folder_main}GDDFAA/step3_actions/

cd ${folder_main}
echo -e "#now at folder: ${PWD}"



####[steps]: step1. configure environment
is_install_environment=1
if [ ${is_install_environment} -eq 1 ]; then
    echo -e "Do configure environment."
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    ##1. configure enviroment in Ubuntu 20.04
    #[note]: If you have configured them but the version is wrong, 
    #\ please deal with them or use a new computer software system.
    
    # sudo apt update
    # sudo apt install gcc g++ gfortran
    # sudo apt install python3 python3-pip
    # sudo apt install git wget cmake mpich
    # sudo apt install libgsl-dev
    # sudo apt install libfftw3-dev
    # sudo apt install libeigen3-dev
    # sudo cp -r /usr/include/eigen3/Eigen /usr/include #??
    # sudo apt install liblapack-dev
    # sudo apt install libhdf5-dev
    # sudo apt install libgtest-dev
    # sudo apt install libopencv-dev
    #[note]: If you can not download them automatically, 
    #\ please download manually and put to the path correctly
    #\ the packages should provided instead of downloading by user
    
    # git packages_XX

    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"
fi



####[steps]: step2. install small
is_install_IC=1
if [ ${is_install_IC} -eq 1 ]; then
    ## 1. install programs for initial conditions and other functions
    echo -e "install DICE:\n"
    #: move (only at the first time)
    # # wget #??
    # tar -zxvf DICE_galaxy_initial_conditions.tar.gz
    # mv DICE_galaxy_initial_conditions/ ${folder_main}GDDFAA/step1_galaxy_IC_preprocess/
    # cd ${folder_main}GDDFAA/step1_galaxy_IC_preprocess/
    # mv DICE_galaxy_initial_conditions/ step2_select_generate_IC_DICE/
    # cd step2_select_generate_IC_DICE/
    cd ${folder_main}GDDFAA/step1_galaxy_IC_preprocess/step2_select_generate_IC_DICE/
    if [ -d ./dice_local/ ]; then
        rm -rf ./dice_local/ #[note] rm
    fi
    if [ -d ./build/ ]; then
        rm -rf ./build/ #[note] rm
    fi
    mkdir dice_local/
    mkdir build/ && cd build/
    # make clean
    CXX=gcc cmake .. -DENABLE_THREADS=ON -DCMAKE_INSTALL_PREFIX=${folder_make_DICE}dice_local/
    make && make install
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    echo -e "install AGAMA:\n"
    cd ${folder_main}GDDFAA/step1_galaxy_IC_preprocess/step2_select_generate_IC_and_get_DFA_AGAMA_not_install/
    #() install
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    echo -e "install cold_python:\n"
    #nothing
    echo -e "#now at folder: ${PWD}"
fi



is_install_simulation=1
if [ ${is_install_simulation} -eq 1 ]; then
    ## 2. install packages for simulation: 
    #\ mpi, gsl, fftw2, hdf5 -> Gadget2.0.7
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    # echo -e "install gsl (some old versions):\n"
    # tar -zxvf gsl-1.9.tar.gz #[learn code] tar -zcvf A.tar.gz A/ #to gzip and tar folder A
    # cd gsl-1.9/
    # ./configure --prefix=${folder_dependencies} #PWD #/usr/local/
    # make && make install #make clean #sudo make install
    # cd ${folder_packages}
    # echo -e "#now at folder: ${PWD}"

    echo -e "install gsl (old version):\n"
    tar -zxvf gsl-1.16.tar.gz
    cd gsl-1.16/
    ./configure --prefix=${folder_dependencies} #PWD #/usr/local/
    make && make install
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    echo -e "install fftw2 (with float):\n"
    tar -zxvf fftw-2.1.5.tar.gz
    # mv fftw-2.1.5/ ../ && cd ../
    cd fftw-2.1.5/
    ./configure --enable-mpi --enable-type-prefix --enable-float --prefix=${folder_dependencies} #PWD #/usr/local/
    make && make install
    ./configure --enable-mpi --enable-type-prefix --prefix=${folder_dependencies} #PWD #/usr/local/
    make && make install
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    echo -e "install hdf5 (optional):\n"
    # sudo apt
    #[select]:
    tar -xzf hdf5-1.6.9.tar.gz
    # mv hdf5-1.6.9/ ../ && cd ../
    cd hdf5-1.6.9/
    ./configure --prefix=${folder_dependencies} #PWD #/usr/local/
    make && make install
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    echo -e "install Gadget2:\n"
    #[note]: Some settings of original package has been changed for this work: 
    #\ folder structure and  makefile contents.
    #\ The link of original package is above.
    #\ at http://wwwmpa.mpa-garching.mpg.de/gadget/gadget-2.0.7.tar.gz
    tar -zxvf Gadget-2.0.7.tar.gz
    cp -rfap Gadget-2.0.7/* ${folder_simulation}Gadget-2.0.7/ #[note] rm
    cd ${folder_simulation}Gadget-2.0.7/Gadget2/
    echo -e "#now at folder: ${PWD}"
    cp ${folder_packages}makefiles/Makefile_Gadget2 ${folder_simulation}Gadget-2.0.7/Gadget2/Makefile
    make clean; make
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"
fi

is_install_process=1
if [ ${is_install_process} -eq 1 ]; then
    ## 4. install python packages #??
    echo -e "install data_process part (mainly python3):\n"
    # pip3 install numpy matplotlib scipy sklearn
    # pip3 install traceback copy pdb tqdm
    # pip3 install emcee corner
    # pip3 install galpy GALA
    # pip3 install astropy pandas pytorch
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"
fi



is_install_actions=1
if [ ${is_install_actions} -eq 1 ]; then
    ## 3. install packages for actions
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    echo -e "make for dependencies:\n"
    #() install gsl-1.16 gtest ebf lapack blas (boost) Torus DataTACT

    echo -e "install gsl (old version):\n"
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    echo -e "install gtest:\n"
    # sudo apt install libgtest-dev
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    echo -e "install ebf:\n"
    tar -zxvf libebf_c_cpp-0.0.3.tar
    cd libebf_c_cpp-0.0.3/
    ./configure --prefix=${folder_dependencies} #PWD #/usr/local/
    make && make install
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    echo -e "install lapack and blas:\n"
    # sudo apt install liblapack-dev
    # #: or
    # #[note]: The make file of lapack has been changed.
    # # tar -zxvf lapack-3.4.2.tgz
    # # cd lapack-3.4.2/
    # # cp make.inc.example make.inc
    # # make
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    echo -e "install boost (optional if changed to python):\n"
    # nothing
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    echo -e "install DataTACT and Torus (with SCF knn other):\n"
    #[note]: The make file of Torus has been changed.
    #\ TACT has been changed to DataTACT.
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"
    tar -zxvf Torus-master.tar.gz
    cp -rfap Torus-master/ ${folder_actions}
    cd ${folder_actions}Torus-master/
    echo -e "#now at folder: ${PWD}"
    # ./configure --prefix=${PWD} #nothing
    make clean
    make
    # cp DataTACT #??
    cd ${folder_packages}
    echo -e "#now at folder: ${PWD}"

    echo -e "make DataTACT and Torus (with SCF knn other):\n"
    cd ${folder_actions}
    echo -e "#now at folder: ${PWD}"
    cd ${folder_actions}step2_Data_TACT/aa/;
    make clean && cd ../;
    make LAPACK=1 TORUS=1;
    cd aa;
    cd ${folder_actions}
    echo -e "#now at folder: ${PWD}"
fi

set +e +u
echo -e "End to install.\n"
