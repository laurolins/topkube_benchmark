setwd("/Users/llins/projects/topkube_benchmark/data")
t <- read.table("topkube_benchmark_problems_stats.psv",header=T,as.is=T,sep="|")

density.all = sort(t$density)
n = length(density.all)

colors = add.alpha(c('#aaaaaa','#e41a1c','#377eb8','#4daf4a','#984ea3','#ff7f00'),0.8)

axis.info = function (ticks, lim, label) { return(list(ticks=ticks,lim=lim,label=label)) }
cum.ratio = function (x) { return((1:length(x))/length(x)) }

pretty.name = function(x) {
  return(sapply(x,function(x) {
    e = log(x,10)
    if (e <= 0) { return(sprintf("%.1f",x)) }
    else if (e < 3) { return(sprintf("%.0f",x)) }
    else if (e < 4) { return(sprintf("%.0fk",x/1e3)) }
    else if (e < 5) { return(sprintf("%.0fk",x/1e3)) }
    else if (e < 6) { return(sprintf("%.0fk",x/1e3)) }
    else if (e < 7) { return(sprintf("%.0fm",x/1e6)) }
    else if (e < 8) { return(sprintf("%.0fm",x/1e6)) }
    else if (e < 9) { return(sprintf("%.0fm",x/1e6)) }
    else if (e < 10) { return(sprintf("%.0fb",x/1e9)) }
    else if (e < 11) { return(sprintf("%.0fb",x/1e9)) }
    else if (e < 12) { return(sprintf("%.0fb",x/1e9)) }
    else { return(sprintf("%.0f",x)) }
  }))
}

## http://www.magesblog.com/2013/04/how-to-change-alpha-value-of-colours-in.html
add.alpha <- function(col, alpha=1){
  if(missing(col))
    stop("Please provide a vector of colours.")
  apply(sapply(col, col2rgb)/255, 2, 
        function(x) 
          rgb(x[1], x[2], x[3], alpha=alpha))  
}


# split.lines = function(x, classes) {
#   xx = split(x, classes)  
#   r = list()
#   for (i in 1:length(xx)) {
#     r[[ names(xx)[i] ]] = data.frame(x=xx[[i]],y=cum.ratio(xx[[i]]))
#   }
#   return(r)  
# }

plot.data = function(xinfo, yinfo, title, values, classes) {
  par(mar=c(6,6,1,1))
  plot(0,type="n",xlim=xinfo$lim,ylim=yinfo$lim,xlab=xinfo$label,ylab=yinfo$label,axes=F)
  axis(1,at=xinfo$ticks,labels=pretty.name(xinfo$ticks))
  axis(2,at=yinfo$ticks,labels=pretty.name(yinfo$ticks),las=2)
  abline(h=yinfo$ticks,lwd=2,col=gray(0.8),lty=2)
  abline(v=xinfo$ticks,lwd=2,col=gray(0.8),lty=2)
  
  lines(values, cum.ratio(values), type="l",lwd=6, col=colors[1]) # overall density vs. population
  values.by.dataset = split(values, classes)
  for (i in 1:length(values.by.dataset)) {
    x = values.by.dataset[[i]]
    lines(x, cum.ratio(x), type="l",lwd=4,col=colors[1+i],lty=1+i) # overall density vs. population
  }

  legend.names  = c("all", names(density.by.dataset))
  legend.lty    = 1:5
  legend.lwd    = c(6,4,4,4,4)
  legend(xinfo$lim[2],0,legend.names,xjust=1,yjust=0,col=colors,lty=legend.lty,lwd=legend.lwd)
  
}


make.cumulative.table = function(name,values, classes) {
  p = order(values)
  return(list(name=name, values=values[p], classes=classes[p]))
}


input.columns = c("keys","num_ranks","entries","density") 
input = lapply(input.columns,
              function(name) {
                  make.cumulative.table(name,t[[name]],t$dataset)
              })
names(input) = input.columns


plot.new()
columns = length(input)
for (col in 1:columns) {
  # left plot
  data = input[[col]]
  xinfo = axis.info(pretty(data$values,6),range(data$values),sprintf("%s",names(input)[col]))
  yinfo = axis.info(seq(0,1,0.1),c(0,1),"cumulative problem count ratio")
  par(fig=c((col-1)/columns,col/columns,0,1), new=TRUE)
  plot.data(xinfo, yinfo, data$name, data$values, data$classes)
}

#
# Density
#

permutation = order(t$density)
values  = t$density[permutation]
classes = t$dataset[permutation]

# left plot
xinfo = axis.info(seq(0,1,0.1),c(0,1),"problem density")
yinfo = axis.info(seq(0,1,0.1),c(0,1),"cumulative problem count ratio")
par(fig=c(0,0.5,0,1), new=FALSE)
plot.data(xinfo, yinfo, "TopKube Benchmark Density \n (overall and by dataset)", values, classes)

