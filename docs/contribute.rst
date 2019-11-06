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

1. Create an organism (genus: Tripalus; species: databasica)
2. Configure the module terms by Navigating to Administration » Tripal » Extensions » Analyzed Phenotypes » Set-up Ontologies and click "Save term configuration" at the bottom of the page.
3. Run the database seeder to populate the database using the following commands:

  .. code::

    cd MODULE_ROOT
    composer up
    ./vendor/bin/tripaltest db:seed PhenotypeSeeder

4. Populate the materialized views by going to Administration » Tripal » Data Storage » Chado » Materialized Views and clicking "Populate" beside ``mview_phenotype`` and ``mview_phenotype_summary``. Finally run the Tripal jobs submitted.

.. warning::

  NEVER run database seeders on production sites. They will insert fictitious data into Chado.
