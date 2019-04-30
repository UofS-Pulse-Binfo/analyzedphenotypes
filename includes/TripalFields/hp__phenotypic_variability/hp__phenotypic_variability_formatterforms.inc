<?php

/**
 * @file
 * Contains AJAX-Enabled form for the hp__phenotypic_variability field.
 * There are multiple forms where each is specific to the content type
 * the field is on (e.g. trait, germplasm, project).
 */

/**
 * AJAX-enabled form for hp__phenotypic_variability field.
 * Specifically, for trait pages.
 */
function analyzedphenotypes_hp_phenotypic_variability_traits_form($form, $form_state, $items) {

  // @debug dpm($items, 'items');

  $form['msg'] = array(
    '#type' => 'item',
    '#markup' => '<p>The phenotypic data is best summarized in a trait distribution chart. To see the summary for your experiment of interest, select it from the drop-down below. If the trait was measured with multiple methods in this experiment, you will see each method displayed in it\'s own chart.</p>',
  );

  // Add JS.
  $module_path = drupal_get_path('module','analyzedphenotypes');
  $form['#attached']['js'][] = 'https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.17/d3.min.js';
  $form['#attached']['js'][] = $module_path . '/theme/js/distroChart.js';

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

  // Ensure that if there is only one option, then we just draw it!
  if (sizeof($expopt) == 1) {
    $experiment_id = key($expopt);
  }

  $form['experiment_id'] = array(
    '#type' => 'select',
    '#title' => 'Experiment',
    '#options' => $expopt,
    '#default_value' => $experiment_id,
    '#empty_option' => '- Experiment -',
    '#description' => 'Select the experiment you are insterested in.',
    '#ajax' => array(
      'callback' => 'hp_phenotypic_variability_ajax_callback',
      'wrapper' => 'distribution-charts',
    ),
  );

  $form['charts'] = array(
    '#prefix' => '<div id="distribution-charts">',
    '#suffix' => '</div>',
  );

  // For each item for the current experiment, draw a chart.
  foreach ($items as $i) {
    if ($i['value']['SIO:001066']['data:2091'] == $experiment_id) {
      analyzedphenotypes_hp_phenotypic_variability_single_chart_form($form, $form_state, $i);
    }
  }

  return $form;
}

/**
 * AJAX-enabled form for hp__phenotypic_variability field
 * Specifically, for germ plasm pages.
 */
function analyzedphenotypes_hp_phenotypic_variability_germplasm_form($form, $form_state, $items) {

  // @debug dpm($items, 'items');

  $form['msg'] = array(
    '#type' => 'item',
    '#markup' => '<p>The phenotypic data is best summarized in a trait distribution chart. To see the summary for a trait/experiment of interest, select them from the drop-downs below. If the trait was measured with multiple methods in a given experiment, you will see each method displayed in it\'s own chart.</p>',
  );

  // Add JS.
  $module_path = drupal_get_path('module','analyzedphenotypes');
  $form['#attached']['js'][] = 'https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.17/d3.min.js';
  $form['#attached']['js'][] = $module_path . '/theme/js/distroChart.js';

  // EXPERIMENT
  // ------------
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

  // Ensure that if there is only one option, then we just draw it!
  if (sizeof($expopt) == 1) {
    $experiment_id = key($expopt);
  }

  $form['experiment_id'] = array(
    '#type' => 'select',
    '#title' => 'Experiment',
    '#options' => $expopt,
    '#default_value' => $experiment_id,
    '#empty_option' => '- Experiment -',
    '#description' => 'Select the experiment you are insterested in.',
    '#ajax' => array(
      'callback' => 'hp_phenotypic_variability_ajax_callback',
      'wrapper' => 'distribution-charts',
    ),
  );

  // TRAIT
  // ---------
  // Determine the trait_id, if set.
  $trait_id = NULL;
  if (isset($form_state['values']) AND isset($form_state['values']['trait_id'])) {
    $trait_id = $form_state['values']['trait_id'];
  }

  // Get a unique list of traits.
  $traitopts = array();
  foreach($items as $i) {
    $id = $i['value']['NCIT:C85496']['data:2091'];
    $name = $i['value']['NCIT:C85496']['schema:name'];
    $traitopts[ $id ] = $name;
  }

  // Ensure that if there is only one options, then we just select it!
  if (sizeof($traitopts) == 1) {
    $trait_id = key($traitopts);
  }

  $form['trait_id'] = array(
    '#type' => 'select',
    '#title' => 'Trait',
    '#options' => $traitopts,
    '#default_value' => $trait_id,
    '#empty_option' => '- Trait -',
    '#description' => 'Select the trait you are interested in.',
    '#ajax' => array(
      'callback' => 'hp_phenotypic_variability_ajax_callback',
      'wrapper' => 'distribution-charts',
    ),
  );

  // CHARTS.
  // --------
  $form['charts'] = array(
    '#prefix' => '<div id="distribution-charts">',
    '#suffix' => '</div>',
  );

  // For each item for the current experiment, draw a chart.
  foreach ($items as $i) {
    if (($i['value']['SIO:001066']['data:2091'] == $experiment_id) AND ($i['value']['NCIT:C85496']['data:2091'] == $trait_id)) {
      analyzedphenotypes_hp_phenotypic_variability_single_chart_form($form, $form_state, $i);
    }
  }

  return $form;
}

