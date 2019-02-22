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

/** 
 * AJAX-Enabled form for hp__phenotypic_variability
 * Specifically, for trait pages.
 */
function analyzedphenotypes_hp_phenotypic_variability_traits_form($form, $form_state, $items) {

  // @debug dpm($items, 'items');

  // Determine the experiment_id, if set.
  $experiment_id = NULL;
  if (isset($form_state['values']) AND isset($form_state['values']['experiment_id'])) {
    $experiment_id = $form_state['values']['experiment_id'];
  }

  // Get a unique list of experiments.
  $expopt = array();
  foreach ($items as $i) {
    $exp_id = $i['value']['SIO:001066']['data:2091'];
    $exp_name = $i['value']['SIO:001066']['schema:name'];
    $expopt[ $exp_id ] = $exp_name;
  }

  // @debug
  $experiment_id = key($expopt);

  $form['experiment_id'] = array(
    '#type' => 'select',
    '#title' => 'Experiment',
    '#options' => $expopt,
    '#default_value' => $experiment_id,
    '#empty_option' => '- Experiment -',
    '#description' => 'Select the experiment you are insterested in.', 
  );

  // For each item for the current experiment, draw a chart.
  foreach ($items as $i) {
    if ($i['value']['SIO:001066']['data:2091'] == $experiment_id) {

      $key = $i['value']['NCIT:C71460']['data:2091'] . '-' . $i['value']['uo:unit']['data:2091'];
      $name = $i['value']['NCIT:C71460']['schema:name'] . ' (' . $i['value']['uo:unit']['schema:name'] . ')';
      $form[$key] = array(
        '#type' => 'fieldset',
        '#title' => $name,
      );

      // First get all the constituent parts...
      $experiment_name = $i['value']['SIO:001066']['schema:name'];
      $trait_name = $i['value']['NCIT:C85496']['schema:name'];
      $trait_id = $i['value']['NCIT:C85496']['data:2091'];
      $definition = chado_query('SELECT definition FROM {cvterm} WHERE cvterm_id=:id', 
        array(':id' => $trait_id))->fetchField();
      $unit = $i['value']['uo:unit']['schema:name']; 
      $method_id = $i['value']['NCIT:C71460']['data:2091'];
      $method_name = $i['value']['NCIT:C71460']['schema:name'];
      $method = chado_query('SELECT definition FROM {cvterm} WHERE cvterm_id=:id', 
        array(':id' => $method_id))->fetchField();
      $unit_id = $i['value']['uo:unit']['data:2091'];
      $phenotype_type = chado_query("SELECT value FROM {cvtermprop}
        WHERE cvterm_id=:id AND type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name='additionalType')",
        array(':id' => $unit_id))->fetchField();

      // Then form the figure legend based on those parts.
      if ($phenotype_type == 'quantitative' OR empty($phenotype_type)) {
        $chart_type = 'violin';
        $xaxis = 'Site-Year';
        $yaxis = $trait_name.' ('.$unit.')';
        $figtitle = "Comparison of observed $trait_name ($method_name) between site years for <em>$experiment_name</em>.";
        $figlegend = "$trait_name was measured in $unit."
        . $method . " Replicates were then averaged per germplasm within a single site-year."
        . " The chart shows the traditional box plot with the kernel density estimation flanking it."
        . " Thus values in a wider section of the plot represent higher probability that "
        . "members of the sampled germplasm collection will show that phenotype.";
      }
      else {
        $chart_type = 'multibar';
        $xaxis = 'Site-Year';
        $yaxis = $trait_name.' ('.$unit.')';
        $figtitle = "Comparison of observed $trait_name ($method_name) between site years for <em>$experiment_name</em>.";
        $figlegend = "$trait_name was measured as a $unit. "
        . $method . " Replicates were then averaged per germplasm within a single site-year."
        . " The chart shows the number of germplasm with a specific $trait_name where site-years are "
        . "shown as different coloured bars. Germplasm exhibiting multiple $trait_name values are shown "
        . "as [first value]/[second value] where order is not meaningful.";
      }

      // Add the needed JS for the chart.
      $module_path = drupal_get_path('module','analyzedphenotypes');
      $form['#attached']['js'][] = 'https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.17/d3.min.js';
      $form['#attached']['js'][] = $module_path . '/theme/js/distroChart.js';
      $form['#attached']['js'][] = $module_path . "/theme/js/$chart_type.chart.js";
      $data['analyzedPhenotypes'] = array();
      $data['analyzedPhenotypes'][] = array(
          'id' => 'tripal-ap-violin-plot-'.$key,
          'experiment_id' => $experiment_id,
          'trait_id' => $trait_id,
          'method_id' => $method_id,
          'unit_id' => $unit_id,
          'xaxis' => $xaxis,
          'yaxis' => $yaxis,
      );
      $form['#attached']['js'][] = array(
        'type' => 'setting',
        'data' => $data,
      );
       
 
      // Add the CSS for the chart.
      $form['#attached']['css'][] = $module_path . '/theme/css/'.$chart_type."plot.chart.css";
      
      $form[$key]['figure']['chart'] = array(
        '#type' => 'item',
        '#markup' => '<div class="chart-wrapper" id="tripal-ap-violin-plot-'.$key.'"></div>'
      );

      $form[$key]['figure']['legend'] = array(
        '#type' => 'markup',
        '#prefix' => '<div class="legend">',
        '#suffix' => '</div>',
      );
      $form[$key]['figure']['legend']['title'] = array(
        '#type' => 'markup',
        '#prefix' => '<div class="title">',
        '#markup' => 'Figure: '. $figtitle,
        '#suffix' => '</div>',
      );
      $form[$key]['figure']['legend']['description'] = array(
        '#type' => 'markup',
        '#prefix' => '<div class="description">',
        '#markup' => $figlegend,
        '#suffix' => '</div>',
      );

    }
  }

  return $form;
}
