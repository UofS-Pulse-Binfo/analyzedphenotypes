[![Build Status](https://travis-ci.org/UofS-Pulse-Binfo/analyzedphenotypes.svg?branch=7.x-3.x)](https://travis-ci.org/UofS-Pulse-Binfo/analyzedphenotypes)
# Analyzed Phenotypes
This module provides support and visualization for partially analyzed data stored in a modified GMOD Chado schema. It is meant to support **large scale phenotypic data** through backwards compatible improvements to the Chado schema including the addition of a project and stock foreign key to the existing phenotype table, optimized queries and well-choosen indexes. For benchmarking demonstrating the efficiency of this module, see our [wiki](https://github.com/UofS-Pulse-Binfo/analyzedphenotypes/wiki/Benchmarking).

## Dependencies
1. Tripal 3.x
2. Tripal Download API
3. PostgreSQL 9.3 (9.4 recommended)

## Installation
1. Install the dependencies listed above.
2. Install this module as you would any other Drupal module.

## Features 
### Release 1.0
- Specialized upload form for end users to submit filtered phenotypic data.
- Summary page showing how many traits, experiments, germplasm, etc. have phenotypic data.
- Dynamic quantitative trait chart summarizing the data for a given trait-project combination. This chart keeps site-years seperate but averages replicates.
- Specialized, permission controlled download form for end users to extract ether the raw, filtered or replicate-averaged phenotypic data.

### Release 2.0
- All the functionality mentioned in Release 1.0.
- Dynamic qualitative trait chart summarizing the data for a given trait-project combination. This chart also keeps site-years seperate but averages replicates.
- Trait Tripal Content Type providing pages for each Trait (One content type per organism/crop supported by your site). Include a number of specialized Tripal 3 Fields including the following functionality:
   - describe the trait, 
   - link to ontology terms (e.g. crop ontologies, plant trait ontology), 
   - define the method and units used to collect the data,
   - list experiments and site years with data for this trait,
   - summarize observed values per experiment (same quantitative trait chart released in 1.0 but embedded on trait pages)
   - list mean/min/max for each germplasm, site year, experiment combination
- Additional Tripal 3 Fields including
   - Project Phenotype Summary: provides a summary of all phenotypic data associated with a given project.
   - Germplasm Phenotype Summary: provides a summary of all phenotypic data collected from a given germplasm.
- Integration of all fields with Tripal 3 web services allowing you to share your genotypic data with other groups.

## Data Storage
Phenotypic data is stored in the existing Chado phenotype table with the addition of a project and stock foreign key. This allows phenotypic data measurements to be linked directly to the germplasm they were taken from rather then through the Chado nd_experiment tables providing a huge efficiency boost.
![database schema diagram](https://cloud.githubusercontent.com/assets/1566301/26503442/eec1a3a6-41fd-11e7-9ca5-ea7316439643.png)

This allows the trait (attr_id), measurement (value or cvalue_id), germplasm (stock_id) combination for a given project (project_id) to be stored as a single record. The location, year, data collector, etc for that data point are then stored in the phenotypeprop table.
