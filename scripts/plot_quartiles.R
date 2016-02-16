setwd("/Users/llins/projects/topkube_benchmark/data")
setwd("/Users/llins/projects/topkube_benchmark/data")

system("mkdir -p analysis/plots")

load("topkube_benchmark_timing_nano4_runs5_threads20_median.Rdata")

t = robust.table

n = nrow(t)

cum.ratio = function (x) { return((1:length(x))/length(x)) }

make.cumulative.table = function(name,values, classes) {
  p = order(values)
  return(list(name=name, values=values[p], classes=classes[p]))
}


render_quartiles = function(criteria, criteria_name) {
  
  # check the different quantiles for "keys"
  problems = unique(data.frame(problem=t$problem,x=t[,criteria]))
  breaks = quantile(problems$x, probs=seq(0,1.0,0.25))
  groups = cut(problems$x, breaks, labels=FALSE, include.lowest=TRUE)
  problems = data.frame(problems, group=groups)
  
  # aggregate(rep(1,1000),by=list(group=groups),sum)
  
  tables = list()
  for (group in unique(sort(problems$group))) {
  
    problem_names = sort(problems$problem[problems$group==group])
    
    sel = t$threshold!=1.0 & t$problem %in% problem_names
    # input = list(speedup=make.cumulative.table("speedup",log(t$speedup[sel],10),t$threshold[sel]))
    input = list(speedup=make.cumulative.table("speedup",t$speedup[sel],t$threshold[sel]))
    
    result = lapply(1:length(input), function(i) {
      variable        = input[[i]]$name
      #print(variable)
      value.groups = split(input[[i]]$values, input[[i]]$classes)
      # print(names(value.groups))
    
      # create a quantile matrix ordered for each value group (in row order of value.groups)
      quantile.matrix = do.call(rbind, lapply(names(value.groups), function(i){ quantile(value.groups[[i]],probs=seq(0,1,0.01)) }))
      rownames(quantile.matrix) = 1:nrow(quantile.matrix)
      colnames(quantile.matrix) = sapply(0:100,function(i) sprintf("p%d",i))
      
      # create the variable data frame
      variable.df = data.frame(variable=rep(variable,nrow(quantile.matrix)),
                               dataset=names(value.groups), 
                               data.frame(quantile.matrix))
      
      # rename rows to numbers
      rownames(variable.df) = 1:nrow(variable.df)
      variable.df
      })
    # names(result) = input.columns
    
    table = do.call(rbind.data.frame,result)
    rownames(table) = 1:nrow(table)
    
    tables[[group]] = table
  }
    
  
  
  pretty.name = function(x) {
    return(sapply(x,function(x) {
      return(sprintf("%.1f",x))
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
  
  
  
  pretty.name = function(x) {
    return(sapply(x,function(x) {
      return(sprintf("%.1f",x))
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
  
  
  # colors
  colors = rep(gray(0.85),4)
  colors[1] = gray(0.75)
  colors[2] = gray(0.55)
  colors[3] = gray(0.35)
  colors[4] = gray(0.0)
  
  # lwd
  lwd = rep(1.5,4)
  lwd[1] = 3.0
  lwd[2] = 3.5
  lwd[3] = 4.0
  lwd[4] = 5.0
  
  # lty
  lty = rep(1,4)
  lty[[1]] = 1
  lty[[2]] = 5
  lty[[3]] = 4
  lty[[4]] = 3
  
  pdf(sprintf("analysis/plots/plot_speedup_by_%s_quartiles.pdf",criteria),
      width=8,
      height=5.5,
      pointsize=13)
  
  par(mar=c(4,3,1.5,0.5))
  xticks = seq(-2,5,1)
  yticks = 0:10/10
  plot(0,type="n",xlim=c(-2.1,5.1),ylim=c(0,1),xlab="",ylab="",axes=F)
  abline(v=xticks,col=gray(0.85))
  abline(h=yticks,col=gray(0.85))
  axis(1,xticks,labels=pretty.e10.name(xticks))
  axis(2,yticks,labels=pretty.name(yticks),las=2)
  for (i in 1:length(tables)) {
    ti = tables[[i]]
    x = as.numeric(ti[6,3:ncol(ti)])
    lines(log(x,10), 0:100/100, type="l",lwd=lwd[i],lty=lty[i],col=colors[i])
  }
  mtext(sprintf("Cumulative speedup of Hybrid 0.25 by #%s quartiles",criteria_name), 
        side=3, 
        line=0)
  mtext("speedup (log scale)", side=1, line=2.5)
  
  op <- par(family="mono")
  legend.names = sprintf("(%d) |%s| in [%.2E,%.2E%s", 
                         1:4, 
                         rep(criteria_name,4),
                         head(breaks,-1), 
                         tail(breaks,-1), 
                         c(rep(")",3),"]"))
  legend(5,0,
         cex=0.9,
         legend.names,
         xjust=1,
         yjust=0,
         col=colors,
         lty=lty,
         lwd=lwd,
         box.lwd=0,
         box.col="white",
         bg="#ffffffaa")
  par(op)
  
  dev.off()
  
}

criteria = c("keys","num_ranks","entries")

render_quartiles("keys","keys")
render_quartiles("num_ranks","ranks")
render_quartiles("entries","entries")




 
#
#   # levels = c(1.05,1.25,1.50,2.0,5,10,10000000)
#   levels = c(1.1,1.5,2.0,5.0,10.0,100.0,10000000)
#   breaks = c(1/rev(levels),levels)
#   length(breaks)
#   
#   A = as.matrix(table[,3:ncol(table)])
#   m = nrow(A)
#   n = ncol(A)
#   x = matrix(cut(A,breaks=breaks,labels=F),nrow=m)
#   
#   #
#   # assuming two column data frame with cx and cy
#   # return coordinates data frame
#   #
#   cells.to.coords <- function(cells) { data.frame(x0=cells[,1]-1,y0=cells[,2]-1,x1=cells[,1],y1=cells[,2]) }
#   
#   # blues  = c('#f7fbff','#deebf7','#c6dbef','#9ecae1','#6baed6','#4292c6','#2171b5','#08519c','#08306b')
#   # reds   = c('#fff5f0','#fee0d2','#fcbba1','#fc9272','#fb6a4a','#ef3b2c','#cb181d','#a50f15','#67000d')
#   
#   fwhite = function(x,lambda) {
#     x = (lambda * col2rgb(x) + (1-lambda) * 255)/255.0
#     rgb(x['red',],x['green',],x['blue',])
#   }
#   
#   blues  = c('#f7fbff','#deebf7','#c6dbef','#9ecae1','#6baed6','#4292c6','#2171b5','#08519c','#08306b')
#   reds   = c('#fff5f0','#fee0d2','#fcbba1','#fc9272','#fb6a4a','#ef3b2c','#cb181d','#a50f15','#67000d')
#   white  = c('#ffffff')
#   colors = c(rev(reds[2:7]),white,blues[2:7])
#   colors = fwhite(colors,0.7)
#   
#   coords = cells.to.coords(merge(1:n,1:m))
#   valtext   = sapply(1:nrow(coords),function(i) sprintf("%.2E",A[m+1-coords$y1[i],coords$x1[i]]))
#   valcolors = sapply(1:nrow(coords),function(i) colors[x[m+1-coords$y1[i],coords$x1[i]]])
#   
#   # col header
#   col.header.coords = cells.to.coords(merge(1:n,m+1))
#   col.header.text   = sapply( seq(0,100,10), function(x) sprintf("%d%%",x) )
#   
#   # row header
#   row.header.coords = cells.to.coords(merge(0,1:(m+1)))
#   row.header.text   = c("TA",sapply( seq(0.05,0.95,0.05), function(x) sprintf("%.2f",x) ))
#   row.header.text   = c(rev(row.header.text),expression(paste(theta," \\ %"))) # expression(theta percentile))
#   
#   # plot rectangles
#   plot.rectangles = function(coords, colors, texts) {
#     rect(coords$x0, coords$y0, coords$x1, coords$y1, col=colors, border=NA)
#     text((coords$x0+coords$x1)/2, (coords$y0+coords$y1)/2, texts)
#   }
#   
#   legend = function(xlim, ylim, colors, labels) {
#     ncol = length(colors)
#     dx = xlim[2] - xlim[1]
#     dy = ylim[2] - ylim[1]
#     w = dx/ncol
#     x0 = xlim[1] + dx/ncol * (0:(ncol-1))
#     # print(x0)
#     coords = data.frame(x0=x0,y0=rep(ylim[1],ncol),x1=x0+dx/ncol,y1=rep(ylim[2],ncol))
#     print(coords)
#     rect(coords$x0,coords$y0,coords$x1,coords$y1, col=colors, border = "white")
#     text(coords$x0 + w/2,(ylim[1]+ylim[2])/2,labels,adj=c(0.5,0.5),cex=0.9)
#   }
#   
#   pdf(sprintf("analysis/plots/speedup_group_%d.pdf",group),width=8,height=6.5,pointsize=9,family="Helvetica")
#   par(mar=rep(0,4))
#   plot(0,type="n",xlim=c(-1,n),ylim=c(-1,m+1),axes=F,xlab="",ylab="",xaxs = "i", yaxs = "i")
#   plot.rectangles(coords, valcolors, valtext)
#   plot.rectangles(row.header.coords, gray(0.8), row.header.text)
#   plot.rectangles(col.header.coords, gray(0.8), col.header.text)
#   abline(h=seq(0,m+1,1),col=gray(1),lwd=1)
#   abline(v=seq(0,n+1,1),col=gray(1),lwd=1)
#   legend(c(-1,11),c(-1,-0.3),tail(colors,-1),
#          c("1/100x <",
#            "1/10x < ",
#            "1/5x < ",
#            "1/2x < ",
#            "1/1.5x < ",
#            "1/1.1x <", 
#            "1.1x <", 
#            "1.5x <",
#            "2x <",
#            "5x <",
#            "10x <",
#            "100x"))
#          # sprintf("%.2E",breaks)) # ,function(s) sprintf("%s <",fancy_seconds(s))))
#   
#   dev.off()
# }
# 
# # number of problems
# length(unique(t$problem))
#