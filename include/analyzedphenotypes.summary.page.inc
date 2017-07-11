<?php
/**
 * @file
 * Functions related to summarizing all phenotypic data.
 *
 * Future Ideas:
 *  - Map indicating locations where phenotyping was done
 *     - each location indicated by pie chart
 *     - size of pie chart represents the number of experiments
 *     - allow selection of...
 *        - experiment: show locations for specific experiment, size fixed
 *        - crop: simplify to only for that crop, colour fixed
 *
 * - Timeline indicating amount of data per year
 *     - x-axis is year, y-axis is # measurements taken
 *     - allow filter by
 *        1. crop,
 *        2a. experiment (stacked barchart to show locations),
 *        2b. trait (stacked to show experiments)
 */

/**
 * Renders the phenotype summary page.
 *
 * Purpose:
 *   _ Summarize quantity of data available and crops.
 *   - Provide links to searches for traits, experiments, and germplasm.
 *   - Highlight tools (link!) such as trait distribution
 * Concept: a Graphic Table summarizing phenotypic data.
 *   - rows represent each crop/species (with picture?).
 *   - columns represent totals:
 *      - traits (qualitative/quantitative),
 *      - experiments (locations/years)
 *      - germplasm
 *      - measurements (link to beanplot)
 *   - each cell should link to somewhere useful
 *      - header: not linked since each tool is crop-specific.
 *      - column 1: links to organism page
 *      - column 2-4: links to crop-specific searches for trait, experiment, germplasm
 *      - column 5: links to beanplot which contains links to download
 *   - Provide some instruction so people know it's clickable? Perhaps on hover
 *     over of a cell the number is replaced by link label?
 * NOTE: Change mview_phenotype to include crop.
 */
function analyzed_phenotypes_all_data_summary_page() {
  $output = '';

  // Add CSS
  $module_path = drupal_get_path('module', 'analyzedphenotypes');
  drupal_add_css($module_path . '/include/css/summary.css');

  // User style settings.
  $user_style = array(
    'genus colours' => array(
      'Tripalus' => '#56AC0D',
    ),
  );
  // Add inline styles to colour-code the crops as specified by the user.
  foreach ($user_style['genus colours'] as $genus => $colour) {
    $css = "
#phenoype-summary-table tbody tr.$genus td {
  color: $colour;
}
#phenoype-summary-table tbody tr.$genus td.Crop {
  background-color: $colour;
  color: white;
  border-bottom: 1px solid $colour;
}
#phenoype-summary-table tbody tr.$genus td.Traits:before {
    border-left: 25px solid $colour;
}
#phenoype-summary-table tbody tr.$genus td.Traits:before {
    border-left-color: $colour;
}
    ";
    drupal_add_css($css, array('type' => 'inline'));
  }

  // Initialize the table.
  $table = array(
    'header' => array('Crop', 'Traits', 'Experiments', 'Germplasm', 'Measurements'),
    'rows' => array(),
    'attributes' => array(
      'id' => 'phenoype-summary-table'
    ),
    'sticky' => FALSE,
    'empty' => 'You have no phenotypic data sync\'d with your site.',
  );

  // Fix formatting of header.
  $header = array();
  foreach ($table['header'] as $h) {
    $header[] = array('data' => $h, 'class' => array($h));
  }
  $table['header'] = $header;

  // Retrieve results for summary table.
  // Build Query.
  // First use a static optimized subquery to calculate the individuals counts.
  $subquery = 'SELECT organism_genus, :column_num as num, count(1) FROM (SELECT DISTINCT organism_genus, :column_name FROM {mview_phenotype}) d:column_num GROUP BY organism_genus';
  $query_parts = array();
  $column_num = 2;
  foreach (array('trait_id', 'experiment_id','stock_id') as $column_name) {
    $query_parts[] = strtr($subquery, array(':column_num' => $column_num, ':column_name' => $column_name));
    $column_num++;
  }
  // Then combine those subqueries with simple queries grabbing the organism genus and measurements count
  // by using UNIONs. We use the "num" to order the columns for the table.
  $query = '
    (SELECT organism_genus, 1 as num, 1 as count FROM {mview_phenotype} GROUP BY organism_genus)
    UNION (' . implode(') UNION (', $query_parts) . ')
    UNION (SELECT organism_genus, ' . $column_num . ' as num, count(1) FROM {mview_phenotype} GROUP BY organism_genus)
    ORDER BY num ASC';
  // Finally build the summary table based on the above query.
  $resource = chado_query($query);
  foreach ($resource as $r) {
    // Add colour for row if this is the first value for that row.
    if (!isset($table['rows'][$r->organism_genus])) {
      $table['rows'][$r->organism_genus] = array(
        'data' => array(),
        'class' => array($r->organism_genus),
      );
    }
    // Add current data point to table.
    if ($r->num == 1) {
      $table['rows'][$r->organism_genus]['data'][$r->num] = array(
        'data' => $r->organism_genus,
        'class' => array($table['header'][$r->num-1]['data']),
      );
    }
    else {
      $table['rows'][$r->organism_genus]['data'][$r->num] = array(
        'data' => number_format($r->count),
        'class' => array($table['header'][$r->num-1]['data']),
      );
    }
  }

  $output .= theme('table', $table);

  $output .= '<p>To visualize the distribution of values for a given trait, see the ' . l('Trait Distribution Chart', 'phenotypes/trait-distribution') . '.</p>';

  return $output;
}
