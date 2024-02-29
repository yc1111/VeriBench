import sys

if (len(sys.argv) < 4):
    print("python gen_conf.py n_shards input output [options]\n\n" +
          "n_shards\tThe number of shards will be started.\n" +
          "input\t\tThe directory contains workers file.\n" +
          "output\t\tThe directory where config files will be written to.\n\n" +
          "options:\n\t-r\tgenerate configuration with 3 replicas")
    exit(0)

n_shards = int(sys.argv[1])
folder = sys.argv[2]
target = sys.argv[3]

replication = str(0)
if (len(sys.argv) == 5 and sys.argv[4] == "-r"):
    replication = str(1)

count = int(0)
portno = int(29)
tss_port = int(25)

infile = open(folder + "/workers", "r")
ips = infile.read().splitlines()[:n_shards]
infile.close()

for i in range(n_shards):
    outfile = open(target + "/shard" + str(i) + ".config", "w")
    outfile.write("f " + replication + "\n")
    outfile.write("replica " + ips[i] + ":517" + str(portno) + "\n")
    if replication == "1":
        outfile.write("replica " + ips[(i+1)%n_shards] + ":517" + str(portno) + "\n")
        outfile.write("replica " + ips[(i+2)%n_shards] + ":517" + str(portno) + "\n")
    outfile.close()
    portno = portno + 1

tsserverfile = open(folder + "/workers", "r")
tsserver = tsserverfile.readline()[:-1]
tsserverfile.close()
tss = open(target + "/shard.tss.config", "w")
tss.write("f " + replication + "\n")
tss.write("replica " + tsserver + ":517" + str(tss_port) + "\n")
tss.close()

