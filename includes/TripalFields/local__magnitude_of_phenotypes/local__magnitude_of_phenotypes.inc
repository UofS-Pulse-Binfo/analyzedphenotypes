<?php
/**
 * @class
 * Purpose: Provide quick browse functionality for entity pages
 *
 * Data: No data.
 * Assumptions:
 */
class local__magnitude_of_phenotypes extends TripalField {
  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendant class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------
  // The default label for this field.
  public static $default_label = 'Magnitude of Phenotypes';
  // The default description for this field.
  public static $default_description = 'Magnitude of Phenotypes';
  // The default widget for this field.
  public static $default_widget = 'local__magnitude_of_phenotypes_widget';
  // The default formatter for this field.
  public static $default_formatter = 'local__magnitude_of_phenotypes_formatter';
  // The module that manages this field.
  public static $module = 'analyzedphenotypes';

  // A list of global settings. These can be accessed within the
  // globalSettingsForm.  When the globalSettingsForm is submitted then
  // Drupal will automatically change these settings for all fields.
  // Once instances exist for a field type then these settings cannot be
  // changed.
  public static $default_settings = array(
    'storage' => 'tripal_no_storage',
    // It is expected that all fields set a 'value' in the load() function.
    // In many cases, the value may be an associative array of key/value pairs.
    // In order for Tripal to provide context for all data, the keys should
    // be a controlled vocabulary term (e.g. rdfs:type). Keys in the load()
    // function that are supported by the query() function should be
    // listed here.
    'browseable_keys' => array(),
  );

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instance.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings = array(
    // The short name for the vocabulary (e.g. schema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'local',
    // The name of the term.
    'term_name' => 'magnitude of phenotypes',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'magnitude_of_phenotypes',
    // Set to TRUE if the site admin is not allowed to change the term
    // type, otherwise the admin can change the term mapped to a field.
    'term_fixed' => FALSE,
    // Indicates if this field should be automatically attached to display
    // or web services or if this field should be loaded separately. This
    // is convenient for speed.  Fields that are slow should for loading
    // should have auto_attach set to FALSE so tha their values can be
    // attached asynchronously.
    'auto_attach' => FALSE,
    // The table where the options for this specific field are stored.
    // This can be one of trpfancy_browse_options or trpfancy_browse_options_per_entity
    // based on admin configuration. Default: trpfancy_browse_options.
    'option_storage' => '',
    // A list of browser types this field intends to provide.
    'browser_types' => '',
  );

  // A boolean specifying that users should not be allowed to create
  // fields and instances of this field type through the UI. Such
  // fields can only be created programmatically with field_create_field()
  // and field_create_instance().
  public static $no_ui = FALSE;
  // A boolean specifying that the field will not contain any data. This
  // should exclude the field from web services or downloads.  An example
  // could be a quick browse field that appears on the page that redirects
  // the user but otherwise provides no data.
  public static $no_data = TRUE;

