#!/bin/bash
#By: Ig.Eggman & A cool banana (https://github.com/JorgeGonzalezPascual & https://github.com/Zygmut)
echo -e $'\n'"\e[91m-- Script Niveles 12 y 13 --\e[0m" $'\n'
make clean
make

#Creamos dispositivo
./mi_mkfs disco 100000

#Sacamos la fecha
current_date=`date +%Y%m%d%H%M%S` #yearmonthdayHourMinuteSecond
#Simulamos
echo  -e $'\n'"\e[94m-- Start simulaci贸n -- \e[0m"
echo -e  "\e[32mtime ./simulacion disco\e[0m"
time ./simulacion disco 


make clean