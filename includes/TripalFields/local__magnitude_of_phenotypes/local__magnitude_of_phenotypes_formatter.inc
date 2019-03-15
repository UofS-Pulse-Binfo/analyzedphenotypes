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
    $base_url = $GLOBALS['base_url'];
    // Style information.
    drupal_add_css(drupal_get_path('module', 'analyzedphenotypes') . '/includes/TripalFields/local__magnitude_of_phenotypes/theme/style_mp_field.css');
    // Data to render:
    $data = $items[0]['value']['local:magnitude of phenotypes'];
    
    if ($items[0]['vars']['entity_id']) {
      $entity_ids = $items[0]['vars']['entity_id'];
    }
    
    // Array to hold processed data before rendering.
    $render = array();
    // Themed data.
    $markup_data = '';
    $wrap_data = '<div id="field-magnitude-of-phenotypes-wrapper-%s">%s</div>%s';

    // Estimate if there are more phenotypes to render in which case show more/less button
    // will be required so as not to overwhelm the frontend.
    // On average 6 traits would already take up half the pane height.
    $btn_reveal = '';
    if (count($data['phenotypes']) >= 6) {
      $btn_reveal = '<div id="magnitude-of-phenotypes-btn-reveal">
        <div class="magnitude-of-phenotypes-phenotypes-reveal-on" title="Click to show more or less phenotypes">&nbsp;</div>
      </div>';
      
      // With the reveal button on, add listner for click to reveal more phenotypes.
      drupal_add_js($base_url . '/' . drupal_get_path('module', 'analyzedphenotypes') . '/includes/TripalFields/local__magnitude_of_phenotypes/theme/script.js',
        array(
          'type' => 'file',   // An external file scrip.js in theme/
          'scope' => 'footer' // At foot of DOM to ensure document is ready (container width set).
        )
      ); 
    }

    foreach($data as $i => $d) {
      $ctr = 0;
      foreach($d as $m => $value) { 
        $note = '<small>*Phenotype of interest for this cultivar</small>';
        
        // Trait definition might be long, we want just the 100 chars for layout pupose
        // so that all items will be on a unified height. Link is provided to view full details of trait.
        $value = ($i == 'phenotypes') ? truncate_utf8($value, 130, TRUE, TRUE) . $note : $value;
        $value = ($m == 'Trait Distribution Plot') ? l($m, $value) : $value;

        $m     = ($m == 'Trait Distribution Plot') ? '' : $m . ':';
        $m     = ($i == 'phenotypes' && $entity_ids[$ctr] != '#') ? l($m,  $base_url . '/bio_data/' . $entity_ids[$ctr]) : $m; 
        $render[$i][] = $m . '<span>' . $value . '</span>';
      
        $ctr++;
      }

      $t = theme('item_list', array('items' => $render[$i], 'attributes' => array('id' => 'list-magnitude-of-phenotypes-' . $i)));

      $with_btn = ($i == 'phenotypes') ? $btn_reveal : '';
      $markup_data .= sprintf($wrap_data, $i, $t, $with_btn);
    }

    // All set, render items.
    $element[0] = array(
      '#type' => 'markup',
      '#markup' => '<div id="field-magnitude-of-phenotypes-wrapper">' . $markup_data . '</div>',
    );


    return $element;
  }
}
