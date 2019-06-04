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

  // Because we are expecting data housed in Chado we can use the 'data_table'
  // property of the bundle to determine if this bundle is really the one
  // we want the field to be associated with.
  if (isset($bundle->data_table) AND ($bundle->data_table == 'cvterm')) {

    // Trait Method Field.
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
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );

    // Trait Experiments Field.
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
  }

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
        'term_vocabulary' => 'local',
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

  }

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
  }

  return $instances;
}
