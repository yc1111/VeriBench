import sys
from glob import glob

numSuccess = 0.0
sumSuccess = 0.0
numTotal = 0.0
sumTotal = 0.0
duration = 0.0
numR = 0.0
sumR = 0.0
numW = 0.0
sumW = 0.0
numH = 0.0
sumH = 0.0
numRg = 0.0
sumRg = 0.0
numV = 0.0
sumV = 0.0
sumN = 0.0

path = sys.argv[1]
outpath = sys.argv[2]
for f in glob(path + "/client*log"):
  with open(f, "r") as fp:
    lines = fp.read().splitlines()
    print(f + "\n" + lines[13] + ", " + lines[14] + "\n")
    numSuccess = numSuccess + float(lines[0])
    sumSuccess = sumSuccess + float(lines[1])
    numTotal = numTotal + float(lines[2])
    sumTotal = sumTotal + float(lines[3])
    duration = float(lines[4])
    numR = numR + float(lines[5])
    sumR = sumR + float(lines[6])
    numW = numW + float(lines[7])
    sumW = sumW + float(lines[8])
    numH = numH + float(lines[9])
    sumH = sumH + float(lines[10])
    numRg = numRg + float(lines[11])
    sumRg = sumRg + float(lines[12])
    numV = numV + float(lines[13])
    sumV = sumV + float(lines[14])
    sumN = sumN + float(lines[15])

outfile = open(outpath, "w")
outfile.write(str(numSuccess/duration) + "\n")
outfile.write(str(sumSuccess/numSuccess) + "\n")
outfile.write(str(numTotal/duration) + "\n")
outfile.write(str(sumTotal/numTotal) + "\n")
outfile.write(str((numTotal - numSuccess)/numTotal) + "\n")

if numR > 0:
  outfile.write(str(sumR/numR) + "\n")
else:
  outfile.write("0\n")

if numW > 0:
  outfile.write(str(sumW/numW) + "\n")
else:
  outfile.write("0\n")

if numH > 0:
  outfile.write(str(sumH/numH) + "\n")
else:
  outfile.write("0\n")

if numRg > 0:
  outfile.write(str(sumRg/numRg) + "\n")
else:
  outfile.write("0\n")

if numV > 0:
  outfile.write(str(sumV/numV) + "\n")
else:
  outfile.write("0\n")

if sumN > 0:
  outfile.write(str(sumV/sumN) + "\n")
else:
  outfile.write("0\n")

