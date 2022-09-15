import sys
from glob import glob

numSuccess = 0.0
sumSuccess = 0.0
numTotal = 0.0
sumTotal = 0.0
duration = 0.0
numAM = 0.0
sumAM = 0.0
numGB = 0.0
sumGB = 0.0
numUB = 0.0
sumUB = 0.0
numUS = 0.0
sumUS = 0.0
numSP = 0.0
sumSP = 0.0
numWC = 0.0
sumWC = 0.0
numV = 0.0
sumV = 0.0
sumN = 0.0

path = sys.argv[1]
outpath = sys.argv[2]
for f in glob(path + "/client*log"):
  with open(f, "r") as fp:
    lines = fp.read().splitlines()
    numSuccess = numSuccess + float(lines[0])
    sumSuccess = sumSuccess + float(lines[1])
    numTotal = numTotal + float(lines[2])
    sumTotal = sumTotal + float(lines[3])
    duration = float(lines[4])
    numAM = numAM + float(lines[5])
    sumAM = sumAM + float(lines[6])
    numGB = numGB + float(lines[7])
    sumGB = sumGB + float(lines[8])
    numUB = numUB + float(lines[9])
    sumUB = sumUB + float(lines[10])
    numUS = numUS + float(lines[11])
    sumUS = sumUS + float(lines[12])
    numSP = numSP + float(lines[13])
    sumSP = sumSP + float(lines[14])
    numWC = numWC + float(lines[15])
    sumWC = sumWC + float(lines[16])
    numV = numV + float(lines[17])
    sumV = sumV + float(lines[18])
    sumN = sumN + float(lines[19])

outfile = open(outpath, "w")
outfile.write(str(numSuccess/duration) + "\n")
outfile.write(str(sumSuccess/numSuccess) + "\n")
outfile.write(str(numTotal/duration) + "\n")
outfile.write(str(sumTotal/numTotal) + "\n")
outfile.write(str((numTotal - numSuccess)/numTotal) + "\n")
outfile.write(str(sumAM/numAM) + "\n")
outfile.write(str(sumGB/numGB) + "\n")
outfile.write(str(sumUB/numUB) + "\n")
outfile.write(str(sumUS/numUS) + "\n")
outfile.write(str(sumSP/numSP) + "\n")
outfile.write(str(sumWC/numWC) + "\n")

if numV > 0:
  outfile.write(str(sumV/numV) + "\n")
else:
  outfile.write("0\n")

if sumN > 0:
  outfile.write(str(sumV/sumN) + "\n")
else:
  outfile.write("0\n")
