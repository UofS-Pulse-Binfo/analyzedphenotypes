Configuration
=============

Chado uses controlled vocabulary extensively to allow for flexible storing of data. As such this module supports that flexibility to ensure that regardless of the type used for your data, this module will still be able to navigate the necessary relationships and interpret your types.

Controlled Vocabulary
---------------------

A container of terms where each term is a phenotypic trait that can be measured in your species of interest. This controlled vocabulary should be specific to a given genus and each term will become a trait page on your Tripal site. If you do not already have a trait vocabulary, you can create it and add terms upfront and/or automatically on upload of phenotypic data (add link to create cv).
             
Database
--------

Chado requires a "database" container to be associated with all controlled vocabularies. Please select the "database" container you would like to be associated with your trait vocabulary. If needed, create one by clicking this link (add link to phenotype page)

Ontology
--------

Our experience with breeders has led us to recommend using the trait names your breeder(s) already use in the Trait Vocabulary and then linking them to a more generic crop ontology such as those provided by cropontology.org to facilitate sharing. If you decide to go this route, you can set the species specific crop ontology here and on upload suitable terms will be suggested based on pattern matching.
       
Allow New Traits
----------------

Allow new terms to be added to the Controlled Vocabulary during upload option will allow and prevent new records to be inserted into the Controlled Vocabulary configuration. This option can be used to limit the number of traits that user can upload data to a given experiment.
       
Controlled Vocabulary Terms
---------------------------

Chado uses controlled vocabularies extensively to allow for flexible storing of data. As such, this module supports that flexibility to ensure that you have the ability to choose the terms that best support your data.

