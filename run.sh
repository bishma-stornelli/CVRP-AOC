#!/bin/bash

echo "Borrando directorio stats/"
rm -rf stats
echo "Creando nuevo directorio stats/"
mkdir stats

echo "Corriendo casos de prueba"

i=1
let total=14*3*3*3*3*2
for numCase in 1 2 3 4 5 6 7 8 9 10 11 12 13 14
do
  for p in 1 3 5
  do
    for h in 1 3 5
    do
      for a in 20 50 80
      do
        for e in 0.2 0.5 0.8
        do
          for c in 1 2
          do
            printf "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b$i / $total"
            echo -n "$p $h $a $e " >> stats/"vrpnc$numCase.stat"
            ./cvrp "instanciasCVRP/vrpnc$numCase.txt" -a $a -e $e -h $h -p $p -t "improvement" 30 >> stats/"vrpnc$numCase.stat"
            let i+=1
          done
        done
      done
    done
  done
done