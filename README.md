# concurrent-ll

The `concurrent-ll` package contains the skeleton code for implementing and
evaluating two concurrent linked lists: a lock-free and a lock-based.
The implementations should work on any Linux-based `x86`/`x86_64` environments.

Both lists are sorted and provide three main operations: 
* adding an element to the list (if not already in the list)
* removing an element from the list (if already in the list)
* looking for an element in the list

In our case, a node of the list contains at least an integer key.

The lock-based implementation will use a technique called "hand-over-hand
locking", while the lock-free will be based on Harris' algorithm (reference
below).


Reference
---------
Lock-free linkedlist implementation of Harris' algorithm
> "A Pragmatic Implementation of Non-Blocking Linked Lists" 
> T. Harris, p. 300-314, DISC 2001.


Build
-----
You can compile the code (in Linux) by calling:
```shell
$ make
```
in the base directory.

If the number of cores on your processor is not recognized properly, fix it
in `include/utils.h`.
file under the "#if defined(DEFAULT)" definitions.

You can verify by calling:
```shell
$ make check
```

Benchmarking
------------
You can invoke the benchmarking scripts by calling:
```shell
$ make bench
```

Tools
-----
You can find several useful scripts that will help you test and evaluate your implementations.

In details:
* `scripts/test_correctness.sh`: test the correctness of an implementation, by stressing it
* `scripts/scalability1.sh`: benchmark 1 application and get its throughput and scalability
  E.g., `scripts/scalability1.sh all ./out/test-lock -i128`
* `scripts/scalability2.sh`: benchmark 2 applications and get their throughput and scalability
  E.g., `scripts/scalability2.sh all ./out/test-lock ./out/test-lockfree -i100`
* `scripts/run_ll.sh`: execute the workloads that will be part of the deliverable
* `scripts/create_plots_ll.sh`: generate the plots (int plots folder) of the data generated with
  `scripts/run_ll.sh`
  Note: You need [gnuplot](http://gnuplot.info/) installed		  


Implementation
--------------
You can find an easy-to-use interface for atomic operations in
`include/atomic_ops.h`.

* `list.h`: contains the interface and the structures of the list. 
You only need to change the llist_t and node_t structures to reflect the list
and a node of a list of your implementations respectively. 

* `list.c`: contains the implementations of the operations of the list, i.e.,
creating a new list and a new bucket, freeing the list, and, of course,
adding, removing, and looking for an element in the list.

Additionally, for the lock-based version, you need to implement and use some
locks. You can find the skeletons for initializing, freeing, locking, and
unlocking a lock in `include/lock_if.h`.

Memory management is one of most cumbersome problems on lock-free data
structures. In other words, when a thread removes an element (a node) from
the structure, it cannot always free the memory for that node, because other
threads might be holding a reference to this memory. 

When using locks, memory management is rather straightforward, because of the
mutual exclusion property of locks. You can optionally implement memory
management on the lock-based version.
