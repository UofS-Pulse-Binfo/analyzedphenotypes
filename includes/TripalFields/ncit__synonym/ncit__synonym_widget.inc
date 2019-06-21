<?php
/**
 * @class
 * Purpose: Provide a graphical summary of data stored in a materialized view.
 *   This is a generic, configurable fields to make it easier to add charts
 *   to Tripal Content pages.
 *
 * Allowing edit? No
 */
class ncit__synonym_widget extends ChadoFieldWidget {

  // The default lable for this field.
  public static $default_label = 'No Edits';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('ncit__synonym');

  /**
   * Provides the form for editing of this field.
   *
   * This function corresponds to the hook_field_widget_form()
   * function of the Drupal Field API.
   *
   * This form is diplayed when the user creates a new entity or edits an
   * existing entity.  If the field is attached to the entity then the form
   * provided by this function will be displayed.
   *
   * At a minimum, the form must have a 'value' element.  For Tripal, the
   * 'value' element of a field always corresponds to the value that is
   * presented to the end-user either directly on the page (with formatting)
   * or via web services, or some other mechanism.  However, the 'value' is
   * sometimes not enough for a field.  For example, the Tripal Chado module
   * maps fields to table columns and sometimes those columns are foreign keys
   * therefore, the Tripal Chado modules does not just use the 'value' but adds
   * additional elements to help link records via FKs.  But even in this case
   * the 'value' element must always be present in the return form and in such
   * cases it's value should be set equal to that added in the 'load' function.
   *
   * @param $widget
   * @param $form
   *   The form structure where widgets are being attached to. This might be a
   *   full form structure, or a sub-element of a larger form.
   * @param $form_state
   *   An associative array containing the current state of the form.
   * @param $langcode
   *   The language associated with $items.
   * @param $items
   *   Array of default values for this field.
   * @param $delta
   *   The order of this item in the array of subelements (0, 1, 2, etc).
   * @param $element
   * A form element array containing basic properties for the widget:
   *  - #entity_type: The name of the entity the field is attached to.
   *  - #bundle: The name of the field bundle the field is contained in.
   *  - #field_name: The name of the field.
   *  - #language: The language the field is being edited in.
   *  - #field_parents: The 'parents' space for the field in the form. Most
   *    widgets can simply overlook this property. This identifies the location
   *    where the field values are placed within $form_state['values'], and is
   *    used to access processing information for the field through the
   *    field_form_get_state() and field_form_set_state() functions.
   *  - #columns: A list of field storage columns of the field.
   *  - #title: The sanitized element label for the field instance, ready for
   *    output.
   *  - #description: The sanitized element description for the field instance,
   *    ready for output.
   *  - #required: A Boolean indicating whether the element value is required;
   *    for required multiple value fields, only the first widget's values are
   *    required.
   *  - #delta: The order of this item in the array of subelements; see
   *    $delta above
   */
  public function form(&$widget, &$form, &$form_state, $langcode, $items, $delta, $element) {
    if (count($delta) == 1) {
      // Create field element to allow user to modify trait ontology term.
      $field_name = $this->field['field_name'];
      $field_table = array(
        'table' => 'cvterm_relationship',
        'field' => 'subject_id'
      );

      $linker_field = 'chado-' . $field_table['table'] . '__' . $field_table['field'];

      // Create or upldate.

      // The value presented to the user via load.
      // If $items['delta']['value'] is set then we are updating and already have this
      // information. As such, simply save it again.
      $widget['value'] = array(
        '#type' => 'value',
        '#value' => array_key_exists($delta, $items) ? $items[ $delta ]['value'] : '',
      );

      // Pull out the value previously saved to be used as the default.
      $ontology_term = '';
      if (count($items) > 0 && array_key_exists($linker_field, $items[0])) {
        $ontology_term = $items[0][ $linker_field ];
      }

      // Autocomplete field - instance the same as the ontology field in upload.
      // @see api/ontology api
      //      upload.inc

      // Search ontology terms in genus configuration:
      $widget['ap_fieldset'] = array(
        '#type' => 'fieldset',
        '#title' => t('Crop Ontology'),
      );

      // Fetch all genus with ontology value set.
      $cv_options = array();
      $genus = ap_get_genus();
      foreach($genus as $g) {
        $genus_key = strtolower(str_replace(' ', '_', $g));
        $conf = variable_get('analyzedphenotypes_systemvar_' . $genus_key . '_ontology');
        if ($conf) {
          $term_name = ap_get_cv(
            array('cv_id' => $conf)
          );

          $cv_options[ $g ] = $g . ' : ' . $term_name['name'];
        }
      }

      $widget['ap_fieldset']['ap_source_cv'] = array(
        '#type' => 'select',
        '#title' => t('CV'),
        '#description' => t('Select ontology terms in Genus-CV configuration to limit/filter terms only to this option.'),
        '#options' => array('Select') + $cv_options,
      );

      $widget['ap_fieldset'][ $linker_field ] = array(
        '#type' => 'textfield',
        '#title' => t('Trait Ontology'),
        //'#default_value' => $ontology_term,
        '#autocomplete_path' => 'admin/tripal/extension/analyzedphenotypes/json/ontology/Lens',
      );
    }
    else {
      // This field can only process one field per trait.
      drupal_set_message('Please set the maximum number of values user can enter for this field to 1 in manage field.', 'status', FALSE);
      $widget['no_field'] = array(
        '#markup' => 'Field not available.',
      );
    }
  }

  /**
   * Performs validation of the widgetForm.
   *
   * Use this validate to ensure that form values are entered correctly.
   * The 'value' key of this field must be set in the $form_state['values']
   * array anytime data is entered by the user.  It may be the case that there
   * are other fields for helping select a value. In the end those helper
   * fields must be used to set the 'value' field.
   */
  public function validate($element, $form, &$form_state, $langcode, $delta) {

 }

  /**
   * Performs extra commands when the entity form is submitted.
   *
   * Drupal typically does not provide a submit hook for fields.  The
   * TripalField provides one to allow for behind-the-scenes actions to
   * occur.   This function should never be used for updates, deletes or
   * inserts for the Chado table associated with the field.  Rather, the
   * storage backend should be allowed to handle inserts, updates deletes.
   * However, it is permissible to perform inserts, updates or deletions within
   * Chado using this function.  Those operations can be performed if needed but
   * on other tables not directly associated with the field.
   *
   * An example is the chado.feature_synonym table.  The chado_linker__synonym
   * field allows the user to provide a brand new synonynm and it must add it
   * to the chado.synonym table prior to the record in the
   * chado.feature_synonym table.  This insert occurs in the widgetFormSubmit
   * function.
   *
   *  @param $entity_type
   *    The type of $entity.
   *  @param $entity
   *    The entity for the operation.
   *  @param $field
   *    The field structure for the operation.
   *  @param $instance
   *    The instance structure for $field on $entity's bundle.
   *  @param $langcode
   *    The language associated with $items.
   *  @param $items
   *    $entity->{$field['field_name']}[$langcode], or an empty array if unset.
   *  @param $form
   *    The submitted form array.
   *  @param $form_state.
   *    The form state array.
   */
  public function submit($form, &$form_state, $entity_type, $entity, $langcode, $delta) {
  }
}
