<?php
/**
 * Function callback: construct germplasm search form.
 *
 * @param $form
 *   Drupal form array/object.
 * @param $form_state
 *   Drupal form array/object.
 * @param $items
 *   Array, contains values created in load() method.
 *   Elements include the trait id (cvterm_id) and trait data type (quantitative or qualitative).
 *
 * @return array.
 *   Render array as parameter to drupal_get_form() in field formatter.
 *
 * @see ncit__data_formatter.inc.
 *      ncit__data.inc.
 */
function analyzedphenotypes_germplasm_search_autocomplete_form($form, $form_state, $items, $trait_id) {

  // FIELD: Autocomplete text field.
  $form['ap_germplasm_search_autocomplete_textfield'] = array(
    '#type' => 'textfield',
    '#theme_wrappers' => array(),
    '#autocomplete_path' => 'admin/tripal/extension/analyzedphenotypes/phenotyped-stock/json/'.$trait_id,
    '#attributes' => array(
      'placeholder' => 'Germplasm Name or Accession'
    ),

    '#id' => 'ap-germplasm-search-autocomplete-textfield',
  );

  // BUTTON: Search button.
  $form['ap_germplasm_search_button_submit'] = array(
    '#type' => 'button',
    '#value' => 'Search',
    '#ajax' => array(
      'callback' => 'analyzedphenotypes_germplasm_search_AJAX_callback',
      'wrapper' => 'ap-germplasm-search-ajax-wrapper',
      'effect' => 'slide',
    ),

    '#id' => 'ap-germplasm-search-button-submit',
  );

  // AJAX WRAPPER: Div wrapper element.
  $form['ap_germplasm_search_ajax_wrapper'] = array(
    '#prefix' => '<div id="ap-germplasm-search-ajax-wrapper">',
    '#suffix' => '</div>'
  );

  // FIELD: Storage.
  // Store the trait id # and all values prepared by the loader
  // for it to be available in succeeding AJAX request.
  $form['ap_germplasm_search_cache_items'] = array(
    '#type' => 'value',
    '#value' => $items,
  );


  // AJAX RESPONSE:

  if (isset($form_state['values'])) {
    // AJAX form submitted.
    // When form_state is set, autcomplete field too, and has a value.
    if (isset($form_state['values']['ap_germplasm_search_autocomplete_textfield'])
      && !empty($form_state['values']['ap_germplasm_search_autocomplete_textfield'])) {

      // Check for valid stock id.
      $a = $form_state['values']['ap_germplasm_search_autocomplete_textfield'];
      $stock_id = (preg_match('/\[id: (\d+)\]$/', $a, $matches)) ? $matches[1] : 0;

      if (!$stock_id) {
        // Stock has no phenotype.
        drupal_set_message('Germplasm not measured in this trait.
          Please enter a Germplasm Name or Accession.', 'error');

        return $form;
      }

      // BEGIN AJAX RESPONSE:
      // Athough we are getting the id from autocomplete, still check the stock
      // if it matched to a record in chado stock.
      $stockprop = chado_get_stock(array('stock_id' => $stock_id), array('include_fk' => array()));

      if ($stockprop) {
        // Stock okay, but does it have phenotype?
        $cache = $form_state['values']['ap_germplasm_search_cache_items'];
        list($trait_id, $additional_type) = array_values($cache[0]);

        // Experiment, location and year - this order is important see loop below
        // that uses this information.
        $data = chado_query("
          SELECT t1.values, t2.value AS unit_type,
            ARRAY_TO_JSON(ARRAY[t1.experiment_name, t1.location, t1.year]) AS experiment_siteyear,
            ARRAY_TO_JSON(ARRAY[t1.method_name, t1.unit_name]) AS method_unit
          FROM {mview_phenotype} AS t1 INNER JOIN {cvtermprop} AS t2 ON t1.unit_id = t2.cvterm_id
          WHERE trait_id = :trait_id AND stock_id = :stock_id
          ORDER BY year DESC",
        array(':trait_id' => $trait_id, ':stock_id' => $stock_id));

        $data_count = $data->rowCount();

        if ($data_count > 0) {
          // Has phenotypes, Prepare AJAX response.

          // Hold exp_siteyear and Drupal table row values.
          $tmp = $tables = array();

          // Group and summarize data by method-unit combination.
          foreach($data as $i => $row) {
            $cols = array();

            // Values.
            $values = json_decode($row->values);
            $values_count = count($values);

            // Specific column to either - quan or qual.
            if ($row->unit_type == 'quantitative') {
              // QUANTITY: mean and standard deviation.
              $mean = round((array_sum($values) / $values_count), 2);
              $sdev = round(ap_calculate_standard_deviation($values), 2);

              $cols = array(
                'Mean' => $mean,
                'Standard<br />Deviation' => $sdev
              );
            }
            elseif ($row->unit_type == 'qualitative') {
              // QUALITY: observed phenotypes.
              $obsv = array_unique($values);

              $cols = array(
                'Observed Phenotypes' => implode(', ', $obsv)
              );
            }

            ///
            // Use method-unit combination as unique key.
            $method_unit = implode(' - ', json_decode($row->method_unit));

            // Experiment, method and unit, and location info. With this summary
            // Summary over list items can be generated from this array.
            $tmp[$i] = json_decode($row->experiment_siteyear);

            $tables[ $method_unit ][] = array_merge(
              array(
                'Experiment' => $tmp[$i][0],
                'Site-year' => $tmp[$i][1] . ' - ' . $tmp[$i][2],
                'Number of<br />Measurements' => $values_count),
              $cols);
          } // End group data.

          // Fetch entity id that will form part of the url linking to Tripal 3 germplasm page.
          $chado_entity = chado_get_record_entity_by_table('stock', $stock_id);

          // Take away duplicate values from the overall summary overview.
          // Order is important see query above.
          $list_item = array(
            'experiments' => array_unique(array_column($tmp, 0)),
            'locations'  => array_unique(array_column($tmp, 1)),
            'years'     => array_unique(array_column($tmp, 2))
          );

          $tooltip = '<div class="ap-germplasm-search-tooltip">%s</div>';
          $name = $stockprop->name . ' (' . $stockprop->uniquename . ')';
          if ($chado_entity) {
            $name = l($name, 'bio_data/' . $chado_entity, ['attributes' => ['target' => '_blank']]);
          }
          $element_list = theme('item_list', array(
            'items' => array(
              $name                            . sprintf($tooltip, $stockprop->name . ' (' . $stockprop->uniquename . ')'),
              count($list_item['experiments']) . ' Experiments' . sprintf($tooltip, implode(' &bull; ', $list_item['experiments'])),
              count($list_item['locations'])   . ' Locations'   . sprintf($tooltip, implode(' &bull; ', $list_item['locations'])),
              count($list_item['years'])       . ' Years'       . sprintf($tooltip, implode(' &bull; ', $list_item['years'])),
            )
          ));

          // Theme data and attach to render array.
          $tabs = $element_table = array();
          foreach($tables as $table_header => $table_rows) {
            $element_table[] = theme('table', array(
              'caption' => $table_header,
              'header' => array_keys(current($table_rows)),
              'rows' => $table_rows,
              'sticky' => FALSE
            ));
          } // End theme data.

          $element_tabs = theme('item_list', array('items' => $tabs));

          $btn_reveal = ($data_count > 6)
            ? '<div id="ap-germplasm-search-button-reveal">
                <div class="ap-germplasm-search-table-row-reveal-on" title="Click to show more or less">&nbsp;</div>
              </div>'
             : '';

          $form['ap_germplasm_search_ajax_wrapper']['ap_germplasm_search_ajax_response_table'] = array(
            '#markup' => '
             <div id="ap-germplasm-search-summary-overview-list">' . $element_list  . '<a href="#">Clear Search</a></div>
             <div id="ap-germplasm-search-summary-table">' . implode('', $element_table) . '</div>'
             . $btn_reveal,
          );
        }
        else {
          // Stock has no phenotype.
          drupal_set_message('Germplasm not measured in this trait.
            Please enter a Germplasm Name or Accession.', 'error');
        }
      }
      else {
        // Stock not found.
        drupal_set_message('Germplasm not measured in this trait.
          Please enter a Germplasm Name or Accession.', 'error');
      }
    }
    else {
      // Nothing entered.
      drupal_set_message('Germplasm not measured in this trait.
        Please enter a Germplasm Name or Accession.', 'error');
    }
  } // End form state.


  return $form;
}


/**
 * Function callback.
 * AJAX update wrapper element.
 */
function analyzedphenotypes_germplasm_search_AJAX_callback($form, $form_state) {
  return $form['ap_germplasm_search_ajax_wrapper'];
}
