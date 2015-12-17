# TopKube Benchmark

The TopKube Benchmark consists of set of problems consisting of
m-lists of (key,value,order) triples. These lists are layed out so
that keys appear in increasing order. If we assume a key value pair
(k1,v1) is less than (k2,v2) if v1 < v2 or v1 == v2 && k2 < k1
