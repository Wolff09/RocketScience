#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

NUM_VARS = 10 # should be > 0
DEPTH = 5 # should be > 0

OUTPUTFILE_FORMAT = "big_v%i_d%i.csv"
output_file = None
first_line = True

def write(row):
	global first_line
	# cell ::= src,dst,guard[,action_var,action_formula]*
	if first_line:
		output_file.write(row)
		first_line = False
	else:
		output_file.write("\n")
		output_file.write(row)


def main():
	"""
	This script generates a big StateTransitionSystem .csv example.

	The generated transition system looks as follows:
		* there are variables var_i, i=0..NUM_VARS-1
		* there are nodes init_i, i=0..NUM_VARS
		* there are edges (init_i, init_i+1) setting var_i to true
		* there is a node btree_
		* there is a transition from init_NUM_VARS+1 to btree_ asserting
		  that all variables are true and without actions
		* btree_ is the root of a complete binary tree of depth DEPTH

	The generated transition system will have (NUM_VARS+1)+(2**(DEPTH+1))−1 Knoten nodes.
	All states will be reachable when starting in init_0 with an
	arbitrary variable assignment.
	"""

	assert NUM_VARS >= 1
	assert DEPTH >= 1

	# make init_i nodes
	for i in range(NUM_VARS):
		write("init_%i,init_%i,(true),var_%i,(true)" % (i, i+1, i))

	all_true = "(&) "*(NUM_VARS-1) + " ".join(["var_%i"%i for i in range(NUM_VARS)])
	write("init_%i,btree_,%s" % (NUM_VARS, all_true))

	# generate tree
	def make_tree(prefix, depth_remaining):
		if depth_remaining == 0: return
		for i in [0, 1]:
			write("btree_%s,btree_%s%i,%s" % (prefix, prefix, i, all_true))
			make_tree("%s%i" % (prefix, i), depth_remaining-1)

	make_tree("", DEPTH)


if __name__ == '__main__':
	# TODO: read num_vars, depth from cmd if possible, otherwise use defaults
	if len(sys.argv) == 3:
		NUM_VARS = int(sys.argv[1])
		DEPTH = int(sys.argv[2])

	output_file = open(OUTPUTFILE_FORMAT % (NUM_VARS, DEPTH), "w")
	main()
	output_file.close()
