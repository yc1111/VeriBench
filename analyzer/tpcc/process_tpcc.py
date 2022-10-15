import sys

start, end = -1.0, -1.0

duration = float(sys.argv[2])
warmup = duration/3.0

tLatency = []
sLatency = []
fLatency = []

noLatency = []
osLatency = []
pmLatency = []
dlLatency = []
slLatency = []
vLatency = []

nkey = 0
nkeys = []

for line in open(sys.argv[1]):
  if line.startswith('#') or line.strip() == "":
    continue
  if line.startswith("verifynkeys"):
    nkey = float(line[12:-1])
    continue

  line = line.strip().split()
  if not line[0].isdigit() or len(line) < 6:
    continue

  if start == -1:
    start = float(line[2]) + warmup
    end = start + warmup

  fts = float(line[2])
  
  if fts < start:
    continue

  if fts > end:
    break

  latency = int(line[3])
  status = int(line[4])
  op = int(line[5])

  tLatency.append(latency)

  if status == 0:
    sLatency.append(latency)
  else:
    fLatency.append(latency)

  if op == 4:
    noLatency.append(latency)
  elif op == 5:
    pmLatency.append(latency)
  elif op == 6:
    osLatency.append(latency)
  elif op == 7:
    dlLatency.append(latency)
  elif op == 8:
    slLatency.append(latency)
  elif op == 15 and int(line[0]) > 0:
    nkeys.append(nkey)
    vLatency.append(latency)

if len(tLatency) == 0:
  print "Zero completed transactions.."
  sys.exit()

outfile = open(sys.argv[3], "w")
outfile.write(str(len(sLatency)) + "\n")                                        
outfile.write(str(sum(sLatency)) + "\n")                                        
outfile.write(str(len(tLatency)) + "\n")                                        
outfile.write(str(sum(tLatency)) + "\n")
outfile.write(str(end - start) + "\n")
outfile.write(str(len(noLatency)) + "\n")
outfile.write(str(sum(noLatency)) + "\n")
outfile.write(str(len(pmLatency)) + "\n")
outfile.write(str(sum(pmLatency)) + "\n")
outfile.write(str(len(osLatency)) + "\n")
outfile.write(str(sum(osLatency)) + "\n")
outfile.write(str(len(dlLatency)) + "\n")
outfile.write(str(sum(dlLatency)) + "\n")
outfile.write(str(len(slLatency)) + "\n")
outfile.write(str(sum(slLatency)) + "\n")
outfile.write(str(len(vLatency)) + "\n")
outfile.write(str(sum(vLatency)) + "\n")
outfile.write(str(sum(nkeys)) + "\n")