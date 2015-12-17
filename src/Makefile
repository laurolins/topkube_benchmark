benchmark: FORCE
	ragel -C -o scanner.cc scanner.rl
	clang++ -g -std=c++11 -stdlib=libc++ -o benchmark problem.cc scanner.cc

FORCE:

