#
#
#

setwd("/Users/llins/projects/topkube_benchmark/data")
t <- read.table("topkube_benchmark_timing.psv",header=T,as.is=T,sep="|")

t <- data.frame(t, 
                approx_density=(t$entries - t$sweep_entries + t$sweep_largest_rank) / ((t$num_ranks - (t$sweep_num_ranks - 1)) * t$keys), 
                cost=t$ta_total_access + t$sweep_entries)

colors = c('#fee5d9aa','#fcbba1aa','#fc9272aa','#fb6a4aaa','#ef3b2caa','#cb181daa','#99000daa')


# pdf("/tmp/k40+.pdf",width=12,height=6,pointsize=7)
# tt = t
tt = t[t$k >  20,]
y = split(log(tt$time*1e9,10),list(threshold=tt$threshold,tt$dataset),sep="  ")
par(mar=c(10,3,1,1))
boxplot(y,col=colors,las=2,cex=0.2,cex.axis=1)
abline(h=3:10,col=gray(0.7),lty=2)
# dev.off()

length(unique(t$problem))

#
# Fancy boxplot split by dataset 
#
setwd("/Users/llins/projects/topkube_benchmark/data")
t <- read.table("topkube_benchmark_problems_stats.psv",header=T,as.is=T,sep="|")
boxplot(split(t$entries/t$keys,t$dataset))
boxplot(split(log(t$keys/t$num_ranks,10),t$dataset))

# global axis limits
xlim = c(-3.25,0.0)
ylim = c(-2,5)

#
# Understand the basics: fix k and see how the length of the "proof of top k" grows.
#

ax.info = function (ticks, lim, label) {
  return(list(ticks=ticks,lim=lim,label=label))
}

sel = t$threshold==0.0 & t$entries > 1e6
sum(sel)
p = t[sel,]
num.blocks = 7
intervals = findInterval(p$k,sort(unique(p$k)),all.inside=T)

x = log(p$density,10)
y = log(p$entries/p$ta_total_access,10) # how many times smaller
xinfo = ax.info(seq(-3,0,0.5), xlim, "density (log_10)")
yinfo = ax.info(seq(-2,5,1),   ylim, "speedup (log_10)")

par(mfrow=c(1,1))

margin = 0.2
size   = 0.96

ymar = data.frame(value=sort(y),population.ratio=1:length(y)/length(y))
xmar = data.frame(value=sort(x),population.ratio=1:length(x)/length(x))

par(fig=c(0,size-margin,0,size-margin), new=FALSE)
par(mar=c(6,5,0,0))
plot(0,type="n",xlim=xinfo$lim,ylim=yinfo$lim,xlab=xinfo$label,ylab=yinfo$label)
abline(h=yinfo$ticks,lwd=2,col=gray(0.8),lty=2)
abline(v=xinfo$ticks,lwd=2,col=gray(0.8),lty=2)
points(x,y,col=colors[intervals],cex=0.9,pch=16)
abline(h=0,lwd=3,col=rgb(0,0,0,0.8))

par(fig=c(0.0,size-margin,size-margin,size), new=TRUE)
par(mar=c(0,5,0,0))
plot(0, type="n", xlim=xinfo$lim, ylim=c(0,1), xlab="", ylab="", axes=F)
abline(h=c(0.25,0.5,0.75),col=gray(0.5),lty=2)
abline(v=xinfo$ticks,lwd=2,col=gray(0.8),lty=2)
lines(xmar$value, xmar$population.ratio, lwd=3,col='#0000ffaa')
# axis(2,seq(0,1,0.5),las=2)
box()

par(fig=c(size-margin,size,0.0,size-margin), new=TRUE)
par(mar=c(6,0,0,0))
plot(0, type="n", xlim=c(0,1), ylim=yinfo$lim, xlab="", ylab="", axes=F)
abline(v=c(0.25,0.5,0.75),col=gray(0.5),lty=2)
abline(h=yinfo$ticks,lwd=2,col=gray(0.8),lty=2)
lines(ymar$population.ratio, ymar$value,lwd=3,col='#0000ffaa')
# axis(1,seq(0,1,0.5))
box()


#
# Measure the effect of collapsing problems with the hybrid approach
# and its cost ratio
#
#

# sort(unique(t$threshold))

ax.info = function (ticks, lim, label) {
  return(list(ticks=ticks,lim=lim,label=label))
}


