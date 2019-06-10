<?php
/**
 * Function callback: construct search form.
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
function analyzedphenotypes_germplasm_search_autocomplete_form($form, $form_state, $items) {
  // FIELD: Autocomplete text field.
  $form['ap_germplasm_search_autocomplete_textfield'] = array(
    '#type' => 'textfield',
    '#theme_wrappers' => array(),
    '#autocomplete_path' => 'admin/tripal/storage/chado/auto_name/stock',
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
      // if it matched to a record in stock.
      $stockprop = chado_get_stock(array('stock_id' => $stock_id), array('include_fk' => array()));

      if ($stockprop) {
        // Stock okay, but does it have a phenotype?
        $cache = $form_state['values']['ap_germplasm_search_cache_items'];
        list($trait_id, $additional_type) = array_values($cache[0]);

        $data = chado_query("
          SELECT t1.values, t2.value AS unit_type,
            ARRAY_TO_JSON(ARRAY[t1.experiment_name, t1.location, t1.year]) AS experiment,
            ARRAY_TO_JSON(ARRAY[t1.method_name, t1.unit_name]) AS method
          FROM {mview_phenotype} AS t1 INNER JOIN {cvtermprop} AS t2 ON t1.unit_id = t2.cvterm_id
          WHERE trait_id = :trait_id AND stock_id = :stock_id AND t2.type_id = :additional_type
          ORDER BY year DESC",
          array(':trait_id' => $trait_id, ':stock_id' => $stock_id, ':additional_type' => $additional_type));

        if ($data->rowCount() > 0) {
          // Has phenotypes, Prepare AJAX response.
          $tmp1 = $item = array();

          //
          // SUMMARY TABLE:
          foreach($data as $i => $row) {
             $values = json_decode($row->values);
             $values_count = count($values);

             $cols = array();
             $tem2 = '';

            // Specific columns to either.
            if ($type['data_type'] == 'quantitative') {
              // QUANTITY: mean and standard deviation.
              $mean = round((array_sum($values) / $values_count), 2);
              $sdev = round(ap_calculate_standard_deviation($values), 2);
              $cols = array(
                'Mean' => $mean,
                'Standard<br />Deviation' => $sdev
              );
            }
            elseif ($type['data_type'] == 'qualitative') {
              // QUATLITY: observed phenotypes.
              $obsv = array_unique($values);
              $cols = array(
                'Observed Phenotypes' => implode(', ', $obsv)
              );
            }

            // Create table row items.
            $tmp1[$i] = json_decode($row->experiment);
            $tmp2     = json_decode($row->method);

            // span styled as block element.
            $item[$i] = array_merge(
              array(
                $tmp1[$i][0] . '<span>'  . $tmp1[$i][1] . ' - ' . $tmp1[$i][2] . '</span>',
                $tmp2[0]     . '<span>(' . $tmp2[1]    . ')</span>',
                $values_count
              ),

              array_values($cols)
            );
          }
          //

          $element_table = theme('table', array(
            'header' => array_merge(array('Experiment', 'Method', 'Number of<br />Measurements'), array_keys($cols)),
            'rows' => $item,
            'sticky' => FALSE,
          ));


          //
          // SUMMARY OVERVIEW LIST:
          $chado_bundle = db_query('SELECT t2.name FROM {tripal_term} AS t1 INNER JOIN {tripal_bundle} AS t2 ON t1.id = t2.term_id
            WHERE t1.accession = (SELECT t3.accession FROM chado.dbxref AS t3 INNER JOIN chado.cvterm AS t4 USING(dbxref_id)
              WHERE t4.cvterm_id = :stock_type)', array(':stock_type' => $stockprop->type_id))
            ->fetchField();

          $entity = sprintf("SELECT entity_id FROM chado_%s WHERE record_id = :stock_id LIMIT 1", $chado_bundle);
          $chado_entity = chado_query($entity, array(':stock_id' => $stock_id))
            ->fetchField();

          $list_item = array(
            'experiments' => array_unique(array_column($tmp1, 0)),
            'locations'  => array_unique(array_column($tmp1, 1)),
            'years'     => array_unique(array_column($tmp1, 2))
          );

          $tooltip = '<div class="ap-germplasm-search-tooltip">%s</div>';
          $element_list = theme('item_list', array(
            'items' => array(
              l($stockprop->name . ' (' . $stockprop->uniquename . ')', url('../bio_data/' . $chado_entity), array('attributes' => array('target' => '_blank')))
                                                                . sprintf($tooltip, $stockprop->name . ' (' . $stockprop->uniquename . ')'),
              count($list_item['experiments']) . ' Experiments' . sprintf($tooltip, implode(' &bull; ', $list_item['experiments'])),
              count($list_item['locations'])   . ' Locations'   . sprintf($tooltip, implode(' &bull; ', $list_item['locations'])),
              count($list_item['years'])       . ' Years'       . sprintf($tooltip, implode(' &bull; ', $list_item['years'])),
            )
          ));


          //
          // ATTACH TO - $form.
          // First 5 rows but any more will require the button to show less or more.
          $btn_reveal = (count($item) > 5)
           ? '<div id="ap-germplasm-search-button-reveal">
                <div class="ap-germplasm-search-table-row-reveal-on" title="Click to show more or less">&nbsp;</div>
              </div>'
           : '';

          $form['ap_germplasm_search_ajax_wrapper']['ap_germplasm_search_ajax_response'] = array(
            '#markup' => '
              <div id="ap-germplasm-search-summary-overview-list">' . $element_list  . '<a href="#">Clear Search</a></div>
              <div id="ap-germplasm-search-summary-table">'         . $element_table . '</div>'
              . $btn_reveal
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
  }

  return $form;
}


/**
 * Function callback.
 * AJAX update wrapper element.
 */
function analyzedphenotypes_germplasm_search_AJAX_callback($form, $form_state) {
  return $form['ap_germplasm_search_ajax_wrapper'];
}
