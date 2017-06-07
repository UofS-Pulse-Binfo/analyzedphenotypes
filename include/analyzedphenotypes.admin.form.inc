<?php
/**
 * @file
 * Contains administrative interfaces of this module.
 */


/**
 * Function callback: Construct main administrative page directory of this module.
 */
function analyzedphenotypes_admin_page_directory($form, &$form_state) {
  // Create a directory of links in the module.
  // Array to hold administrative page directory in this module.
  // Key - page id and page info.
  $arr_dir = array(
    'data loader' => array('page_id' => 'data_loader', 'info' => 'Lorem ipsum dolor sit amet'),
    'page xyz' => array('page_id' => 'abc_xyz', 'info' => 'Another page - Lorem ipsum dolor sit amet'),
    'yet another page' => array('page_id' => 'cde_fgh', 'info' => 'Yet Another page - Lorem ipsum dolor sit amet'),
  );

  // Construct markup for each link.
  $links = '<ul>';
  foreach($arr_dir as $key => $page_prop) {
    $url = l(ucwords($key), 'admin/tripal/extension/analyzedphenotypes/' . $page_prop['page_id']);

    $links .= '
      <li>
        <p>' . $url . '<br />' . $page_prop['info'] . '</p>
      </li>';
  }
  $links .= '</ul>';

  $form['admin_directory'] = array(
    '#markup' => '
      <h2>Analyzed Phenotypes Module</h2>' . $links
  );

  return $form;
}


/**
 * Function callback: Construct analyzed phenotypes data loader.
 */
