<?php
/**
 * @file
 * Contains administrative interfaces of this module.
 */


module_load_include('inc', 'analyzedphenotypes', 'include/analyzedphenotypes.validators');
module_load_include('inc', 'analyzedphenotypes', 'api/analyzedphenotypes.api');

/**
 *
 */
function analyzedphenotypes_admin_settings($form, &$form_state, $source) {
  if ($source == 'database_cv' || $source == 'property_terms') {
    $mod_vars = analyzedphenotypes_moduleprop('settings');

    if ($source == 'database_cv') {
      //
      $var_db = $mod_vars['ap_database'];
      $var_cv = $mod_vars['ap_cv'];
      $var_allownew = $mod_vars['ap_allow_new'];
      unset($mod_vars);

      // Database.
      $var_db_value = variable_get($var_db);

      $all_db = analyzedphenotypes_dbprop();
      $db_options = array_map(function($d) {
        return $d['name'];
      }, $all_db);

      // AJAX callback wrapper.
      $form['ap_AJAX_wrapper_dbinfo'] = array(
        '#prefix' => '<div id="ap-AJAX-wrapper-dbinfo">',
        '#suffix' => '</div>',
      );

      $db_field_value = 0;

      if (isset($form_state['values'][$var_db])) {
        $db_field_value = $form_state['values'][$var_db];
      }

      if ($db_field_value > 0) {
        $default_db = $db_field_value;
      }
      else {
        if ($var_db_value == 'not set') {
          $default_db = 0;
          $db_options = array(0 => '- Select -') + $db_options;

        }
        else {
          $default_db = $var_db_value;
        }
      }

      $description_db = ($default_db == 0 || empty($all_db[$default_db]['description']))
        ? 'Description not available'
        : $all_db[$default_db]['description'];

      $form['ap_AJAX_wrapper_dbinfo'][$var_db] = array(
        '#type' => 'select',
        '#title' => t('Database'),
        '#required' => TRUE,
        '#options' => $db_options,
        '#default_value' => $default_db,
        '#description' => $description_db,

        // Load database description.
        '#ajax' => array(
          'event' => 'change',
          'callback' => 'analyzedphenotypes_AJAX_callback',
          'case' => 'dbinfo',
          'wrapper' => 'ap-AJAX-wrapper-dbinfo',
          'progress' => array('type' => 'throbber', 'message' => 'Loading Database description...'),
        ),
        '#id' => $var_db,
      );


      // Controlled Vocabulary.
      $var_cv_value = variable_get($var_cv);
      $all_cv = analyzedphenotypes_cvprop();
      $cv_options = array_map(function($c) {
        return $c['name'];
      }, $all_cv);

      // AJAX callback wrapper.
      $form['ap_AJAX_wrapper_cvinfo'] = array(
        '#prefix' => '<div id="ap-AJAX-wrapper-cvinfo">',
        '#suffix' => '</div>',
      );

      $cv_field_value = 0;

      if (isset($form_state['values'][$var_cv])) {
        $cv_field_value = $form_state['values'][$var_cv];
      }

      if ($cv_field_value > 0) {
        $default_cv = $cv_field_value;
      }
      else {
        if ($var_cv_value == 'not set') {
          $default_cv = 0;
          $cv_options = array(0 => '- Select -') + $cv_options;
        }
        else {
          $default_cv = $var_cv_value;
        }
      }

      $description_cv = ($default_cv == 0 || empty($all_cv[$default_cv]['definition']))
        ? 'Description not available'
        : $all_cv[$default_cv]['definition'];

      $form['ap_AJAX_wrapper_cvinfo'][$var_cv] = array(
        '#type' => 'select',
        '#title' => t('Controlled Vocabulary'),
        '#required' => TRUE,
        '#options' => $cv_options,
        '#default_value' => $default_cv,
        '#description' => $description_cv,
        //
        '#ajax' => array(
          'event' => 'change',
          'callback' => 'analyzedphenotypes_AJAX_callback',
          'case' => 'cvinfo',
          'wrapper' => 'ap-AJAX-wrapper-cvinfo',
          'progress' => array('type' => 'throbber', 'message' => 'Loading CV description...'),
        ),
        '#id' => $var_cv,
      );


      // Allow new traits to be added.
      $var_allownew_value = variable_get($var_allownew);
      $default_allownew = ($var_allownew_value == 'not set') ? 0 : $var_allownew_value;

      $form[$var_allownew] = array(
        '#type' => 'checkbox',
        '#title' => t('Allow new term to be added to the Controlled Vocabulary'),
        '#default_value' => $default_allownew,
        '#id' => $var_allownew,
      );


      return system_settings_form($form);
    }
    elseif ($source == 'property_terms') {
      //
      unset($mod_vars['ap_database'], $mod_vars['ap_cv'], $mod_vars['ap_allow_new']);
      $all_cv = analyzedphenotypes_cvprop();
      $cv_options = array_map(function($c) {
        return $c['name'];
      }, $all_cv);


      foreach($mod_vars as $i => $var) {
        $title = trim(str_replace('ap_', '', $i));
        $var_value = variable_get($var);

        if ($var_value == 'not set' || empty($var_value)) {
          $default_cv = 0;
          $cv_options = array(0 => '- Select -') + $cv_options;
        }
        else {
          $term_prop = analyzedphenotypes_cvprop('get_term', array(
            'cvterm_id' => $var_value
          ));

          $default_cv = $term_prop['cv_id']->cv_id;
        }

        $form[$i . '_cv_select_field'] = array(
          '#type' => 'select',
          '#title' => 'CV & CV term: ' . ucfirst($title),
          '#required' => TRUE,
          '#options' => $cv_options,
          '#default_value' => $default_cv,

          // Load cvterms.
          '#ajax' => array(
            'event' => 'change',
            'callback' => 'analyzedphenotypes_AJAX_callback',
            'case' => 'getcvterms' . $title,
            'wrapper' => 'ap-AJAX-wrapper-getcvterms' . $title,
            'progress' => array('type' => 'throbber', 'message' => 'Loading CV Terms...'),
          ),
          '#id' => $i . '_cv_select_field',
        );

        // AJAX callback wrapper.
        $form['ap_AJAX_wrapper_getcvterms'. $title] = array(
          '#prefix' => '<div id="ap-AJAX-wrapper-getcvterms' . $title . '">',
          '#suffix' => '</div>',
        );

        $cv_field_value = 0;

        if (isset($form_state['values'][$i . '_cv_select_field'])) {
          $cv_field_value = $form_state['values'][$i . '_cv_select_field'];
        }


        if ($cv_field_value > 0) {
          $cv_id = $cv_field_value;
        }
        else {
          if ($var_value == 'not set') {
            $cv_id = 0;
          }
          else {
            $cv_id = $default_cv;
          }
        }

        if ($cv_id > 0) {
          $terms_in_cv = analyzedphenotypes_cvprop('terms_in_cv', $cv_id);

          $term_options = array(0 => '- Select -');

          foreach($terms_in_cv as $i => $v) {
            $term_options[$v['id']] = $i;
          }

          $default_cvterm = ($var_value == 'not set') ? 0 : $var_value;
        }
        else {
          $term_options = array();
          $default_cvterm = null;
        }

        $form['ap_AJAX_wrapper_getcvterms' . $title][$var] = array(
          '#type' => 'select',
          '#title' => '',
          '#options' => $term_options,
          '#default_value' => $default_cvterm,
          '#description' => t('Select Controlled Vocabulary (CV) and CV Term.'),
          '#id' => $i . '_term_select_field',
        );
      }


      return system_settings_form($form);
    }
  }
  else {
    // Main settings.
  }
}


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
    'data loader' => array('page_id' => 'upload', 'info' => 'Lorem ipsum dolor sit amet'),
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
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.stage3.js';

      drupal_add_library('system', 'ui.accordion');

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
      'callback' => 'analyzedphenotypes_AJAX_callback',
      'case' => 'autofillgenus',
      'wrapper' => 'ap-AJAX-wrapper-autofillgenus',
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
  $arr_all_genus = array();
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
  $form['ap_AJAX_wrapper_autofillgenus'] = array(
    '#prefix' => '<div id="ap-AJAX-wrapper-autofillgenus">',
    '#suffix' => '</div>',
  );

  // Genus:
  $form['ap_AJAX_wrapper_autofillgenus']['ap_genus_select_field'] = array(
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
    $columns_row = $rows[0];
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
      if ($j > 0 && !empty($line)) {
        $data = str_getcsv($line, "\t");
        $trait = trim($data[$index]);

        if (in_array($trait, array_keys($arr_trait_describe))) {
          continue;
        }
        else {
          /////////////////////////////////////////////////////////////////////////////////
          $existing_trait = array(
            'term'           => '',
            'name'           => '',
            'definition'     => '',
            'unit'           => '',
            'scale'          => '',
            'crop_ontology'  => '',
            'plant_ontology' => '',
            'photo_1'        => '',
            'photo_2'        => '',
          );

          $ap_fieldset = $main_fieldset . $i;

          // Field group title:
          $form[$ap_fieldset]['ap_group_about_the_trait' . $i] = array(
            '#type' => 'item',
            '#title' => t('&#9660; ABOUT THE TRAIT'),
          );


          // Check if trait is available, then suggest the trait and autfill the form.
          $exists = analyzedphenotypes_traitprop('recordset', $trait);
          $arr_trait_describe[$trait] = $exists['found'];

          $existing_trait['term'] = $exists['term'];
          $existing_trait['name'] = $exists['name'];
          $existing_trait['unit'] = $exists['unit'];

          if ($exists['found']) {
            // Field is description while cvterm field is definition.
            $existing_trait['description']    = $exists['definition'];
            $existing_trait['scale']          = $exists['scale'];
            $existing_trait['crop_ontology']  = $exists['crop_ontology'];
            // NOT SAVED.
            // $existing_trait['plant_ontology'] = $exists['plant_ontology'];
            $existing_trait['photo_1']        = $exists['photo_1'];
            $existing_trait['photo_2']        = $exists['photo_2'];

            // Drupal message:
            $form[$ap_fieldset]['ap_warning' . $i] = array(
              '#type' => 'markup',
              '#id' => 'ap-warning' . $i,
            );
          }

          // CVTerm:
          $form['#ap_cvterm_hidden_field' . $i] = $existing_trait['term'];


          // Trait name:
          $form[$ap_fieldset]['ap_name_text_field' . $i] = array(
            '#type' => 'textfield',
            '#title' => t('Name'),
            '#required' => TRUE,
            '#element_validate' => array('analyzedphenotypes_validate_field_name'),

            '#default_value' => $existing_trait['name'],
            '#description' => t('Lorem ispsum dolor sit amet'),
            '#attributes' => array('class' => array('ap-required')),
            '#id' => 'ap-name-text-field' . $i,
          );

          // Description:
          $form[$ap_fieldset]['ap_description_text_field' . $i] = array(
            '#type' => 'textarea',
            '#title' => t('Description'),
            '#required' => TRUE,

            '#default_value' => $existing_trait['description'],
            '#description' => t('Loren ipsum dolor sit amet'),
            '#attributes' => array('class' => array('ap-required')),
            '#id' => 'ap-description-text-field' . $i,
          );

          // Unit
          $form[$ap_fieldset]['ap_unit_text_field' . $i] = array(
            '#type' => 'textfield',
            '#title' => t('Unit'),
            '#required' => TRUE,
            '#element_validate' => array('analyzedphenotypes_validate_field_unit'),

            '#default_value' => $existing_trait['unit'],
            '#description' => t('Lorem ispsum dolor sit amet'),
            '#attributes' => array('class' => array('ap-required')),
            '#id' => 'ap-unit-text-field' . $i,
          );

          // Scale:
          $form[$ap_fieldset]['ap_scale_text_field' . $i] = array(
            '#type' => 'textfield',
            '#title' => t('Scale'),

            '#default_value' => $existing_trait['scale'],
            '#description' => t('Lorem ispsum dolor sit amet'),
            '#id' => 'ap-scale-text-field' . $i,
          );

          // Field group title:
          $form[$ap_fieldset]['ap_group_mapping_information' . $i] = array(
            '#type' => 'item',
            '#title' => t('&#9660; MAPPING INFORMATION'),
          );

          // Crop ontology:
          $form[$ap_fieldset]['ap_crop_ontology_text_field' . $i] = array(
            '#title' => 'Crop Ontology',
            '#type' => 'textfield',
            '#required' => TRUE,


            '#default_value' => $existing_trait['crop_ontology'],
            '#description' => t('Lorem ipsum dolor sit amet'),
            // @see analyzedphenotypes_autocomplete_search() in function.inc
            // where $dataype = ontology.
            '#autocomplete_path' => $all_path['ap'] . '/json/ontology/' . $existing_trait['name'],
            '#attributes' => array('class' => array('ap-autocomplete-search', 'ap-required')),
            '#id' => drupal_html_id('ap-crop-ontology-text-field' . $i),
          );

          /*
           * Add ontology suggestions.
           */
          $ontology_suggestions = analyzedphenotypes_ontologyprop('suggest', $existing_trait['name']);

          if ($ontology_suggestions) {
            $a = array();
            foreach($ontology_suggestions as $k => $term) {
              $definition = empty($term['definition']) ? $k : $term['definition'];

              $a[] = l($k, '#', array('attributes' => array(
                'alt' => $definition,
                'title' => $definition,
              )));
            }

            $list = theme('item_list', array(
              'items' => $a,
              'title' => 'Ontology Found:',
              'attributes' => array('id' => 'ap-ontology-suggest-' . $i, 'class' => 'ap-item-list'),
            ));

            $form[$ap_fieldset]['ap_ontology_suggestion_markup_field' . $i] = array(
              '#markup' => $list,
            );
          }

          // Plant ontology:
          /*
          $form[$ap_fieldset]['ap_plant_ontology_form_field' . $i] = array(
            '#title' => 'Plant Trait Ontology',
            '#type' => 'textfield',
            '#required' => TRUE,

            '#default_value' => $existing_trait['plant_ontology'],
            '#description' => t('Lorem ipsum dolor sit amet'),
            // @see analyzedphenotypes_autocomplete_search() in function.inc
            // where $dataype = ontology.
            '#autocomplete_path' => $all_path['ap'] . '/json/ontology',
            '#id' => 'ap-plant-ontology-form-field' . $i,
          );
          */

          // Field group title:
          $form[$ap_fieldset]['ap_group_photo_upload' . $i] = array(
            '#type' => 'item',
            '#title' => t('&#9660; PHOTO UPLOAD'),
          );


          // Photo 1:
          if ($existing_trait['photo_1']) {
            $photo_1 = array(
              'path' => $all_path['photo_dir'] . '/' . $existing_trait['photo_1'],
              'height' => 80,
              'width' => 90,
            );

            // Photo #1:
            $form[$ap_fieldset]['ap_photo_1' . $i] = array(
              '#type' => 'markup',
              '#id' => 'ap-photo-1' . $i,
              '#markup' => theme('image', $photo_1),
            );
          }

          $form[$ap_fieldset]['ap_photo_1_file_field' . $i] = array(
            '#type' => 'file',
            '#title' => t('Image 1 of 2'),
            //'#required' => TRUE,
            '#upload_validators' => array(
              'file_validate_extensions' => array('jpg'),
            ),
            '#name' => 'files[ap_photo_1' . $i . ']',
            '#attributes' => array('class' => array('ap-required')),
            '#id' => 'ap-photo-1-file-field' . $i,
          );

          // Photo 2:
          $img_2 = $all_path['photo_dir'] . '/' . $existing_trait['photo_2'];
          $file_exists = image_get_info($img_2);

          if ($existing_trait['photo_2'] && $file_exists) {
            $photo_2 = array(
              'path' => $all_path['photo_dir'] . '/' . $existing_trait['photo_2'],
              'height' => 80,
              'width' => 90,
            );

            // Photo #2:
            $form[$ap_fieldset]['ap_photo_2' . $i] = array(
              '#type' => 'markup',
              '#id' => 'ap-photo-2' . $i,
              '#markup' => theme('image', $photo_2),
            );
          }

          $form[$ap_fieldset]['ap_photo_2_file_field' . $i] = array(
            '#type' => 'file',
            '#title' => t('Image 2 of 2'),
            '#upload_validators' => array(
              'file_validate_extensions' => array('jpg'),
            ),
            '#name' => 'files[ap_photo_2' . $i . ']',
            '#id' => 'ap-photo-2-file-field' . $i,
          );

          // Field group title:
          $form[$ap_fieldset]['ap_group_summary_table' . $i] = array(
            '#type' => 'item',
            '#title' => t('&#9660; SUMMARY TABLE'),
          );

          // Summary table.
          $form[$ap_fieldset]['ap_summary_table' . $i] = array(
            '#type' => 'markup',
          );

          $summary_data = analyzedphenotypes_table_values($rows, $columns, $trait);

          if ($summary_data) {
            $siteyear = $summary_data['siteyear'];
            $syvalues = $summary_data['values'];

            $table_rows = array();

            foreach($siteyear as $sy) {
              $max = max($syvalues[$sy]);
              $min = min($syvalues[$sy]);
              $avg = array_sum($syvalues[$sy]) / count($syvalues[$sy]);
              $sdv = analyzedphenotypes_stats_standard_deviation($syvalues[$sy]);

              $table_rows[] = array($sy, $min, $max, round($avg, 2), round($sdv, 2));
            }

            $form[$ap_fieldset]['ap_summary_table' . $i]['#markup'] = theme('table',
              array(
                'header' => array('Site-year', 'Minimum Value', 'Maximum Value', 'Mean Value', 'Standard Deviation'),
                'rows' => $table_rows,
                'attributes' => array('id' => 'ap_summary_table' . $i)
              )
            );
          }

          $i++;
          /////////////////////////////////////////////////////////////////////////////////
        }
      }
    }
    ///
  }

  $form['#ap_describe_trait'] = $arr_trait_describe;
  $form_state['multistep_values']['new_headers'] = array_keys($form['#ap_describe_trait']);
  $form['#ap_main_fieldset'] = $main_fieldset;


  return $form;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