# zoomed right plot
xinfo = axis.info(seq(0,0.1,0.01),c(0,0.1),"problem density")
yinfo = axis.info(seq(0,1,0.1),c(0,1),"cumulative problem count ratio")
par(fig=c(0.5,1.0,0,1), new=TRUE)
plot.data(xinfo, yinfo, "... same plot zoomed on density range from 0 to 0.05", values, classes)


#
# Keys
#

permutation = order(t$keys)
values  = t$keys[permutation]
classes = t$dataset[permutation]

# left plot
xinfo = axis.info(pretty(values,10),range(values),"problems keys")
yinfo = axis.info(seq(0,1,0.1),c(0,1),"cumulative problem count ratio")
par(fig=c(0,0.5,0,1), new=FALSE)
plot.data(xinfo, yinfo, "TopKube Benchmark Density \n (overall and by dataset)", values, classes)

# zoomed right plot
xinfo = axis.info(pretty(values,10),range(values),"problems keys")
yinfo = axis.info(seq(0,1,0.1),c(0,1),"cumulative problem count ratio")
par(fig=c(0.5,1.0,0,1), new=TRUE)
plot.data(xinfo, yinfo, "... same plot zoomed on density range from 0 to 0.05", values, classes)


#
# Num Ranks
#

permutation = order(t$num_ranks)
values  = t$num_ranks[permutation]
classes = t$dataset[permutation]

# left plot
xinfo = axis.info(pretty(values,10),range(values),"problems keys")
yinfo = axis.info(seq(0,1,0.1),c(0,1),"cumulative problem count ratio")
par(fig=c(0,0.5,0,1), new=FALSE)
plot.data(xinfo, yinfo, "TopKube Benchmark Num Ranks \n (overall and by dataset)", values, classes)

# zoomed right plot
xinfo = axis.info(pretty(values,10),range(values),"problems keys")
yinfo = axis.info(seq(0,1,0.1),c(0,1),"cumulative problem count ratio")
par(fig=c(0.5,1.0,0,1), new=TRUE)
plot.data(xinfo, yinfo, "... same plot zoomed on density range from 0 to 0.05", values, classes)


#
# Entries
#

permutation = order(t$entries)
values  = t$entries[permutation]
classes = t$dataset[permutation]

# left plot
xinfo = axis.info(pretty(values,10),range(values),"problems keys")
yinfo = axis.info(seq(0,1,0.1),c(0,1),"cumulative problem count ratio")
par(fig=c(0,0.5,0,1), new=FALSE)
plot.data(xinfo, yinfo, "TopKube Benchmark Density \n (overall and by dataset)", values, classes)

# zoomed right plot
xinfo = axis.info(pretty(values,10),range(values),"problems keys")
yinfo = axis.info(seq(0,1,0.1),c(0,1),"cumulative problem count ratio")
par(fig=c(0.5,1.0,0,1), new=TRUE)
plot.data(xinfo, yinfo, "... same plot zoomed on density range from 0 to 0.05", values, classes)




















par(mar=c(6,6,1,1))
plot(0,type="n",xlim=xinfo$lim,ylim=yinfo$lim,xlab=xinfo$label,ylab=yinfo$label,axes=F)
axis(1,xinfo$ticks)
axis(2,yinfo$ticks,las=2)
abline(h=yinfo$ticks,lwd=2,col=gray(0.8),lty=2)
abline(v=xinfo$ticks,lwd=2,col=gray(0.8),lty=2)

lines(density.all, cum.ratio(density.all), type="l",lwd=6, col=colors[1]) # overall density vs. population
density.by.dataset = split(density.all, t$dataset)
for (i in 1:length(density.by.dataset)) {
  x = density.by.dataset[[i]]
  lines(x, cum.ratio(x), type="l",lwd=4,col=colors[1+i],lty=1+i) # overall density vs. population
}

legend.names  = c("all", names(density.by.dataset))
legend(0,0,names=legend.names)


#
# right plot
#

par(fig=c(0.5,1.0,0,1), new=TRUE)

xinfo = axis.info(seq(0,0.05,0.005),c(0,0.05),"problem density")
yinfo = axis.info(seq(0,1,0.1),c(0,1),"cumulative problem count ratio")

par(mar=c(6,6,1,1))
plot(0,type="n",xlim=xinfo$lim,ylim=yinfo$lim,xlab=xinfo$label,ylab=yinfo$label,axes=F)
axis(1,xinfo$ticks)
axis(2,yinfo$ticks,las=2)
abline(h=yinfo$ticks,lwd=2,col=gray(0.8),lty=2)
abline(v=xinfo$ticks,lwd=2,col=gray(0.8),lty=2)

lines(density.all, cum.ratio(density.all), type="l",lwd=6,col=colors[i]) # overall density vs. population
density.by.dataset = split(density.all, t$dataset)
for (i in 1:length(density.by.dataset)) {
  x = density.by.dataset[[i]]
  lines(x, cum.ratio(x), type="l",lwd=4,col=colors[1+i],lty=1+i) # overall density vs. population
}













lines(a, type="l",lwd=3, col= colors)

b = cum.ratio(a)

split(t$entries/t$keys,t$dataset))

plot(a)

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
ylim = c(-2,6)

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

