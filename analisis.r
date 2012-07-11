data=read.table('results1',header=T);
p=data[,1]
h=data[,2]
a=data[,3]
e=data[,4]
c=data[,5]

#   jpeg(paste('stats/cost', i:i, '.jpeg', sep = ""))
#   # par(mfrow=c(1,2))
#   boxplot(data[,1])
#   # boxplot(fertility)
#   dev.off()
#   jpeg(paste('stats/iterations', i:i, '.jpeg', sep = ""))
#   # par(mfrow=c(1,2))
#   boxplot(data[,2:3])
#   # boxplot(fertility)
#   dev.off()
#   jpeg(paste('stats/time', i:i, '.jpeg', sep = ""))
#   # par(mfrow=c(1,2))
#   boxplot(data[,4:5])
#   # boxplot(fertility)
#   dev.off()
# TABLA 1

optimas = c(524.61, 835.26,826.14,1028.42,1291.29,555.43,909.68,865.94,1162.55,1395.85,1042.11,819.56,1541.14,866.37)
proy = c(558,889,867,1106,1358,601,971,899,1290,1495,1043,835,1579,871)
tiem = c(0,0,1,2,11.4,0,0.6,3.4,4.8,8,1.2,0,2.2,0.8)
tabla1 = matrix(nrow=15, ncol=6)
tabla2 = matrix(nrow=15, ncol=6)
tabla1[1,] = c("A", "B", "C", "D","E","F")
tabla2[1,] = c("A", "B", "C", "D","E","F")
for (i in 1:14) {
   data=read.table(paste("stats/vrpnc" , i:i , ".stat", sep = ""))

  m=mean(data[,1])
  tabla1[i+1,1] = paste("vrpnc",i:i,sep = "")
  tabla1[i+1,2] = m
  tabla1[i+ 1,3] = (m-optimas[i])*100/optimas[i]
  tabla1[i+1,4] = sd(data[,1])
  mejor=min(data[,1])
  tabla1[i+1,5] = mejor
  tabla1[i+1,6] = length(data[data[,1]==min(data[,1]),1])
  tabla2[i+1,1] = paste("vrpnc",i:i,sep = "")
  tabla2[i+1,2] = optimas[i]
  tabla2[i+1,3] = (proy[i] - optimas[i])*100/optimas[i]
  tabla2[i+1,4] = (mejor-optimas[i])*100/optimas[i]
  tabla2[i+1,5] = tiem[i]
  tabla2[i+1,6] = mean(data[,5])
}

  write(t(tabla1),file = "stats/tabla1", ncol=6,sep = "\t")
  write(t(tabla2),file = "stats/tabla2", ncol=6,sep = "\t")