 /**
   * Loads the field values from the underlying data store.
   *
   * @param $entity
   *
   * @return
   *   An array of the following format:
   *     $entity->{$field_name}['und'][0]['value'] = $value;
   *   where:
   *     - $entity is the entity object to which this field is attached.
   *     - $field_name is the name of this field
   *     - 'und' is the language code (in this case 'und' == undefined)
   *     - 0 is the cardinality.  Increment by 1 when more than one item is
   *       available.
   *     - 'value' is the key indicating the value of this field. It should
   *       always be set.  The value of the 'value' key will be the contents
   *       used for web services and for downloadable content.  The value
   *       should be of the follow format types: 1) A single value (text,
   *       numeric, etc.) 2) An array of key value pair. 3) If multiple entries
   *       then cardinality should incremented and format types 1 and 2 should
   *       be used for each item.
   *   The array may contain as many other keys at the same level as 'value'
   *   but those keys are for internal field use and are not considered the
   *   value of the field.
   *
   *
   */
  public function load($entity) {
    // Display settings:
    $mofp = array(
      // Show magnitude of phenotypes summary - phenotypes count, # of datapoints
      // and provide a link to trait distribution chart.
      'magnitude'  => TRUE,
      // Show listing of phenotypes.
      'phenotypes' => TRUE,
    );

    // The record.
    $chado_record_id = $entity->chado_record_id;

    // Create a summary of phenotyopes for this stock.
    $result = chado_query("
      SELECT attr_id AS id, COUNT(attr_id) AS magnitude, t2.name, t2.definition
      FROM {phenotype} INNER JOIN {cvterm} AS t2 ON attr_id = t2.cvterm_id
      WHERE stock_id = :stock_id
      GROUP BY attr_id, t2.name, t2.definition
      ORDER BY magnitude DESC"
      , array(':stock_id' => $chado_record_id));

    $data = array();
    $trait_count = $result->rowCount();
    $magnitude_of_phenotypes = array();

    // We need the field name to ensure we save the data in the correct field!
    $field_name = $this->instance['field_name'];

    // Set this property for formatteer to render.
    $entity->{$field_name}['und'][0] = array();

    if ($trait_count > 0) {
      $result = $result->fetchAll();
      $result = array_map(function($d) { return (array) $d; },$result);

      // Magnitude of datapoints.
      $mg = array_column($result, 'magnitude');
      $data_count = array_sum($mg);
      $url = $GLOBALS['base_url'] . '/phenotypes/trait-distribution';
      $data['magnitude']  = array(
        'Traits' => number_format($trait_count),
        'Measurements' => number_format($data_count),
        'Trait Distribution Plot' => $url
      );

      // # of phenotypes.
      $entity_ids = array_column($result, 'id');
      $data['phenotypes'] = array_column($result, 'definition', 'name');

      foreach($mofp as $m => $config) {
        if ($config) {
          $magnitude_of_phenotypes[$m] = $data[$m];
        }
      }
    }

    $entity->{$field_name}['und'][0]['value']['local:magnitude of phenotypes'] = $magnitude_of_phenotypes;

    if ($mofp['phenotypes'] && $trait_count > 0) {
      // When phenotypes were requested, perform this additional process to fetch entity id
      // so it will make trait name linkable to trait page.
      foreach($entity_ids as $i => $d) {
        $entity_id = mofp_get_trait_entity_id($d);
        $entity_ids[$i] = $entity_id;
      }

      $entity->{$field_name}['und'][0]['vars'] = array('entity_id' => $entity_ids);
    }
  }

  /**
   * Provides a form for the 'Field Settings' of an instance of this field.
   *
   * This function corresponds to the hook_field_instance_settings_form()
   * function of the Drupal Field API.
   *
   * Validation of the instance settings form is not supported by Drupal, but
   * the TripalField class does provide a mechanism for supporting validation.
   * To allow for validation of your setting form you must call the parent
   * in your child class:
   *
   * @code
   *   $element = parent::instanceSettingsForm();
   * @endcode
   *
   * Please note, the form generated with this function does not easily
   * support AJAX calls in the same way that other Drupal forms do.  If you
   * need to use AJAX you must manually alter the $form in your ajax call.
   * The typical way to handle updating the form via an AJAX call is to make
   * the changes in the form function itself but that doesn't work here.
   */
  public function instanceSettingsForm() {

    // Retrieve the current settings.
    // If this field was just created these will contain the default values.
    $settings = $this->instance['settings'];

    // Allow the parent Tripal Field to set up the form element for us.
    $element = parent::instanceSettingsForm();

    return $element;
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

  /**
   * Function fetch entity id number of a trait.
   * The id will be used to construct link to Tripal 3 page for a trait.
   *
   * @param $trait_id
   *   Integer, a number corresponding to cverm_id in chado.cvterm table.
   *
   * @return
   *   Integer, a number corresponding to a entity id in tripal_bundle table.
   */
  function mofp_get_trait_entity_id($trait_id) {
    $m = chado_query("
      SELECT bundle_id FROM chado_bundle WHERE type_value = (
        SELECT CAST(cv_id AS text) FROM {cvterm} WHERE cvterm_id = :trait_id
      )", array(':trait_id' => $trait_id))
      ->fetchField();

    $entity_id = '#';

    if ($m) {
      $chado_bundle = 'chado_bio_data_' . $m;

      $sql = sprintf('SELECT entity_id FROM %s WHERE record_id = :trait_id LIMIT 1', $chado_bundle);
      $result = chado_query($sql, array(':trait_id' => $trait_id));

      if ($result->rowCount() > 0) {
        $entity_id = $result->fetchField();
      }
    }

    return $entity_id;
  }
