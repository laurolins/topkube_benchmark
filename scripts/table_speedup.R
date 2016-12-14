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

pretty.row.name = function(x) {
  x = as.numeric(x)
  print(x)
  if (x == 0.0) { "TA" } 
  else if (x < 1.0) { sprintf("%.2f", x) } 
  else { sprintf("%d",x) }
}

#
# expected:
#
#    input list of variables with two column datatrames
#        values
#        classes
#    options:
#        filename
#        width
#        height
#
render_tableimage = function(input, options) {
  
  result = lapply(1:length(input), function(i) {
    variable        = input[[i]]$name
    #print(variable)
    value.groups = split(input[[i]]$values, input[[i]]$classes)
    # print(names(value.groups))

    # create a quantile matrix ordered for each value group (in row order of value.groups)
    quantile.matrix = do.call(rbind, lapply(names(value.groups), function(i){ quantile(value.groups[[i]],probs=seq(0.1,1,0.1)) }))
    rownames(quantile.matrix) = names(value.groups)
    colnames(quantile.matrix) = sapply(1:10,function(i) sprintf("p%d",10*i))
  
    # create the variable data frame
    variable.df = data.frame(variable=rep(variable,nrow(quantile.matrix)),
                             dataset=names(value.groups), 
                             data.frame(quantile.matrix))
  
    # rename rows to numbers
    rownames(variable.df) = names(value.groups) # 1:nrow(variable.df)
    variable.df
  })
  # names(result) = input.columns

  table = do.call(rbind.data.frame,result)
  # rownames(table) = 1:nrow(table)
  # print(table)

  # levels = c(1.05,1.25,1.50,2.0,5,10,10000000)
  levels = c(1.1,1.5,2.0,5.0,10.0,100.0,10000000)
  breaks = c(1/rev(levels),levels)
  length(breaks)

  A = as.matrix(table[,3:ncol(table)])
  m = nrow(A)
  n = ncol(A)
  x = matrix(cut(A,breaks=breaks,labels=F),nrow=m)
  rownames(x) = sapply(rownames(table),pretty.row.name)
  
  # print(x)

  #
  # assuming two column data frame with cx and cy
  # return coordinates data frame
  #
  cells.to.coords <- function(cells) { data.frame(x0=cells[,1]-1,y0=cells[,2]-1,x1=cells[,1],y1=cells[,2]) }

  fwhite = function(x,lambda) {
    x = (lambda * col2rgb(x) + (1-lambda) * 255)/255.0
    rgb(x['red',],x['green',],x['blue',])
  }
  
  fscale = function(colors, lambda, decay=1, pow=1) {
    lambdas = lambda * decay^(0:(length(colors)-1))
    f = function(x) {
      m   = col2rgb(x)/255
      mm  = lapply(2:ncol(m),function(i) m[,i-1] * (1-lambdas[i-1]) + m[,i] * lambdas[i-1])
      mmm = cbind(m[,1],do.call(cbind,mm))
      rgb(mmm[1,],mmm[2,],mmm[3,],maxColorValue=1)
    }
    if (pow > 0) {
      for (i in 1:pow) {
        colors = f(colors)
      }
    }
    colors
  }

  # ['#f7fbff','#deebf7','#c6dbef','#9ecae1','#6baed6','#4292c6','#2171b5','#08519c','#08306b']
  blues  = c('#f7fbff','#deebf7','#c6dbef','#9ecae1','#6baed6','#4292c6','#2171b5','#08519c','#08306b')
  blues  = blues[2:7]
  #blues[1] = fwhite(blues[2],0.5)
  #blues[4:6] = fwhite(blues[4:6],0.9)
  # blues  = fscale(blues,1,0.75,2)
  blues  = fwhite(blues,0.6)
  reds   = c('#fff5f0','#fee0d2','#fcbba1','#fc9272','#fb6a4a','#ef3b2c','#cb181d','#a50f15','#67000d')
  reds   = reds[2:7]
  reds[3:5] = fwhite(reds[3:5],0.9)
  # reds   = fscale(reds,0.5,1,1)
  white  = c('#ffffff')
  colors = c(rev(reds),white,blues)

  coords = cells.to.coords(merge(1:n,1:m))
  valtext   = sapply(1:nrow(coords),function(i) sprintf("%.2E",A[m+1-coords$y1[i],coords$x1[i]]))
  valcolors = sapply(1:nrow(coords),function(i) colors[x[m+1-coords$y1[i],coords$x1[i]]])
  
  # col header
  col.header.coords = cells.to.coords(merge(1:n,m+1))
  col.header.text   = sapply( seq(10,100,10), function(x) sprintf("%d%%",x) )
  
  # row header
  row.header.coords = cells.to.coords(merge(0,1:(m+1)))
  row.header.text   = rownames(x) # c("TA",sapply( seq(0.05,0.95,0.05), function(x) sprintf("%.2f",x) ))
  row.header.text   = c(rev(row.header.text),options[["header"]]) # expression(theta percentile))
  
  # plot rectangles
  plot.rectangles = function(coords, colors, texts) {
    rect(coords$x0, coords$y0, coords$x1, coords$y1, col=colors, border=NA)
    text((coords$x0+coords$x1)/2, (coords$y0+coords$y1)/2, texts)
  }
  
  legend = function(xlim, ylim, colors, labels) {
    ncol = length(colors)
    dx = xlim[2] - xlim[1]
    dy = ylim[2] - ylim[1]
    w = dx/ncol
    x0 = xlim[1] + dx/ncol * (0:(ncol-1))
    # print(x0)
    coords = data.frame(x0=x0,y0=rep(ylim[1],ncol),x1=x0+dx/ncol,y1=rep(ylim[2],ncol))
    print(coords)
    rect(coords$x0,coords$y0,coords$x1,coords$y1, col=colors, border = "white")
    text(coords$x0 + w/2,(ylim[1]+ylim[2])/2,labels,adj=c(0.5,0.5))
  }
  
  pdf(options[["filename"]],
      width=options[["width"]],
      height=options[["height"]],
      pointsize=9,
      family="Helvetica")
  par(mar=rep(0,4))
  
  legend.xlim = c(-1,10)
  legend.ylim = c(-1.3,-0.3)

  plot(0,type="n",xlim=c(-1,n),ylim=c(legend.ylim[1],m+1),axes=F,xlab="",ylab="",xaxs = "i", yaxs = "i")
  plot.rectangles(coords, valcolors, valtext)
  plot.rectangles(row.header.coords, gray(0.8), row.header.text)
  plot.rectangles(col.header.coords, gray(0.8), col.header.text)
  abline(h=seq(0,m+1,1),col=gray(1),lwd=1)
  abline(v=seq(0,n+1,1),col=gray(1),lwd=1)
  legend(legend.xlim,
         legend.ylim,
         tail(colors,-2),
         c("1/10x < ",
           "1/5x < ",
           "1/2x < ",
           "1/1.5x < ",
           "1/1.1x <", 
           "1.1x <", 
           "1.5x <",
           "2x <",
           "5x <",
           "10x <",
           "100x"))
         # sprintf("%.2E",breaks)) # ,function(s) sprintf("%s <",fancy_seconds(s))))
  
  dev.off()
}

sel = t$threshold!=1.0
input = list(speedup=make.cumulative.table("speedup",t$speedup[sel],t$threshold[sel]))
render_tableimage(input, list(filename="analysis/plots/table_speedup_by_threshold.pdf", width=8, height=6.5, header=expression(paste(theta," \\ %"))))

sel = t$threshold==0.25
input = list(speedup=make.cumulative.table("speedup",t$speedup[sel],t$k[sel]))
render_tableimage(input, list(filename="analysis/plots/table_speedup_hybrid_025_by_k.pdf", width=8, height=3, header="k \\ %"))
