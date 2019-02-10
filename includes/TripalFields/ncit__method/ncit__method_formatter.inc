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

    // Get the settings
    $settings = $display['settings'];

    // Add the styling.
    $path = drupal_get_path('module','analyzedphenotypes') . '/includes/TripalFields/ncit__method/theme';
    $element['#attached']['css'][] = $path . '/ncit__method.css';

    foreach ($items as $i => $item) {
      $units = array();
      foreach ($item['value']['UO:unit'] as $u) {
        $units[] = $u['schema:name'];
      }
      $element[$i] = array(
        '#type' => 'markup',
	'#markup' => '<p class="definition">'.$item['value']['IAO:0000115'].'</p>'
	. '<p class="unit-statement">Measured in '
	. '<span class="unit">'.implode(' OR ', $units).'</span></p>',
        '#prefix' => '<div class="method">',
	'#suffix' => '</div>',
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
