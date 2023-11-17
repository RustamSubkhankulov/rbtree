#!/usr/bin/python3

import sys

#==================

def parse_args(argv):
  """
  Parse sys.argv for arguments of prog.
  """
  if len(argv) != 3:
    print("Usage: ./query_gen.py elem_num rep_num filename \n")
    print("where 1. elem_num - number of elements - max distance between elements in tree \n")
    print("      2. filename - name of the output file with queries. \n")
    sys.exit(1)

  elem_num = int(argv[1])
  filename = argv[2]

  if elem_num < 0:
    print("Number of elements should be positive number. \n")
    sys.exit(1)

  return (elem_num, filename)

#------------------

def gen_queries(elem_num: int):
  """
  Generate insert and distance queries strings to be written to file.
  """
  queries = []
  for ind in range(elem_num):
    queries.append(f"k {ind} ")

  for first in range(elem_num):
    for second in range(elem_num):
      if first < second:
        queries.append(f"q {first} {second} ")
      elif first > second:
        queries.append(f"q {second} {first} ")

  return queries

#------------------

def write_to_file(out_file, queries):
  """
  Write previously generated queries to file.
  """
  for query in queries:
    out_file.write(query)

#==================

elem_num, filename = parse_args(sys.argv)

with open(filename, mode = 'w') as out_file:

  queries = gen_queries(elem_num)
  write_to_file(out_file, queries)
