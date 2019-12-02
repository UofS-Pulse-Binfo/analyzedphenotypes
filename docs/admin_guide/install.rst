Installation
============

This module requires the following system be setup prior to installation:

- `Drupal 7 <https://www.drupal.org/>`_
- `Tripal 3.x <http://tripal.info/>`_
- `PostgresSQL 9.3 <https://www.postgresql.org/>`_

Additionally, the following extension modules and libraries are pre-requisites:
Unpack the following in your ``sites/all/modules`` directory:

- `Tripal Download API <https://github.com/tripal/trpdownload_api>`_
- `Tripal D3.js <https://github.com/tripal/tripald3>`_

and unpack these libraries in the ``sites//all/libraries`` directory:

- `PHP Excel Writer Libraries <https://github.com/SystemDevil/PHP_XLSXWriter_plus>`_
- `D3 JavaScript Library <https://github.com/d3/d3/releases/download/v3.5.14/d3.zip>`_

Quickstart
------------

1. Install pre-requisites (see above).
2. Download this module into your ``sites/all/modules`` directory and enable it.
3. Set-up ontology terms at Admin > Tripal > Extensions > Analyzed Phenotypes > Setup Ontologies.
4. Upload data at Admin > Tripal > Data Loaders > Phenotypic Data Importer.
