<?php
/**
 * @class
 * Purpose:
 *
 * Allowing edit?
 * Data:
 * Assumptions:
 */
class ncit__method_widget extends ChadoFieldWidget {

  // The default label for this field.
  public static $default_label = 'Collection Methods';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('ncit__method');


 /**
  * @see ChadoFieldWidget::form()
  *
  **/

  public function form(&$widget, &$form, &$form_state, $langcode, $items, $delta, $element) {
    parent::form($widget, $form, $form_state, $langcode, $items, $delta, $element);

    $widget[$delta] = array(
      '#type' => 'textarea',
      '#title' => 'Method: '.$items[$delta]['value']['schema:name'],
      '#default_value' => $items[$delta]['value']['IAO:0000115'],
      '#disabled' => TRUE,
      '#description' => 'Data has already been saved using this method. As such this field has been disabled to maintain accurate metadata. If an error was made during upload, contact your administrator who can change it through the controlled vocabulary admininstrative interface.',
    );
  }

  /**
  * @see ChadoFieldWidget::validate()
  *
  **/
  public function validate($element, $form, &$form_state, $langcode, $delta) {
  }

   /**
  * @see ChadoFieldWidget::submit()
  *
  **/
  public function submit($form, &$form_state, $entity_type, $entity, $langcode, $delta) {
  }

}
