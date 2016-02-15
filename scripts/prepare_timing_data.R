setwd("/Users/llins/projects/topkube_benchmark/data")

system("mkdir -p analysis/tables")


# read table
ptm <- proc.time()
original.table <- read.table(gzfile("topkube_benchmark_timing_nano4_runs5_threads20.psv.gz"),
#original.table <- read.table("y",
                             header = T,
                             as.is  = T,
                             sep    ="|")
proc.time() - ptm

# select info needed for latency and speedup analysis

t <- aggregate(t$time, by=list(
               problem=original.table$problem,
               dataset=original.table$dataset,
                num_ranks=original.table$num_ranks,
                largest_rank=original.table$largest_rank,
                keys=original.table$keys,
                entries=original.table$entries,
                density=original.table$density,
                algorithm=original.table$algorithm,
                k=original.table$k,
                threshold=original.table$threshold),
               median)

tt = t[order(t$problem,t$k,t$threshold),]
rownames(tt) = 1:nrow(tt)
colnames(tt)[11] = "time"
x = tt$time[tt$algorithm=="sweep"]
y = rep(x,each=21)
ttt = data.frame(tt,speedup=y/tt$time)
robust.table = ttt

save(robust.table,file="topkube_benchmark_timing_nano4_runs5_threads20_median.Rdata")

