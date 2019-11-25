<?php
/**
 * @file
 * Contains all field specific code outside the classes.
 */

/**
 * Implements hook_bundle_fields_info().
 *
 * This hook tells Drupal/Tripal about your new field type. Make sure you've created the
 * field (handles basic storage of your data), widget (provides user UI to set data),
 * and formatter (describes display of data on Entity Page) classes. These should be
 * located in the following directory: [your module]/includes/TripalFields/[classname].inc
 * with one file per class. Your field name should be [cv name]__[cvterm name] and the
 * classes should be named [field name], [field_name]_widget, [field name]_formatter
 * for the field, widget and formatter respectively. MAKE SURE YOU'VE CLEARED THE CACHE
 * SINCE ADDING THESE FILES so Tripal magic can find them or the following will fail.
 *
 * @param $entity_type
 *   This should be 'TripalEntity' for all Tripal Content.
 * @param $bundle
 *   This object describes the Type of Tripal Entity (e.g. Organism or Gene) this hook is
 *   being called for. However, since this hook creates field types (by definition not
 *   tied to a specific Tripal Content Type (bundle)) and since a field type will only be
 *   created if it doesn't already exist, this parameter doesn't actually matter.
 *   NOTE: If you do need to determine the bundle in this hook, we suggest inspecting
 *   the data_table since the label can be changed by site administrators.
 *
 * @return
 *   An array of field definitions. Each field in this array will be created if it
 *   doesn't already exist. To trigger create of fields when developing call
 *   tripal_refresh_bundle_fields() for the specific bundle.
 */
