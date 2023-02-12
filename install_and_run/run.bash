#!/usr/bin/env bash
# #### Instructions for DDFA
# 0. download and install programs
# 1. initial condition of galaxy
# 2. Nbody simulation
# 3. calculate actions
# 4. data process for fitting distribution functions of actions
# 5. paramters tabel
# #### End





echo -e "Begin to run ...\n"
set -e -u
ls;

#### all of the commands about galaxy-actions
folder_main=${HOME}/workroom/0prog/GDDFAA_work/
folder_thisFile=${folder_main}install_and_run/ #the path of this file

folder_packages=${folder_main}packages/
folder_dependencies=${folder_main}GDDFAA/dependencies/
folder_make_DICE=${folder_main}GDDFAA/step1_galaxy_IC_preprocess/step2_select_generate_IC_DICE/

folder_initial_condition=${folder_main}GDDFAA/step1_galaxy_IC_preprocess/
folder_IC_setting=${folder_initial_condition}step1_set_IC_DDFA/
folder_IC_trans=${folder_initial_condition}step3_preprocess_IC/step1_from_ascii_to_g1_and_run/
folder_DICE=${folder_initial_condition}step2_select_generate_IC_DICE/
folder_cold=${folder_initial_condition}step2_select_generate_IC_python/
folder_process=${folder_main}GDDFAA/step4_data_process/data_process/
folder_simulation=${folder_main}GDDFAA/step2_Nbody_simulation/gadget/
folder_simu_setting=${folder_simulation}Gadget-2.0.7/
folder_gm=${folder_simulation}Gadget-2.0.7/galaxy_general/
folder_snapshot_txt=${folder_simulation}Gadget-2.0.7/galaxy_general/txt/
folder_actions=${folder_main}GDDFAA/step3_actions/
folder_SCF=${folder_actions}step2_Data_TACT/DataInterface/SCF/SCF_coeff_pot/
folder_AA=${folder_actions}step2_Data_TACT/aa/

cd ${folder_thisFile}
echo -e "#now at folder: ${PWD}"
#: one can change the ${filename_user_settings_change_manully} and some other setting files
filename_user_settings_change_manully="user_settings_multi.txt"
filename_user_settings="user_settings.txt"

is_run_outputNewFolder=0 #cmd 4
# if [ -d ./${foldername_startTime} ]; then 
#     mv ./${foldername_startTime} ./${foldername_startTime}_bak
# mkdir ${foldername_startTime}
foldername_startTime=`date +%Y%m%dday%H%M%Ssec` #set name by date
if [ ${is_run_outputNewFolder} -eq 1 ]; then
    echo "${foldername_startTime}" > output_folder_name.txt
fi
cp ${filename_user_settings_change_manully} ${filename_user_settings}



## functions
DEBUG_PRINT_INFO(){
    echo -e "DEBUG_PRINT_V:"
    echo -e "${2}"
    if [ ${1} -eq 0 ]; then
        echo -e "Then exit compulsively."
        exit
    fi
}

