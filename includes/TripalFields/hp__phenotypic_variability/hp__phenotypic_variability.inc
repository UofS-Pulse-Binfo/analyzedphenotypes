<?php
/**
 * @class
 * Purpose:
 *
 * Data:
 * Assumptions:
 */
class hp__phenotypic_variability extends ChadoField {

  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendant class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------

  // The default label for this field.
  public static $default_label = 'Phenotypic Data Distribution';

  // The default description for this field.
  public static $default_description = 'The distribution of phenotypic data for a given trait experiment combination.';

  // The default widget for this field.
  public static $default_widget = 'hp__phenotypic_variability_widget';

  // The default formatter for this field.
  public static $default_formatter = 'hp__phenotypic_variability_formatter';

  // The module that manages this field.
  // If no module manages the field (IE it's added via libraries)
  // set this to 'tripal_chado'
  public static $module = 'analyzedphenotypes';

  // A list of global settings. These can be accessed within the
  // globalSettingsForm.  When the globalSettingsForm is submitted then
  // Drupal will automatically change these settings for all fields.
  // Once instances exist for a field type then these settings cannot be
  // changed.
  public static $default_settings = array(
    'storage' => 'field_chado_storage',
     // It is expected that all fields set a 'value' in the load() function.
     // In many cases, the value may be an associative array of key/value pairs.
     // In order for Tripal to provide context for all data, the keys should
     // be a controlled vocabulary term (e.g. rdfs:type). Keys in the load()
     // function that are supported by the query() function should be
     // listed here.
     'searchable_keys' => array(),
  );

  // Indicates the download formats for this field.  The list must be the
  // name of a child class of the TripalFieldDownloader.
  public static $download_formatters = array(
     'TripalTabDownloader',
     'TripalCSVDownloader',
  );

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instance.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings = array(
    // The DATABASE name, as it appears in chado.db.  This also builds the link-out url.  In most cases this will simply be the CV name.  In some cases (EDAM) this will be the SUBONTOLOGY.
    'term_vocabulary' => 'HP',
    // The name of the term.
    'term_name' => 'phenotypic variability',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => '0003812',
    // Set to TRUE if the site admin is not allowed to change the term
    // type, otherwise the admin can change the term mapped to a field.
    'term_fixed' => FALSE,
    // Indicates if this field should be automatically attached to display
    // or web services or if this field should be loaded separately. This
    // is convenient for speed.  Fields that are slow should for loading
    // should have auto_attach set to FALSE so tha their values can be
    // attached asynchronously.
    'auto_attach' => FALSE,
    // The table in Chado that the instance maps to.
    'chado_table' => 'cvterm',
    // The column of the table in Chado where the value of the field comes from.
    'chado_column' => 'cvterm_id',
    // The base table.
    'base_table' => 'cvterm',
  );

  // A boolean specifying that users should not be allowed to create
  // fields and instances of this field type through the UI. Such
  // fields can only be created programmatically with field_create_field()
  // and field_create_instance().
  public static $no_ui = FALSE;

  // A boolean specifying that the field will not contain any data. This
  // should exclude the field from web services or downloads.  An example
  // could be a quick search field that appears on the page that redirects
  // the user but otherwise provides no data.
  public static $no_data = FALSE;

  /**
   * @see ChadoField::load()
   *
   */
  public function load($entity) {
    $field_name = $this->instance['field_name'];

    // What type are we on? trait, germplasm, experiment?
    // These can best be determined based on the base chado table.
    // How we load our data depends on the base table;
    // however, the end format of the data remains the same.
    switch ($entity->chado_table) {
      case 'cvterm':
        $entity->{$field_name}['und'] = $this->loadForTrait($entity);
        break;
      case 'project':
        $entity->{$field_name}['und'] = $this->loadForExperiment($entity);
        break;
      case 'stock':
        $entity->{$field_name}['und'] = $this->loadForGermplasm($entity);
        break;
      default:
        // Don't load data.
        break;
    }

    // @debug dpm($entity, 'entity');
  }

