import sys

start, end = -1.0, -1.0

duration = float(sys.argv[2])
warmup = duration/3.0

tLatency = []
sLatency = []
fLatency = []

amLatency = []
gbLatency = []
ubLatency = []
usLatency = []
spLatency = []
wcLatency = []
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

  if op == 1:
    amLatency.append(latency)
  elif op == 2:
    gbLatency.append(latency)
  elif op == 3:
    ubLatency.append(latency)
  elif op == 4:
    usLatency.append(latency)
  elif op == 5:
    spLatency.append(latency)
  elif op == 6:
    wcLatency.append(latency)
  elif op == 9 and int(line[0]) > 0:
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
outfile.write(str(len(amLatency)) + "\n")
outfile.write(str(sum(amLatency)) + "\n")
outfile.write(str(len(gbLatency)) + "\n")
outfile.write(str(sum(gbLatency)) + "\n")
outfile.write(str(len(ubLatency)) + "\n")
outfile.write(str(sum(ubLatency)) + "\n")
outfile.write(str(len(usLatency)) + "\n")
outfile.write(str(sum(usLatency)) + "\n")
outfile.write(str(len(spLatency)) + "\n")
outfile.write(str(sum(spLatency)) + "\n")
outfile.write(str(len(wcLatency)) + "\n")
outfile.write(str(sum(wcLatency)) + "\n")
outfile.write(str(len(vLatency)) + "\n")
outfile.write(str(sum(vLatency)) + "\n")
outfile.write(str(sum(nkeys)) + "\n")
