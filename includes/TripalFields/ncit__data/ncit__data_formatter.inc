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
class ncit__data_formatter extends TripalFieldFormatter {
  // The default label for this field.
  public static $default_label = 'Germplasm Phenotype Search';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('ncit__data');

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
    // CSS & JS.
    $theme_path = drupal_get_path('module','analyzedphenotypes') . '/includes/TripalFields/ncit__data/theme/';
    drupal_add_css($theme_path . 'style_ncitdata_field.css', 'file');
    drupal_add_js ($theme_path . 'script.js');

    $trait_id = $entity->chado_record_id;
    $autocomplete_field = drupal_get_form('analyzedphenotypes_germplasm_search_autocomplete_form', $items, $trait_id);
    $field = drupal_render($autocomplete_field);

    // Refer to this ID for CSS styling.
    $id = 'ap-germplasm-search-field-wrapper';

    // All set, render items.
    $element[] = array(
      '#type' => 'markup',
      '#markup' => '<div id="' . $id . '">' . $field . '</div>',
    );

    return $element;
  }
}