function analyzedphenotypes_validate_field_name(&$element, &$form, &$form_state) {
  $i = preg_replace('/[a-z\_]/', '', $element['#name']);
  $describe_traits = $form['multistep_values']['new_headers'];
  $in_trait = $describe_traits[trim($i)];

  if (strpbrk($element['#value'], '()')) {
    form_set_error($element['#name'], t('Characters "(" and/or ")" found in column header name of @trait.
    Please remove these characters and try again.', array('@trait' => $in_trait)));
  }
}


function analyzedphenotypes_validate_field_unit(&$element, &$form, &$form_state) {
  $i = preg_replace('/[a-z\_]/', '', $element['#name']);
  $describe_traits = $form['multistep_values']['new_headers'];
  $in_trait = $describe_traits[trim($i)];

  if (strpbrk($element['#value'], '()')) {
    form_set_error($element['#name'], t('Characters "(" and/or ")" found in column header unit of @trait.
    Please remove these characters and try again.', array('@trait' => $in_trait)));
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Function callback: Construct STAGE 1 - SAVE form interface.
 */
function analyzedphenotypes_loader_form_save($form, &$form_state) {
  $vars = analyzedphenotypes_moduleprop('settings');
  $allow_new = variable_get($vars['ap_allow_new']);

  if ($allow_new) {
    $arr_trait_describe = $form_state['multistep_values']['new_headers'];

    if (count($arr_trait_describe) > 0) {
      $project_name = $form_state['values']['ap_project_select_field'];
      $project_genus = $form_state['values']['ap_genus_select_field'];
      $data_file = $form_state['values']['ap_file_fid_field'];

      foreach ($arr_trait_describe as $i => $trait) {
        $description = $form_state['values']['ap_description_text_field' . $i];

        $cvterm = analyzedphenotypes_traitprop('insert_trait', array(
          'cvterm'      => $trait,
          'description' => $description,
        ));

        if ($cvterm) {
          // Save Unit.
          // $unit = $form_state['values']['ap_unit_text_field' . $i];

          // Save Scale.
          // $scale = $form_state['values']['ap_scale_text_field' . $i];

          // Save Ontology
          // $crop_ontology = $form_state['values']['ap_crop_ontology_text_field' . $i];
          // $plant_ontology = $form_state['values']['ap_plant_ontology_form_field' . $i];

          // Save Photo.
          $photo_1        = 'ap_photo_1' . $i;
          $photo_2        = 'ap_photo_2' . $i;

          $photo = array(
            'cvterm_id' => $cvterm,
            'photo' => array($photo_1, $photo_2),
          );

          analyzedphenotypes_fileprop('save_photo', $photo);
        }
      }
    }
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