function analyzedphenotypes_bundle_fields_info($entity_type, $bundle) {
  $fields = array();


  // IN TRAIT PAGE ONLY:
  // Because we are expecting data housed in Chado we can use the 'data_table'
  // property of the bundle to determine if this bundle is really the one
  // we want the field to be associated with.
  if (isset($bundle->data_table) AND ($bundle->data_table == 'cvterm')) {

    // Trait Method Field.
    // Cardinality set to unlimited to support multiple entries.
    tripal_insert_cvterm(array(
      'id' => 'NCIT:C71460',
      'name' => 'method',
      'cv_name' => 'NCIT',
      'definition' => 'A means, manner of procedure, or systematic course of action that have to be performed in order to accomplish a particular goal.',
    ));
    $field_name = 'ncit__method';
    $field_type = 'ncit__method';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => FIELD_CARDINALITY_UNLIMITED,
      'locked' => FALSE,
      'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );

    // Trait Experiments Field.
    // Cardinality set to unlimited to support multiple entries.
    tripal_insert_cvterm(array(
      'id' => 'SIO:001066',
      'name' => 'study',
      'cv_name' => 'SIO',
      'definition' => 'A study is a process that realizes the steps of a study design.',
    ));
    $field_name = 'sio__study';
    $field_type = 'sio__study';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => FIELD_CARDINALITY_UNLIMITED,
      'locked' => FALSE,
      'storage' => array(
	      'type' => 'field_chado_storage',
      ),
    );

    // Germplasm Phenotype Search.
    tripal_insert_cvterm(array(
      'id' => 'NCIT:data',
      'name' => 'data',
      'cv_name' => 'NCIT',
      'definition' => 'Used in AP Germplasm Phenotype search field - Data',
    ));
    $field_name = 'ncit__data';
    $field_type = 'ncit__data';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );

    // Number of germplasm.
    tripal_insert_cvterm(array(
      'id' => 'local:number_of_germplasm',
      'name' => 'number of germplasm',
      'cv_name' => 'local',
      'definition' => 'Used in AP Summary fields - Number of germplasm',
    ));
    $field_name = 'local__number_of_germplasm';
    $field_type = 'local__number_of_germplasm';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => array(
	      'type' => 'field_chado_storage',
      ),
    );

    // Number of values recorded.
    tripal_insert_cvterm(array(
      'id' => 'local:number_of_values_recorded',
      'name' => 'number of values recorded',
      'cv_name' => 'local',
      'definition' => 'Used in AP Summary fields - Number of values recorded',
    ));
    $field_name = 'local__number_of_values_recorded';
    $field_type = 'local__number_of_values_recorded';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => array(
	      'type' => 'field_chado_storage',
      ),
    );

    // Phenotype Ontology.
    tripal_insert_cvterm(array(
      'id' => 'NCIT:C52469',
      'name' => 'Synonym',
      'cv_name' => 'NCIT',
      'definition' => 'A word having the same or nearly the same meaning as another word or other words in a language; a word or an expression that serves as a figurative or symbolic substitute for another',
    ));
    $field_name = 'ncit__synonym';
    $field_type = 'ncit__synonym';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => array(
	      'type' => 'field_chado_storage',
      ),
    );

    // Phenotype Image
    tripal_insert_cvterm(array(
      'id' => 'NCIT:C48179',
      'name' => 'Image',
      'cv_name' => 'NCIT',
      'definition' => 'Phenotype Image.',
    ));
    $field_name = 'ncit__image';
    $field_type = 'ncit__image';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => array(
	      'type' => 'field_chado_storage',
      ),
    );
  }

  // IN GERMPLASM PAGE ONLY:
  if (isset($bundle->data_table) AND ($bundle->data_table == 'stock')) {
    // Number of traits.
    tripal_insert_cvterm(array(
      'id' => 'local:number_of_traits',
      'name' => 'number of traits',
      'cv_name' => 'local',
      'definition' => 'Used in AP Summary fields - Number of traits',
    ));
    $field_name = 'local__number_of_traits';
    $field_type = 'local__number_of_traits';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => array(
	      'type' => 'field_chado_storage',
      ),
    );

    // Magnitude of Phenotypes
    tripal_insert_cvterm(array(
      'id' => 'local:magnitude_of_phenotypes',
      'name' => 'magnitude of phenotypes',
      'cv_name' => 'local',
      'definition' => 'Magnitude of Phenotypes.',
    ));
    $field_name = 'local__magnitude_of_phenotypes';
    $field_type = 'local__magnitude_of_phenotypes';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => array(
	      'type' => 'field_chado_storage',
      ),
    );

  }

  // IN TRAIT, GERMPLASM AND PROJECT PAGE:
  // Trait Distribution Chart Field.
  if (isset($bundle->data_table) AND
    (($bundle->data_table == 'cvterm') OR ($bundle->data_table == 'project') OR ($bundle->data_table == 'stock'))) {

    tripal_insert_cvterm(array(
      'id' => 'HP:0003812',
      'name' => 'phenotypic variability',
      'cv_name' => 'HP',
      'definition' => 'A variability of phenotypic features.',
    ));
    $field_name = 'hp__phenotypic_variability';
    $field_type = 'hp__phenotypic_variability';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );

    if ($bundle->data_table == 'cvterm' || $bundle->data_table == 'stock') {
      // Field child_studies to go into stock page only.
      if ($bundle->data_table == 'stock') {
        // Child Studies.
        tripal_insert_cvterm(array(
          'id' => 'local:child_studies',
          'name' => 'child_studies',
          'cv_name' => 'local',
          'definition' => 'Used in AP Summary fields - Child Studies',
        ));

        $field_name = 'local__child_studies';
        $field_type = 'local__child_studies';
        $fields[$field_name] = array(
          'field_name' => $field_name,
          'type' => $field_type,
          'cardinality' => 1,
          'locked' => FALSE,
          'storage' => array(
            'type' => 'field_chado_storage',
          ),
        );
      }

      // Number of experiments.
      tripal_insert_cvterm(array(
        'id' => 'local:number_of_experiments',
        'name' => 'number of experiments',
        'cv_name' => 'local',
        'definition' => 'Used in AP Summary fields - Number of experiments',
      ));
      $field_name = 'local__number_of_experiments';
      $field_type = 'local__number_of_experiments';
      $fields[$field_name] = array(
        'field_name' => $field_name,
        'type' => $field_type,
        'cardinality' => 1,
        'locked' => FALSE,
        'storage' => array(
          'type' => 'field_chado_storage',
        ),
      );

      // Number of years.
      tripal_insert_cvterm(array(
        'id' => 'local:number_of_years',
        'name' => 'number of years',
        'cv_name' => 'local',
        'definition' => 'Used in AP Summary fields - Number of years',
      ));
      $field_name = 'local__number_of_years';
      $field_type = 'local__number_of_years';
      $fields[$field_name] = array(
        'field_name' => $field_name,
        'type' => $field_type,
        'cardinality' => 1,
        'locked' => FALSE,
        'storage' => array(
          'type' => 'field_chado_storage',
        ),
      );

      // Number of locations.
      tripal_insert_cvterm(array(
        'id' => 'local:number_of_locations',
        'name' => 'number of locations',
        'cv_name' => 'local',
        'definition' => 'Used in AP Summary fields - Number of locations',
      ));
      $field_name = 'local__number_of_locations';
      $field_type = 'local__number_of_locations';
      $fields[$field_name] = array(
        'field_name' => $field_name,
        'type' => $field_type,
        'cardinality' => 1,
        'locked' => FALSE,
        'storage' => array(
          'type' => 'field_chado_storage',
        ),
      );
    }
  }

  return $fields;
}

