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

Was haben wir gemacht? (technisch)
----------------------------------
Alle

Ergebnisse (Berechnungen und vid von Sim)
-----------------------------------------
können wir noch nicht

Fazit (hats sich gelohnt?)
--------------------------
können wir noch nicht