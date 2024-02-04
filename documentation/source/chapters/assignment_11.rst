.. _ch:Task_11:

Individual Phase - Final Report
================================

Introduction
------------

With the conclusion of the Tsunami Lab project, our final task was to implement a mechanism of our choosing. The program developed in the project 
takes netCDF data as input, runs the simulation, and outputs the results. Due to the potentially large domain of these input files, the simulation 
can be lengthy. As a result, the simulation may take some time to complete. One way to improve program efficiency is to utilize parallelization, which 
involves instructing multiple cores to run independent parts of the code simultaneously. As dealt with in assignment 9 we already implemented 
parallelization on a smaller scale using OpenMP. Now, our team decided on taking it to a greater scale, splitting our domain into several 
subdomains and running each corresponding simulation on a respective core. Furthermore, to make each simulation more memory efficient, we 
optimize the cache usage. In this section, we will outline the necessary concepts for the optimization process and the strategy we developed 
to integrate them into the simulation.

Methods
-------

Message Passing Interface (MPI)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When parallelizing our solver, we have used OpenMP in the past to partially parallelize our code.
OpenMP enables a program to be processed by multiple processors by declaring parallel regions that are executed on each core. 
One solution is to run several processes in parallel, all of which execute the entire code.
However, sharing data and results of local calculations among multiple processors in OpenMP can be complex.
This is where the Message Passing Interface is used.
MPI spawns a certain number of processes each on one respective core. If there were no restrictions, this would result in every process executing exactly the same work.
Therefore, each core has an ID called 'rank' to identify the process running the code and distinguish the work done on each process. 
MPI provides functions to facilitate communication, allowing for the sending and receiving of single or multiple data to or from another processor.

MPI provides a mechanism for arranging the used processes on a Cartesian coordinate system. By using the shifting method, each core can 
determine which processes are adjacent in the grid. When a process relies on n neighbors it is in a so-called n-point-stencil.

Parallelizing with MPI requires consideration of blocking and non-blocking communication. Blocking communication requires the sending process to wait for confirmation, that the data has been received and communication has ended. 
Although communication that is blocked is secure, it can result in longer runtimes for large data transfers due to idle time. 
Non-blocking communication functions, on the other hand, run the communication in the background and return immediately after being called, allowing for other tasks to be processed simultaneously. 
This can be used to process other tasks while the communication is ongoing. The MPI_Wait function enables communication synchronization before continuing computation. 
Non-blocking communication can be more complex. This is because a process cannot continue working with the transmitted data until the communication has ended. 
However, if used correctly, it can be faster than working with blocking communication.

Cache
^^^^^

Caches are small, but fast memory units on a processor chip that are supposed to reduce the number of slow memory accesses. They are a less expensive alternative to 
registers which are even faster. Each processor has three levels of cache: the core-specific are the level 1 data (L1D), level 1 instruction (L1I), and level 2 
(L2) cache. The level (L3) cache is shared by the cores on a processor. Memory can only be transferred in a cache line consisting of a specific number of bytes.

If a query does not find the data in the cache, it is referred to as a cache miss; otherwise, it is called a cache hit. When a program queries the memory for a particular 
data, it first looks in the L1D cache; if it does not find the seached data, it continues to look for it in the next lower level, in this case the L2 cache, and so on. 
If the data is found, its cache line is loaded directly into the L1D cache.

On the Ara cluster [1]_ each Hadoop node consists of 2 Intel Xeon Gold 6140 [2]_ processors with 18 cores each. These have an L1D cache size of 576 KiB, an L2 cache size 
of 18 MiB, and an L3 cache size of 24.75 MiB. In addition, each cache is divided into multiple sets, which are consisting of n lines, which is called an n-way 
set associative cache. Finally, all caches of the processors' caches are write-back, meaning that they write the changes to data in to the memory only when the 
data in the cache needs to be replaced in cache. These specifications must be taken into account when writing cache-optimized code on the Ara cluster.

.. note::
    16 Hadoop nodes on Ara cluster with each:

    - 36 CPU-cores (2x Intel Xeon Gold 6140 18 Core 2,3 Ghz)
    - 192 GB RAM
    - one local SSD

    L1 = 1.125 MiB	
        - L1I	576 KiB	18x32 KiB	8-way set associative	 
        - L1D	576 KiB	18x32 KiB	8-way set associative	write-back

    L2 = 18 MiB
 	 	- 18x1 MiB	16-way set associative	write-back
    
    L3 = 24.75 MiB	
 	 	- 18x1.375 MiB	11-way set associative	write-back



Was haben wir gemacht? (technisch)
----------------------------------


Parallelization with MPI
^^^^^^^^^^^^^^^^^^^^^^^^^^
Domain decomp
comm vor x sweep (left / right) und vor y sweep (up down)

In order to reduce the computation time, the global compute domain was divided into several smaller subgrids that run in parallel.
The input files are read in the root process and distributed to the subgrids. 
Once each process has received its local domain and the associated data, the calculation is performed as normal with the respective smaller domain.

Since our solver uses a two-point-stencil, a column or row of values is missing at the edges of the subgrids for the calculation, which can be found in the respective neighboring grids.

.. warning::

    Grafik über data dependency in zwischen subgrids

