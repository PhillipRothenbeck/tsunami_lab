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
Moritz
Erklären was MPI ist (mehrere Prozesse laufen echt Parallel, alle führen gesamten Code aus)
MPI ist message parsing interface - parallele prozesse kommunizieren untereinander
verwendet um compute domain aufzuteilen und Rechenlast zu verringern / compute time zu verringern


Was haben wir gemacht? (technisch)
----------------------------------
Alle
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



Ergebnisse (Berechnungen und vid von Sim)
-----------------------------------------
können wir noch nicht

Fazit (hats sich gelohnt?)
--------------------------
können wir noch nicht