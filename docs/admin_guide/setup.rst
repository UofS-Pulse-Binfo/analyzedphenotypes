Set-up Ontologies
===================

Trait Ontologies
------------------

When collecting phenotypic data, it is very important to ensure you use the same methodology including units across replicates. If, for example, you were to measure plant height in inches one year and centimetres in another you would not be able to combine the data for analysis. The same is true if you stretch the plant to measure it's height in one location and in the next you do not. For this reason, this module attaches each measurement to
 - the trait it was measuring (e.g. plant height)
 - the method used to measure it (e.g. stretching the plant)
 - the unit it was measured with (e.g. centimetres)

The trait, method and unit are all stored as controlled vocabulary terms with each in it's own ontology. The Trait Ontologies fieldset in the Set-up Ontologies form allows you to set these ontologies.

There are two schools of thought when it comes to storing phenotypic data:

1. Use published ontologies directly
2. Create custom controlled vocabularies which link to published ontologies.

This module supports both methodologies although it caters more to the second one. This is due to my experience working directly with both breeder's and researchers who are very particular in how traits are named. By using the same names they use, we ensure we are accurately capturing the trait which was measured and by linking that trait to the correct published ontologies (preferably with input from the data collector) we facilitate sharing of data. In my opinion this is the "best of both worlds" so to speak.

Controlled Vocabulary Terms
-----------------------------

Chado uses controlled vocabulary extensively to allow for flexible storing of data. As such this module supports that flexibility to ensure that regardless of the types used for your data, this module will still be able to navigate the necessary relationships and interpret the data.

To provide ease of use, we have already chosen a set of controlled vocabulary terms and inserted them by default. This makes this portion of the set-up optional.

.. warning:: Once you upload data, you can no longer change these terms.
