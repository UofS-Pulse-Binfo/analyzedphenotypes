<?php
/**
 * @class
 * Purpose:
 *
 * Data:
 * Assumptions:
 */
class sio__study extends ChadoField {

  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendant class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------

  // The default label for this field.
  public static $default_label = 'Phenotyping Experiments';

  // The default description for this field.
  public static $default_description = 'Lists experiments measuring the given trait.';

  // The default widget for this field.
  public static $default_widget = 'sio__study_widget';

  // The default formatter for this field.
  public static $default_formatter = 'sio__study_formatter';

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
    'term_vocabulary' => 'SIO',
    // The name of the term.
    'term_name' => 'study',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => '001066',
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
    $field_name = $this->field['field_name'];

    // ChadoFields automatically load the chado column specified in the
    // default settings above. If that is all you need then you don't even
    // need to implement this function. However, if you need to add any
    // additional data to be used in the display, you should add it here.
    parent::load($entity);

    // First we need the trait cvterm_id.
    // Luckily it is the primary key for this content type.
    $trait_id = $entity->chado_record_id;
    $sql = 'SELECT DISTINCT project_id FROM {phenotype} WHERE attr_id=:id';
    $projects = chado_query($sql, array(':id' => $trait_id))->fetchCol();

    // For each project we need to retrieve additional details.
    $entity->{$field_name}['und'] = array();
    foreach ($projects as $index => $project_id) {

      // Retrieve additional details for this project.
      $proj = ap_match_projectid(array('project_id' => $project_id));

      // Retrieve the site-years for this project.
      $sql = 'SELECT location, year, method_name, unit_name FROM {mview_phenotype}
        WHERE trait_id=:trait AND experiment_id=:project
        GROUP BY location, year, method_name, unit_name';
      $siteyears_raw = chado_query($sql,
        array(':trait' => $trait_id, ':project' => $project_id))->fetchAll();

      // Then condense to unique location combination,
      // compiling years for a given location.
      // Also compile methods used at the same time.
      $locations = array();
      $methods = array();
      foreach ($siteyears_raw as $r) {
        $locations[ $r->location ]['schema:name'] = $r->location;
        $locations[ $r->location ]['ncit:year'][] = $r->year;

        $methods[ $r->method_name ]['schema:name'] = $r->method_name;
        $methods[ $r->method_name ]['uo:unit'][] = $r->unit_name;
      }

      // Now Bring it all together.
      $value = array(
        'schema:name' => $proj['project_name'],
        'ncit:location' => array(),
      );

      foreach ($locations as $loc) {
        $loc['ncit:year'] = array_unique($loc['ncit:year']);
        $value['ncit:location'][] = $loc;
      }

      foreach ($methods as $m) {
        $m['uo:unit'] = array_unique($m['uo:unit']);
        $value['ncit:method'][] = $m;
      }

      // For each project, create elements for Tripal WS, Formatter and Chado.
      $entity->{$field_name}['und'][$index] = array(
        'value' => ['SIO:001066' => $proj['project_name']],
        $field_name => $value,
        'chado-prodject__project_id' => $project_id,
      );

      // Tripal WS, Locations and Year, together with the Experiments.
      $l = 0;
      foreach ($locations as $loc) {
        $entity->{$field_name}['und'][$index]['value']['hydra:member'][$l] =
          array(
            'NCIT:C25341' => $loc['schema:name'],  // Locations.
            'NCIT:C29848' => implode(', ', array_unique($loc['ncit:year']))  // Years.
          );

        $l++;
      }
    }
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
        'operations' => [],
        'sortable' => FALSE,
        'searchable' => FALSE,
        'label' => 'Phenotyping Experiments',
        'type' => 'xs:complexType',
        'readonly' => TRUE,
        'elements' => array(
          'SIO:001066' => array(
            'searchable' => TRUE,
            'name' => 'Phenotyping Experiments - Study',
            'operations' => ['eq', 'ne', 'contains', 'starts'],
            'sortable' => TRUE,
            'readonly' => TRUE,
            'type' => 'xs:string',
            'required' => FALSE,
          ),
        ),
      ),
    );
  }

}