generate_data_model(){
    ModelX=${1}
    ModelName=${2}
    ModelInfo=${3}
    tag_IC=${4}
    is_simulation_little=${5} #no use
    is_preprocess_before_simulation=${6}

    echo -e "Run for a galaxy model ${2} ... "
    cd ${folder_main}GDDFAA/
    echo -e "#now at folder: ${PWD}"

    is_run_IC=0 #cmd 1_1
    if [ ${is_run_IC} -eq 1 ]; then
        ## (1) choose the set IC
        cd ${folder_IC_setting}
        echo -e "#now at folder: ${PWD}"
        filename_IC="IC_param_"${ModelName}".txt"
        filename_IC_method1=${filename_IC}
        filename_IC_method21="IC_DICE_"${ModelName}".params"
        filename_IC_method22=galaxy_general.config
        filename_IC_method3="IC_AGAMA_"${ModelName}".txt"
        echo -e "filename_IC: ${filename_IC}"
        cp ${filename_IC} IC_param.txt

        ## (2) generate IC and preprocess
        rm -rf ${folder_IC_trans}*.g1 #[note] rm
        local_is_trans_txt=0
        cd ${folder_initial_condition}
        echo -e "#now at folder: ${PWD}"

        if [ ${tag_IC} -eq 1 ]; then
            echo -e "generate IC by DICE:\n"
            cd ${folder_DICE}
            echo -e "#now at folder: ${PWD}"

            if [ -d ./run_bak/ ]; then
                rm -rf ./run_bak/ #[note] rm
            fi
            if [ -d ./run/ ]; then
                mv run/ run_bak/ #[note] rm
            fi
            mkdir run/ run/params_files/
            bash step1_compile.bat

            cp build/bin/dice run/
            cp ${folder_IC_setting}${filename_IC_method21} run/params_files/galaxy_general.params
            cp ${folder_IC_setting}${filename_IC_method22} run/galaxy_general.config
            bash step2_run.bat

            local_is_trans_txt=0 #[learn code] here do not add ${} for integer variable
            cp -r run/* ${folder_IC_trans} #[learn code] no [-r] to omit inner directories #cp to ${folder_IC_trans}
            echo -e "generate IC by DICE. Done.\n"

        elif [ ${tag_IC} -eq 2 ]; then
            local_is_trans_txt=1
            echo -e "generate IC by cold_python:\n"
            cd ${folder_cold}
            echo -e "#now at folder: ${PWD}"
            bash step1_compile.bat
            bash step2_run.bat #gen IC and cp to ${folder_IC_trans}
            echo -e "generate IC by cold_python. Done.\n"

        elif [ ${tag_IC} -eq 2 ]; then
            local_is_trans_txt=0
            echo -e "generate IC by AGAMA:\n"
            echo -e "Not provided now."
            echo -e "generate IC by AGAMA. Done.\n"
            exit
            #~ ...

        else
            local_is_trans_txt=0
            echo -e "\nNo such method provided."
            # echo -e "Skip to generate IC!\n\n\n"
            echo -e "Exit."
            exit #[learn code] exit can only exit this loop
        fi

        #: convert each other (start)
        cd ${folder_IC_trans}
        echo -e "#now at folder: ${PWD}"
        bash step1_compile.bat
        bash step2_run.bat ${local_is_trans_txt} #convert g1 to txt or convert txt to g1
        # exit #debug

        ## (3) run a little and cp
        cd ${folder_IC_trans}
        echo -e "#now at folder: ${PWD}"
        if [ ${is_simulation_little} -eq 1 ]; then
            cd ${folder_IC_trans}
            echo -e "#now at folder: ${PWD}"
            bash step3_simulation_a_little.bat

            cd ${folder_IC_trans}
            echo -e "#now at folder: ${PWD}"
            bash step2_run.bat ${local_is_trans_txt} #convert g1 to txt or convert txt to g1
        fi
        
        if [ ${is_preprocess_before_simulation} -eq 1 ]; then
            cd ${folder_initial_condition}step3_preprocess_IC/step2_some_preprocess/
            bash step1_compile.bat
            bash step2_run.bat

            cd ${folder_IC_trans}
            echo -e "#now at folder: ${PWD}"
            bash step2_run.bat ${local_is_trans_txt} #convert g1 to txt or convert txt to g1
        fi
        # exit #debug
    fi

    is_run_simulation=0 #cmd 1_2
    if [ ${is_run_simulation} -eq 1 ]; then
        ##:: 2. simulation
        echo -e "Build folders ..."
        cd ${folder_simu_setting}
        echo -e "#now at folder: ${PWD}"
        folder_gm_name="galaxy_general"
        if [ -d ${folder_gm_name}_bak ]; then
            rm -rf ${folder_gm_name}_bak/
        fi
        if [ -d ${folder_gm_name} ]; then
            mv ${folder_gm_name} ${folder_gm_name}_bak
        fi
        mkdir ${folder_gm_name}

        cd ${folder_gm}
        echo -e "#now at folder: ${PWD}"
        mkdir snapshot/ debug/ intermediate/ txt/ particle/ orbit/ aa/
        cp ../Gadget2/Gadget2 ./
        cp ../../../../step1_galaxy_IC_preprocess/step1_set_IC_DDFA/IC_param.txt ./
        cp ../../../../step1_galaxy_IC_preprocess/step3_preprocess_IC/step1_from_ascii_to_g1_and_run/galaxy_general.g1 ./
        cp -r ../../settings_and_info/* ./
        cp -r ../../settings_and_info/convert_to_txt.bat ./txt/
        cp -r ../../settings_and_info/example_snapshot_DF_params_fit.xv.txt ./aa/

        echo -e "Simlation ..."
        cd ${folder_gm}
        echo -e "#now at folder: ${PWD}"
        bash simulate.bat
        # exit #debug
    fi

    is_run_actions=1 #cmd 1_3
    is_run_fit=0 #cmd 1_4
    ##:: 3. actions (run snapshots of a galaxy_model)
    #\ note: run any of the two will update the old argvs of snapshots name
    cd ${folder_thisFile}
    echo -e "#now at folder: ${PWD}"

    cd ${folder_actions}step1_preprocess/
    echo -e "#now at folder: ${PWD}"
    bash step1_compile.bat #python3 tell_shell_read_what_argv.py #[value] of snapshot time: [0., 9.](0.1)
    bash step2_run.bat > snapshots_dealwith.txt
    echo -e "redirect snapshots_dealwith"
    filename_target_snapshots="argv_target_snapshots.txt" #[value]: [0,900](10)
    argv_target_snapshots=`cat ${filename_target_snapshots}`
    ar_ats=(${argv_target_snapshots})
    snapshot_interval_stde=`expr ${ar_ats[1]} - ${ar_ats[0]}` #[value]: 10
    echo -e "ar_ats: ${ar_ats[1]} ${ar_ats[0]}"
    DEBUG_PRINT_INFO 1 ${snapshot_interval_stde} #debug
    snapshot_interval_stde=`expr ${snapshot_interval_stde} / 1` #[learn code]: If a/b but b<a, then error.
    DEBUG_PRINT_INFO 1 ${snapshot_interval_stde} #debug

    ii=0
# #tmp add
#     argv_target_snapshots="160 180"
    echo -e "ss_${argv_target_snapshots}"
    for i in ${argv_target_snapshots};
    do
        ## (1) preprocess (fit DF of Cartesians)
        echo -e "AA for snapshot_${i}:"
        cd ${folder_snapshot_txt}
        echo -e "#now at folder: ${PWD}"
        snapshot_ID_fill_zero=`eval echo ${i} | awk '{printf("%03d",$0)}'` #[value]: ${i} with zero, each snapshot 
        echo -e "snapshot_ID_fill_zero: ${snapshot_ID_fill_zero}"
        bash convert_to_txt.bat snapshot_${snapshot_ID_fill_zero}

        ## (2) data_process 1 (fit DF of acitons)
        if [ ${is_run_actions} -eq 1 ]; then
            cd ${folder_process}
            echo -e "#now at folder: ${PWD}"
            echo -e "Read snapshot_${i}, preprocess and fit DF_1, record and write SCF"
            python3 fit_galaxy_distribution_function.py 1 ${i} default
# #tmp add
#             python3 fit_galaxy_distribution_function.py 1 80 default #debug
            cd ${folder_process}
            echo -e "#now at folder: ${PWD}"
        fi

        ## (3) SCF and foci and actions running
        ## SCF and foci
        if [ ${is_run_actions} -eq 1 ]; then
            # echo -e "aaaa\n\n\n\n\n"
            cd ${folder_actions}step1_preprocess/
            echo -e "#now at folder: ${PWD}"

            filename_run_AA="argv_run_AA_${i}.txt"
            echo -e "cp argv_run_AA: ${filename_run_AA}"
            cp ${filename_run_AA} ${folder_AA}step2_run.bat
            cp ${filename_run_AA} ${folder_gm}argv_AA.bat

            cd ${folder_AA}
            echo -e "#now at folder: ${PWD}"
            echo -e "Estimate angle-actions"
            #: This prog is not completed, so one use it to change the SCF potential
            # bash step1_1_compile_all.bat #g++ #debug lmn and aa #use the example snapshot SCF
            bash step1_2_compile_SCF.bat #use the current snapshot
            bash step1_3_prepare_foci.bat #run foci extimating prog independently
            cp ../DataInterface/SCF/orbitIntegSCF_adjust_a2b2/src/some_lmn_foci_Pot.txt \
            ${folder_gm}intermediate/snapshot_${i}_lmn_foci_Pot.txt #then copy with name of snapshot
        fi

        ## actions
        if [ ${is_run_actions} -eq 1 ]; then
            cd ${folder_AA}
            echo -e "#now at folder: ${PWD}"
            # mpirun -np 2 mains/./data.exe 8.000 8.000 0.10 0.10 0. 0 0 5 4 -1 -1 -1 8.000 8.001 1. 1000 100 0 1000000 0 -3. -2. 1 1 1. 1. 1. 1. #debug
            bash step2_run.bat #run with the argvs output by python writting 
            ii=$((${ii}+1))
        fi

        ## (4) data process 2 (fit DF of local average acitons)
        if [ ${ii} -eq 5 ] && [ ${i} -lt ${ar_ats[-1]} ] && [ ${is_run_fit} -eq 1 ]; then #only run a snapshot DF each 5 snapshots
            cd ${folder_process}
            echo -e "#now at folder: ${PWD}"
            echo -e "Read AA, preprocess and fit DF_2, record and compare paramters"
            snapshot_dealwith=`expr ${i} - ${snapshot_interval_stde} \* 3` #like using snapshot_"14 = 20-2*3"
            python3 stde_action_by_near_time.py ${snapshot_dealwith} ${snapshot_interval_stde}
            python3 fit_galaxy_distribution_function.py 2 ${snapshot_dealwith} default
            cd ${folder_process}
            echo -e "#now at folder: ${PWD}"
            ii=$((5))
        fi
    done

#tmp
    # cd ${folder_julia}
    # echo -e "#now at folder: ${PWD}"
    # julia a.jl

    echo -e "\nRun model ${ModelX}, ${ModelName}, ${ModelInfo}. Done.\n"
}

compare_fits(){
    MGs=${1}

    echo -e "Record fits and compare:\n"
    cd ${folder_process}
    echo -e "#now at folder: ${PWD}"
    python3 fit_galaxy_distribution_function.py 3 0 ${MGs}
}





#### main()
echo -e "Begin:\n"

## (1) settings
#@ tag_IC
#[elect] 1: by prog DICE in C; default
#[elect] 2: by prog cold in python
#[elect] 3: by prog AGAMA
#[elect] !: by prog other suppers
tag_IC=1

#@ is_simulation_little #no use much
#[elect] 0: do nothing for generated IC; default
#[elect] 1: run a little bit and change unbound particles
is_simulation_little=0

#@ is_preprocess_before_simulation #no use much
#[elect] 0: do nothing
#[elect] 1: delete unbound
is_preprocess_before_simulation=0

# #@ is_run
# #[elect] 0: do not run AA
# #[elect] 1: run AA

#@ MGs
unset MGs
# MGs=("Sersic")
# MGs=("DPL" "Sersic")



## (2) check data folder
cd ${folder_thisFile}
echo -e "#now at folder: ${PWD}"
idx_line=0
# cat ${filename_user_settings} | while read line; do #[learn code] why cannot know outer vars ??
# IFS="\n"; for line in `cat ${filename_user_settings}`; do [learn code] why cannot cat the correct filename ??
while read line; do
    echo -e "${line}"
    if [ ${idx_line} -eq 6 ]; then
        echo -e "models names: ${line}, index: ${idx_line}"
        for i in ${line}; do
            MGs+=(${i})
        done
        break
    fi
    idx_line=$((idx_line+1))
done <${filename_user_settings}
echo -e "MGs: ${MGs[@]}"

is_run_model=1 #cmd 1
cd ${folder_simulation}Gadget-2.0.7/
echo -e "#now at folder: ${PWD}"
for m in ${MGs[@]};
do
    gm_rename=galaxy_general_${m}
    if [ -d ${gm_rename} ]; then
        echo -e "The data folder `${gm_rename}` has been exist. Skip to run the models."
        is_run_model=0
    else
        echo -e "Now start to run galaxy model ${gm_rename} ..."
    fi
done
# exit #debug



## (2) run MG models one by one
is_rename_folder=0 #cmd 2
if [ ${is_run_model} -eq 1 ]; then
    ##: run
    cd ${folder_thisFile}
    echo -e "#now at folder: ${PWD}"
    for m in ${MGs[@]};
    do
        echo -e "MG: ${m}"
        ModelX="1" #no use
        ModelName=${m}
        ModelInfo="_type-halo_theExpectedValues_totalMass-137._radiusScale-19.6_axisRatxio-1.-0.6-0.3_count-1e6_End"
        generate_data_model ${ModelX} \
            ${ModelName} ${ModelInfo} ${tag_IC} \
            ${is_simulation_little} ${is_preprocess_before_simulation}

        if [ ${is_rename_folder} -eq 1 ]; then
            cd ${folder_simulation}Gadget-2.0.7/
            echo -e "#now at folder: ${PWD}"
            m=${ModelName}
            echo -e "MG: ${m}"
            gm_rename=galaxy_general_${m}
            mv galaxy_general/ ./${gm_rename}/ #rename
        fi
    done
fi



## (3) compare
cd ${folder_thisFile}
echo -e "#now at folder: ${PWD}"
is_run_compare=0 #cmd 3
if [ ${is_rename_folder} -eq 1 ] && [${is_run_compare} -eq 1] ; then
    compare_fits ${MGs}
fi

set +e +u
echo -e "End to run."
