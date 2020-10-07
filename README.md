![Tripal Dependency](https://img.shields.io/badge/tripal-%3E=3.0-brightgreen)
![Module is Generic](https://img.shields.io/badge/generic-tested%20manually-yellow)
![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/UofS-Pulse-Binfo/analyzedphenotypes?include_prereleases)

[![Build Status](https://travis-ci.org/UofS-Pulse-Binfo/analyzedphenotypes.svg?branch=7.x-3.x)](https://travis-ci.org/UofS-Pulse-Binfo/analyzedphenotypes)
[![Maintainability](https://api.codeclimate.com/v1/badges/201831f4d8bcca2e16c0/maintainability)](https://codeclimate.com/github/UofS-Pulse-Binfo/analyzedphenotypes/maintainability)
[![Test Coverage](https://api.codeclimate.com/v1/badges/201831f4d8bcca2e16c0/test_coverage)](https://codeclimate.com/github/UofS-Pulse-Binfo/analyzedphenotypes/test_coverage)

[![DOI](https://zenodo.org/badge/91645305.svg)](https://zenodo.org/badge/latestdoi/91645305)

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
- Specialized Tripal Importer for end users to submit filtered phenotypic data.
- Summary page showing how many traits, experiments, germplasm, etc. have phenotypic data.
- Dynamic trait distribution chart summarizing the data for a given trait-project combination. This chart keeps site-years separate but averages replicates. For quantitative data a violin plot shows the distribution and structure of these data and the qualitative data a histrogram is used.
- Specialized, permission controlled download form for end users to extract replicate-averaged phenotypic data.
- Trait Tripal Content Type providing pages for each Trait (One content type per organism/crop supported by your site). 
- Anumber of specialized Tripal 3 Fields which are [demonstrated in our documentation](https://analyzedphenotypes.readthedocs.io/en/latest/user_guide/fields.html).
- Integration of all fields with Tripal 3 web services allowing you to share your genotypic data with other groups.

## Documentation
Further documentation is available on [ReadtheDocs](https://analyzedphenotypes.readthedocs.io/en/latest/index.html).

## Funding
This work is supported by Saskatchewan Pulse Growers [grant: BRE1516, BRE0601], Western Grains Research Foundation, Genome Canada [grant: 8302, 16302], Government of Saskatchewan [grant: 20150331], and the University of Saskatchewan.

## Citation [![DOI](https://zenodo.org/badge/91645305.svg)](https://zenodo.org/badge/latestdoi/91645305)
Sanderson, L.A., Tan R. (2020). Analyyzed Phenotypes: Tripal support for analyzed phenotypic data including data loaders, exporters, trait pages and summaries on germplasm pages. Version 1.0. University of Saskatchewan, Pulse Crop Research Group, Saskatoon, SK, Canada. Zenodo, doi:10.5281/zenodo.4072330.
