<?php
/**
 * @class
 * Purpose:
 *
 * Data:
 * Assumptions:
 */
class ncit__method extends ChadoField {

  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendant class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------

  // The default label for this field.
  public static $default_label = 'Collection Methods';

  // The default description for this field.
  public static $default_description = 'The method with which the given phenotypic data were collected.';

  // The default widget for this field.
  public static $default_widget = 'ncit__method_widget';

  // The default formatter for this field.
  public static $default_formatter = 'ncit__method_formatter';

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
    'term_vocabulary' => 'NCIT',
    // The name of the term.
    'term_name' => 'method',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'C71460',
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

    // First, retrieve the chado cvterm_id/trait_id for the current
    // Tripal Content, as we requested.
    $trait_id = $entity->ncit__method['und'][0]['chado-cvterm__cvterm_id'];

    // The method is defined by cvterms attached to the trait via a
    // specific cvterm_relationship type. Next, we need to grab
    // our configuration.
    // Note: we can't use ap_get_trait_methods() since we want methods
    // for all associated projects.
    $sysvar = ap_get_variablenames(
      array('variablename' => 'method'),
      array('set' => 'terms')
    );
    $type_id = variable_get($sysvar);

    // Retrieve all methods for the current trait.
    $results = chado_select_record('cvterm_relationship', array('object_id'),
      array('subject_id' => $trait_id, 'type_id' => $type_id));

    $entity->{$field_name}['und'] = array();
    foreach ($results as $index => $r) {
      // Get the details for the method cvterm.
      $method_r = chado_select_record('cvterm', array('cvterm_id', 'name', 'definition'),
        array('cvterm_id' => $r->object_id));
      $method = array(
	      'schema:name' => $method_r[0]->name,
	      'IAO:0000115' => $method_r[0]->definition,
      );

      // Next get the unit.
      // NOTE: the API function will not use the project genus if the method_id is supplied.
      $units = ap_get_method_units('UNKNOWN', $method_r[0]->definition, [], $method_r[0]->cvterm_id);
      foreach ($units as $u) {
	      $method['UO:unit'][] = array(
          'schema:name' => $u,
	      );
      }

      // Finally add it to the field.
      $entity->{$field_name}['und'][$index] = array(
	      'value' => ['NCIT:C71460' => $method_r[0]->name],
        $field_name => $method,
	      'chado-cvterm__cvterm_id' => $method_r[0]->cvterm_id,
      );
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
        'label' => 'Phenotyping Method',
        'type' => 'xs:complexType',
        'readonly' => TRUE,
        'elements' => array(
          'NCIT:C71460' => array(
            'searchable' => TRUE,
            'name' => 'Phenotyping Method',
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
