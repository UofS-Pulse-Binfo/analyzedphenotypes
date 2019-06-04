<?php
function analyzedphenotypes_germplasm_search_autocomplete_form($form, $form_state, $items) {
  $module_path = drupal_get_path('module','analyzedphenotypes');
  $form['#attached']['css'][] = $module_path . '/includes/TripalFields/ncit__data/theme/style_ncitdata_field.css';
  $form['#attached']['js'][] = $module_path . '/includes/TripalFields/ncit__data/theme/script.js';

  $form['ap_germplasm_autocomplete_textfield'] = array(
    '#type' => 'textfield',
    '#title' => '',
    '#autocomplete_path' => 'admin/tripal/storage/chado/auto_name/stock',
    '#theme_wrappers' => array(),
    '#attributes' => array(
      'placeholder' => 'Germplasm Name or Accession'
    ),
    '#id' => 'ap-search-autocomplete-textfield',
  );

  $form['ap_search_button'] = array(
    '#type' => 'button',
    '#value' => 'Search',
    '#id' => 'ap-search-button',
    '#ajax' => array(
      'callback' => 'analyzedphenotypes_ncitdata_AJAX_callback',
      'wrapper' => 'ap-ncitdata-field-wrapper',
    ),
  );


  $form['a_w'] = array(
    '#prefix' => '<div id="ap-ncitdata-field-wrapper">',
    '#suffix' => '</div>'
  );



/*
  $items['items'] = array(
    l('CDC Red Berry', '#'),
    '5 Experiments',
    '2 Locations',
    '5 Years',
  );

  $summary_overview = theme('item_list', $items);
  $form['ap_summary_overview'] = array(
    '#type' => 'markup',
    '#markup' => '<div id="ap-summary-overview-list">' . $summary_overview . '</div>',
  );

  $tbl1 = theme('table', array(
    'header' => array('Experiment', 'Location', 'Year', 'Reo', 'Mean', 'Standard Deviation'),
    'rows'   => array(array('AGILE: Application of Genomic Innovation in Lentil Economy', 'Nardiya, Nepal', 2017, 3, 22.5, 2.5)),
    'attributes' => array('id' => 'ap-table-default-headers')
  ));

  // Preview Headers.
  $form['ap_summary_table'] = array(
    '#markup' => $tbl1,
  );
*/
  return $form;
}




function analyzedphenotypes_ncitdata_AJAX_callback($form, $form_state) {
  if (isset($form_state['values']) && isset($form_state['values']['ap_germplasm_autocomplete_textfield'])) {
    // Raw germplasm id returned by the autocomplete field in [id: 111] format.
    $stock_id = $form_state['values']['ap_germplasm_autocomplete_textfield'];

    // Extract the id number part:
    if (preg_match('/\[id: (\d+)\]/', $stock_id, $matches)) {
      $stock_id =  $matches[1];

      // AJAX Response:
      list($trait_id, $traitprop) = array_values($items[0]) ;
      $sql = "SELECT experiment_name AS experiment, location, year, method_name AS method, unit_name AS unit, values
        FROM {mview_phenotype}
        WHERE trait_id = :trait_id AND stock_id = :stock_id
        ORDER BY year, location, experiment DESC";

        $data = chado_query($sql, array(':trait_id' => $trait_id, ':stock_id' => $stock_id));
        dpm($data->fetchAll());


      // Construct AJAX response:
      $form['a_w']['#markup'] = $germplasm_id;
    }
  }

  return $form['a_w'];
}


