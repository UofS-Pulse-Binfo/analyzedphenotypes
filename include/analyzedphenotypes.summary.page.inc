<?php
/**
 * @file
 * Functions related to summarizing all phenotypic data.
 */

/**
 * Renders the summary page.
 */
function analyzed_phenotypes_all_data_summary_page() {
  $output = '';

  $output .= '<p>For phenotypic data summary see the ' . l('Trait Distribution Chart', 'phenotypes/trait-distribution') . '.</p>';
  
  return $output;
}
