<?php
/**
 * @class
 * Purpose:
 *
 * Display:
 * Configuration:
 */
class sio__study_formatter extends ChadoFieldFormatter {

  // The default label for this field.
  public static $default_label = 'Phenotyping Experiments';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('sio__study');

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

    // Add the styling.
    $path = drupal_get_path('module','analyzedphenotypes') . '/includes/TripalFields/sio__study/theme';
    $element['#attached']['css'][] = $path . '/sio__study.css';

    // Add library for collapsing fieldsets.
    drupal_add_library('system', 'drupal.collapse');

    // @debug dpm($items, 'items');
    // For each experiment...
    $collapse = (sizeof($items) > 1) ? TRUE : FALSE;
    foreach ($items as $delta => $item) {

      // Add a containing element for this experiment.
      $element[$delta] = array(
        '#type' => 'fieldset',
        '#title' => 'Experiment: '.$item['value']['schema:name'],
        '#attributes' => array (
          'class' => array(
            'collapsible',
          )
        ),
      );
      if ($collapse && ($delta != 1)) {
        $element[$delta]['#attributes']['class'][] = 'collapse';
      }

      // Summarize the methodology used for this experiment.
      $methods = array();
      foreach ($item['value']['ncit:method'] as $m) {
        $methods[] = $m['schema:name'] . ' (' . implode(',', $m['uo:unit']). ')';
      }
      $statement = '<p>'
        .'This experiment was measured using '
        .'<em class="method">'.implode(', ', $methods).'</em>'
        .'.</p>';
      $element[$delta]['methods'] = array(
        '#type' => 'markup',
        '#markup' => $statement,
      );

      // Then summarize the site-years in a table.
      $caption = 'Table '.($delta+1).': Site-years for '.$item['value']['schema:name'].'.';
      $header = array('Location', 'Years');
      $rows = array();
      foreach ($item['value']['ncit:location'] as $loc) {
        $rows[] = array(
          $loc['schema:name'],
          implode(', ', $loc['ncit:year']),
        );
      }
      $element[$delta]['siteyears'] = array(
        '#theme' => 'table',
        '#caption' => $caption . '<sup>*</sup>',
        '#header' => $header,
        '#rows' => $rows,
        '#suffix' => '<span class="footnote">* Only contains site years with data for this trait.</span>'
      );
    }
  }

  /**
   * @see ChadoFieldFormatter::settingsSummary()
   *
   */
  public function settingsSummary($view_mode) {
    return '';
  }

}
