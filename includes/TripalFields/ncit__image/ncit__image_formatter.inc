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
class ncit__image_formatter extends TripalFieldFormatter {
  // The default label for this field.
  public static $default_label = 'Phenotype Image';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('ncit__image');

  // The list of default settings for this formatter.
  // Due to the different charts available and the dependance upon the data,
  // it is too difficult to make a good default for the title and legend.
  // As such I am making a horrible one in the hopes of making admin change it ;-p.
  public static $default_settings = array(
    'image_stack' => 'Left to Right',
    'image_position' => 'Top to Bottom',
  );


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
    drupal_add_css(drupal_get_path('module', 'analyzedphenotypes') . '/includes/TripalFields/ncit__image/theme/style_phenotype_image.css');
    drupal_add_js(drupal_get_path('module', 'analyzedphenotypes') . '/includes/TripalFields/ncit__image/theme/script.js');

    // If there is no photo then don't render anything.
    if (empty($items[0]['value'])) {
      return '';
    }
    $trait_photo = array_values($items[0]['value']['NCIT:C48179']);

    // Refer to this ID for CSS styling.
    $id = 'ap-field-phenotype-image-wrapper';

    $element_photo = array();
    foreach($trait_photo as $img) {
      if ($img && file_exists($img['uri'])) {
        $scale = ap_scale_photo($img['height'], $img['width'], 300);

        $element_photo[] = theme('image', array(
          'path' => $img['uri'],
          'attributes' => array('style' => array('width' => 0)),
        ));
      }
    }

    // From the settings: apply class.
    $class_stack = 'ap-' . str_replace(' ', '-', strtolower($display['settings']['image_stack']));
    $class_position = 'ap-' . str_replace(' ', '-', strtolower($display['settings']['image_position']));


    // Render Number of Experimients.
    $element[0] = array(
      '#type' => 'markup',
      '#markup' => '<div id="' . $id . '" class="' . $class_stack . ' ' . $class_position . '">'
        . implode('', $element_photo) . '</div><div class="ap-clear-float">&nbsp;</div>'
    );

    return $element;
  }


    /**
   * Provides a form for the 'Field Settings' of an instance of this field.
   *
   * This function corresponds to the hook_field_instance_settings_form()
   * function of the Drupal Field API.
   *
   * Validation of the instance settings form is not supported by Drupal, but
   * the TripalField class does provide a mechanism for supporting validation.
   * To allow for validation of your setting form you must call the parent
   * in your child class:
   *
   * @code
   *   $element = parent::instanceSettingsForm();
   * @endcode
   *
   * Please note, the form generated with this function does not easily
   * support AJAX calls in the same way that other Drupal forms do.  If you
   * need to use AJAX you must manually alter the $form in your ajax call.
   * The typical way to handle updating the form via an AJAX call is to make
   * the changes in the form function itself but that doesn't work here.
   */
  public function settingsForm($view_mode, $form, &$form_state) {
    // Illustrations showing configurations available.
    $src = drupal_get_path('module', 'analyzedphenotypes') . '/includes/TripalFields/ncit__c48179/theme/configuration.gif';
    $element['configuration'] = array(
      '#type' => 'markup',
      '#markup' => theme('image', array(
        'path' => $src,
        'height' => 150,
        'width' => 480,
      ))
    );

    $display = $this->instance['display'][$view_mode];
    $settings = $display['settings'];

    // FIELD: Stacking order.
    $element['image_stack'] = array(
      '#type' => 'radios',
      '#title' => 'Image stacking',
      '#options' => array('Left to Right' => 'Left to Right', 'Top to Bottom' => 'Top to Bottom'),
      '#default_value' => array_key_exists('image_stack', $settings) ? $settings['image_stack'] : 'Left to Right',
    );

    // FIELD: Float position.
    $element['image_position'] = array(
      '#type' => 'radios',
      '#title' => 'Image position',
      '#options' => array('Float Left' => 'Float Left', 'Float Right' => 'Float Right', 'None' => 'None - Use field order/weight'),
      '#default_value' => array_key_exists('image_position', $settings) ? $settings['image_position'] : 'Float Right',
    );


    return $element;
  }

  /**
   * Provides a summary of the formatter settings.
   *
   * This function corresponds to the hook_field_formatter_settings_summary()
   * function of the Drupal Field API.
   *
   * On the 'Manage Display' page of the content type administration page,
   * fields are allowed to provide a settings form.  This settings form can
   * be used to allow the site admin to define how the field should be
   * formatted.  The settings are then available for the formatter()
   * function of this class.  This function provides a text-based description
   * of the settings for the site developer to see.  It appears on the manage
   * display page inline with the field.  A field must always return a
   * value in this function if the settings form gear button is to appear.
   *
   * See the hook_field_formatter_settings_summary() function for more
   * information.
   *
   * @param $field
   * @param $instance
   * @param $view_mode
   *
   * @return string
   *   A string that provides a very brief summary of the field settings
   *   to the user.
   *
   */
  public function settingsSummary($view_mode) {
    $display = $this->instance['display'][$view_mode];
    $settings = $display['settings'];

    $summary = t('Stack: @image_stack<br>Position: @image_position',
      array(
        '@image_stack' => $settings['image_stack'] ?? '',
        '@image_position' => $settings['image_position'] ?? '')
     );

    return $summary;
  }
}
