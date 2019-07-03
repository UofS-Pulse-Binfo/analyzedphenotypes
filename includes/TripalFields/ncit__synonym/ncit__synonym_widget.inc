<?php
/**
 * @class
 * Purpose: Provide a graphical summary of data stored in a materialized view.
 *   This is a generic, configurable fields to make it easier to add charts
 *   to Tripal Content pages.
 *
 * Allowing edit? No
 */
class ncit__synonym_widget extends TripalFieldWidget {

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
    parent::form($widget, $form, $form_state, $langcode, $items, $delta, $element);

    if ($delta == 0) {
      // By the default, field allows for multiple/unlimited values to be entered. AP only
      // supports 1 and only 1 crop ontology term per trait name.

      // Field name.
      $field_name = $this->field['field_name'];

      // Elements of chado.cvterm_relationship table:
      $chado_table = 'chado-cvterm_relationship__';


      // FOR CHADO MECHNANISM:
      // Column - cvterm_relationship_id primary key.
      $widget['value'] = array(
        '#type' => 'value',
        '#value' => $items[$delta][ $chado_table . 'cvterm_relationship_id' ] ?? '',
      );

      // Column - cvterm_relationship_id primary key.
      $widget[ $chado_table . 'cvterm_relationship_id' ] = array(
        '#type' => 'value',
        '#value' => $items[$delta][ $chado_table . 'cvterm_relationship_id' ] ?? '',
      );

      // Column - object_id, the trait id (cvterm_id) # the crop ontology term maps to.
      // Has value even when trait has no crop ontology.
      $widget[ $chado_table . 'object_id' ] = array(
        '#type' => 'value',
        '#value' => $items[$delta][ $chado_table . 'object_id' ],
      );

      // Column - subject_id, the crop ontology id (cvterm_id) that maps to a trait.
      $widget[ $chado_table . 'subject_id' ] = array(
        '#type' => 'value',
        '#value' => $items[$delta][ $chado_table . 'subject_id' ] ?? '',
      );

      // Column - type_id, refers to ontology term Related in AP configuration.
      // Has value even when trait has no crop ontology.
      $widget[ $chado_table . 'type_id' ] = array(
        '#type' => 'value',
        '#value' => $items[$delta][ $chado_table . 'type_id' ],
      );


      // CURRENT SETTINGS:
      // Style information.
      drupal_add_css(drupal_get_path('module', 'analyzedphenotypes') . '/includes/TripalFields/ncit__synonym/theme/style_cp_synonym_field.css');

      // Current crop ontology.
      if (count($items[0]['value']) > 0) {
        @list($term, $accession, $cv) = array_values($items[0]['value']);
        // Refer to this ID for CSS styling.
        $id = 'ap-field-crop_ontology_synonym';

        $crop_ontology = '<div id="' . $id . '">' . $term . '</div> ';
        $cvterm_lookup = l('Vocabulary Details', 'cv/lookup/' .  $cv . '/' . $accession, array('attributes' => array('target' => '_blank'))) . ' | ';
      }
      else {
        $crop_ontology = 'Crop ontology not set | ';
        $cvterm_lookup = '';
      }

      // FIELDSET - Main fieldset - search ontology terms.
      $widget['ap_fieldset'] = array(
        '#type' => 'fieldset',
        '#title' => t('Crop Ontology'),
      );

      // MARKUP - Show current record if any.
      $widget['ap_fieldset']['ap_ontology'] = array(
        '#type' => 'markup',
        '#markup' => $crop_ontology . $cvterm_lookup . l('Update term', '#', array('attributes' => array('class' => 'ap-link-form-revel'))),
      );


      // FORM ELEMENTS:
      // @credits to Tripal developer - sbo relationship.
      $vocs = [];
      $vocs = chado_get_cv_select_options();
      $cv_id = isset($form_state['values'][$field_name][$langcode][$delta]['ap_fieldset']['ap_ajax_response']['ap_source_cv'])
        ? $form_state['values'][$field_name][$langcode][$delta]['ap_fieldset']['ap_ajax_response']['ap_source_cv'] : 0;

      // ITEM - Describe both CV field and CV Term field.
      $widget['ap_fieldset']['ap_describe'] = array(
        '#type' => 'item',
        '#title' => t('CV and Term'),
        '#description' => t('Select CV and Term to change the ontology term for this trait.'),
        //
        // Wrap.
        '#prefix' => '<div id="ap-crop-ontology-form-fields">',
      );

        // MARKUP - AJAX response wrapper. Update CV id filter in autocomplete field below.
        $widget['ap_fieldset']['ap_ajax_response'] = array(
          '#prefix' => '<div id="ap-ajax-response">',
          '#suffix' => '</div>',
        );

        // FIELD SELECT - Select CV.
        $widget['ap_fieldset']['ap_ajax_response']['ap_source_cv'] = array(
          '#type' => 'select',
          '#options' => $vocs,
          '#theme_wrappers' => array(),
          '#ajax' => array(
            'callback' => 'analyzedphentypes_synonym_callback',
            'wrapper' => 'ap-ajax-response',
            'method' => 'replace',
          ),
        );

        // FIELD AUTOCOMPLETE - Search term in a CV.
        $widget['ap_fieldset']['ap_ajax_response']['ap_ontology_term'] = array(
          '#type' => 'textfield',
          '#size' => 60,
          '#autocomplete_path' => 'admin/tripal/storage/chado/auto_name/cvterm/' . $cv_id,
          '#disabled' => TRUE,
          '#theme_wrappers' => array(),
          '#attributes' => array('style' => 'margin: 0 10px;'),
          //
          // Wrap.
          '#suffix' => '</div>',
        );

        if (isset($cv_id)) {
          // Enable field when a cv selected.
          $widget['ap_fieldset']['ap_ajax_response']['ap_ontology_term']['#disabled'] = FALSE;
        }

        // Listen to link to update ontology term.
        drupal_add_js("jQuery('.ap-link-form-revel').click(function(e) {
          e.preventDefault();
          jQuery('#ap-crop-ontology-form-fields').fadeIn();
        })", array('type' => 'inline', 'scope' => 'footer'));
    }
    else {
      drupal_set_message('Crop ontology configuration can only be set to a single value. Please set the allowed value to one in mange field.', 'warning', FALSE);
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
    $field_name = $this->field['field_name'];

    if (isset($form_state['values'][$field_name][$langcode][$delta])) {
      $form_values = $form_state['values'][$field_name][$langcode][$delta];

      if (isset($form_values['ap_fieldset']['ap_ajax_response']['ap_source_cv'])
        && isset($form_values['ap_fieldset']['ap_ajax_response']['ap_ontology_term'])) {

          $termprop = chado_generate_var('cvterm', array(
            'cv_id' => $form_values['ap_fieldset']['ap_ajax_response']['ap_source_cv'],
            'name' => $form_values['ap_fieldset']['ap_ajax_response']['ap_ontology_term'],
          ));

          if ($termprop && $termprop->cvterm_id > 0) {
            // Term is valid.

            // From this point on, set the chado_table settings to cvterm_relationship from cvterm.
            $this->instance['settings']['chado_table'] = 'cvterm_relationship';
            $field_table = $this->instance['settings']['chado_table'];


            // Create or Update?

            if (empty($form_values['value'])) {
              // Create.
              $form_state['values'][$field_name][$langcode][$delta]['chado-' . $field_table . '__object_id']
                = $form_state['values'][$field_name][$langcode][$delta]['chado-' . $field_table . '__object_id'];

              $form_state['values'][$field_name][$langcode][$delta]['chado-' . $field_table . '__type_id']
                = $form_state['values'][$field_name][$langcode][$delta]['chado-' . $field_table . '__type_id'];

              // As entered in the autocomplete field.
              $form_state['values'][$field_name][$langcode][$delta]['chado-' . $field_table . '__subject_id'] = $termprop->cvterm_id;
            }
            else {
              // Update.
              // As entered in the autocomplete field.
              $form_state['values'][$field_name][$langcode][$delta]['chado-' . $field_table . '__subject_id'] = $termprop->cvterm_id;
           }

           // After create or update, restore chado_table setting to original value (cvterm).
           $this->instance['settings']['chado_table'] = 'cvterm';
         }
      }
    }
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


/**
 * Function callback - AJAX response.
 * @credits to Tripal developer - sbo relationship.
 */
function analyzedphentypes_synonym_callback($form, $form_state) {
  // Get the triggering element
  $form_element_name = $form_state['triggering_element']['#name'];
  preg_match('/(.+?)\[(.+?)\]\[(.+?)\]/', $form_element_name, $matches);
  $field = $matches[1];
  $lang = $matches[2];
  $delta = $matches[3];

  // Return the widget that triggered the AJAX call, but only the ontology section.
  if (isset($form[$field][$lang][$delta])) {
    return $form[$field][$lang][$delta]['ap_fieldset']['ap_ajax_response'];
  }
}
