######################## 
#set term postscript landscape enhanced color solid "Text" 16
#set term postscript eps enhanced color solid "Text" 16
#set term postscript portrait enhanced color "Text" 16

set term png giant enhanced size 1280, 1024

#set size 6.0/7.0, 6.0/10.0

#set size ratio 1, 1

#set size 1, 1.25
set size square

set zeroaxis
set grid
set pointsize 1.0
set mxtics 10
set mytics 10

set key top right

#set title 'Uniform and isotropic gas: DEN=1, DOTDEN=0; N=10KB; M_{BH}=0; {/Symbol e}=10^{-4};'
#set title "Plummer, N=64k, M_{bh}=0.01, R_{bh}=0.5, VZ_{bh}=0.0, {/Symbol e}=10^{-5}"

########################

########################
#set format x "%.1f"
#set format x "%.1t 10^{%T}"

#set format y "%.1f"
#set format y "10^{%T}"

#set logscale y 10

set output 'Orbit-XY.png'

set xlabel 'X [kpc]'
set ylabel 'Y [kpc]'

set xrange [-4:4]
set yrange [-4:4]

#set xrange [-6:-4]
#set yrange [-1:1]


plot [] [] 'Orbit.dat' every 1 u ($2):($3) t ''  w l lt 1 lw 0.5

#########################
set output 'Orbit-XZ.png'

set xlabel 'X [kpc]'
set ylabel 'Z [kpc]'

plot [] [] 'Orbit.dat' every 1 u ($2):($4) t ''  w l lt 1 lw 0.5

#########################
set output 'Orbit-YZ.png'

set xlabel 'Y [kpc]'

plot [] [] 'Orbit.dat' every 1 u ($3):($4) t ''  w l lt 1 lw 0.5


########################
###  quit
########################
