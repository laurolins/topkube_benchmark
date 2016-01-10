setwd("/Users/llins/projects/topkube_benchmark/data")
t <- read.table("topkube_benchmark_problems_stats.psv",header=T,as.is=T,sep="|")

rename = c("all", "twitter", "github", "flickr", "wikipedia")
names(rename) = c("all", "tweets_40M","github_58M","flickr_84M","wikipedia_120M")

n = nrow(t)

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




make.cumulative.table = function(name,values, classes) {
  p = order(values)
  return(list(name=name, values=values[p], classes=classes[p]))
}

input.columns = c("keys","num_ranks","entries","density") 
input = lapply(input.columns, function(name) { make.cumulative.table(name,t[[name]],t$dataset) })
names(input) = input.columns

result = lapply(1:length(input), function(i) {
  variable        = input[[i]]$name
  #print(variable)
  value.groups = split(input[[i]]$values, input[[i]]$classes)
  value.groups = c(list(all=input[[i]]$values), value.groups)
  # print(names(value.groups))

  # create a quantile matrix ordered for each value group (in row order of value.groups)
  quantile.matrix = do.call(rbind, lapply(names(value.groups), function(i){ quantile(value.groups[[i]],probs=seq(0,1,0.1)) }))
  rownames(quantile.matrix) = 1:nrow(quantile.matrix)
  colnames(quantile.matrix) = sapply(0:10,function(i) sprintf("p%d",10*i))
  
  # create the variable data frame
  variable.df = data.frame(variable=rep(variable,nrow(r)),
                           dataset=sapply(names(value.groups), function(n) rename[[n]]), 
                           data.frame(quantile.matrix))
  
  # rename rows to numbers
  rownames(variable.df) = 1:nrow(variable.df)
  variable.df
  })
names(result) = input.columns

table = do.call(rbind.data.frame,result)
rownames(table) = 1:nrow(table)

write.csv(table,"/tmp/table.csv")


