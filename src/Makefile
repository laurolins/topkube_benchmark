benchmark: FORCE
	ragel -C -o scanner.cc scanner.rl
	clang++ -g -std=c++11 -stdlib=libc++ -o benchmark benchmark.cc rank.cc rank_parser.cc scanner.cc

FORCE:

