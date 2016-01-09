setwd("/Users/llins/projects/topkube_benchmark/data")
t <- read.table("topkube_benchmark_timing.psv",header=T,as.is=T,sep="|")


sel = t$algorithm == "sweep"
a = data.frame(problem=t$problem[sel],k=t$k[sel],time=t$time[sel])
speedup = sapply(1:nrow(t),function(i) {
  index = min(which(a$problem==t$problem[i] & a$k==t$k[i]))
  sweep_time = a$time[index]
  #print(sprintf("i: %d sweep_time: %f time: %f speedup: %f",i,sweep_time,t$time[i],sweep_time/t$time[i]))
  return(sweep_time/t$time[i])
})

t = data.frame(t,speedup=speedup)

# will call speedup the ratio  sweep time / time

density.all = sort(t$density)
n = length(density.all)

colors = add.alpha(c('#aaaaaa','#e41a1c','#377eb8','#4daf4a','#984ea3','#ff7f00'),0.8)

axis.info = function (ticks, lim, label) { return(list(ticks=ticks,lim=lim,label=label)) }
cum.ratio = function (x) { return((1:length(x))/length(x)) }

pretty.name = function(x) {
  return(sapply(x,function(x) {
    if (x == 0) return("0")
    e = log(abs(x),10)
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

  legend.names  = c("all", names(values.by.dataset))
  legend.lty    = 1:5
  legend.lwd    = c(6,4,4,4,4)
  legend(xinfo$lim[2],0,legend.names,xjust=1,yjust=0,col=colors,lty=legend.lty,lwd=legend.lwd)
  
}


make.cumulative.table = function(name,values, classes) {
  p = order(values)
  return(list(name=name, values=values[p], classes=classes[p]))
}


sel = t$threshold==0.20
input = list(speedup = make.cumulative.table("speedup",log(t$speedup[sel],10),t$dataset[sel]))

plot.new()
columns = length(input)

rng = c(0,1.0)

for (col in 1:columns) {
  # left plot
  data = input[[col]]
  v = data$values;
  n = length(v)
  # find value where
  a = v[1 + as.integer((n-1)*rng[1])]
  b = v[1 + as.integer((n-1)*rng[2])]
  xinfo = axis.info(pretty(c(a,b),6),c(a,b),sprintf("%s",names(input)[col]))
  yinfo = axis.info(seq(0,1,0.1),c(0,1),"cumulative problem count ratio")
  par(fig=c((col-1)/columns,col/columns,0,1), new=TRUE)
  plot.data(xinfo, yinfo, data$name, data$values, data$classes)
}

