[![Build Status](https://travis-ci.org/UofS-Pulse-Binfo/analyzedphenotypes.svg?branch=7.x-3.x)](https://travis-ci.org/UofS-Pulse-Binfo/analyzedphenotypes)
[![Documentation Status](https://readthedocs.org/projects/analyzedphenotypes/badge/?version=latest)](https://analyzedphenotypes.readthedocs.io/en/latest/?badge=latest)
![Tripal Dependency](https://img.shields.io/badge/tripal-%3E=3.0-brightgreen)
![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/UofS-Pulse-Binfo/analyzedphenotypes?include_prereleases)


# Analyzed Phenotypes
This module provides support and visualization for partially analyzed data stored in a modified GMOD Chado schema. It is meant to support **large scale phenotypic data** through backwards compatible improvements to the Chado schema including the addition of a project and stock foreign key to the existing phenotype table, optimized queries and well-choosen indexes. For benchmarking demonstrating the efficiency of this module, see our [documentation](https://analyzedphenotypes.readthedocs.io/en/latest/admin_guide/benchmarking.html).

## Dependencies

1. [Drupal 7](https://www.drupal.org/)
2. [Tripal 3.x](http://tripal.info/)
3. [Tripal Download API](https://github.com/tripal/trpdownload_api)
4. [PostgresSQL 9.3](https://www.postgresql.org/)
5. [PHP Excel Writer Libraries](https://github.com/SystemDevil/PHP_XLSXWriter_plus)
6. [D3 JavaScript Library](https://github.com/d3/d3/releases/download/v3.5.14/d3.zip)

## Installation
1. Install the dependencies listed above.
2. Install this module as you would any other Drupal module.

[See our documentation for more detailed installation instructions](https://analyzedphenotypes.readthedocs.io/en/latest/admin_guide/install.html).

## Features
- Specialized upload form for end users to submit filtered phenotypic data.
- Summary page showing how many traits, experiments, germplasm, etc. have phenotypic data.
- Dynamic quantitative trait chart summarizing the data for a given trait-project combination. This chart keeps site-years separate but averages replicates.
- Specialized, permission controlled download form for end users to extract replicate-averaged phenotypic data.
- Trait Tripal Content Type providing pages for each Trait (One content type per organism/crop supported by your site). Include a number of specialized Tripal 3 Fields including the following functionality:
   - define the method and units used to collect the data,
   - list experiments and site years with data for this trait,
   - summarize observed values per experiment (same quantitative trait chart released in 1.0 but embedded on trait pages)
- Additional Tripal 3 Fields including
   - Project Phenotype Summary: provides a summary of all phenotypic data associated with a given project.
   - Germplasm Phenotype Summary: provides a summary of all phenotypic data collected from a given germplasm.
- Integration of all fields with Tripal 3 web services allowing you to share your genotypic data with other groups.

## Documentation
Further documentation is available on [ReadtheDocs](https://analyzedphenotypes.readthedocs.io/en/latest/index.html).
