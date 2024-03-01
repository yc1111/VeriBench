import sys

path = sys.argv[1]
systems = sys.argv[2].split(",")
txnrates = sys.argv[3].split(",")
filename = sys.argv[4]

fp = open(path + "/" + filename, "w")
fp.write("\"request rate\"")
for r in txnrates:
  fp.write("\t\"" + r + "\"")
fp.write("\n")

for s in systems:
  fp.write(s)
  for r in txnrates:
    infile = open(path + "/" + s + "_" + r, "r")
    data = infile.read().splitlines()
    fp.write("\t\"" + data[0] + "\"")
    infile.close()
  fp.write("\n")

fp.close()
