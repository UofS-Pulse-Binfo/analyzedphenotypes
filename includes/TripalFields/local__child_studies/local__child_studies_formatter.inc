<?php
/**
 * @class
 * Purpose: Provide a quick search on entity pages which submits/redirects to a full search.
 *
 * Display: A simple textfield search form.
 * Configuration:
 *   - path to the full search.
 *   - the URL token (query parameter) the value applies to.
 *   - help text.
 *   - textfield placeholder.
 *   - search button text.
 *   - autocomplete path.
 */
class local__child_studies_formatter extends TripalFieldFormatter {
  // The default label for this field.
  public static $default_label = 'Stock Child Studies';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('local__child_studies');

  /**
   *  Provides the display for a field
   *
   * This function corresponds to the hook_field_formatter_view()
   * function of the Drupal Field API.
   *
   *  This function provides the display for a field when it is viewed on
   *  the web page.  The content returned by the formatter should only include
   *  what is present in the $items[$delta]['values] array. This way, the
   *  contents that are displayed on the page, via webservices and downloaded
   *  into a CSV file will always be identical.  The view need not show all
   *  of the data in the 'values' array.
   *
   *  @param $element
   *  @param $entity_type
   *  @param $entity
   *  @param $langcode
   *  @param $items
   *  @param $display
   *
   *  @return
   *    An element array compatible with that returned by the
   *    hook_field_formatter_view() function.
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
    // The record.
    $chado_record_id = $entity->chado_record_id;

    // Inspect phenotype if stocks have been measured.
    $sql = "SELECT COUNT(stock_id) stock_count FROM {phenotype} WHERE stock_id = :stock_id";
    $result = chado_query($sql, array(':stock_id' => $chado_record_id))
      ->fetchField();

    if ($result < 1) {
      // See if this stock has children.
      // Since stock does not have any phenotypes, inspect instance/a child/selection of.
      $sql = "SELECT stock_id FROM {stock} WHERE stock_id IN (
        SELECT subject_id FROM {stock_relationship} WHERE object_id = :stock_id AND type_id = (
          SELECT cvterm_id FROM {cvterm} WHERE name = 'is_selection_of' LIMIT 1
        )
      ) ORDER BY name ASC";

      $result = chado_query($sql, array(':stock_id' => $chado_record_id));

      if ($result->rowCount() > 0) {
        // Stock has instance (a selection of).
        // Prepare Stock - Traits - Experiment tree.
        $child_stock = $result->fetchAllKeyed(0, 0);

        $sql = "SELECT t1.attr_id, t2.name AS trait, t3.name AS experiment, t4.name AS stock, t4.stock_id
          FROM {phenotype} AS t1 INNER JOIN {cvterm} AS t2 ON t1.attr_id = t2.cvterm_id
            INNER JOIN {project} AS t3 USING(project_id)
            INNER JOIN {stock} AS t4 USING(stock_id)
          WHERE stock_id IN (:stock_id) GROUP BY t1.attr_id, t2.name, t3.name, t4.name, t4.stock_id
          ORDER BY t2.name ASC";

        $result = chado_query($sql, array(':stock_id' => $child_stock));

        // Assign a colour code to a stock.
        $stock_colour = array();
        foreach($child_stock as $i => $stock) {
          $stock_colour[ $stock ] = mt_rand(0, 255) . ', ' . mt_rand(0, 255) . ' ,' . mt_rand(0, 255);
        }


        // Construct tiles for each trait. Same traits will be in one card
        // with each child/instance stock summarized.
        $arr_trait = array();
        $arr_tiles = array();
        $format_item = '<div style="background-color: rgba(%s, 0.25);">%s</div><span>%s</span>';
        foreach($result as $trait) {
          $child = sprintf($format_item, $stock_colour[ $trait->stock_id ], $trait->stock, $trait->experiment);

          if (in_array($trait->attr_id, $arr_trait)) {
            // A new trait, create tile.
            $arr_tiles[ $trait->attr_id ]['children'][] = $child;
          }
          else {
            // Push to matching tile.
            array_push($arr_trait, $trait->attr_id);
            $arr_tiles[ $trait->attr_id ] = array(
              'data' => $trait->trait,
              'children' => array($child),
            );
          }
        }
      }
    }

    drupal_add_css(drupal_get_path('module', 'analyzedphenotypes') . '/includes/TripalFields/local__child_studies/theme/style_cs_field.css');
    // Refer to this ID for CSS styling.
    $id = 'ap-field-child-studies-wrapper';

    // All set, render items.
    $element[0] = array(
      '#type' => 'markup',
      '#markup' => '<div id="' . $id . '">' . theme('item_list', array('items' =>  $arr_tiles)) . '</div><div style="clear: both"></div>'
    );

    return $element;
  }
}

 /**
   * Function fetch entity id number of a trait.
   * The id will be used to construct link to Tripal 3 page for a trait.
   *
   * @param $trait_id
   *   Integer, a number corresponding to cverm_id in chado.cvterm table.
   *
   * @return
   *   Integer, a number corresponding to a entity id in tripal_bundle table.
   */
  function ap_childstudies_get_trait_entity_id($trait_id) {
    $m = chado_query("
      SELECT bundle_id FROM chado_bundle WHERE type_value = (
        SELECT CAST(cv_id AS text) FROM {cvterm} WHERE cvterm_id = :trait_id
      )", array(':trait_id' => $trait_id))
      ->fetchField();

    $entity_id = '#';

    if ($m) {
      $chado_bundle = 'chado_bio_data_' . $m;

      $sql = sprintf('SELECT entity_id FROM %s WHERE record_id = :trait_id LIMIT 1', $chado_bundle);
      $result = chado_query($sql, array(':trait_id' => $trait_id));

      if ($result->rowCount() > 0) {
        $entity_id = $result->fetchField();
      }
    }

    return $entity_id;
  }