/**
 * Implements hook_bundle_instances_info().
 *
 * This hook tells Drupal/Tripal to create a field instance of a given field type on a
 * specific Tripal Content type (otherwise known as the bundle). Make sure to implement
 * hook_bundle_create_fields() to create your field type before trying to create an
 * instance of that field.
 *
 * @param $entity_type
 *   This should be 'TripalEntity' for all Tripal Content.
 * @param $bundle
 *   This object describes the Type of Tripal Entity (e.g. Organism or Gene) the field
 *   instances are being created for. Thus this hook is called once per Tripal Content Type on your
 *   site. The name of the bundle is the machine name of the type (e.g. bio_data_1) and
 *   the label of the bundle (e.g. Organism) is what you see in the interface. Since the
 *   label can be changed by site admin, we suggest checking the data_table to determine
 *   if this is the entity you want to add field instances to.
 * @return
 *   An array of field instance definitions. This is where you can define the defaults
 *   for any settings you use in your field. Each entry in this array will be used to
 *   create an instance of an already existing field.
 */
function analyzedphenotypes_bundle_instances_info($entity_type, $bundle) {
  $instances = array();

  // IN TRAITS PAGE ONLY:
  if (isset($bundle->data_table) AND ($bundle->data_table == 'cvterm')) {

    // Trait Methods Field.
    $field_name = 'ncit__method';
    $field_type = 'ncit__method';
    $instances[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Collection Methods',
      'description' => 'The method with which the given phenotypic data were collected.',
      'required' => FALSE,
      'settings' => array(
        'term_vocabulary' => 'NCIT',
        'term_name' => 'method',
        'term_accession' => 'C71460',
        'auto_attach' => FALSE,
        'chado_table' => $bundle->data_table,
        'chado_column' => 'cvterm_id',
        'base_table' => $bundle->data_table,
      ),
      'widget' => array(
        'type' => 'ncit__method_widget',
        'settings' => array(),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'ncit__method_formatter',
          'settings' => array(),
        ),
      ),
    );

    // Trait Experiments Field.
    $field_name = 'sio__study';
    $field_type = 'sio__study';
    $instances[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Phenotyping Experiments',
      'description' => 'Lists experiments measuring the given trait.',
      'required' => FALSE,
      'settings' => array(
        'term_vocabulary' => 'SIO',
        'term_name' => 'study',
        'term_accession' => '001066',
        'auto_attach' => FALSE,
        'chado_table' => $bundle->data_table,
        'chado_column' => 'project_id',
        'base_table' => $bundle->data_table,
      ),
      'widget' => array(
        'type' => 'sio__study_widget',
        'settings' => array(),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'sio__study_formatter',
          'settings' => array(),
        ),
      ),
    );

    // Germplasm Phenotype Search Field.
    $field_name = 'ncit__data';
    $field_type = 'ncit__data';
    $instances[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
       'label' => 'Germplasm Phenotype Search',
      'definition' => 'Used in AP Germplasm Phenotype search field - Data',
      'required' => FALSE,
      'settings' => array(
        'term_vocabulary' => 'NCIT',
        'term_name' => 'data',
        'term_accession' => 'data',
        'auto_attach' => FALSE,
        'chado_table' => $bundle->data_table,
        'chado_column' => 'cvterm_id',
        'base_table' => $bundle->data_table,
      ),
      'widget' => array(
        'type' => 'ncit__data_widget',
        'settings' => array(),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'ncit__data_formatter',
          'settings' => array(),
        ),
      ),
    );

    // Number of Germplasm.
    $field_name = 'local__number_of_germplasm';
    $field_type = 'local__number_of_germplasm';
    $instances[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Phenotype Summary: Number of Germplasm',
      'description' => 'Used in AP Summary fields - Number of germplasm.',
      'required' => FALSE,
      'settings' => array(
        'term_vocabulary' => 'local',
        'term_name' => 'number of germplasm',
        'term_accession' => 'number_of_germplasm',
        'auto_attach' => FALSE,
        'chado_table' => $bundle->data_table,
        'chado_column' => 'cvterm_id',
        'base_table' => $bundle->data_table,
      ),
      'widget' => array(
        'type' => 'local__number_of_germplasm_widget',
        'settings' => array(),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'local__number_of_germplasm_formatter',
          'settings' => array(),
        ),
      ),
    );

    // Number of Values Recorded.
    $field_name = 'local__number_of_values_recorded';
    $field_type = 'local__number_of_values_recorded';
    $instances[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Phenotype Summary: Number of Values Recorded',
      'description' => 'Used in AP Summary fields - Number of values recorded.',
      'required' => FALSE,
      'settings' => array(
        'term_vocabulary' => 'local',
        'term_name' => 'number of values recorded',
        'term_accession' => 'number_of_values_recorded',
        'auto_attach' => FALSE,
        'chado_table' => $bundle->data_table,
        'chado_column' => 'cvterm_id',
        'base_table' => $bundle->data_table,
      ),
      'widget' => array(
        'type' => 'local__number_of_values_recorded_widget',
        'settings' => array(),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'local__number_of_values_recorded_formatter',
          'settings' => array(),
        ),
      ),
    );

    // Phenotype Ontology - Synonym.
    $field_name = 'ncit__synonym';
    $field_type = 'ncit__synonym';
    $instances[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Phenotype Ontology',
      'description' => 'Phenotype Ontology.',
      'required' => FALSE,
      'settings' => array(
        'term_vocabulary' => 'NCIT',
        'term_name' => 'Synonym',
        'term_accession' => 'C52469',
        'auto_attach' => FALSE,
        'chado_table' => $bundle->data_table,
        'chado_column' => $bundle->data_table . '_id',
        'base_table' => $bundle->data_table,
      ),
      'widget' => array(
        'type' => 'ncit__synonym_widget',
        'settings' => array(),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'ncit__synonym_formatter',
          'settings' => array(),
        ),
      ),
    );

    // Phenotype Image.
    $field_name = 'ncit__image';
    $field_type = 'ncit__image';
    $instances[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Phenotype Image',
      'description' => 'Phenotype Image.',
      'required' => FALSE,
      'settings' => array(
        'term_vocabulary' => 'NCIT',
        'term_name' => 'Image',
        'term_accession' => 'C48179',
        'auto_attach' => FALSE,
        'chado_table' => $bundle->data_table,
        'chado_column' => 'cvterm_id',
        'base_table' => $bundle->data_table,
      ),
      'widget' => array(
        'type' => 'ncit__image_widget',
        'settings' => array(),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'ncit__image_formatter',
          'settings' => array(
            'image_stack' => 'Left to Right',
            'image_position' => 'Top to Bottom',
          ),
        ),
      ),
    );
  }

  // IN GERMPLASM PAGE ONLY:
  if (isset($bundle->data_table) AND ($bundle->data_table == 'stock')) {
    // Number of Values Recorded.
    $field_name = 'local__number_of_traits';
    $field_type = 'local__number_of_traits';
    $instances[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Phenotype Summary: Number of Traits',
      'description' => 'Used in AP Summary fields - Number of traits.',
      'required' => FALSE,
      'settings' => array(
        'term_vocabulary' => 'local',
        'term_name' => 'number of traits',
        'term_accession' => 'number_of_traits',
        'auto_attach' => FALSE,
        'chado_table' => $bundle->data_table,
        'chado_column' => $bundle->data_table . '_id',
        'base_table' => $bundle->data_table,
      ),
      'widget' => array(
        'type' => 'local__number_of_traits_widget',
        'settings' => array(),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'local__number_of_traits_formatter',
          'settings' => array(),
        ),
      ),
    );

    // Child Studies Field.
    $field_name = 'local__child_studies';
    $field_type = 'local__child_studies';
    $instances[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Phenotype Summary: Child Studies',
      'description' => 'Used in AP Summary fields - Child Studies.',
      'required' => FALSE,
      'settings' => array(
        'term_vocabulary' => 'local',
        'term_name' => 'child_studies',
        'term_accession' => 'child_studies',
        'auto_attach' => FALSE,
        'chado_table' => $bundle->data_table,
        'chado_column' => $bundle->data_table . '_id',
        'base_table' => $bundle->data_table,
      ),
      'widget' => array(
        'type' => 'local__child_studies_widget',
        'settings' => array(),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
           'type' => 'local__child_studies_formatter',
          'settings' => array(),
        ),
      ),
    );

    // Magnitude of phenotypes Field.
    $field_name = 'local__magnitude_of_phenotypes';
    $field_type = 'local__magnitude_of_phenotypes';
    $instances[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Magnitude of Phenotypes',
      'description' => 'Magnitude of phenotypic data.',
      'required' => FALSE,
      'settings' => array(
        'term_vocabulary' => 'local',
        'term_name' => 'magnitude of phenotypes',
        'term_accession' => 'magnitude_of_phenotypes',
        'auto_attach' => FALSE,
        'chado_table' => $bundle->data_table,
        'chado_column' => 'cvterm_id',
        'base_table' => $bundle->data_table,
      ),
      'widget' => array(
        'type' => 'local__magnitude_of_phenotypes_widget',
        'settings' => array(),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'local__magnitude_of_phenotypes_formatter',
          'settings' => array(),
        ),
      ),
    );
  }

  // IN TRAIT, GERMPLASM AND PROJECT PAGE:
  // Trait Distribution Plot Field.
  if (isset($bundle->data_table) AND
    (($bundle->data_table == 'cvterm') OR ($bundle->data_table == 'project') OR ($bundle->data_table == 'stock'))) {

    $field_name = 'hp__phenotypic_variability';
    $field_type = 'hp__phenotypic_variability';
    $instances[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Trait Distribution Plot',
      'description' => 'Embed the Phenotypic Trait distribution plot in Tripal content pages.',
      'required' => FALSE,
      'settings' => array(
        'term_vocabulary' => 'HP',
        'term_name' => 'phenotypic variability',
        'term_accession' => '0003812',
        'auto_attach' => FALSE,
        'chado_table' => $bundle->data_table,
        'chado_column' => $bundle->data_table . '_id',
        'base_table' => $bundle->data_table,
      ),
      'widget' => array(
        'type' => 'hp__phenotypic_variability_widget',
        'settings' => array(),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'hp__phenotypic_variability_formatter',
          'settings' => array(),
        ),
      ),
    );


    if ($bundle->data_table == 'cvterm' || $bundle->data_table == 'stock') {
      $field_name = 'local__number_of_experiments';
      $field_type = 'local__number_of_experiments';
      $instances[$field_name] =  array(
        'field_name' => $field_name,
        'entity_type' => $entity_type,
        'bundle' => $bundle->name,
        'label' => 'Phenotype Summary: Number of Experiments',
        'description' => 'Used in AP Summary fields - Number of experiments.',
        'required' => FALSE,
        'settings' => array(
          'term_vocabulary' => 'local',
          'term_name' => 'number of experiments',
          'term_accession' => 'number_of_experiments',
          'auto_attach' => FALSE,
          'chado_table' => $bundle->data_table,
          'chado_column' => $bundle->data_table . '_id',
          'base_table' => $bundle->data_table,
        ),
        'widget' => array(
          'type' => 'local__number_of_experiments_widget',
          'settings' => array(),
        ),
        'display' => array(
          'default' => array(
            'label' => 'hidden',
            'type' => 'local__number_of_experiments_formatter',
            'settings' => array(),
          ),
        ),
      );

      $field_name = 'local__number_of_locations';
      $field_type = 'local__number_of_locations';
      $instances[$field_name] =  array(
        'field_name' => $field_name,
        'entity_type' => $entity_type,
        'bundle' => $bundle->name,
        'label' => 'Phenotype Summary: Number of Locations',
        'description' => 'Used in AP Summary fields - Number of locations.',
        'required' => FALSE,
        'settings' => array(
          'term_vocabulary' => 'local',
          'term_name' => 'number of locations',
          'term_accession' => 'number_of_locations',
          'auto_attach' => FALSE,
          'chado_table' => $bundle->data_table,
          'chado_column' => $bundle->data_table . '_id',
          'base_table' => $bundle->data_table,
        ),
        'widget' => array(
          'type' => 'local__number_of_locations_widget',
          'settings' => array(),
        ),
        'display' => array(
          'default' => array(
            'label' => 'hidden',
            'type' => 'local__number_of_locations_formatter',
            'settings' => array(),
          ),
        ),
      );

      $field_name = 'local__number_of_years';
      $field_type = 'local__number_of_years';
      $instances[$field_name] =  array(
        'field_name' => $field_name,
        'entity_type' => $entity_type,
        'bundle' => $bundle->name,
        'label' => 'Phenotype Summary: Number of Years',
        'description' => 'Used in AP Summary fields - Number of years.',
        'required' => FALSE,
        'settings' => array(
          'term_vocabulary' => 'local',
          'term_name' => 'number of years',
          'term_accession' => 'number_of_years',
          'auto_attach' => FALSE,
          'chado_table' => $bundle->data_table,
          'chado_column' => $bundle->data_table . '_id',
          'base_table' => $bundle->data_table,
        ),
        'widget' => array(
          'type' => 'local__number_of_years_widget',
          'settings' => array(),
        ),
        'display' => array(
          'default' => array(
            'label' => 'hidden',
            'type' => 'local__number_of_years_formatter',
            'settings' => array(),
          ),
        ),
      );
    }
  }

  return $instances;
}
