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


  $form['ap_ncitdata_field_wrapper'] = array(
    '#prefix' => '<div id="ap-ncitdata-field-wrapper">',
    '#suffix' => '</div>'
  );


  // Store the trait id # and all values prepared by the loader
  // for it to be available in succeeding AJAX request.
  $form['ap_cache_items'] = array(
    '#type' => 'value',
    '#value' => $items,
  );

  if (isset($form_state['values']) && isset($form_state['values']['ap_germplasm_autocomplete_textfield'])) {
    $s = $form_state['values']['ap_germplasm_autocomplete_textfield'];
    if (preg_match('/\[id: (\d+)\]/', $s, $matches)) {
      $stock_id =  $matches[1];

      $p = chado_query("SELECT name, uniquename, type_id FROM {stock} WHERE stock_id = :stock_id LIMIT 1"
        , array(':stock_id' => $stock_id));

      if ($p->rowCount() > 0) {
        $items = $form_state['values']['ap_cache_items'];
        list($trait_id, $type) = array_values($items[0]);

        $data = chado_query("
          SELECT
            CONCAT(experiment_name, '<span>', location, ' - ', year , '</span>') AS experiment,
            CONCAT(method_name, '<span>(', unit_name, ')</span>') AS method,
            ARRAY_TO_JSON(ARRAY[experiment_name, location, year]) AS summary,
            values
          FROM {mview_phenotype}
          WHERE trait_id = :trait_id AND stock_id = :stock_id
          ORDER BY year, location, experiment_name DESC",
          array(':trait_id' => $trait_id, ':stock_id' => $stock_id));

        if ($data->rowCount() > 0) {
          // Has data.
          $header = $item = $tmp = array();
          array_push($header, 'Experiment', 'Method', '# Measurements');

          $col1 = $col2 = '';

          foreach($data as $i => $row) {
            $values = json_decode($row->values);
            $replicates = count($values);

            // Specific to either types.
            if ($type['data_type'] == 'quantitative') {
              // QUAN: mean value and standard deviation.
              $mn = array_sum($values) / count($values);
              $mn = $mn ?? '-';
              $col1 = round($mn, 2);

              $sd = ap_calculate_standard_deviation($values);
              $sd = $sd ?? '-';
              $col2 = round($sd, 2);

              if ($i == 0) {
                // Add specific header.
                array_push($header, 'Mean', 'Standard Deviation');
              }
            }
            elseif ($type['data_type'] == 'qualitative') {
              // QUAL: observed phenotype (unique) and # measurements.
              $col1 = array_uniqe($values);

              if ($i == 0) {
                // Add specific header.
                array_push($header, 'Observed Phenotype');
              }
            }

            // Common items to both quantitative and qualitative types.
            // Experiment, Trait, # Measurement.

            // Add to item.
            $item[] = array($row->experiment, $row->method, $replicates, $col1, $col2);

            // Store the summary for quick count/reference in summary overview.
            $tmp[] = json_decode($row->summary);
          }

          $stockprop = $p->fetchObject();

          // Construct: Summary Overview.
          $chado_bundle = db_query('SELECT t2.name FROM {tripal_term} AS t1 INNER JOIN {tripal_bundle} AS t2 ON t1.id = t2.term_id
            WHERE t1.accession = (SELECT t3.accession FROM chado.dbxref AS t3 INNER JOIN chado.cvterm AS t4 USING(dbxref_id)
              WHERE t4.cvterm_id = :stock_type)', array(':stock_type' => $stockprop->type_id)
          )
          ->fetchField();

          $e = sprintf("SELECT entity_id FROM chado_%s WHERE record_id = :stock_id LIMIT 1", $chado_bundle);
          $chado_entity = chado_query($e, array(':stock_id' => $stock_id))
            ->fetchField();

          $base_cols = array(
            'experiments' => array_unique(array_column($tmp, 0)),
            'locations'  => array_unique(array_column($tmp, 1)),
            'years'     => array_unique(array_column($tmp, 2))
          );

          $tool_tip = '<div class="ap-tooltip">%s</div>';

          $summary_overview_val['items'] = array(
            l($stockprop->name, url('../bio_data/' . $chado_entity), array('attributes' => array('target' => '_blank'))) . sprintf($tool_tip, $stockprop->name . ' (' . $stockprop->uniquename . ')'),
            count($base_cols['experiments']) . ' Experiments' . sprintf($tool_tip, implode(' &bull; ', $base_cols['experiments'])),
            count($base_cols['locations']) . ' Locations' . sprintf($tool_tip, implode(' &bull; ', $base_cols['locations'])),
            count($base_cols['years']) . ' Years' . sprintf($tool_tip, implode(' &bull; ', $base_cols['years'])),
          );

          $summary_overview_markup = theme('item_list', $summary_overview_val);
          $form['ap_ncitdata_field_wrapper']['ap_summary_overview'] = array(
            '#type' => 'markup',
            '#markup' => '<div id="ap-summary-overview-list">' . $summary_overview_markup . '</div>',
          );

          $summary_table_markup = theme('table', array(
            'header' => $header,
            'rows' => $item,
            'sticky' => FALSE,
            'attributes' => array('id' => 'ap-table-default-headers')
          ));

          // First 5 rows but any more will require the button to show less or more.
          $btn_reveal = (count($item) > 6)
            ? '<div id="ap-ncitdata-btn-reveal"><div class="ap-ncitdata-table-row-reveal-on" title="Click to show more or less">&nbsp;</div></div'
            : '';

          $form['ap_ncitdata_field_wrapper']['ap_summary_table'] = array(
            '#markup' => '<div id="ap-ncitdata-table-summary-wrapper" style="max-height: 300px; overflow-y: hidden">' . $summary_table_markup . '</div>' . $btn_reveal,
          );
        } // has data.
      }  // has stock
    } // has match
  }

  return $form;
}


function analyzedphenotypes_ncitdata_AJAX_callback($form, $form_state) {
  return $form['ap_ncitdata_field_wrapper'];
}