  /**
   * Load all trait/experiment/method/unit combinations
   * for the given trait.
   */
  private function loadForTrait (&$entity) {
    $trait_id = $entity->chado_record_id;
    $data = array();

    $resource = chado_query(
      'SELECT trait_id, trait_name, experiment_id, experiment_name, method_id, method_name, unit_id, unit_name 
       FROM {mview_phenotype}
       WHERE trait_id = :id
       GROUP BY trait_id, trait_name, experiment_id, experiment_name, method_id, method_name, unit_id, unit_name',
      array(':id' => $trait_id));
    while ($r = $resource->fetchObject()) {
      $data[] = array(
        'type' => 'trait',
        'value' => array(
          // Trait.
          'NCIT:C85496' => array(
            'data:2091' => $r->trait_id,
            'schema:name' => $r->trait_name,
          ),
          // Experiment.
          'SIO:001066' => array(
            'data:2091' => $r->experiment_id,
            'schema:name' => $r->experiment_name,
          ),
          // Method.
          'NCIT:C71460' => array(
            'data:2091' => $r->method_id,
            'schema:name' => $r->method_name,
          ),
          // Unit.
          'uo:unit' => array(
            'data:2091' => $r->unit_id,
            'schema:name' => $r->unit_name,
          ),
          'schema:url' => 'json/phenotypes/traitplot/'.$r->trait_id.'/'.$r->experiment_id.'/'.$r->method_id.'/'.$r->unit_id,
        ),
      );
    }
    return $data;
  }

  /**
   * Load all trait/experiment/method/unit combinations
   * for the given germplasm.
   */
  function loadForGermplasm(&$entity) {
    $germplasm_id = $entity->chado_record_id;
    $data = array();

    $resource = chado_query(
      'SELECT trait_id, trait_name, experiment_id, experiment_name, method_id, method_name, unit_id, unit_name 
       FROM {mview_phenotype}
       WHERE stock_id = :id
       GROUP BY trait_id, trait_name, experiment_id, experiment_name, method_id, method_name, unit_id, unit_name',
      array(':id' => $germplasm_id));
    while ($r = $resource->fetchObject()) {
      $data[] = array(
        'type' => 'germplasm',
        'value' => array(
          // Trait.
          'NCIT:C85496' => array(
            'data:2091' => $r->trait_id,
            'schema:name' => $r->trait_name,
          ),
          // Experiment.
          'SIO:001066' => array(
            'data:2091' => $r->experiment_id,
            'schema:name' => $r->experiment_name,
          ),
          // Method.
          'NCIT:C71460' => array(
            'data:2091' => $r->method_id,
            'schema:name' => $r->method_name,
          ),
          // Unit.
          'uo:unit' => array(
            'data:2091' => $r->unit_id,
            'schema:name' => $r->unit_name,
          ),
          'schema:url' => 'json/phenotypes/traitplot/'.$r->trait_id.'/'.$r->experiment_id.'/'.$r->method_id.'/'.$r->unit_id,
        ),
      );
    }

    return $data;
  }

  /**
   * Load all trait/experiment/method/unit combinations
   * for the given experiment.
   */
  function loadForExperiment(&$entity) {
    $experiment_id = $entity->chado_record_id;
    $data = array();

    $resource = chado_query(
      'SELECT trait_id, trait_name, experiment_id, experiment_name, method_id, method_name, unit_id, unit_name 
       FROM {mview_phenotype}
       WHERE experiment_id = :id
       GROUP BY trait_id, trait_name, experiment_id, experiment_name, method_id, method_name, unit_id, unit_name',
      array(':id' => $experiment_id));
    while ($r = $resource->fetchObject()) {
      $data[] = array(
        'type' => 'experiment',
        'value' => array(
          // Trait.
          'NCIT:C85496' => array(
            'data:2091' => $r->trait_id,
            'schema:name' => $r->trait_name,
          ),
          // Experiment.
          'SIO:001066' => array(
            'data:2091' => $r->experiment_id,
            'schema:name' => $r->experiment_name,
          ),
          // Method.
          'NCIT:C71460' => array(
            'data:2091' => $r->method_id,
            'schema:name' => $r->method_name,
          ),
          // Unit.
          'uo:unit' => array(
            'data:2091' => $r->unit_id,
            'schema:name' => $r->unit_name,
          ),
          'schema:url' => 'json/phenotypes/traitplot/'.$r->trait_id.'/'.$r->experiment_id.'/'.$r->method_id.'/'.$r->unit_id,
        ),
      );
    }

    return $data;
  }

  /**
   * @see ChadoField::query()
   *
   */
  public function query($query, $condition) {
  }

  /**
   * @see ChadoField::queryOrder()
   *
   */
  public function queryOrder($query, $order) {
  }


  /**
   * @see ChadoField::elementInfo()
   *
   */  
  public function elementInfo() {
    $field_term = $this->getFieldTermID();
    return array(
      $field_term => array(
        'operations' => array('eq', 'ne', 'contains', 'starts'),
        'sortable' => TRUE,
        'searchable' => TRUE,
      ),
    );
  }

}


