Contributing
==============

We’re excited to work with you! Post in the issues queue with any questions, feature requests, or proposals.

Automated Testing
--------------------

This module uses `Tripal Test Suite <https://tripaltestsuite.readthedocs.io/en/latest/installation.html#joining-an-existing-project>`_. To run tests locally:

.. code:: bash

  cd MODULE_ROOT
  composer up
  ./vendor/bin/phpunit

This will run all tests associated with the Analyzed Phenotypes extension module. If you are running into issues, this is a good way to rule out a system incompatibility.

.. warning::

  It is highly suggested you ONLY RUN TESTS ON DEVELOPMENT SITES. We have done our best to ensure that our tests clean up after themselves; however, we do not guarantee there will be no changes to your database.

Manual Testing (Demonstration)
--------------------------------

We have provided a `Tripal Test Suite Database Seeder <https://tripaltestsuite.readthedocs.io/en/latest/db-seeders.html>` to make development and demonstration of functionality easier. To populate your development database with fake phenotypic data:

1. Install this module according to the instructions in the administration guide.
2. Create an organism (genus: Tripalus; species: databasica)
3. Configure the module terms by Navigating to Administration » Tripal » Extensions » Analyzed Phenotypes » Set-up Ontologies and click "Save term configuration" at the bottom of the page.
4. Run the database seeder to populate the database using the following commands:

  .. code::

    cd MODULE_ROOT
    composer up
    ./vendor/bin/tripaltest db:seed PhenotypeSeeder

4. Populate the materialized views by going to Administration » Tripal » Data Storage » Chado » Materialized Views and clicking "Populate" beside ``mview_phenotype`` and ``mview_phenotype_summary``. Finally run the Tripal jobs submitted.
5. Create the trait content type by going to Administration » Structure » Tripal Content Types » Add Tripal Content Type. We suggest the following values:

  - Chado Base Table: cvterm
  - Use a Parent Chado cvterm?	No. All records belong to a single controlled vocabulary.
  - Restrict to Vocabulary: [get value from set-up ontologies page]

6. To play with trait pages go to Administration » Structure » Tripal Content Types » Publish Tripal Content and select the term used in step 5 to create pages.

.. warning::

  NEVER run database seeders on production sites. They will insert fictitious data into Chado.

Stress Testing
---------------

We have also provided a `Tripal Test Suite Database Seeder <https://tripaltestsuite.readthedocs.io/en/latest/db-seeders.html>` to be used for stress testing this module. It inserts 3 billion phenotype records including associated metadata. To populate your development database with this fake phenotypic dataset:

1. Install this module according to the instructions in the administration guide.
2. Run the database seeder to populate the database using the following commands:

  .. code::

    cd MODULE_ROOT
    composer up
    ./vendor/bin/tripaltest db:seed Massive2500MillionPhenotypeSeeder

3. Populate the materialized views by going to Administration » Tripal » Data Storage » Chado » Materialized Views and clicking "Populate" beside ``mview_phenotype`` and ``mview_phenotype_summary`` and run the Tripal jobs submitted.
4. Run the timings script to determine how specific queries in the module may respond to the current dataset.

  .. code::

    cd MODULE_ROOT
    drush php-script tests/massivePhenotypesTimings.php

  .. warning::

    This script will take at least 4 hours to run due to 9 spaced replicates. Additionally, the execution time will increase depending on how your system handles these queries.

5. You can also stress test through the UI after you configure the ``Tripalus`` genus to the correct controlled vocabularies.
