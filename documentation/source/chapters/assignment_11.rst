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

Einführung MPI
--------------

Erklären was MPI ist (mehrere Prozesse laufen echt Parallel, alle führen gesamten Code aus)
MPI ist message parsing interface - parallele prozesse kommunizieren untereinander
verwendet um compute domain aufzuteilen und Rechenlast zu verringern / compute time zu verringern

During parallelization, several processes are executed in parallel. If not further specified, each process executes the entire code.

However, if there are dependencies between data on different processes, there needs to be a way to communicate this data between the processes.
oder
In order to parallelize a program, you need a way to transfer data between the parallel running processes for critical sections or data dependencies.

This is where MPI comes into play. Each process executes the program itself and is initially independent of the others. 
This means that every process has its own address space and therefore does not share a global address space.
MPI means nothing other than Message Parsing Interface. It is used to move data from the address space of one process to that of another (through cooperative operations on each process).

For example Send Data ... to process ... Beispiel

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

In order to make the cache usage of our solver more efficient, we first looked at the cache specifications of the ARA cluster.

.. warning::

    Hier bitte Cache Specs von ARA

    16 Hadoop-Knoten mit jeweils:

    - 36 CPU-Kernen (2x Intel Xeon Gold 6140 18 Core 2,3 Ghz)
    - 192 GB Arbeitsspeicher
    - einer lokalen SSD

    L1 = 1.125 MiB	
        - L1I	576 KiB	18x32 KiB	8-way set associative	 
        - L1D	576 KiB	18x32 KiB	8-way set associative	write-back

    L2 = 18 MiB
 	 	- 18x1 MiB	16-way set associative	write-back
    
    L3 = 24.75 MiB	
 	 	- 18x1.375 MiB	11-way set associative	write-back

Anzahl an Sets in Cache: cache size / (block size * set size)

Cache line füllen und dann möglichst alle Operationen durchführen um capacity misses zu minimieren


Ergebnisse (Berechnungen und vid von Sim)
-----------------------------------------

Anmerken, welche Zwischenergebnisse wir hatten, die falsch waren? (z.B. kaputte Bathymetry etc.)

video von fehlschlägen

MPI läuft korrekt

video of Sim with [?] number of processes als Beweis

Speedup im vergleich zu kein MPI (hoffentlich positiv)

Eventuell Theoretische Speedups vs. praktische Speedups (wird vllt nicht möglich sein, weil wir die Daten dafür nicht haben)

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