As the calculation of the NetUpdates is split up into x and y sweep and takes place one after the other, our data is interdependent.
This means that we have to communicate twice. The columns (left and right border) are communicated before the x-sweep and the rows (top and bottom border) before the y-sweep.

.. warning::

    Grafik einfügen was wann kommuniziert wird.


Cache optimization
^^^^^^^^^^^^^^^^^^^^

In order to make the cache usage of our solver more efficient, we first looked at the cache specifications of the ARA cluster Hadoop nodes.

16 Hadoop nodes each with:

    - 36 CPU cores (2x Intel Xeon Gold 6140 18 Core 2.3 Ghz)
    - 192 GB RAM

Intel Xeon Gold 6140 18 Core 2.3 Ghz Cache specifications:

L1 Cache: 8-way set associative, write-back

    L1i = 576 KiB (18 x 32 KiB) 
    L1d = 576 KiB (18 x 32 KiB) 

L2 Cache: 16-way set associative, write-back

    L2 = 18 MiB (18 x 1 MiB) 

L3 Cache: 11-way set associative, write-back

    L3 = 24.75 MiB (18 x 1.375 MiB) 

Anzahl an Sets in Cache: cache size / (block size * set size)

Cache line füllen und dann möglichst alle Operationen durchführen um capacity misses zu minimieren

Alignement check.

Blocking?


Ergebnisse (Berechnungen und vid von Sim)
-----------------------------------------

Anmerken, welche Zwischenergebnisse wir hatten, die falsch waren? (z.B. kaputte Bathymetry etc.)

video von fehlschlägen

MPI läuft korrekt

video of Sim with [?] number of processes als Beweis

We have successfully MPI-parallelized our solver. In the following video you can see the simulation of the tsunami event in Chile from 2010 with a magnitude of 8.8 and a cell size of 1000m, divided into 10 subgrids.

The following measured values were recorded in comparison to the non-parallelized version of our solver.

+---------------+----------+----------+
| Simulation    |  time    |   icpc   |
+===============+==========+==========+
|      1000     | 13.1988s | 12.6979s |
+---------------+----------+----------+
|   1000 | 5    | 11.1114s |  13.11s  |
+---------------+----------+----------+
|   1000 | 10   |          | 10.9384s |
+---------------+----------+----------+

You can see that the time needed to read and set the initial grids is longer in the parallelized version, for which in turn the time needed to calculate is shorter.

You can see that the normal version is faster than the parallelized version when reading / loading data and defining the Grid. On the other hand, the parallelized version requires less computing time than the normal version.
The larger the computational domain (or the smaller the cells become), the greater the time difference between normal and parallelized version when calculating the NetUpdates.

Speedup :math:`S_p` of computation time :math:`T_{comp}` for various simulations with different numbers of subgrids: 

.. math::   
    
    S_p &= \frac{T_1}{T_p} \\
    S_{72} &= \frac{2078.36s}{37.2162s} = 55.845

Amdahl vs. Gustafson

.. warning::

    Auszug aus pc1 Website als Kontext (https://scalable.uni-jena.de/opt/pc/chapters/assignment_performance_metrics.html)
    
    Kann später wieder weg

Amdahl's Law predicts the speedup of a computation when only a fraction of the computation can be parallelized. The law is named after Gene Amdahl, who introduced it in 1967:

.. math::

   S = \frac{1}{{(1 - \alpha) + \frac{\alpha}{p}}}

Where:
   - :math:`S` is the speedup of the parallelized computation,
   - :math:`\alpha` is the fraction of the computation that can be parallelized,
   - :math:`p` is the number of processors.

Gustafson's Law emphasizes scalability, stating that as the problem size increases, the parallelizable portion grows, allowing better scaling with more resources. 
Gustafson's Law introduces the idea that we can adjust the problem size to  utilize larger parallel systems. While Amdahl's Law highlights limitations in fixed-sized problems. 
This law is named after John Gustafson, who introduced it in 1988.

In mathematical terms, Gustafson's Law can be expressed as:

.. math:: T_p = (1 - \alpha) + \alpha

.. math:: T_1(p) = (1 - \alpha) + \alpha \cdot p

.. math:: S(p) = \frac{T_1(p)}{T_p} = \frac{1 - \alpha + \alpha \cdot p}{1}

where:

- :math:`T_p` is the execution time for parallel processing with :math:`p > 1` worker,
- :math:`T_1(p)` is the execution time on one worker,
- :math:`\alpha` is the fraction of the problem that can be parallelized.
- :math:`S(p)` is the speedup with :math:`p` workers

Additionally:

.. math:: E(p) = \frac{S(p)}{p} = \frac{1 - \alpha}{p} + \alpha

This expression represents the parallel efficiency :math:`E(p)` and is defined in terms of speedup :math:`S(p)` and the number of resources :math:`p`. 
It shows that as the number of resources approaches infinity, the efficiency goes towards the parallel fraction :math:`\alpha`.

Fazit (hats sich gelohnt?)
--------------------------
können wir noch nicht

References
----------

.. [1] Ara cluster specifications: https://wiki.uni-jena.de/pages/viewpage.action?pageId=22453005 (04.02.2024)
.. [2] Intel Xeon Gold 6140 specifications: https://en.wikichip.org/wiki/intel/xeon_gold/6140 (04.02.2024)