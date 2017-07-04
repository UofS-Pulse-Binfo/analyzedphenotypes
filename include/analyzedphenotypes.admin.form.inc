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
  $arr_directory = array(
    'data loader' => array('page_id' => 'data_loader', 'info' => 'Lorem ipsum dolor sit amet'),
    'page xyz' => array('page_id' => 'abc_xyz', 'info' => 'Another page - Lorem ipsum dolor sit amet'),
    'yet another page' => array('page_id' => 'cde_fgh', 'info' => 'Yet Another page - Lorem ipsum dolor sit amet'),
  );

  $form['#ap_admin_directory'] = $arr_directory;


  return $form;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

  $current_stage = $form_state['stage'];

  // Construct markup of stage title and stage tracker.
  list($stage_title, $stage_indicators) = analyzedphenotypes_get_stage($current_stage);
  $form['#ap_stage_title'] = $stage_title;
  $form['#ap_stage_indicators'] = $stage_indicators;

  $form['#ap_current_stage'] = $current_stage;

  // Main switchboard.
  // Invoke function to construct form for the current stage.
  switch($current_stage) {
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
      $form['#attached']['css'][] = $path . 'css/analyzedphenotypes.loader.stage2.css';
      drupal_add_js('misc/progress.js');
      $form['#attached']['js']    = array($path . 'js/analyzedphenotypes.loader.progress.js');

      $form = analyzedphenotypes_loader_form_validates($form, $form_state);
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
  if ($current_stage != 'save') {
    $form['ap_next_step_button'] = array(
      '#type' => 'submit',
      '#value' => 'Next Step',
      '#weight' => 100,
      '#id' => 'ap-next-stage-submit-field',
    );
  }


  return $form;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// FUNCTION CALLBACK: HOOK_FORM().

/**
 * Function callback: Construct STAGE 1 - UPLOAD form interface.
 */
function analyzedphenotypes_loader_form_upload($form, &$form_state) {
  // Autocomplete search project form field.
  // AUTOCOMPLETE FIELD.
  $form['ap_project_select_field'] = array(
    '#title' => 'Experiment',
    '#type' => 'textfield',
    '#maxlength' => 200,
    '#size' => 100,
    '#required' => TRUE,

    // @see analyzedphenotypes_autocomplete_search() in function.inc
    // where $dataype = projects.
    '#autocomplete_path' => 'admin/tripal/extension/analyzedphenotypes/json/projects',

    // When project is selected, autofill the genus field with corresponding project genus.
    '#ajax' => array(
      'callback' => 'analyazedphenotypes_AJAX_callback',
      'case' => 'autocomplete_search_project',
      'wrapper' => 'ap-AJAX-wrapper',
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
  $form['ap_AJAX_wrapper'] = array(
    '#prefix' => '<div id="ap-AJAX-wrapper">',
    '#suffix' => '</div>',
  );

  $arr_all_genus = analyzedphenotypes_genusprop();
  array_unshift($arr_all_genus, '- Select -');

  if (isset($form_state['values']['ap_project_select_field'])
      && !empty($form_state['values']['ap_project_select_field'])) {

    // Autocomple provides the project name, with the name find the project id.
    $project_name = $form_state['values']['ap_project_select_field'];
    $project_id = analyzedphenotypes_projectprop('project_match_name', $project_name);

    if ($project_id) {
      $arr_project_genus = analyzedphenotypes_projectprop('project_genus', $project_id);
    }
  }

  $genus_options = (isset($arr_project_genus) && $arr_project_genus != 0) ? $arr_project_genus : $arr_all_genus;

  // SELECT FIELD.
  $form['ap_AJAX_wrapper']['ap_genus_select_field'] = array(
    '#type' => 'select',
    '#title' => t('Genus'),
    //'#required' => TRUE,
    '#options' => $genus_options,
    '#default_value' => array_keys($genus_options)[0],
    '#description' => t('Lorem ipsum dolor sit amet'),
    '#id' => 'ap-genus-select-field',
  );

  // Drag and Drop file upload form field.
  // DND FIELD.
  $form = analyzedphenotypes_dragdrop($form, $form_state);


  return $form;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *
 */
function analyzedphenotypes_admin_data_loader_submit($form, &$form_state) {
  // Which button triggers a submit action.
  $btn_submit = $form_state['triggering_element']['#value'];

  // Save file id when present.
  if (isset($form_state['values']['ap_dnd_field'])) {
    $form_state['multistep_values']['fid'] = $form_state['values']['ap_dnd_field'];
  }

  if ($btn_submit == 'Next Step') {
    // Definitely save the form id.
    if(isset($form_state['multistep_values']['form_build_id'])) {
      $form_state['values']['form_build_id'] = $form_state['multistep_values']['form_build_id'];
    }

    // Save the values from the current step.
    $form_state['multistep_values'][$form_state['stage']] = $form_state['values'];

    // Iterate to the next step.
    $form_state['new_stage'] = analyzedphenotypes_next_stage($form, $form_state);

    // Ensure the form state is saved and the form is rebuilt.
    $form_state['multistep_values']['form_build_id'] = $form_state['values']['form_build_id'];
    $form_state['stage'] = $form_state['new_stage'];
    $form_state['rebuild'] = TRUE;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
function analyzedphenotypes_next_stage($form, &$form_state) {
  // Get the address/name of the next page based on the current stage.
  $next_stage = '';

  switch($form_state['stage']) {
    //
    case 'upload':
      $next_stage = 'validate';
      break;

    //
    case 'validate':
      $next_stage = 'describe';
      break;

    //
    case 'describe':
      $next_stage = 'save';
      break;
  }


  return $next_stage;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



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
    $project_genus = $_POST['ap_genus_select_field'];

    // Validation is at file level.
    $dataset = array(
      'project_name'  => $project_name,
      'project_genus' => $project_genus,
      'data_file'     => $file,
    );

    // Begin validation process.
    $status = analyzedphenotypes_validate_tsv_file($dataset);

    // We want to show the user which steps passed/failed even if all of them passed,
    // so lets do that now. We use drupal_set_message() because returning from this function
    // creates an error message and halts file upload, whereas, using drupal_set_message()
    // allows us to print to the screen regardless of failure/success.
    drupal_set_message(analyzedphenotypes_validation_result('file', $status), 'ap-content-window');

    $all_results = array_values($status);

    // Find a single validator that returned FALSE.
    // One false result should trigger a validation error.
    foreach($all_results as $result) {
      if ($result['type'] == 'failed') {
        return FALSE;
      }
    }

    // NOTE: ap-validator-success class signals that it passed all validation
    //       may proceed to the next stage.
    drupal_set_message('Your file uploaded successfully. Please click "Next Step" to continue', 'status ap-validator-success', $repeat = FALSE);
    return array();
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/**
 * Function callback: Construct STAGE 2 - VALIDATE form interface.
 */
function analyzedphenotypes_loader_form_validates($form, &$form_state) {

 $job_id = 2632;

  // Experiment.
  $project_name = $form_state['values']['ap_project_select_field'];
  // Genus.
  $project_genus = $form_state['values']['ap_genus_select_field'];
  // Data file.
  $data_file = $form_state['multistep_values']['fid'];

  global $user;


  $job_id = tripal_add_job(
    'Validate Analyzed Phenotypes',
    'analyzedphenotypes',
    'analyzedphenotypes_validate_tsv_data',
    array(
      $project_name,
      $project_genus,
      $data_file
    ),
    $user->uid
  );


  $path_tripalext = base_path() . 'admin/tripal/extension/analyzedphenotypes';

  $settings = array(
    'job_id' => $job_id,
    'path_json' => $path_tripalext . '/json/validate_jobstatus/',
    'path_validation' => $path_tripalext . '/validation_result/data/',
  );

  drupal_add_js(array('analyzedphenotypes' => array('vars' => $settings)), array('type' => 'setting'));


  return $form;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/**
 * Function callback: Construct STAGE 1 - DESCRIBE form interface.
 */
function analyzedphenotypes_loader_describe_form($form, &$form_state) {

  return $form;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Function callback: Construct STAGE 1 - SAVE form interface.
 */
function analyzedphenotypes_loader_save_form($form, &$form_state) {

  return $form;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// HOOK VALIDATE


// HOOK SUBMIT


// HELPER FUNCTION

/**
 * Helper function: Construct Drag and Drop file upload area.
 */
function analyzedphenotypes_dragdrop($form, &$form_state) {
  $form['ap_dnd_field'] = array(
    '#type' => 'dragndrop_upload',
    '#title' => t('TSV Data File'),
    '#file_upload_max_size' => '10M',
    '#upload_location' => 'public://',
    '#upload_event' => 'auto',
    '#upload_validators' => array(
      'file_validate_extensions' => array(),
    ),
    //'#required' => TRUE,
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Helper function: Create markup for stage indicators.
 */
function analyzedphenotypes_get_stage($stage) {
  $arr_stages = array(
    'Upload'   => '', // Stage 1 - Select project, genus and upload data file.
    'Validate' => '', // Stage 2 - Validate data.
    'Describe' => '', // Stage 3 - Fully describe all traits.
    'Save'     => '', // Stage 4 - Save data and file.
  );

  // Current stage.
  $stage = ucfirst($stage);
  $stage_titles = array_keys($arr_stages);
  $stage_id = array_search($stage, $stage_titles);

  $i = 0;
  foreach($arr_stages as $stage => $status) {
    $s = ($i <= $stage_id) ? 'stage' : 'todo';
    $arr_stages[$stage] = $s;

    $i++;
  }

  $stage_title = 'stage ' . ($stage_id + 1) . ' of ' . count($arr_stages) . ' - ' . $stage;


  return array($stage_title, $arr_stages);
}