function analyzedphenotypes_admin_data_loader($form, &$form_state) {
  // Attache styles, js and libraries.
  $path = drupal_get_path('module', 'analyzedphenotypes') . '/theme/';
  $form['#attached']['css'] = array(
    $path . 'css/analyzedphenotypes.loader.global.css',
    $path . 'css/analyzedphenotypes.loader.dragdrop.css');


  // Inform user when and how to load data.
  $m = 'Phenotypic data should be <strong>filtered for outliers and mis-entries</strong> before being uploaded here.
    Do not upload data that should not be used in the final analysis for a scientific article.
    Furthermore, data should <strong>NOT be averaged across replicates or site-year.</strong>';

  drupal_set_message(t($m), 'warning');


  // On page load, initialize loader to STAGE 1 - Upload.
  if (!isset($form_state['stage'])) {
    $form_state['stage'] = 'upload';
  }
  $current_page = $form_state['stage'];

  // Construct markup of stage title and stage tracker.
  list($stage_title, $stage_indicators) = analyzedphenotypes_get_stage($current_page);

  // Main fieldset to contain all form elements.
  $main_fieldset = 'ap_main_fieldset';
  $form['ap_main_fieldset_title']['#value'] = $main_fieldset;

  // Main fieldset form container. Use the markup stage title as title.
  $form[$main_fieldset] = array(
    '#type' => 'fieldset',
    '#title' => $stage_title,
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  // Render stage indicator markup
  $form[$main_fieldset]['ap_stage_indicator'] = array(
    '#type' => 'markup',
    '#markup' => $stage_indicators
  );


  // Invoke function to construct form for the current stage.
  switch($current_page) {
    case 'upload':
      // Is STAGE 1.
      // This stage is to let user select project, genus and upload file.
      $form = analyzedphenotypes_loader_form_upload($form, $form_state);
      break;

    case 'validate':
      // Is STAGE 2.
      // This will validate data in the file provided.
      $form = analyzedphenotypes_loader_validate_form($form, $form_state);
      break;

    case 'describe':
      // Is STAGE 3.
      // This stage will allow user to fully define/describe traits.
      $form = analyzedphenotypes_loader_describe_form($form, $form_state);
      break;

    case 'save':
      // Is STAGE 4.
      // Finally, save data, file and all.
      $form = analyzedphenotypes_loader_save_form($form, $form_state);
      break;
  }

  return $form;
}



// FUNCTION CALLBACK: HOOK_FORM().

/**
 * Function callback: Construct STAGE 1 - UPLOAD form interface.
 */
function analyzedphenotypes_loader_form_upload($form, &$form_state) {
  // Reference the main fieldset container.
  $main_fieldset = $form['ap_main_fieldset_title']['#value'];

  // Autocomplete search project form field.
  $form[$main_fieldset]['ap_project_select_field'] = array(
    '#title' => 'Experiment',
    '#type' => 'textfield',
    '#maxlength' => 50,
    '#size' => 100,
    '#required' => TRUE,
    '#autocomplete_path' => 'admin/tripal/extension/analyzedphenotypes/autocompletesearch/project',
    '#description' => t('Lorem ipsum dolor sit amet'),
    '#id' => 'ap-project-select-field'
  );

  // Select genus form field.
  // Fetch genus.
  $arr_genus = analyzedphenotypes_fetch_data('genus');

  $form[$main_fieldset]['ap_genus_select_field'] = array(
    '#type' => 'select',
    '#title' => t('Genus'),
    '#required' => TRUE,
    '#options' => $arr_genus,
    '#default_value' => array_keys($arr_genus)[0],
    '#description' => t('Lorem ipsum dolor sit amet'),
    '#id' => 'ap-genus-select-field',
  );

  // Drag and Drop file upload form field.
  $form = analyzedphenotypes_loader_dnd($form, $form_state);

  return $form;
}

/**
 * Function callback: Construct STAGE 2 - VALIDATE form interface.
 */
function analyzedphenotypes_loader_validate_form($form, &$form_state) {

  return $form;
}

/**
 * Function callback: Construct STAGE 1 - DESCRIBE form interface.
 */
function analyzedphenotypes_loader_describe_form($form, &$form_state) {

  return $form;
}

/**
 * Function callback: Construct STAGE 1 - SAVE form interface.
 */
function analyzedphenotypes_loader_save_form($form, &$form_state) {

  return $form;
}


// HOOK VALIDATE


// HOOK SUBMIT


// HELPER FUNCTION

/**
 * Helper function: Construct Drag and Drop file upload area.
 */
function analyzedphenotypes_loader_dnd($form, &$form_state) {
  $main_fieldset = $form['ap_main_fieldset_title']['#value'];

  $form[$main_fieldset]['ap_dnd_field'] = array(
    '#type' => 'dragndrop_upload',
    '#file_upload_max_size' => '10M',
    '#upload_location' => 'public://',
    '#upload_event' => 'auto',
    // NOTE: Accept the listed file extension and let the spreadsheet reader tell if file is valid to generate an an error message.
    //       No silent treatment.
    '#upload_validators' => array(
      'file_validate_extensions' => array('xlsx xls jpg jpeg gif png txt doc pdf ppt pps odt ods odp csv'),
    ),
    '#droppable_area_text' => t('Drag and drop your tsv file here'),
    '#progress_indicator' => 'throbber',
    '#progress_message' => 'Validating your file. Please wait...',
    '#allow_replace' => 1,
    '#standard_upload' => 1,
    '#upload_button_text' => '',

    // We are adding our own element process function so that we can make a successfully
    // uploaded/validated file permanent during the AJAX process rather than waiting for
    // them to click the "Next" button.
    '#id' => 'ap-dnd-field'
  );

  return $form;
}





/**
 * Helper function: Create markup for stage indicators.
 *
 * @param $stage
 *   A string containing the current stage. On initial page load,
 *   this value contains the first stage. It will also be used to
 *   correctly mark/highlight the stage in the stage indicators.
 *
 * @return
 *   An array containing 2 elements.
 *   1. Markup of the Title of the current page.
 *   2. Markup of each stage
 *
 *   Both values will make up the title and stage indicator on top of
 *   each page/stage.
 */
function analyzedphenotypes_get_stage($stage) {
  // Array to hold stages.
  $arr_stages = array(
    1 => 'upload',   // STAGE 1 - Select project, genus and upload file.
    2 => 'validate', // Validate data.
    3 => 'describe', // Fully describe all traits.
    4 => 'save',     // Save data and file.
  );

  // Current stage.
  $stage_id = array_search($stage, $arr_stages);

  // Construct markup for each stage indicator.
  $markup_stages = '<div id="ap-container-progress">';
  foreach($arr_stages as $i => $v) {
    // Add the right class to stage indicator to show what is completed and todo.
    $class = ($i <= $stage_id) ? '' : 'ap-progress-stage-todo';

    $markup_stages .= '
      <div class="' . $class . ' ap-progress-stage" title="Data loader progress indicator">
        <span>&nbsp;</span>' . $i . '. ' . ucfirst($v) . '<span>&nbsp;</span>
      </div>';
  }

  $markup_stages .= '<div class="ap-clear-both">&nbsp;</div>';

  // Construct markup for stage title.
  // eg. STAGE X OF Y - ABCD
  $markup_title = 'stage ' . $stage_id . ' of ' . count($arr_stages) . ' - ' . $stage;

  return array($markup_title, $markup_stages);
}


/**
 * Helper function: Register an array to contain stages.
 */




