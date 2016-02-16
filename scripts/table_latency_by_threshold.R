setwd("/Users/llins/projects/topkube_benchmark/data")

system("mkdir -p analysis/tables")

load("topkube_benchmark_timing_nano4_runs5_threads20_median.Rdata")

t = robust.table

sel = rep(TRUE,nrow(original.table))
# sel = original.table$num_ranks > 1 & original.table$keys > 5e4
t <- original.table[sel,]

rename = c("all", "microblog", "github", "flickr", "wikipedia")
names(rename) = c("all", "tweets_40M","github_58M","flickr_84M","wikipedia_120M")

n = nrow(t)

cum.ratio = function (x) { return((1:length(x))/length(x)) }

make.cumulative.table = function(name,values, classes) {
  p = order(values)
  return(list(name=name, values=values[p], classes=classes[p]))
}

# sel = t$threshold!=1.0 & t$threshold!=0.025
sel = t$threshold!=0.025

# input = list(speedup=make.cumulative.table("speedup",log(t$speedup[sel],10),t$threshold[sel]))
# input = list(speedup=make.cumulative.table("speedup",t$speedup[sel],t$threshold[sel]))
# input = list(speedup=make.cumulative.table("speedup",t$speedup[sel],t$threshold[sel]))

input = list(latency=make.cumulative.table("latency",t$time[sel],t$threshold[sel]))