for (th in sort(unique(t$threshold))) {

  sel = t$threshold==th & !is.na(t$approx_density) & !is.na(t$cost) & t$num_ranks>1
  
  # t$entries > 1e6 & 
  
  if (sum(sel) == 0)
    next

  p = t[sel,]
  
  print(th)  
  pdf(sprintf("all-plot-%f.pdf",th),width=7,height=7,pointsize=12)
  
  num.blocks = 7
  intervals = findInterval(p$k,sort(unique(p$k)),all.inside=T)
  
  x = log(p$approx_density,10)
  y = log(p$entries/p$cost,10) # how many times smaller
  xinfo = ax.info(seq(-3,0,0.5), xlim, "density (log_10)")
  yinfo = ax.info(seq(-2,5,1),   ylim, "speedup (log_10)")
  
  par(mfrow=c(1,1))
  
  margin = 0.2
  size   = 0.96
  
  ymar = data.frame(value=sort(y),population.ratio=1:length(y)/length(y))
  xmar = data.frame(value=sort(x),population.ratio=1:length(x)/length(x))
  
  par(fig=c(0,size-margin,0,size-margin), new=FALSE)
  par(mar=c(6,5,0,0))
  plot(0,type="n",xlim=xinfo$lim,ylim=yinfo$lim,xlab=xinfo$label,ylab=yinfo$label)
  abline(h=yinfo$ticks,lwd=2,col=gray(0.8),lty=2)
  abline(v=xinfo$ticks,lwd=2,col=gray(0.8),lty=2)
  points(x,y,col=colors[intervals],cex=0.9,pch=16)
  abline(h=0,lwd=3,col=rgb(0,0,0,0.8))
  
  par(fig=c(0.0,size-margin,size-margin,size), new=TRUE)
  par(mar=c(0,5,0,0))
  plot(0, type="n", xlim=xinfo$lim, ylim=c(0,1), xlab="", ylab="", axes=F)
  abline(h=c(0.25,0.5,0.75),col=gray(0.5),lty=2)
  abline(v=xinfo$ticks,lwd=2,col=gray(0.8),lty=2)
  lines(xmar$value, xmar$population.ratio, lwd=3,col='#0000ffaa')
  # axis(2,seq(0,1,0.5),las=2)
  box()
  
  par(fig=c(size-margin,size,0.0,size-margin), new=TRUE)
  par(mar=c(6,0,0,0))
  plot(0, type="n", xlim=c(0,1), ylim=yinfo$lim, xlab="", ylab="", axes=F)
  abline(v=c(0.25,0.5,0.75),col=gray(0.5),lty=2)
  abline(h=yinfo$ticks,lwd=2,col=gray(0.8),lty=2)
  lines(ymar$population.ratio, ymar$value,lwd=3,col='#0000ffaa')
  # axis(1,seq(0,1,0.5))
  box()
  
  dev.off()
}














min(which(t$threshold == 0.25))


x = t[43,]









f = data.frame(m=log(p$density,10),t=log(p$ta_total_access/p$entries,10) )
model = lm(t ~ m , data=f)



# plot(xmar$value, xmar$population.ratio, type="l",axes=F,xlab="",ylab="",lwd=3,col=4)
# plot(xmar$value, xmar$population.ratio, type="l",axes=F,xlab="",ylab="",lwd=3,col=4)


#
# Understand the basics: fix k and see how the length of the "proof of top k" grows.
#

sel = t$threshold==0.0 & t$k==40 & t$num_ranks < 40
#& t$entries > 1e6
sum(sel)
p = t[sel,]
num.blocks = 7
breaks = quantile(p$density,seq(0,1.0,1.0/num.blocks))
intervals = findInterval(p$density,breaks,all.inside=T)
plot(log(p$density,10),log(p$ta_total_access/p$entries,10),col=colors[intervals],cex=0.7,pch=16)
abline(h=0,lwd=2,col=2)
f = data.frame(m=log(p$density,10),t=log(p$ta_total_access/p$entries,10) )
model = lm(t ~ m , data=f)


sapply(t$k,function(i) grep(i,t$k))

match(t$k, sort(unique(t$k)))


















#
# Unsorted
#

k = 20
rng = seq(1,250,1)
nanosec=t$time * 1e9
lognanosec=log(nanosec,10);
selection = t$k==k & t$problem>=min(rng) & t$problem<=max(rng) & t$keys>10000
xlim=c(0,1)
ylim=range(lognanosec[selection])

xticks = seq(0,1.01,0.05)
yticks = seq(0,9,0.5)
plot(0,type="n",xlab="threshold",ylab="time (seconds)",xlim=xlim,ylim=ylim)
abline(v=xticks,col=gray(0.8),lty=2)
abline(h=yticks,col=gray(0.8),lty=2)

for (problem in rng) {
  sel = selection & (t$problem == problem)
  x = t$threshold[sel]
  y = lognanosec[sel]
  p = order(x)
  lines(x[p],y[p],type="l",col=rgb(0,0,0,0.5),lwd=2)
  text(1,tail(y[p],1),label=problem,cex=0.6,adj=c(-0.1,0))
}


x = split(log(t$time*1e9,10),list(threshold=t$threshold,k=t$k))
boxplot(x,col=colors)

tt = t[t$keys>100000,]
y = split(log(tt$time*1e9,10),list(k=tt$k,threshold=tt$threshold),sep="  ")
boxplot(y,col=colors,las=2,cex=0.5,cex.axis=0.5)

