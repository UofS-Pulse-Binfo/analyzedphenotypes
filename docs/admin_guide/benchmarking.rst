
Benchmarking
============

We decided to do more formal benchmarking on two of our modules for the ISMB 2017 Conference. The details of such are included here for the benefit of the community :-). 

Caveats
-------

1. All timings were done on the same hardware (see specification below).
2. Queries were timed at the database level using PostgreSQL 9.4.10 EXPLAIN ANALYZE [query] and as such don't include rendering time in Tripal. Note: the addition of the analyze keyword ensures the query is actually run and the actual total time was reported.
3. The system the tests were run on includes a production Tripal site with small and uneven load. The tests were run 3 times on the same day over the span of at least 4 hours to help mitigate the differences in load.
4. Datasets are computationally derived with no missing data points.

Timings
-------

Timings were done on July 18,2017

+---------+-----------------------+-------------+-------------+------------+-----------+
| Dataset | Query                 | Rep1        | Rep2        | Rep3       | Average   |
+=========+=======================+=============+=============+============+===========+
| #1      | Quantitative Mview    | 32.709 ms   | 25.628 ms   | 25.981 ms  | 28.106 ms |
+---------+-----------------------+-------------+-------------+------------+-----------+
| #1      | Quantitative Directly | 1167.909 ms | 1159.963 ms | 1158.73 ms | 1162.2 ms |
+---------+-----------------------+-------------+-------------+------------+-----------+
| #1      | Summary               | 0.011 ms    | 0.004 ms    | 0.003 ms   | 0.006 ms  |
+---------+-----------------------+-------------+-------------+------------+-----------+

- See "Datasets" for a description of the datasets the tests were run on and how they were generated.
- See "Queries" section below for the exact queries executed.
- See "Hardware" section for the specification of the database server all tests were run on.

Datasets
--------

The queries were tested on two phenotypic datasets with different composition. Both datasets were generated using the `Generate Tripal Data Drush module <https://github.com/UofS-Pulse-Binfo/generate_trpdata>`_; specifically, the drush generate-phenotypes command. While the data is computationally derived, it does attempt to simulate real data by choosing the range of values for each trait and then generating quantitative values along a normal distribution. Furthermore, it is ensures that replicate values are within 3 units of each other.

+------------+-------+-----------+-----------+-------------------------------------+
| Name       | Trait | SiteYears | Germplasm | Measurements (Averaged across reps) |
+============+=======+===========+===========+=====================================+
| Dataset #1 | 100   | 100       | 4500      | 135 million                         |
+------------+-------+-----------+-----------+-------------------------------------+
| Dataset #2 | 100   | 10,000    | 45        | 135 million                         |
+------------+-------+-----------+-----------+-------------------------------------+

Queries
-------

The queries executed represent those used to summarize phenotypic data results. Keep in mind that the results from the queries may be further processed before display and that times reported here do not include render times as stated in the caveats section above.

Quantitative Measurement Distribution
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This is the query executed to extract the quantitative data collected for a single trait within a single experiment. The data retrieved represents pre-computed means per germplasm and site-year combination for a given trait (denoted :trait_id) and experiment (denoted :project_id).

.. code:: sql

  SELECT location, year, stock_name, mean
  FROM chado.mview_phenotype
  WHERE experiment_id=:project_id AND trait_id=:trait_id


This query is made much simpler thanks to the use of a materialized view. For context, the following query is used to generate the materialized view:

.. code:: sql

  SELECT
    o.genus as organism_genus,
    trait.cvterm_id as trait_id,
    trait.name as trait_name,
    proj.project_id as project_id,
    proj.name as project_name,
    loc.value as location,
    yr.value as year,
    s.stock_id as germplasm_id,
    s.name as germplasm_name,
    avg( CAST(p.value as FLOAT) ) as mean
  FROM chado.phenotype p
    LEFT JOIN chado.cvterm trait ON trait.cvterm_id=p.attr_id
    LEFT JOIN chado.project proj USING(project_id)
    LEFT JOIN chado.stock s USING(stock_id)
    LEFT JOIN chado.organism o ON o.organism_id=s.organism_id
    LEFT JOIN chado.phenotypeprop loc ON loc.phenotype_id=p.phenotype_id 
      AND loc.type_id IN (SELECT cvterm_id FROM chado.cvterm WHERE name='Location')
    LEFT JOIN chado.phenotypeprop yr ON yr.phenotype_id=p.phenotype_id 
      AND yr.type_id IN (SELECT cvterm_id FROM chado.cvterm WHERE name='Year')
  GROUP BY trait.cvterm_id, trait.name, proj.project_id, proj.name, loc.value, yr.value, s.stock_id, s.name, o.genus;


Experiment Summary
^^^^^^^^^^^^^^^^^^

This is the query executed on the main phenotype page which summarizes how many traits, experiments, unique site-years and measurements (averaged across reps) in the current Tripal site broken down by crop/organism. This query is greatly improved by the use of a materialized view.

.. code:: sql

  SELECT * FROM chado.mview_phenotype_summary;

System Specification
--------------------

Our Production Tripal site is setup on a dedicated two-box system (webserver + database server) with Apache + PHP installed on the first box and PostgreSQL installed on the second box. All testing for this benchmarking was done on a clean Tripal v3 site setup on the same two boxes in order to show queries time on a Production Server versus a less powerful Development server.

- RAID 10 configuration
- Debian GNU/Linux 8.7 (jessie)
- PostgreSQL 9.4.10
- Minimal PostgreSQL configuration tuning
- Hardware Specification (Database Server only)

  - Lenovo X3650 M5 2U Rackmount
  - Server 2x Xeon 6C E52643 V3 3.4GHz
  - 128GB RAM (8x 16GB TruDDR4 Memory (2Rx4, 1.2V) LP RDIMM) 1x ServeRAID M5210 Controller w/ 1GB Flash/RAID 5 Upgrade
  - 8x 600GB 15K 6Gbps SAS 2.5in G3HS HDD
  - Redundant Power Supplies
  - 4x 1GbE Onboard Ethernet