result = lapply(1:length(input), function(i) {
  variable        = input[[i]]$name
  #print(variable)
  value.groups = split(input[[i]]$values, input[[i]]$classes)
  # print(names(value.groups))

  # create a quantile matrix ordered for each value group (in row order of value.groups)
  quantile.matrix = do.call(rbind, lapply(names(value.groups), function(i){ quantile(value.groups[[i]],probs=seq(0.1,1,0.1)) }))
  rownames(quantile.matrix) = 1:nrow(quantile.matrix)
  colnames(quantile.matrix) = sapply(1:10,function(i) sprintf("p%d",10*i))
  
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

# levels = c(1.05,1.25,1.50,2.0,5,10,10000000)
levels = c(5e-4, 1e-3, 5e-3, 1e-2, 5e-2, 1e-1, 0.5, 1, 5, 10, 1000)
# levels = c(1e-6,1e-3,1e-2,0.5e-1,1e-1,5e-1,1,10,100,1000)
breaks=levels


blues  = c('#f7fbff','#deebf7','#c6dbef','#9ecae1','#6baed6','#4292c6','#2171b5','#08519c','#08306b')
reds   = c('#fff5f0','#fee0d2','#fcbba1','#fc9272','#fb6a4a','#ef3b2c','#cb181d','#a50f15','#67000d')
colors = c(rev(blues[2:6]),reds[1:6])


# breaks = c(1/rev(levels),levels)
length(breaks)

A = as.matrix(table[,3:ncol(table)])
m = nrow(A)
n = ncol(A)
x = matrix(cut(A,breaks=breaks,labels=F),nrow=m)

#
# assuming two column data frame with cx and cy
# return coordinates data frame
#
cells.to.coords <- function(cells) { data.frame(x0=cells[,1]-1,y0=cells[,2]-1,x1=cells[,1],y1=cells[,2]) }

# blues  = c('#f7fbff','#deebf7','#c6dbef','#9ecae1','#6baed6','#4292c6','#2171b5','#08519c','#08306b')
# reds   = c('#fff5f0','#fee0d2','#fcbba1','#fc9272','#fb6a4a','#ef3b2c','#cb181d','#a50f15','#67000d')

fwhite = function(x,lambda) {
  x = (lambda * col2rgb(x) + (1-lambda) * 255)/255.0
  rgb(x['red',],x['green',],x['blue',])
}

# blues  = c('#f7fbff','#deebf7','#c6dbef','#9ecae1','#6baed6','#4292c6','#2171b5','#08519c','#08306b')
# reds   = c('#fff5f0','#fee0d2','#fcbba1','#fc9272','#fb6a4a','#ef3b2c','#cb181d','#a50f15','#67000d')
# white  = c('#ffffff')
# colors = c(rev(reds[2:7]),white,blues[2:7])
colors = fwhite(colors,0.6) # c(rev(reds[2:7]),white,blues[2:7])





# colors = fwhite(colors,0.7)

coords = cells.to.coords(merge(1:n,1:m))
valtext   = sapply(1:nrow(coords),function(i) sprintf("%.2E",A[m+1-coords$y1[i],coords$x1[i]]))
valcolors = sapply(1:nrow(coords),function(i) colors[x[m+1-coords$y1[i],coords$x1[i]]])

# col header
col.header.coords = cells.to.coords(merge(1:n,m+1))
col.header.text   = sapply( seq(10,100,10), function(x) sprintf("%d%%",x) )

# row header
row.header.coords = cells.to.coords(merge(0,1:(m+1)))
row.header.text   = sapply( seq(0.05,0.95,0.05), function(x) sprintf("%.2f",x) )
row.header.text   = c("TA",row.header.text,"Sweep")
row.header.text   = c(rev(row.header.text),expression(paste(theta," \\ %"))) # expression(theta percentile))

# legend
legend.coords = cells.to.coords(merge(1:(length(breaks)-1),-1))
legend.colors = colors[1:(length(breaks)-1)]
legend.text = sapply(1:(length(breaks)-1), function(i) sprintf("[%f,\n%f)", breaks[i], breaks[i+1]))
# legend.text = c(expression(1 mu s.,"1sapply(1:(length(breaks)-1), function(i) sprintf("[%.2E,\n%.2E)", breaks[i], breaks[i+1]))

# plot rectangles
plot.rectangles = function(coords, colors, texts, cex=1) {
  rect(coords$x0, coords$y0, coords$x1, coords$y1, col=colors, border=NA)
  text((coords$x0+coords$x1)/2, (coords$y0+coords$y1)/2, texts, cex=cex)
}

create.legend = function(xlim, ylim, colors, labels) {
  ncol = length(colors)
  dx = xlim[2] - xlim[1]
  dy = ylim[2] - ylim[1]
  w = dx/ncol
  x0 = xlim[1] + dx/ncol * (0:(ncol-1))
  # print(x0)
  coords = data.frame(x0=x0,y0=rep(ylim[1],ncol),x1=x0+dx/ncol,y1=rep(ylim[2],ncol))
  print(coords)
  rect(coords$x0,coords$y0,coords$x1,coords$y1, col=colors, border = "white")
  print(ylim[1]+dy/2.0)
  text(coords$x0 + w/2, ylim[1]+dy/2.0, labels, adj=c(0.5,0.5))
}

fancy_seconds = function(secs) {
  if (secs < 1e-6) {
    sprintf("%f s.", secs)
  }
  else if (secs < 1e-5) {
    sprintf("%.0f \U00B5s.",secs * 1e6)
  }
  else if (secs < 1e-4) {
    sprintf("%.2f ms.",secs * 1e3)
  }
  else if (secs < 1e-3) {
    sprintf("%.1f ms.",secs * 1e3)
  }
  else if (secs < 1e-2) {
    sprintf("%.0f ms.",secs * 1e3)
  }
  else if (secs < 1e-1) {
    sprintf("%.0f ms.",secs * 1e3)
  }
  else if (secs < 1e-0) {
    sprintf("%.0f ms.",secs * 1e3)
  }
  else { # if (secs < 1e-0) {
    sprintf("%.0f s.", secs)
  }
}

pdf("analysis/plots/table_latency_by_threshold.pdf",width=8,height=6.5,pointsize=9,family="Helvetica")
par(mar=rep(0,4))

legend.xlim = c(-1,10)
legend.ylim = c(-1.3,-0.3)

plot(0,type="n",xlim=c(-1,n),ylim=c(legend.ylim[1],m+1),axes=F,xlab="",ylab="",xaxs = "i", yaxs = "i")
plot.rectangles(coords, valcolors, valtext)
plot.rectangles(row.header.coords, gray(0.8), row.header.text)
plot.rectangles(col.header.coords, gray(0.8), col.header.text)
plot.rectangles(legend.coords, legend.colors, legend.text, cex=0.7)
# plot.rectangles(cells.to.coords(cbind(0,-1)), gray(0.8), "Color Map", cex=0.9)
abline(h=seq(0,m+1,1),col=gray(1),lwd=1)
abline(v=seq(0,n+1,1),col=gray(1),lwd=1)
legend.text = sapply(head(breaks,-1), function (s) sprintf("%s",fancy_seconds(s)))
create.legend(
  legend.xlim,
  legend.ylim,
  head(colors,-1),
  legend.text)
dev.off()

# number of problems
length(unique(t$problem))
