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

sel = t$algorithm == "sweep"
a = data.frame(problem=t$problem[sel],k=t$k[sel],time=t$time[sel])
speedup = sapply(1:nrow(t),function(i) {
  index = min(which(a$problem==t$problem[i] & a$k==t$k[i]))
  sweep_time = a$time[index]
  #print(sprintf("i: %d sweep_time: %f time: %f speedup: %f",i,sweep_time,t$time[i],sweep_time/t$time[i]))
  return(sweep_time/t$time[i])
})
t = data.frame(t,speedup=speedup)

rename = c("all", "microblog", "github", "flickr", "wikipedia")
names(rename) = c("all", "tweets_40M","github_58M","flickr_84M","wikipedia_120M")

n = nrow(t)

cum.ratio = function (x) { return((1:length(x))/length(x)) }

make.cumulative.table = function(name,values, classes) {
  p = order(values)
  return(list(name=name, values=values[p], classes=classes[p]))
}

sel = t$threshold==0.25
#input = list(speedup=make.cumulative.table("speedup",log(t$speedup[sel],10),t$k[sel]))
input = list(speedup=make.cumulative.table("speedup",t$speedup[sel],t$k[sel]))

result = lapply(1:length(input), function(i) {
  variable        = input[[i]]$name
  #print(variable)
  value.groups = split(input[[i]]$values, input[[i]]$classes)
  # print(names(value.groups))
  
  # create a quantile matrix ordered for each value group (in row order of value.groups)
  quantile.matrix = do.call(rbind, lapply(names(value.groups), function(i){ quantile(value.groups[[i]],probs=seq(0,1,0.1)) }))
  rownames(quantile.matrix) = 1:nrow(quantile.matrix)
  colnames(quantile.matrix) = sapply(0:10,function(i) sprintf("p%d",10*i))
  
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

write.csv(table,"analysis/tables/topkube_benchmark_hybrid_025_speedup_by_k.csv")

