#!/bin/sh
LD_BIND_NOW=1 LD_PRELOAD=libhugetlbfs.so HUGETLB_MORECORE=yes ./bin/RELEASE/benchmark-rt $@ 2> benchmark-results
