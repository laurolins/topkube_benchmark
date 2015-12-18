```
/san_data/research/itopk/data/log/flickr_84M-log-merges.psv
/san_data/research/itopk/data/log/flickr_84M-timing.psv
/san_data/research/itopk/data/log/github_58M-log-merges.psv
/san_data/research/itopk/data/log/github_58M-query-timing.psv
/san_data/research/itopk/data/log/tweets_40M-log-merges.psv
/san_data/research/itopk/data/log/tweets_40M-query-timing.psv
/san_data/research/itopk/data/log/wikipedia_2005_2014_112M-log-merges.psv
/san_data/research/itopk/data/log/wikipedia_2005_2014_112M-query-timing.psv

ssh vidaserver1.poly.edu 'head -n 10 /san_data/research/itopk/data/log/tweets_40M-log-merges.psv' > sample.psv
```



llins@nano3:~/data/topkube_benchmark$ time bzip2 -kv topkube_benchmark_problems.psv
  topkube_benchmark_problems.psv:  2.927:1,  2.734 bits/byte, 65.83% saved, 15342286996 in, 5242366207 out.

real    28m48.262s
user    27m53.602s
sys     0m14.834s