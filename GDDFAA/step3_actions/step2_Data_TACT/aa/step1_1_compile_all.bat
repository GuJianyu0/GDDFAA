
cd ./;
make clean && cd ..;
make LAPACK=1 TORUS=1;
cd aa;
#?? there is SCF_coef and need coordtrans .o before