/**
 * AJAX-enabled form for hp__phenotypic_variability field
 * Specifically, for experiment pages.
 */
function analyzedphenotypes_hp_phenotypic_variability_experiment_form($form, $form_state, $items) {

  // @debug dpm($items, 'items');

  $form['msg'] = array(
    '#type' => 'item',
    '#markup' => '<p>The phenotypic data is best summarized in a trait distribution chart. To see the summary for your trait of interest, select it from the drop-down below. If the trait was measured with multiple methods in this experiment, you will see each method displayed in it\'s own chart.</p>',
  );

  // Add JS.
  $module_path = drupal_get_path('module','analyzedphenotypes');
  $form['#attached']['js'][] = 'https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.17/d3.min.js';
  $form['#attached']['js'][] = $module_path . '/theme/js/distroChart.js';

  // TRAIT
  // ---------
  // Determine the trait_id, if set.
  $trait_id = NULL;
  if (isset($form_state['values']) AND isset($form_state['values']['trait_id'])) {
    $trait_id = $form_state['values']['trait_id'];
  }

  // Get a unique list of traits.
  $traitopts = array();
  foreach($items as $i) {
    $id = $i['value']['NCIT:C85496']['data:2091'];
    $name = $i['value']['NCIT:C85496']['schema:name'];
    $traitopts[ $id ] = $name;
  }

  // Ensure that if there is only one options, then we just select it!
  if (sizeof($traitopts) == 1) {
    $trait_id = key($traitopts);
  }

  $form['trait_id'] = array(
    '#type' => 'select',
    '#title' => 'Trait',
    '#options' => $traitopts,
    '#default_value' => $trait_id,
    '#empty_option' => '- Trait -',
    '#description' => 'Select the trait you are interested in.',
    '#ajax' => array(
      'callback' => 'hp_phenotypic_variability_ajax_callback',
      'wrapper' => 'distribution-charts',
    ),
  );

  // CHARTS.
  // --------
  $form['charts'] = array(
    '#prefix' => '<div id="distribution-charts">',
    '#suffix' => '</div>',
  );

  // For each item for the current experiment, draw a chart.
  foreach ($items as $i) {
    if ($i['value']['NCIT:C85496']['data:2091'] == $trait_id) {
      analyzedphenotypes_hp_phenotypic_variability_single_chart_form($form, $form_state, $i);
    }
  }

  return $form;
}

/**
 * Build the form for a single chart.
 *
 * @param $form
 *   The full form so far.
 * @param $form_state
 *   The state of the form.
 * @param $i
 *   The current item.
 */
function analyzedphenotypes_hp_phenotypic_variability_single_chart_form(&$form, $form_state, $i) {
  $module_path = drupal_Get_path('module', 'analyzedphenotypes');

  $key = $i['value']['NCIT:C71460']['data:2091'] . '-' . $i['value']['uo:unit']['data:2091'];
  $name = $i['value']['NCIT:C71460']['schema:name'] . ' (' . $i['value']['uo:unit']['schema:name'] . ')';
  $form['charts'][$key] = array(
    '#type' => 'fieldset',
	  '#title' => $name,
	  '#collapsible' => TRUE,
	  '#collpased' => FALSE,
  );

  // First get all the constituent parts...
  $experiment_id = $i['value']['SIO:001066']['data:2091'];
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
    $germplasm_name = $i['name'];
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
  drupal_add_js($module_path . "/theme/js/$chart_type.chart.js", ['weight' => -3]);
  $data['analyzedPhenotypes'] = array();
  $data['analyzedPhenotypes'][] = array(
	  'type' => $chart_type,
    'id' => 'tripal-ap-'.$chart_type.'-plot-'.$key,
    'experiment_id' => $experiment_id,
    'trait_id' => $trait_id,
    'method_id' => $method_id,
    'unit_id' => $unit_id,
    'xaxis' => $xaxis,
    'yaxis' => $yaxis,
    'germplasm' => $germplasm_name,
  );
  drupal_add_js($data, 'setting');

  // Add the CSS for the chart.
  drupal_add_css($module_path . '/theme/css/violinplot.chart.css');

  $form['charts'][$key]['figure']['chart'] = array(
    '#type' => 'item',
    '#markup' => '<div class="chart-wrapper" id="tripal-ap-'.$chart_type.'-plot-'.$key.'"></div>'
  );

  $form['charts'][$key]['figure']['legend'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="legend">',
    '#suffix' => '</div>',
  );

  $form['charts'][$key]['figure']['legend']['title'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="title">',
    '#markup' => 'Figure: '. $figtitle,
    '#suffix' => '</div>',
  );

  $form['charts'][$key]['figure']['legend']['description'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="description">',
    '#markup' => $figlegend,
    '#suffix' => '</div>',
  );
}


/**
 * AJAX CALLBACK: refreshes charts on hp__phenotypic_variability charts.
 */
function hp_phenotypic_variability_ajax_callback($form, $form_state) {
  return $form['charts'];
}
