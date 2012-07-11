#!/bin/bash

echo "Borrando directorio stats/"
rm -rf stats
echo "Creando nuevo directorio stats/"
mkdir stats

echo "Corriendo casos de prueba"

i=1
let total=14*10
for numCase in 1 2 3 4 5 6 7 8 9 10 11 12 13 14
do

  for c in 1 2 3 4 5
  do
    printf "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b$i / $total"
    echo -n "$p $h $a $e " >> stats/"vrpnc$numCase.stat"
    ./cvrp "instanciasCVRP/vrpnc$numCase.txt" -t "improvement" 60 >> stats/"vrpnc$numCase.stat"
    let i+=1
  done

done