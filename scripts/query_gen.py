#!/usr/bin/python3

import sys

#==================

def parse_args(argv):
  """
  Parse sys.argv for arguments of prog.
  """
  if len(argv) != 4:
    print("Usage: ./query_gen.py elem_num rep_num filename \n")
    print("where 1. elem_num - number of elements - max distance between elements in tree \n")
    print("      2. rep_num - repetition number - is how many times distance will be calculated. \n")
    print("      3. filename - name of the output file with queries. \n")
    sys.exit(1)

  elem_num = int(argv[1])
  rep_num  = int(argv[2])
  filename = argv[3]

  if elem_num < 0:
    print("Number of elements should be positive number. \n")
    sys.exit(1)

  if rep_num < 0:
    print("Number of repetitions should be positive number. \n")
    sys.exit(1)

  return (elem_num, rep_num, filename)

#------------------

def gen_queries(elem_num: int, rep_num: int):
  """
  Generate insert and distance queries strings to be written to file.
  """
  queries = []
  for ind in range(elem_num):
    queries.append(f"k {ind} ")

  for ind in range(rep_num):
    queries.append(f"q 0 {elem_num - 1} ")

  return queries

#------------------

def write_to_file(out_file, queries):
  """
  Write previously generated queries to file.
  """
  for query in queries:
    out_file.write(query)

#==================

if len(sys.argv) != 4:
    print("Usage: ./query_gen.py elem_num rep_num filename")
    print("where 1. elem_num - number of elements - max distance between elements in tree")
    print("      2. rep_num - repetition number - is how many times distance will be calculated.")
    print("      3. filename - name of the output file with queries.")
    sys.exit(1)

elem_num = int(sys.argv[1])
rep_num  = int(sys.argv[2])
filename = sys.argv[3]

if elem_num < 0:
  print("Number of elements should be positive number.")
  sys.exit(1)

if rep_num < 0:
  print("Number of repetitions should be positive number.")
  sys.exit(1)

elem_num, rep_num, filename = parse_args(sys.argv)

with open(filename, mode = 'w') as out_file:

  queries = gen_queries(elem_num, rep_num)
  write_to_file(out_file, queries)
