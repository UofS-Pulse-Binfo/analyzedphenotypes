<?php
/**
 * @class
 * Purpose:
 *
 * Display:
 * Configuration:
 */
class hp__phenotypic_variability_formatter extends ChadoFieldFormatter {

  // The default label for this field.
  public static $default_label = 'Phenotypic Data Distribution';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('hp__phenotypic_variability');

  // The list of default settings for this formatter.
  public static $default_settings = array(
    'setting1' => 'default_value',
  );

  /**
   * @see ChadoFieldFormatter::settingsForm()
   *
   */
  public function settingsForm($view_mode, $form, &$form_state) {

  }

  /**
   * @see ChadoFieldFormatter::View()
   *
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {

    // Get the settings
    $settings = $display['settings'];

    $form = drupal_get_form('analyzedphenotypes_hp_phenotypic_variability_traits_form', $items);
    $content = drupal_render($form);
    $element[] = array(
      '#type' => 'markup',
      '#markup' => $content,
    );

    return $element; 
  }

  /**
   * @see ChadoFieldFormatter::settingsSummary()
   *
   */
  public function settingsSummary($view_mode) {
    return '';
  }

}

