setwd("/Users/llins/projects/topkube_benchmark/data")

system("mkdir -p analysis/tables")

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

# colors
colors = rep(gray(0.85),7)
names(colors) = sprintf("%d",unique(sort(t$threshold)))
colors[['5']]   = gray(0)
colors[['10']]  = gray(0.1)
colors[['20']]  = gray(0.18)
colors[['40']]  = gray(0.32)
colors[['80']]  = gray(0.50)
colors[['160']] = gray(0.60)
colors[['320']] = gray(0.75)
colors = add.alpha(colors, 0.7)

# lwd
lwd = rep(1.5,7)
names(lwd) = sprintf("%d",unique(sort(t$threshold)))
lwd[['5']] = 1.0
lwd[['10']] = 1.5
lwd[['20']] = 2.0
lwd[['40']] = 2.5
lwd[['80']] = 3.0
lwd[['160']] = 3.5
lwd[['320']] = 4.0

# lty
lty = rep(1,7)
names(lty) = sprintf("%d",unique(sort(t$threshold)))
lty[['5']]   = 1
lty[['10']]  = 2
lty[['20']]  = 3
lty[['40']]  = 4
lty[['80']]  = 5
lty[['160']] = 1
lty[['320']] = 2

axis.info = function (ticks, lim, label) { return(list(ticks=ticks,lim=lim,label=label)) }
cum.ratio = function (x) { return((1:length(x))/length(x)) }

pretty.name = function(x) {
  return(sapply(x,function(x) {
    return(sprintf("%.1f",x))
    #    if (x < 0) { return(sprintf("%.1f",x)) }
    #    if (x == 0) return("0")
    #    e = log(x,10)
    #    if (e < -1) { return(sprintf("%.3f",x)) }
    #    else if (e <= 0) { return(sprintf("%.1f",x)) }
    #    else if (e < 3) { return(sprintf("%.0f",x)) }
    #    else if (e < 4) { return(sprintf("%.0fk",x/1e3)) }
    #    else if (e < 5) { return(sprintf("%.0fk",x/1e3)) }
    #    else if (e < 6) { return(sprintf("%.0fk",x/1e3)) }
    #    else if (e < 7) { return(sprintf("%.0fm",x/1e6)) }
    #    else if (e < 8) { return(sprintf("%.0fm",x/1e6)) }
    #    else if (e < 9) { return(sprintf("%.0fm",x/1e6)) }
    #    else if (e < 10) { return(sprintf("%.0fb",x/1e9)) }
    #    else if (e < 11) { return(sprintf("%.0fb",x/1e9)) }
    #    else if (e < 12) { return(sprintf("%.0fb",x/1e9)) }
    #    else { return(sprintf("%.0f",x)) }
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


plot.data = function(xinfo, yinfo, title, values, classes, options) {
  
  values.by.dataset = split(values, classes)
  
  # line.width = c(4,2.5,2.5,2.5,2.5)
  # line.type  = c(1,3,2,4,5)
  
  par(mar=c(6,3,0.5,0.5))
  # yinfo$label
  plot(0,type="n",xlim=xinfo$lim,ylim=yinfo$lim,xlab=xinfo$label,ylab="",axes=F)
  axis(1,at=xinfo$ticks,labels=pretty.name(xinfo$ticks))
  axis(2,at=yinfo$ticks,labels=pretty.name(yinfo$ticks),las=2)
  abline(h=yinfo$ticks,lwd=1,col=gray(0.8),lty=1)
  abline(v=xinfo$ticks,lwd=1,col=gray(0.8),lty=1)
  
  
  legend.names  = sprintf("%d",unique(sort(classes)))
  print(legend.names)
  
  n = length(legend.names)
  
  legend.lty    = if(options$use_lty) rev(1:length(legend.names)) else rep(1,n)
  
  legend(xinfo$lim[2],0,
         legend.names,
         xjust=1,
         yjust=0,
         col=sapply(legend.names,function(n) colors[[n]]),
         lty=sapply(legend.names,function(n) lty[[n]]),
         lwd=sapply(legend.names,function(n) lwd[[n]]),
         box.lwd=0,
         box.col="white",
         bg="#ffffffaa")
  
  # print(title)
  # print(quantile(values,probs=seq(0,1,0.1)))
  
  # lines(values, cum.ratio(values), type="l",lwd=line.width[1], lty=line.type[1], col=colors[1]) # overall density vs. population
  
  indices = 1:length(values.by.dataset)
  if (options$reverse_lines) indices = rev(indices)
  for (i in indices) {
    x = values.by.dataset[[i]]
    lines(x, cum.ratio(x), 
          type="l",
          col=colors[[legend.names[i]]],
          lwd=lwd[[legend.names[i]]],
          lty=lty[[legend.names[i]]]) # overall density vs. population
    
    # print(names(values.by.dataset)[i])
    # print(quantile(x,probs=seq(0,1,0.1)))
  }
  
}



render = function(filename, input, rng, options) {
  pdf(filename,width=5,height=5,pointsize=11)
  plot.new()
  for (col in 1:length(input)) {
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
    plot.data(xinfo, yinfo, data$name, data$values, data$classes, options)
  }
  dev.off()
}

options = list(reverse_lines=T, use_lty=T)
sel = t$threshold==0.25
input = list(speedup=make.cumulative.table("speedup",log(t$speedup[sel],10),t$k[sel]))
render("/tmp/speedup_025_by_k.pdf",input,c(0,1), options)

# 
# options = list(reverse_lines=T, use_lty=F, lwd_a=2, lwd_b=4)
# sel = t$threshold!=1.0 & t$threshold %in% seq(0.25,0.95,0.05)
# # print(100*sum(sel)/sum(t$threshold==0.50))
# input = list(speedup=make.cumulative.table("speedup",log(t$speedup[sel],10),t$threshold[sel]))
# render("/tmp/speedup_025_to095.pdf",input,c(0,1), options)
