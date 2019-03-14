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
class local__magnitude_of_phenotypes_formatter extends TripalFieldFormatter {
  // The default label for this field.
  public static $default_label = 'Magnitude of Phenotypes';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('local__magnitude_of_phenotypes');

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
    // Style information.
    drupal_add_css(drupal_get_path('module', 'analyzedphenotypes') . '/includes/TripalFields/local__magnitude_of_phenotypes/theme/style_mp_field.css');
    // Data to render:
    $data = $items[0]['value']['local:magnitude of phenotypes']; dpm($items);
    // Array to hold processed data before rendering.
    $render = array();
    // Themed data.
    $markup_data = '';
    $wrap_data = '<div id="field-magnitude-of-phenotypes-wrapper-%s">%s</div>';

    foreach($data as $i => $d) {
      foreach($d as $m => $value) { 
        // Trait definition might be long, we want just the 100 chars for layout pupose
        // so that all items will be on a unified height.
        $value = ($i == 'phenotypes') ? truncate_utf8($value, 100, TRUE, TRUE) : $value;
        $value = ($m == 'Trait Distribution Plot') ? l($m, $value) : $value;

        $m     = ($m == 'Trait Distribution Plot') ? '' : $m . ':';

        $render[$i][] = $m . '<span>' . $value . '</span>';
      }

      $t = theme('item_list', array('items' => $render[$i], 'attributes' => array('id' => 'list-magnitude-of-phenotypes-' . $i)));
      $markup_data .= sprintf($wrap_data, $i, $t);
    }

    // All set, render items.
    $element[0] = array(
      '#type' => 'markup',
      '#markup' => '<div id="field-magnitude-of-phenotypes-wrapper">' . $markup_data . '</div>',
    );


    return $element;
  }
}
