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
    if (isset($items[0]['value']['local:child_studies'])) {
      $arr_tiles = $items[0]['value']['local:child_studies'];

      // Refer to this ID for CSS styling.
      $id = 'ap-field-child-studies-wrapper';

      drupal_add_css(drupal_get_path('module', 'analyzedphenotypes') . '/includes/TripalFields/local__child_studies/theme/style_cs_field.css');
      drupal_add_js(drupal_get_path('module', 'analyzedphenotypes') . '/includes/TripalFields/local__child_studies/theme/script.js');

      // All set, render items.
      $element[0] = array(
        '#type' => 'markup',
        '#prefix' => '<p>The following traits are measured in related germplasm:</p>',
        '#markup' => '<div id="' . $id . '">' . theme('item_list', array('items' =>  $arr_tiles, 'attributes'=> array('class' => 'trait-list'))) . '</div><div style="clear: both"></div>
          <div id="ap-field-child-studies-btn-reveal">
            <div class="ap-field-child-studies-reveal-on" title="Click to show more or less phenotypes">&nbsp;</div>
          </div>'
      );
    }

    return $element;
  }
}
