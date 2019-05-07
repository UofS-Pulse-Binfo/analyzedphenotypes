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
class local__number_of_experiments_formatter extends TripalFieldFormatter {
  // The default label for this field.
  public static $default_label = 'Number of experiments';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('local__numnber_of_experiments');

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
    // The source of the request: trait page or the germplasm page.
    // Number of experiment in germplasm page is by stock_id and trait id in trait page.
    $source = ($entity->chado_record->tablename == 'stocks') ? 'stock_id' : 'attr_id';
  
    // Get count:
    $sql = sprintf("SELECT COUNT(DISTINCT project_id) FROM {phenotype} WHERE %s = :source", $source);
    $number = chado_query($sql, array(':source' => $entity->chado_record_id))
      ->fetchField();
  
    dpm($number);

    // All set, render items.
    $element[0] = array(
      '#type' => 'markup',
      '#markup' => 'lorem',
    );


    return $element;
  }
}
