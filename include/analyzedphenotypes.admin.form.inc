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
  drupal_session_start();

  if (!isset($form_state['stage'])) {
    // Default loader to upload stage.
    $form_state['stage'] = 'upload';

    if (isset($_SESSION['analyzedphenotypes']['projectprop'])) {
      $p = $_SESSION['analyzedphenotypes']['projectprop'];
      $projectprop = unserialize($p);

      $job_id = $projectprop['job_id'];
      $job_id = analyzedphenotypes_tripaljobprop('get_job', $job_id);

      if ($job_id) {
        $form_state['stage'] = 'validate';
        unset($_SESSION['analyzedphenotypes']['projectprop']);
      }
    }
  }

  $current_stage = $form_state['stage'];
  $form['#ap_current_stage'] = $current_stage;
  $form['ap_stage_hidden_field'] = array(
    '#type' => 'hidden',
    '#value' => $current_stage,
    '#id' => 'ap-stage-hidden-field',
  );

  list($stage_title, $stage_indicators) = analyzedphenotypes_get_stage($current_stage);
  $form['#ap_stage_title'] = $stage_title;
  $form['#ap_stage_indicators'] = $stage_indicators;

  // Include CSS and JavaScript.
  $all_path = analyzedphenotypes_moduleprop();
  $path_theme = $all_path['module'] . '/theme/';

  $form['#attached']['css'] = array();
  $form['#attached']['js']  = array();

  // Main stylesheet.
  $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.global.css';

  // Main switchboard.
  switch($current_stage) {
    //
    case 'upload':
      $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.dragdrop.css';
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.dragdrop.js';
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.stage1.js';

      $form = analyzedphenotypes_loader_form_upload($form, $form_state);

      break;

    //
    case 'validate':
      $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.dragdrop.css';
      $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.stage2.css';
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.progress.js';
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.dragdrop.js';
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.stage2.js';

      drupal_add_js('misc/progress.js');

      $form = analyzedphenotypes_loader_form_validates($form, $form_state, $projectprop);

      break;

    //
    case 'describe':
      $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.stage3.css';

      $form = analyzedphenotypes_loader_form_describe($form, $form_state);

      break;

    //
    case 'save':
      $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.stage2.css';
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.progress.js';
      drupal_add_js('misc/progress.js');


      $form = analyzedphenotypes_loader_form_save($form, $form_state);

      break;
  }

  // Next stage submit button.
  if ($current_stage != 'save') {
    // No button on last stage.
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


// STAGE 1

/**
 *
 */
function analyzedphenotypes_loader_form_upload($form, &$form_state) {
  $all_path = analyzedphenotypes_moduleprop();
  $path_ap = $all_path['ap'];

  // Experiment:
  $form['ap_project_select_field'] = array(
    '#title' => 'Experiment',
    '#type' => 'textfield',
    '#maxlength' => 200,
    '#size' => 100,
    '#required' => TRUE,

    // @see analyzedphenotypes_data_json() in functions.inc
    // where $dataype = projects.
    '#autocomplete_path' => $path_ap . '/json/projects/experiment',

    // When project is selected, autofill the genus field with corresponding project genus.
    // @see analyazedphenotypes_AJAX_callback() in functions.inc
    // where $case = autofill_genus.
    '#ajax' => array(
      'callback' => 'analyazedphenotypes_AJAX_callback',
      'case' => 'autofill_genus',
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

  // All genus.
  $arr_all_genus = analyzedphenotypes_genusprop();
  array_unshift($arr_all_genus, '- Select -');

  // Project genus - autofill.
  if (isset($form_state['values']['ap_project_select_field'])) {
    $project = $form_state['values']['ap_project_select_field'];
    $project = trim($project);

    if (!empty($project)) {
      $project_id = analyzedphenotypes_projectprop('match_name', $project);

      if ($project_id) {
        $project_genus = analyzedphenotypes_genusprop('project_genus', $project_id);

        if ($project_genus) {
          $arr_project_genus = array($project_genus => $project_genus);
        }
      }
    }
  }

  $genus_options = (isset($arr_project_genus)) ? $arr_project_genus : $arr_all_genus;

  // AJAX callback wrapper.
  $form['ap_AJAX_wrapper'] = array(
    '#prefix' => '<div id="ap-AJAX-wrapper">',
    '#suffix' => '</div>',
  );

  // Genus:
  $form['ap_AJAX_wrapper']['ap_genus_select_field'] = array(
    '#type' => 'select',
    '#title' => t('Genus'),
    '#required' => TRUE,
    '#options' => $genus_options,
    '#default_value' => array_keys($genus_options)[0],
    '#description' => t('Lorem ipsum dolor sit amet'),
    '#id' => 'ap-genus-select-field',
  );

  // Drag and Drop:
  // @see analyzedphenotypes_dragdrop() below.
  $form = analyzedphenotypes_dragdrop($form, $form_state, 'ap_dnd_field');

  $form_state['storage']['testing'] = 'hello';

  return $form;
}


/**
 *
 */
function analyzedphenotypes_dragdrop($form, &$form_state, $source_name) {
  $file_extensions = analyzedphenotypes_fileprop();

  foreach($file_extensions as $extension => $info) {
    $text[] = $info . ' (' . $extension . ')';
  }

  $text = implode(', ', $text) . ' only';
  $source_id = str_replace('_', '-', $source_name);

  // Drag and Drop:
  $form[$source_name] = array(
    '#type' => 'dragndrop_upload',
    '#title' => t('TSV Data File'),
    '#file_upload_max_size' => '10M',
    '#upload_location' => 'public://',
    '#upload_event' => 'auto',
    '#upload_validators' => array(
      'file_validate_extensions' => array(),
    ),
    '#droppable_area_text' => t('- Drag and drop your file here -'),
    '#progress_indicator' => 'throbber',
    '#progress_message' => 'Validating your file. Please wait...',
    '#allow_replace' => 1,
    '#standard_upload' => 1,
    '#upload_button_text' => '',
    '#description' => t($text),
    '#id' => $source_id,
  );

  $settings = array(
    'source' => $source_id,
  );

  drupal_add_js(array('analyzedphenotypes' => array('vars' => $settings)), array('type' => 'setting'));


  return $form;
}


function analyzedphenotypes_file_insert($file) {
  $source = $file->source;

  if ($source == 'ap_dnd_field_reupload') {
    if (isset($_POST['ap_project_select_field']) &&
        isset($_POST['ap_genus_select_field'])) {

      $project_name = $_POST['ap_project_select_field'];
      $project_genus = $_POST['ap_genus_select_field'];
      $data_file = $file->fid;

      $dataset = array(
        'description' => 'Validate Analyzed Phenotypes',
        'callback'    => 'analyzedphenotypes_validate_tsv_data',
        'data' => array(
          'project_name'  => $project_name,
          'project_genus' => $project_genus,
          'data_file'     => $data_file,
        )
      );

      $job_id = analyzedphenotypes_tripaljobprop('new_job', $dataset);

      if ($job_id) {
        $dataset['data']['job_id'] = $job_id;
        $_SESSION['analyzedphenotypes']['projectprop'] = serialize($dataset['data']);

        $all_path = analyzedphenotypes_moduleprop();
        drupal_add_js(array('analyzedphenotypes' => array('redirect' => $all_path['loader'])), array('type' => 'setting'));
      }
    }
  }
}



/**
 *
 */
function analyzedphenotypes_file_validate($file) {
  $source = $file->source;

  if ($source == 'ap_dnd_field' || $source == 'ap_dnd_field_reupload') {
    if (isset($_POST['ap_project_select_field']) &&
        isset($_POST['ap_genus_select_field'])) {

      $project_name = $_POST['ap_project_select_field'];
      $project_genus = $_POST['ap_genus_select_field'];

      // Validation is at file level.
      $dataset = array(
        'project_name'  => $project_name,
        'project_genus' => $project_genus,
        'data_file'     => $file,
      );

      // Begin validation process.
      $status = analyzedphenotypes_validate_tsv_file($dataset);
      drupal_set_message(analyzedphenotypes_validation_result('file', $status), 'ap-result-panel');

      $all_results = array_values($status);

      foreach($all_results as $result) {
        if ($result['type'] == 'failed') {
          return FALSE;
        }
      }

      return array();
    }
  }
}





// STAGE 2.

/**
 *
 */
function analyzedphenotypes_loader_form_validates($form, &$form_state, $projectprop) {
  $all_path = analyzedphenotypes_moduleprop();

  if (isset($form_state['values']['ap_project_select_field']) &&
      isset($form_state['values']['ap_genus_select_field'])   &&
      isset($form_state['multistep_values']['fid'])) {

    $project_name = $form_state['values']['ap_project_select_field'];
    $project_genus = $form_state['values']['ap_genus_select_field'];
    $data_file = $form_state['multistep_values']['fid'];

    $dataset = array(
      'description' => 'Validate Analyzed Phenotypes',
      'callback'    => 'analyzedphenotypes_validate_tsv_data',
      'data' => array(
        'project_name'  => $project_name,
        'project_genus' => $project_genus,
        'data_file'     => $data_file,
      )
    );

    $job_id = analyzedphenotypes_tripaljobprop('new_job', $dataset);
  }
  elseif ($projectprop) {
    $job_id = $projectprop['job_id'];
    $job_id = analyzedphenotypes_tripaljobprop('get_job', $job_id);

    $project_name  = $projectprop['project_name'];
    $project_genus = $projectprop['project_genus'];
    $data_file     = $projectprop['data_file'];
  }

  if ($job_id) {
    $js_var = array(
      'stage'    => 'validate',
      'job_id'    => $job_id,
      'path_JSON' => $all_path['base_admin'] . '/json/validate_jobstatus/',
      'path_VR'   => $all_path['base_admin'] . '/validation_result/data/',
    );

    drupal_add_js(array('analyzedphenotypes' => array('vars' => $js_var)), array('type' => 'setting'));
  }

  // Hidden field: Experiment/project name.
  $form['ap_project_select_field'] = array(
    '#type' => 'hidden',
    '#value' => $project_name,
  );

  // Hidden field: Genus.
  $form['ap_genus_select_field'] = array(
    '#type' => 'hidden',
    '#value' => $project_genus,
  );

  // Hidden field: File FID.
  $form['ap_file_fid_field'] = array(
    '#type' => 'hidden',
    '#value' => $data_file,
  );

  // Drag and Drop:
  // @see analyzedphenotypes_dragdrop() above.
  $form = analyzedphenotypes_dragdrop($form, $form_state, 'ap_dnd_field_reupload');


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





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/**
 * Function callback: Construct STAGE 1 - DESCRIBE form interface.
 */
function analyzedphenotypes_loader_form_describe($form, &$form_state) {
  $project_name = $form_state['values']['ap_project_select_field'];
  $project_genus = $form_state['values']['ap_genus_select_field'];
  $data_file = $form_state['values']['ap_file_fid_field'];


  // Hidden field: Experiment/project name.
  $form['ap_project_select_field'] = array(
    '#type' => 'hidden',
    '#value' => $project_name,
  );

  // Hidden field: Genus.
  $form['ap_genus_select_field'] = array(
    '#type' => 'hidden',
    '#value' => $project_genus,
  );

  // Hidden field: File FID.
  $form['ap_file_fid_field'] = array(
    '#type' => 'hidden',
    '#value' => $data_file,
  );


  $data_file = file_load($data_file);
  $file_content = analyzedphenotypes_fileprop('get_contents', $data_file);

  if ($file_content) {
    ///
    $all_path = analyzedphenotypes_moduleprop();

    $rows = explode("\n", $file_content);
    unset($file_content);

    $t = analyzedphenotypes_columnsprop();
    $trait_name = array_keys($t)[0];

    $c = str_getcsv($columns_row, "\t");
    $columns = array_map('trim', $c);

    $index = array_search($trait_name, $columns);

    $arr_trait_describe = array();

    $i = 0;
    $main_fieldset = 'ap_trait_form_fieldset';

    foreach($rows as $j => $line) {
      if ($j > 0) {
        $data = str_getcsv($line, "\t");
        $trait = trim($data[$index]);

        if (in_array($trait, $arr_trait_describe)) {
          continue;
        }
        else {
          /////////////////////////////////////////////////////////////////////////////////
          $arr_trait_describe[] = $trait;
          $class_collapsed = ($i == 0) ? '' : 'collapsed';
          $ap_fieldset = $main_fieldset . $i;

          // Fieldset:
          $form[$ap_fieldset] = array(
            '#type' => 'fieldset',
            '#title' => '#' . ($i + 1) . '. ' . $trait,
            '#collapsible' => TRUE,
            '#attributes' => array(
              'class' => array('ap-form-fieldset-container', $class_collapsed)
            ),
            '#id' => $main_fieldset . $i,
          );

          // Field group title:
          $form[$ap_fieldset]['ap_group_about_the_trait' . $i] = array(
            '#type' => 'item',
            '#title' => t('&#9660; ABOUT THE TRAIT'),
          );

          // Trait name:
          $form[$ap_fieldset]['ap_name_form_field' . $i] = array(
            '#type' => 'textfield',
            '#title' => t('Name'),
            '#description' => t('Lorem ispsum dolor sit amet'),
            '#id' => 'ap-name-form-field' . $i,
          );

          // Description:
          $form[$ap_fieldset]['ap_description_form_field' . $i] = array(
            '#type' => 'textarea',
            '#title' => t('Description'),
            '#description' => t('Loren ipsum dolor sit amet'),
            '#id' => 'ap-description-form-field' . $i,
          );

          // Unit
          $form[$ap_fieldset]['ap_unit_form_field' . $i] = array(
            '#type' => 'textfield',
            '#title' => t('Unit'),
            '#description' => t('Lorem ispsum dolor sit amet'),
            '#id' => 'ap-unit-form-field' . $i,
          );

          // Scale:
          $form[$ap_fieldset]['ap_scale_form_field' . $i] = array(
            '#type' => 'textfield',
            '#title' => t('Scale'),
            '#description' => t('Lorem ispsum dolor sit amet'),
            '#id' => 'ap-scale-form-field' . $i,
          );

          // Field group title:
          $form[$ap_fieldset]['ap_group_mapping_information' . $i] = array(
            '#type' => 'item',
            '#title' => t('&#9660; MAPPING INFORMATION'),
          );

          // Crop ontology:
          $form[$ap_fieldset]['ap_crop_ontology_form_field' . $i] = array(
            '#title' => 'Crop Ontology',
            '#type' => 'textfield',

            // @see analyzedphenotypes_autocomplete_search() in function.inc
            // where $dataype = ontology.
            '#autocomplete_path' => $all_path['ap'] . '/json/ontology',
            '#id' => 'ap-crop-ontology-form-field' . $i,
          );

          // Plant ontology:
          $form[$ap_fieldset]['ap_plant_ontology_form_field' . $i] = array(
            '#title' => 'Plant Trait Ontology',
            '#type' => 'textfield',

            // @see analyzedphenotypes_autocomplete_search() in function.inc
            // where $dataype = ontology.
            '#autocomplete_path' => $all_path['ap'] . '/json/ontology',
            '#id' => 'ap-plant-ontology-form-field' . $i,
          );

          // Field group title:
          $form[$ap_fieldset]['ap_group_photo_upload' . $i] = array(
            '#type' => 'item',
            '#title' => t('&#9660; PHOTO UPLOAD'),
          );

          // Photo 1:
          $form[$ap_fieldset]['ap_photo_1_form_field' . $i] = array(
            '#type' => 'file',
            '#title' => t('Image 1 of 2'),
            '#id' => 'ap-photo-1-form-field' . $i,
          );

          // Photo 2:
          $form[$ap_fieldset]['ap_photo_2_form_field' . $i] = array(
            '#type' => 'file',
            '#title' => t('Image 2 of 2'),
            '#id' => 'ap-photo-2-form-field' . $i,
          );

          // Field group title:
          $form[$ap_fieldset]['ap_group_summary_table' . $i] = array(
            '#type' => 'item',
            '#title' => t('&#9660; SUMMARY TABLE'),
          );

          $table_rows[] = array('Regina 2014', 213.5, 9.5, 22.58, 55.3);
          $table_rows[] = array('Saskatton 2016', 23.5, 29.5, 12.58, 85.3);
          $table_rows[] = array('Broadway Ave, Saskatton', 1.5, 3.5, 5.58, 3.3);

          // Summary table.
          $form[$ap_fieldset]['ap_summary_table' . $i] = array(
            '#type' => 'markup',
            '#markup' => theme('table',
              array(
                'header' => array('Site-year', 'Minimum Value', 'Maximum Value', 'Mean Value', 'Standard Deviation'),
                'rows' => $table_rows,
                'attributes' => array('id' => 'ap_summary_table' . $i)
              )
            ),
          );


          $i++;
          /////////////////////////////////////////////////////////////////////////////////
        }
      }
    }
    ///
  }

  $form_state['multistep_values']['new_headers'] = $arr_trait_describe;
  $form['#ap_describe_trait_count'] = count($arr_trait_describe);
  $form['#ap_main_fieldset'] = $main_fieldset;

  return $form;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Function callback: Construct STAGE 1 - SAVE form interface.
 */
function analyzedphenotypes_loader_form_save($form, &$form_state) {
  $project_name     = $form_state['values']['ap_project_select_field'];
  $project_genus    = $form_state['values']['ap_genus_select_field'];
  $data_file        = $form_state['values']['ap_file_fid_field'];

  $arr_trait_describe = $form_state['multistep_values']['new_headers'];

  foreach ($arr_trait_describe as $i => $trait) {
    $name           = $form_state['values']['ap_name_form_field'           . $i];
    $description    = $form_state['values']['ap_description_form_field'    . $i];
    $unit           = $form_state['values']['ap_unit_form_field'           . $i];
    $scale          = $form_state['values']['ap_scale_form_field'          . $i];
    $crop_ontology  = $form_state['values']['ap_crop_ontology_form_field'  . $i];
    $plant_ontology = $form_state['values']['ap_plant_ontology_form_field' . $i];
    $photo_1        = $form_state['values']['ap_photo_1_form_field'        . $i];
    $photo_2        = $form_state['values']['ap_photo_2_form_field'        . $i];

    analyzedphenotypes_traitprop('insert', array(
      'name'           => $name,
      'description'    => $description,
      'unit'           => $unit,
      'scale'          => $scale,
      'crop_ontology'  => $crop_ontology,
      'plant_ontology' => $plant_ontology,
      'photo_1'        => $photo_1,
      'photo_2'        => $photo_2,
    ));
  }


  $dataset = array(
    'description' => 'Save Analyzed Phenotype',
    'callback' => 'analyzedphenotypes_save_tsv_data',
    'data' => array(
      'project_name' => $project_name,
      'project_genus' => $project_genus,
      'data_file' => $data_file
    )
  );

  // Register a job.
  $job_id = analyzedphenotypes_tripaljobprop('new_job', $dataset);

  if ($job_id) {
    $all_path = analyzedphenotypes_moduleprop();

    $js_var = array(
      'stage'    => 'save',
      'job_id'    => $job_id,
      'path_JSON' => $all_path['base_admin'] . '/json/validate_jobstatus/',
      'path_VR'   => $all_path['base_admin'] . '/validation_result/data/',
    );

    drupal_add_js(array('analyzedphenotypes' => array('vars' => $js_var)), array('type' => 'setting'));
  }


  return $form;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// HOOK VALIDATE


// HOOK SUBMIT


// HELPER FUNCTION

/**
 * Helper function: Construct Drag and Drop file upload area.
 */



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Helper function: Create markup for stage indicators.
 */
function analyzedphenotypes_get_stage($stage) {
  $all_stage = analyzedphenotypes_moduleprop('stages');

  // Current stage.
  $stage = ucfirst($stage);
  $stage_id = array_search($stage, $all_stage);

  foreach($all_stage as $i => $stage) {
    $s = ($i <= $stage_id) ? 'stage' : 'todo';
    $arr_stages[$stage] = $s;
  }

  $stage_title = 'stage ' . ($stage_id + 1) . ' of ' . count($arr_stages) . ' - ' . $stage;


  return array($stage_title, $arr_stages);
}
