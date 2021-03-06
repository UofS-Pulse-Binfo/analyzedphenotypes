<?php
/**
 * @class
 * Purpose:
 *
 * Display:
 * Configuration:
 */
class ncit__method_formatter extends ChadoFieldFormatter {

  // The default label for this field.
  public static $default_label = 'Collection Methods';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('ncit__method');

  // The list of default settings for this formatter.
  public static $default_settings = array(
    'setting1' => 'default_value',
  );

   /**
    * @see ChadoFieldFormatter::settingsForm()
    *
    **/

  public function settingsForm($view_mode, $form, &$form_state) {

  }

    /**
    * @see ChadoFieldFormatter::View()
    *
    **/

  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
    $field_name = $this->field['field_name'];

    // Get the settings
    $settings = $display['settings'];

    // Add the styling.
    $path = drupal_get_path('module','analyzedphenotypes') . '/includes/TripalFields/ncit__method/theme';
    $element['#attached']['css'][] = $path . '/ncit__method.css';

    foreach ($items as $delta => $item) {

      // Set up a container for the current method.
      $element[$delta] = array(
        '#type' => 'fieldset',
        '#title' => $item[ $field_name ]['schema:name'],
        '#attributes' => array (
          'class' => array(
            'collapsible',
	          'method',
	          'collapsed',
          )
        ),
      );

      // Add the definition.
      $element[$delta]['definition'] = array(
        '#type' => 'markup',
        '#markup' => '<p class="definition">' . $item[ $field_name ]['IAO:0000115'].'</p>',
      );

      // Add the unit
      // Process the units for display.
      $units = array();
      foreach ($item[ $field_name ]['UO:unit'] as $u) {
        $units[] = $u['schema:name'];
      }
      $element[$delta]['unit'] = array(
        '#type' => 'markup',
        '#markup' => '<p class="unit-statement">Measured in '.
          '<span class="unit">'.implode(' OR ',$units).'</span></p>',
      );
    }
  }

    /**
    * @see ChadoFieldFormatter::settingsSummary()
    *
    **/

  public function settingsSummary($view_mode) {
    return '';
  }

}
