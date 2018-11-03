
Data Storage
=============

Phenotypic data is stored in the existing Chado phenotype table with the addition of a project and stock foreign key. This allows phenotypic data measurements to be linked directly to the germplasm they were taken from rather then through the Chado nd_experiment tables providing a huge efficiency boost.

.. image:: https://cloud.githubusercontent.com/assets/1566301/26503442/eec1a3a6-41fd-11e7-9ca5-ea7316439643.png

This allows the trait (attr_id), measurement (value or cvalue_id), germplasm (stock_id) combination for a given project (project_id) to be stored as a single record. The location, year, data collector, etc for that data point are then stored in the phenotypeprop table.

