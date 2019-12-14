SRC=../src
simulate: $(SRC)/simulate.cc
	g++ -o simulate $(SRC)/simulate.cc $(SRC)/replacement_policies.h $(SRC)/memory.h $(SRC)/game.h --std=c++11

generalize: $(SRC)/generalize.cc
	g++ -o generalize $(SRC)/generalize.cc $(SRC)/replacement_policies.h $(SRC)/memory.h $(SRC)/game.h --std=c++11

debug_generalize: $(SRC)/generalize.cc
	g++ -g -o generalize $(SRC)/generalize.cc $(SRC)/replacement_policies.h $(SRC)/memory.h $(SRC)/game.h --std=c++11

debug_simulate: $(SRC)/generalize.cc
	g++ -g -o simulate $(SRC)/simulate.cc $(SRC)/replacement_policies.h $(SRC)/memory.h $(SRC)/game.h --std=c++11
