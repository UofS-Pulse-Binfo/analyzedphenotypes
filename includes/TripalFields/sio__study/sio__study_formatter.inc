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
   * Terms in use) Year: NCIT:C29848; Location: NCIT:C25341;
   *   Unit: UO:0000000; NCIT:C71460
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
    $field_name = $this->field['field_name'];

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
        '#title' => $item['value']['schema:name'],
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
      foreach ($item['value']['NCIT:C71460'] as $m) {
        $methods[] = $m['schema:name'] . ' (' . implode(',', $m['UO:0000000']). ')';
      }
      $statement = '<p>'
        .'This experiment was measured using the following method(s): '
        .'<em class="method">'.implode(' OR ', $methods).'</em>'
        .'.</p>';
      $element[$delta]['methods'] = array(
        '#type' => 'markup',
        '#markup' => $statement,
      );

      // Then summarize the site-years in a table.
      $caption = 'Table ' . ($delta+1) . ': Site-years for ' . $item['value']['schema:name'];
      $header = array('Location', 'Years');
      $rows = array();
      foreach ($item['value']['NCIT:C25341'] as $loc) {
        $rows[] = array(
          $loc['schema:name'],
          implode(', ', $loc['NCIT:C29848']),
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
