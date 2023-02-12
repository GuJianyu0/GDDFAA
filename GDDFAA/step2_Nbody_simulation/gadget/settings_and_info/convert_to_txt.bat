
cp ../snapshot/${1} ./
cp ../../../../../step1_galaxy_IC_preprocess/step3_preprocess_IC/step1_from_ascii_to_g1_and_run/read_snapshot.exe ./
echo -e "convert ${1} ti txt:"
./read_snapshot.exe 2 ${1}
