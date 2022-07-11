AUX_SOURCE_DIRECTORY(ycsb ycsb_srcs)
LIST(APPEND srcs ${ycsb_srcs})

AUX_SOURCE_DIRECTORY(smallbank smallbank_srcs)
LIST(APPEND srcs ${smallbank_srcs})

AUX_SOURCE_DIRECTORY(tpcc tpcc_srcs)
LIST(APPEND srcs ${tpcc_srcs})

ADD_LIBRARY(workload_objs OBJECT ${srcs})
ADD_LIBRARY(workload SHARED $<TARGET_OBJECTS:workload_objs>)
