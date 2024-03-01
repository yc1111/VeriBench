import sys

path = sys.argv[1]
systems = sys.argv[2].split(",")
nodes = sys.argv[3].split(",")
filename = sys.argv[4]

fp = open(path + "/" + filename, "w")
fp.write("\"nodes\"")
for r in nodes:
  fp.write("\t\"" + r + "\"")
fp.write("\n")

for s in systems:
  fp.write(s)
  for r in nodes:
    infile = open(path + "/" + s + "_node" + r, "r")
    data = infile.read().splitlines()
    fp.write("\t\"" + data[0] + "\"")
    infile.close()
  fp.write("\n")

fp.close()
