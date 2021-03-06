## http://www.magesblog.com/2013/04/how-to-change-alpha-value-of-colours-in.html
add.alpha <- function(col, alpha=1){
  if(missing(col))
    stop("Please provide a vector of colours.")
  apply(sapply(col, col2rgb)/255, 2, 
        function(x) 
          rgb(x[1], x[2], x[3], alpha=alpha))  
}

setwd("/Users/llins/projects/topkube_benchmark/data")

system("mkdir -p analysis/plots")

load("topkube_benchmark_timing_nano4_runs5_threads20_median.Rdata")

t = robust.table

n = nrow(t)


colors = add.alpha(c('#aaaaaa','#e41a1c','#377eb8','#4daf4a','#984ea3','#ff7f00'),0.8)

axis.info = function (ticks, lim, label) { return(list(ticks=ticks,lim=lim,label=label)) }
cum.ratio = function (x) { return((1:length(x))/length(x)) }

pretty.name = function(x) {
  return(sapply(x,function(x) {
    if (x == 0) return("0")
    e = log(x,10)
    if (e < -1) { return(sprintf("%.3f",x)) }
    else if (e <= 0) { return(sprintf("%.1f",x)) }
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

# split.lines = function(x, classes) {
#   xx = split(x, classes)  
#   r = list()
#   for (i in 1:length(xx)) {
#     r[[ names(xx)[i] ]] = data.frame(x=xx[[i]],y=cum.ratio(xx[[i]]))
#   }
#   return(r)  
# }

plot.data = function(xinfo, yinfo, title, values, classes) {

  values.by.dataset = split(values, classes)
  
  line.width = c(4,2.5,2.5,2.5,2.5)
  line.type  = c(1,3,2,4,5)
  
  par(mar=c(4,3,0.5,0.5))
  # yinfo$label
  plot(0, type="n", xlim=xinfo$lim, ylim=yinfo$lim, xlab="", ylab="",axes=F)
  
  # mtext(side = 2, text = xlab=xinfo$label, line = 3)
  axis(1,at=xinfo$ticks,labels=pretty.name(xinfo$ticks))
  axis(2,at=yinfo$ticks,labels=pretty.name(yinfo$ticks),las=2)
  abline(h=yinfo$ticks,lwd=1,col=gray(0.8),lty=1)
  abline(v=xinfo$ticks,lwd=1,col=gray(0.8),lty=1)

  mtext(sprintf("Cumulative distribution of %s",xinfo$label), side = 3, line = -1)
  mtext(xinfo$label, side = 1, line = 2.5)
  
  legend.names  = c("all", sapply(names(values.by.dataset),function(n) rename[[n]]))
  legend.lty    = 1:5
  legend(xinfo$lim[2],0,legend.names,xjust=1,yjust=0,col=colors,lty=line.type,lwd=line.width,box.lwd=0,box.col="white",bg="#ffffffaa")
  
  # print(title)
  # print(quantile(values,probs=seq(0,1,0.1)))
  
  lines(values, cum.ratio(values), type="l",lwd=line.width[1], lty=line.type[1], col=colors[1]) # overall density vs. population
  for (i in 1:length(values.by.dataset)) {
    x = values.by.dataset[[i]]
    lines(x, cum.ratio(x), type="l",lwd=line.width[i+1],col=colors[1+i],lty=line.type[1+i]) # overall density vs. population

    # print(names(values.by.dataset)[i])
    # print(quantile(x,probs=seq(0,1,0.1)))
  }

}


make.cumulative.table = function(name,values, classes) {
  p = order(values)
  return(list(name=name, values=values[p], classes=classes[p]))
}



render = function(input, filename, rng) {
  pdf(filename,width=4.5,height=4,pointsize=13)
  plot.new()
  columns = length(input)
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
  dev.off()
}

for (variable in c("keys","num_ranks","entries","density")) {
  input.columns = c(variable) 
  input = lapply(input.columns,
                 function(name) {
                   make.cumulative.table(name,t[[name]],t$dataset)
                 })
  names(input) = input.columns
  render(input,sprintf("analysis/plots/topkube_benchmark_%s.pdf",variable),c(0,0.8))
}

# input.columns = c("keys","num_ranks") 
# input = lapply(input.columns,
#                function(name) {
#                  make.cumulative.table(name,t[[name]],t$dataset)
#                })
# names(input) = input.columns
# render(input,"analysis/plots/topkube_benchmark_keys_num_ranks.pdf",c(0,0.8))
# 
# input.columns = c("entries","density") 
# input = lapply(input.columns,
#                function(name) {
#                  make.cumulative.table(name,t[[name]],t$dataset)
#                })
# names(input) = input.columns
# render(input,"analysis/plots/topkube_benchmark_entries_density.pdf",c(0,0.8))