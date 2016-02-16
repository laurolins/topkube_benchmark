setwd("/Users/llins/projects/topkube_benchmark/data")

system("mkdir -p analysis/plots")

load("topkube_benchmark_timing_nano4_runs5_threads20_median.Rdata")

t = robust.table

n = nrow(t)

# colors
colors = rep(gray(0.85),22)
names(colors) = sprintf("%.3f",unique(sort(t$threshold)))
colors[['0.100']] = gray(0.75)
colors[['0.150']] = gray(0.55)
colors[['0.200']] = gray(0.35)
colors[['0.250']] = gray(0.0)
colors[['0.300']] = gray(0.35)
colors[['0.350']] = gray(0.55)
colors[['0.400']] = gray(0.75)
colors = add.alpha(colors, 0.9)

# lwd
lwd = rep(1.5,22)
names(lwd) = sprintf("%.3f",unique(sort(t$threshold)))
lwd[['0.100']] = 2.0
lwd[['0.150']] = 2.5
lwd[['0.200']] = 3.0
lwd[['0.250']] = 1.5
lwd[['0.300']] = 3.0
lwd[['0.350']] = 2.5
lwd[['0.400']] = 2.0

# lty
lty = rep(1,22)
names(lty) = sprintf("%.3f",unique(sort(t$threshold)))
lty[['0.000']] = 1
lty[['0.050']] = 5
lty[['0.100']] = 4
lty[['0.150']] = 3
lty[['0.200']] = 2
lty[['0.250']] = 1
lty[['0.300']] = 2
lty[['0.350']] = 3
lty[['0.400']] = 4


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

pretty.e10.name = function(x) {
  return(sapply(x,function(x) {
    y = 10^x
    if (y < 0.1) {
      return(sprintf("%.2fx",y))
    }
    if (y < 1) {
      return(sprintf("%.1fx",y))
    }
    else {
      return(sprintf("%.0fx",y))
    }
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

convert_threshold_name = function(x) {
  x = as.numeric(x)
  if (x == 0) {
    "TA"
  }
  else {
    sprintf("%.2f",x)
  }
}

plot.data = function(xinfo, yinfo, title, values, classes, options) {
  
  values.by.dataset = split(values, classes)

  # line.width = c(4,2.5,2.5,2.5,2.5)
  # line.type  = c(1,3,2,4,5)
  
  par(mar=c(4.5,3,0.5,0.5))
  # yinfo$label
  plot(0,type="n",xlim=xinfo$lim,ylim=yinfo$lim,xlab="",ylab="",axes=F)
  axis(1,at=xinfo$ticks,labels=pretty.e10.name(xinfo$ticks))
  axis(2,at=yinfo$ticks,labels=pretty.name(yinfo$ticks),las=2)
  abline(h=yinfo$ticks,lwd=1,col=gray(0.8),lty=1)
  abline(v=xinfo$ticks,lwd=1,col=gray(0.8),lty=1)
  
  
  mtext(xinfo$label,side=1,line=2.5)
  
  legend.names  = sprintf("%.3f",unique(sort(classes)))
  print(legend.names)
  
  n = length(legend.names)
  
  legend.lty    = if(options$use_lty) rev(1:length(legend.names)) else rep(1,n)

  legend(xinfo$lim[2],0,
         sapply(legend.names, convert_threshold_name),
         xjust=1,
         yjust=0,
         col=sapply(legend.names,function(n) colors[[n]]),
         lty=sapply(legend.names,function(n) lty[[n]]),
         lwd=sapply(legend.names,function(n) lwd[[n]]),
         box.lwd=0,
         box.col="white",
         bg="#ffffff")

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
columns = 1
render = function(filename, input, rng, options) {
  pdf(filename,width=6.3,height=5,pointsize=13)
  plot.new()
  for (col in 1:length(input)) {
    # left plot
    data = input[[col]]
    v = data$values;
    n = length(v)
    # find value where
    a = -2 # v[1 + as.integer((n-1)*rng[1])]
    b =  4.2 # v[1 + as.integer((n-1)*rng[2])]
    # xinfo = axis.info(c("0.1x","1x","10x","100x","1000x","10000x"),
    # print(sprintf("%s",names(input)[col]))
    xinfo = axis.info(pretty(c(a,b),7),
                      c(a,b),
                      sprintf("%s",names(input)[col]))
    yinfo = axis.info(seq(0,1,0.1),c(0,1),"cumulative problem count ratio")
    par(fig=c((col-1)/columns,col/columns,0,1), new=TRUE)
    plot.data(xinfo, yinfo, data$name, data$values, data$classes, options)
  }
  dev.off()
}

make.cumulative.table = function(name,values, classes) {
  p = order(values)
  return(list(name=name, values=values[p], classes=classes[p]))
}

options = list(reverse_lines=F, use_lty=T, lwd_a=2, lwd_b=4)
sel = t$threshold!=1.0 & t$threshold %in% c(0,0.05,0.1,0.15,0.2,0.25)
# print(100*sum(sel)/sum(t$threshold==0.50))
input = list(speedup=make.cumulative.table("speedup",log(t$speedup[sel],10),t$threshold[sel]))
render("analysis/plots/plot_speedup_0_to_025_by_005.pdf",input,c(0,1), options)

options = list(reverse_lines=T, use_lty=F, lwd_a=2, lwd_b=4)
sel = t$threshold!=1.0 & t$threshold %in% seq(0.25,0.95,0.05)
# print(100*sum(sel)/sum(t$threshold==0.50))
input = list(speedup=make.cumulative.table("speedup",log(t$speedup[sel],10),t$threshold[sel]))
render("analysis/plots/plot_speedup_025_to_095_by_005.pdf",input,c(0,1), options)
