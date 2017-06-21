<?php
/**
 * @file
 * Contains administrative interfaces of this module.
 */


module_load_include('inc', 'analyzedphenotypes', 'include/analyzedphenotypes.validators');


/**
 * Function callback: Construct main administrative page directory of this module.
 *
 * Update $arr_dir to add or remove page directories. Order of links is the order of the array.
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

  $links = '';
  foreach($arr_dir as $key => $page_prop) {
    $url = l(ucwords($key), 'admin/tripal/extension/analyzedphenotypes/' . $page_prop['page_id']);
    // String separator: @ and # should be replaced with newline.
    $links .= '@' . $url . '#' . $page_prop['info'];
  }

  // Page title.
  $form['admin_directory'] = array(
    '#markup' => 'Analyzed Phenotypes Module',
    '#item' => $links,
  );


  return $form;
}


/**
 * Function callback: Construct analyzed phenotypes data loader stages.
 */
function analyzedphenotypes_admin_data_loader($form, &$form_state) {
  // Attach styles, js and libraries.
  $path = drupal_get_path('module', 'analyzedphenotypes') . '/theme/';

  $form['#attached']['css'] = array();
  $form['#attached']['css'][] = $path . 'css/analyzedphenotypes.loader.global.css';


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
    '#id' => 'ap-main-form-fieldset',
  );

  // Render stage indicator markup
  $form[$main_fieldset]['ap_stage_indicator'] = array(
    '#type' => 'markup',
    '#markup' => $stage_indicators
  );


  // Main switchboard.
  // Invoke function to construct form for the current stage.
  switch($current_page) {
    case 'upload':
      // Is STAGE 1.
      // This stage is to let user select project, genus and upload file.
      $form['#attached']['css'][] = $path . 'css/analyzedphenotypes.loader.dragdrop.css';
      $form['#attached']['js']    = array($path . 'js/analyzedphenotypes.loader.dragdrop.js');

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


  // Next stage submit button.
  // No button on last stage and by default is hidden.
  if ($current_page != 'save') {
    $form['next_step'] = array(
      '#type' => 'submit',
      '#value' => 'Next Step',
      '#weight' => 100,
      '#id' => 'ap-next-stage-submit-field',
    );
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
  // AUTOCOMPLETE FIELD.
  $form[$main_fieldset]['ap_project_select_field'] = array(
    '#title' => 'Experiment',
    '#type' => 'textfield',
    '#maxlength' => 200,
    '#size' => 100,
    '#required' => TRUE,

    // @see analyzedphenotypes_autocomplete_search() in function.inc
    // where $dataype = projects.
    '#autocomplete_path' => 'admin/tripal/extension/analyzedphenotypes/autocompletesearch/projects',

    // When project is selected, autofill the genus field with corresponding project genus.
    '#ajax' => array(
      'callback' => 'analyazedphenotypes_AJAX_callback',
      'wrapper' => 'ap-AJAX-container',
      'method' => 'replace',
      'progress' => array(
        'type' => 'throbber',
        'message' => 'Loading project genus...'
      ),
    ),
    '#description' => t('Lorem ipsum dolor sit amet'),
    '#id' => 'ap-project-select-field'
  );

  // Select genus form field.
  // Autofill project genus when set.
  // AJAX WRAPPER.
  $form[$main_fieldset]['ap_AJAX_container'] = array(
    '#prefix' => '<div id="ap-AJAX-container">',
    '#suffix' => '</div>',
  );

  $arr_genus = analyzedphenotypes_genus_property();
  array_unshift($arr_genus, '- Select -');

  if (isset($form_state['values']['ap_project_select_field'])
      && !empty($form_state['values']['ap_project_select_field'])) {

    // Default the genus to project genus.
    $project_name = $form_state['values']['ap_project_select_field'];

    // Fetch the project id given project name. This is redunant due to how
    // autocomplete search requirement where the option is both the key and value.
    $project_id = analyzedphenotypes_project_properties('project_match_name', $project_name);

    if ($project_id) {
      $arr_genus = analyzedphenotypes_project_properties('project_genus', $project_id);
    }
  }

  // SELECT FIELD.
  $form[$main_fieldset]['ap_AJAX_container']['ap_genus_select_field'] = array(
    '#type' => 'select',
    '#title' => t('Genus'),
    '#required' => TRUE,
    '#options' => $arr_genus,
    '#default_value' => array_keys($arr_genus)[0],
    '#description' => t('Lorem ipsum dolor sit amet'),
    '#id' => 'ap-genus-select-field',
  );

  // Drag and Drop file upload form field.
  // DND FIELD.
  $form = analyzedphenotypes_loader_dnd($form, $form_state);


  return $form;
}

/**
 * Implements hook_file_validate().
 * Ensure that file is a valid Tab Separated Values (.tsv) or Text file (.txt) file.
 *
 * Validate file in STAGE 1 - UPLOAD.
 * @see analyzedphenotypes_loader_form_upload().
 */
function analyzedphenotypes_file_validate($file) {
  $source = $file->source;

  if ($source == 'ap_dnd_field') {
    // Project details.
    $project_name = $_POST['ap_project_select_field'];

    // Genus details.
    $genus = $_POST['ap_genus_select_field'];

    // Validattion is at file level.
    $status = analyzedphenotypes_validate_tsv_file($project_name, $genus, $file);


    dpm($status);

    return FALSE;

  }
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
    '#title' => t('TSV Data File'),
    '#file_upload_max_size' => '10M',
    '#upload_location' => 'public://',
    '#upload_event' => 'auto',
    '#required' => TRUE,
    '#droppable_area_text' => t('- Drag and drop your tsv file here -'),
    '#progress_indicator' => 'throbber',
    '#progress_message' => 'Validating your file. Please wait...',
    '#allow_replace' => 1,
    '#standard_upload' => 1,
    '#upload_button_text' => '',
    '#description' => t('Tab Separated Values (.tsv) or Text file (.txt) only'),
    '#id' => 'ap-dnd-field',
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





