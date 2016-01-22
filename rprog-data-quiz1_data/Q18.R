good<-complete.cases(data[1],data[4])
d<-data[good,]
c<-d[d[1]>31 & d[4]>90,]
mean(c[1])