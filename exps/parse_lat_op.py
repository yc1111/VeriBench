import sys

path = sys.argv[1]
systems = sys.argv[2].split(",")
txnrates = sys.argv[3].split(",")
opnames = sys.argv[4].split(",")
ops = sys.argv[5].split(",")
filename = sys.argv[6]

fp = open(path + "/" + filename, "w")
fp.write("\"Op\"")
for r in opnames:
  fp.write("\t\"" + r + "\"")
fp.write("\n")

for s in systems:
  fp.write(s)
  infile = open(path + "/" + s + "_" + txnrates[-1], "r")
  data = infile.read().splitlines()
  for r in ops:
    fp.write("\t\"" + data[int(r)] + "\"")
    infile.close()
  fp.write("\n")

fp.close()
