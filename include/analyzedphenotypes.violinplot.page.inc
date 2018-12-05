<?php
/**
 * @file
 * Contains functions related to rendering the violin plots.
 */

/**
 * Violin Plot Page.
 *
 * This page allows users to select an experiment + trait-method-unit
 * combination and visualize the data in violin plot form.
 */
function analyzedphenotypes_violinplot_form($form, $form_state) {

  // Add needed JS for the chart.
  $module_path = drupal_get_path('module','analyzedphenotypes');
  $form['#attached']['js'][] = 'https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.17/d3.min.js';
  $form['#attached']['js'][] = $module_path . '/theme/js/distroChart.js';
  $form['#attached']['js'][] = $module_path . '/theme/js/violin.chart.js';

  // Add CSS for the chart.
  $form['#attached']['css'][] = $module_path . '/theme/css/violinplot.chart.css';

  // Add a canvas for the chart.
  $form['chart'] = array(
    '#type' => 'item',
    '#markup' => '<div class="chart-wrapper" id="tripal-ap-violin-plot"></div>'
  );

  return $form;
}

/**
 * Generates the CSV needed for the chart.
 *
 * Specifically, this chart expects
 *   - the category for the X-axis (siteyear) and
 *   - the value measured.
 */
function analyzedphenotypes_violinplot_csv($project_id, $trait_id, $method_id, $unit_id) {
  $results = array();

  $resource = chado_query(
    "SELECT location, year, stock_name, mean
      FROM chado.mview_phenotype
      WHERE experiment_id=:project_id AND trait_id=:trait_id
      ORDER BY year ASC, location ASC",
    array(':project_id' => $project_id, 'trait_id' => $trait_id));
  foreach($resource as $r) {

    $results[] = array(
      'category' => $r->location . ' ' . $r->year,
      'value' => $r->mean,
    );
  }

  // Sort the results by key.
  ksort($results);

  return drupal_json_output($results);
}
