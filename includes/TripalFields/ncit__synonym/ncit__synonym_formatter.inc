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
class ncit__synonym_formatter extends TripalFieldFormatter {
  // The default label for this field.
  public static $default_label = 'Crop Ontology';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('ncit__synonym');

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
    // No show when no crop ontology for a trait.
    $element = null;

    // Render crop ontology for this trait.
    if ($items[0]['value']) {
      // Style information.
      drupal_add_css(drupal_get_path('module', 'analyzedphenotypes') . '/includes/TripalFields/ncit__synonym/theme/style_cp_synonym_field.css');

      // Refer to this ID for CSS styling.
      $id = 'ap-field-crop_ontology_synonym';

      list($term, $accession, $cv) = array_values($items[0]['value']);

      // If we can't find the term then do not provide the link as it will just cause a term not found
      // in the term lookup module.
      // @see tripal_vocabulary_lookup_term_page()
      // Tripal.term_lookup.inc.
      $lookterm = tripal_get_term_details($cv, $accession);

      $cvterm_lookup = (!$lookterm) ? ''
        : l('Vocabulary Details', 'cv/lookup/' .  $cv . '/' . $accession, array('attributes' => array('target' => '_blank')));

      $element[0] = array(
        '#type' => 'markup',
        '#markup' => '<div id="' . $id . '">' . $term . '</div> ' . $cvterm_lookup,
      );
    }

    return $element;
  }
}
