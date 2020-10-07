<?php
/**
 * @class
 * Purpose: Provide quick browse functionality for entity pages
 *
 * Data: No data.
 * Assumptions:
 */
class local__child_studies extends TripalField {
  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendant class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------
  // The default label for this field.
  public static $default_label = 'Stock Child Studies';
  // The default description for this field.
  public static $default_description = 'Stock Child Studies';
  // The default widget for this field.
  public static $default_widget = 'local__child_studies_widget';
  // The default formatter for this field.
  public static $default_formatter = 'local__child_studies_formatter';
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
    'term_name' => 'child_studies',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'child_studies',
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
    $field_name = $this->field['field_name'];

    // The record.
    $chado_record_id = $entity->chado_record_id;

    // Clear the field.
    $this->entity{$field_name}['und'][0] = array();
    $this->entity{$field_name}['und'][0]['value'] = NULL;

    // Inspect phenotype if stocks have been measured.
    $sql = "SELECT COUNT(stock_id) stock_count FROM {phenotype} WHERE stock_id = :stock_id";
    $result = chado_query($sql, array(':stock_id' => $chado_record_id))
      ->fetchField();

    if ($result < 1) {
      // See if this stock has children.
      // Since stock does not have any phenotypes, inspect instance/a child/selection of.
      $sql = "SELECT stock_id FROM {stock} WHERE stock_id IN (
        SELECT subject_id FROM {stock_relationship} WHERE object_id = :stock_id AND type_id = (
          SELECT cvterm_id FROM {cvterm} WHERE name = 'is_selection_of' LIMIT 1
        )
      ) ORDER BY name ASC";

      $result = chado_query($sql, array(':stock_id' => $chado_record_id));

      if ($result->rowCount() > 0) {

        // Stock has instance (a selection of).
        // Prepare Stock - Traits - Experiment tree.
        $child_stock = $result->fetchAllKeyed(0, 0);

        $sql = "SELECT t1.attr_id, t2.name AS trait, t3.name AS experiment, t4.name AS stock, t4.stock_id, t4.type_id
          FROM {phenotype} AS t1 INNER JOIN {cvterm} AS t2 ON t1.attr_id = t2.cvterm_id
            INNER JOIN {project} AS t3 USING(project_id)
            INNER JOIN {stock} AS t4 USING(stock_id)
          WHERE stock_id IN (:stock_id) GROUP BY t1.attr_id, t2.name, t3.name, t4.name, t4.stock_id, t4.type_id
          ORDER BY t2.name ASC";

        $result = chado_query($sql, array(':stock_id' => $child_stock));

        // Assign a colour code to a stock.
        $stock_colour = array();
        foreach($child_stock as $i => $stock) {
          $stock_colour[ $stock ] = mt_rand(0, 255) . ', ' . mt_rand(0, 255) . ' ,' . mt_rand(0, 255);
        }

        // Construct tiles for each trait. Same traits will be in one card
        // with each child/instance stock summarized.
        $arr_trait = array();
        $arr_tiles = array();
        $format_item = '<div style="background-color: rgba(%s, 0.25);">%s</div><span>%s</span>';
        foreach($result as $trait) {
          $stock_entity_id = ap_childstudies_fetch_get_stock_entity_id($trait->stock_id, $trait->type_id);
          $stock_link = l($trait->stock, $GLOBALS['base_url'] . '/bio_data/' . $stock_entity_id);
          $child = sprintf($format_item, $stock_colour[ $trait->stock_id ], $stock_link, $trait->experiment);

          if (in_array($trait->attr_id, $arr_trait)) {
            // A new trait, create tile.
            $arr_tiles[ $trait->attr_id ]['children'][] = $child;
          }
          else {
            // Push to matching tile.
            array_push($arr_trait, $trait->attr_id);

            $trait_entity_id = ap_childstudies_get_trait_entity_id($trait->attr_id);
            $trait_link = l($trait->trait, $GLOBALS['base_url'] . '/bio_data/' . $trait_entity_id, array('attributes' => array('target' => '_blank')));
            $arr_tiles[ $trait->attr_id ] = array(
              'data' => $trait_link,
              'children' => array($child),
            );
          }
        }

	if (!empty($arr_tiles)) {
	  $entity->{$field_name}['und'][0]['value'] = array();
          $entity->{$field_name}['und'][0]['value']['local:child_studies'] = $arr_tiles;
        }
      }
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
        'elements' => array(
          'local:child_studies' => array(
            'searchable' => FALSE,
            'name' => 'Child Studies',
            'operations' => array(),
            'sortable' => FALSE,
            'type' => 'xs:string',
            'readonly' => TRUE,
            'required' => FALSE,
          ),
        ),
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
 function ap_childstudies_get_trait_entity_id($trait_id) {
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

/**
 * Fetch entity id that will form part of the url linking to Tripal 3 germplasm page.
 *
 * @param $stock_id
 *   Integer, stock id number corresponding to stock_id field in chado.stock table.
 * @param $stock_type
 *   Integer, type id number corresponding to type_id field in chado.stock table.
 *
 * @return Integer
 *   Chado entity id number that forms part of the url linking to Tripal 3 page or # null page.
 */
function ap_childstudies_fetch_get_stock_entity_id($stock_id, $stock_type) {
  if ($stock_id > 0 && $stock_type > 0) {
    // Fetch entity id that will form part of the url linking to Tripal 3 germplasm page.
    $chado_bundle = db_query('SELECT t2.name FROM {tripal_term} AS t1 INNER JOIN {tripal_bundle} AS t2 ON t1.id = t2.term_id
      WHERE t1.accession = (SELECT t3.accession FROM chado.dbxref AS t3 INNER JOIN chado.cvterm AS t4 USING(dbxref_id)
        WHERE t4.cvterm_id = :stock_type)', array(':stock_type' => $stock_type))
      ->fetchField();

    if ($chado_bundle) {
      $entity = sprintf("SELECT entity_id FROM chado_%s WHERE record_id = :stock_id LIMIT 1", $chado_bundle);
      $chado_entity = chado_query($entity, array(':stock_id' => $stock_id))
        ->fetchField();
    }
    else {
      tripal_report_error('germ_summary', TRIPAL_WARNING, 'Unable to find bundle for :bundle.', [':bundle' => $stock_type]);
    }
  }

  return $chado_entity ?? '#';
}
