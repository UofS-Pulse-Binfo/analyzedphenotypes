<?php
/**
 * @file
 * Contains administrative interfaces of this module.
 */


module_load_include('inc', 'analyzedphenotypes', 'include/analyzedphenotypes.validators');
module_load_include('inc', 'analyzedphenotypes', 'api/analyzedphenotypes.api');


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_admin_data_loader($form, &$form_state) {
  drupal_session_start();

  if (!isset($form_state['stage'])) {
    // Default loader to upload stage.
    $form_state['stage'] = 'upload';

    // This is a page reload, thus will reset stage. In that case
    // see if this session variable was set and therefore load stage 2.
    if (isset($_SESSION['analyzedphenotypes']['projectprop'])) {
      $p = $_SESSION['analyzedphenotypes']['projectprop'];
      $projectprop = unserialize($p);

      if (isset($projectprop['job_id']) && $projectprop['job_id'] > 0) {
        $job_id = analyzedphenotypes_tripaljobprop('get_job', array(
          'job_id' => $projectprop['job_id'],
        ));

        if ($job_id) {
          $form_state['stage'] = 'validate';
          unset($_SESSION['analyzedphenotypes']['projectprop']);
        }
      }
    }
  }
  else {
    $projectprop = null;
  }

  $all_stage = array(
    'Upload',   // Stage 1 - Select project, genus and upload data file.
    'Validate', // Stage 2 - Validate data.
    'Describe', // Stage 3 - Fully describe all traits.
    'Save',     // Stage 4 - Save data and file.
  );

  $current_stage = $form_state['stage'];
  $form['#ap_current_stage'] = $current_stage;

  // Unaccounted : confirm delete!
  $form['ap_stage_hidden_field'] = array(
    '#type' => 'hidden',
    '#value' => $current_stage,
    '#id' => 'ap-stage-hidden-field',
  );
  //

  $active_stage = ucfirst($current_stage);
  $stage_id = array_search($active_stage, $all_stage);

  foreach($all_stage as $i => $stage) {
    $s = ($i <= $stage_id) ? 'stage' : 'todo';
    $stage_indicators[$stage] = $s;
  }

  $stage_title = 'stage ' . ($stage_id + 1) . ' of ' . count($all_stage) . ' - ' . $all_stage[$stage_id];
  $form['#ap_stage_title'] = $stage_title;
  $form['#ap_stage_indicators'] = $stage_indicators;

  // Include CSS and JavaScript.
  $path_theme = drupal_get_path('module', 'analyzedphenotypes') . '/theme/';

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

      // # 1.
      $form = analyzedphenotypes_loader_form_upload($form, $form_state);

      break;

    //
    case 'validate':
      $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.dragdrop.css';
      $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.progress.css';
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.progress.js';
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.dragdrop.js';
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.stage2.js';

      drupal_add_js('misc/progress.js');

      // # 2.
      $form = analyzedphenotypes_loader_form_validates($form, $form_state, $projectprop);

      break;

    //
    case 'describe':
      $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.stage3.css';
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.stage3.js';

      drupal_add_library('system', 'ui.accordion');

      // # 3.
      $form = analyzedphenotypes_loader_form_describe($form, $form_state);

      break;

    //
    case 'save':
      $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.progress.css';
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.progress.js';

      drupal_add_js('misc/progress.js');

      // # 4.
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


/**
 * # 1.
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_loader_form_upload($form, &$form_state) {
  // Autocomplete search field: Experiment/project title.
  $form['ap_project_select_field'] = array(
    '#title' => 'Experiment',
    '#type' => 'textfield',
    '#maxlength' => 200,
    '#size' => 100,
    '#required' => TRUE,

    // @see analyzedphenotypes_data_json() in functions.inc where $dataype = projects.
    '#autocomplete_path' => 'admin/tripal/extension/analyzedphenotypes/json/projects/experiment',

    // When project is selected, autofill the genus field with corresponding project genus.
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
    '#description' => t('Type in the experiment or project title your data is specific to.'),
    '#id' => 'ap-project-select-field'
  );

  // All genus.
  $arr_all_genus = array(0 => '- Select -');
  $genusprop = analyzedphenotypes_genusprop();

  foreach($genusprop as $g) {
    $arr_all_genus[$g] = $g;
  }

  // Project genus - autofill.
  if (isset($form_state['values']['ap_project_select_field'])) {
    $project = $form_state['values']['ap_project_select_field'];
    $project = trim($project);

    if (!empty($project)) {
      $projectprop = analyzedphenotypes_projectprop('match_name', array(
        'project_name' => $project,
      ));

      if ($projectprop) {
        $project_id = array_keys($projectprop);
        $project_genus = analyzedphenotypes_genusprop('project_genus', array(
          'project_id' => $project_id,
        ));

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


  if (isset($form_state['values']['ap_genus_select_field']) &&
     !empty($form_state['values']['ap_genus_select_field'])) {

    $default_genus = $form_state['values']['ap_genus_select_field'];
  }
  else {
    $default_genus = array_keys($genus_options)[0];
  }

  // Select field: Genus.
  $form['ap_AJAX_wrapper_autofillgenus']['ap_genus_select_field'] = array(
    '#type' => 'select',
    '#title' => t('Genus'),
    '#required' => TRUE,
    '#options' => $genus_options,
    '#default_value' => $default_genus,
    '#description' => t('Select Genus. When experiment or project has genus set, a value will be automatically selected.'),
    '#id' => 'ap-genus-select-field',
  );

  // Drag and Drop: Data file.
  $form = analyzedphenotypes_dragdrop($form, $form_state, 'ap_dnd_field');


  return $form;
}


/**
 * # 2.
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_loader_form_validates($form, &$form_state, $projectprop = null) {
  if (isset($form_state['values']['ap_project_select_field']) &&
      isset($form_state['values']['ap_genus_select_field'])   &&
      isset($form_state['multistep_values']['fid'])) {

    $project_name  = $form_state['values']['ap_project_select_field'];
    $project_genus = $form_state['values']['ap_genus_select_field'];
    $data_file     = $form_state['multistep_values']['fid'];

    $tripal_dataset = array(
      'description' => 'Validate Analyzed Phenotypes',
      'callback'    => 'analyzedphenotypes_validate_tsv_data',
      'dataset' => array(
        'project_name'  => $project_name,
        'project_genus' => $project_genus,
        'data_file'     => $data_file,
      )
    );

    $job_id = analyzedphenotypes_tripaljobprop('new_job', $tripal_dataset);
  }
  elseif ($projectprop) {
    $project_name  = $projectprop['project_name'];
    $project_genus = $projectprop['project_genus'];
    $data_file     = $projectprop['data_file'];

    $job = analyzedphenotypes_tripaljobprop('get_job', array(
      'job_id' => $projectprop['job_id'],
    ));

    if ($job) {
      $job_id = $job['job_id'];
    }
  }

  if ($job_id) {
    $base_admin = base_path() . 'admin/tripal/extension/analyzedphenotypes';

    $js_var = array(
      'stage'    => 'validate',
      'job_id'    => $job_id,
      'path_JSON' => $base_admin . '/json/validate_jobstatus/',
      'path_VR'   => $base_admin . '/validation_result/data/',
    );

    drupal_add_js(array('analyzedphenotypes' => array('vars' => $js_var)), array('type' => 'setting'));
  }

  // Hidden field: Experiment/project name.
  $form['ap_project_select_field'] = array(
    '#type' => 'hidden',
    '#value' => $project_name,
    '#id' => 'ap-project-select-field',
  );

  // Hidden field: Genus.
  $form['ap_genus_select_field'] = array(
    '#type' => 'hidden',
    '#value' => $project_genus,
    '#id' => 'ap-genus-select-field',
  );

  // Hidden field: File FID.
  $form['ap_file_fid_field'] = array(
    '#type' => 'hidden',
    '#value' => $data_file,
    '#id' => 'ap-file-fid-field',
  );

  // Drag and Drop: Data file.
  $form = analyzedphenotypes_dragdrop($form, $form_state, 'ap_dnd_field_reupload');


  return $form;
}


/**
 * # 3.
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_loader_form_describe($form, &$form_state) {
  if (isset($form_state['values']['ap_project_select_field']) &&
      isset($form_state['values']['ap_genus_select_field'])   &&
      isset($form_state['values']['ap_file_fid_field'])) {

    $project_name = $form_state['values']['ap_project_select_field'];
    $project_genus = $form_state['values']['ap_genus_select_field'];
    $data_file = $form_state['values']['ap_file_fid_field'];

    // Hidden field: Experiment/project name.
    $form['ap_project_select_field'] = array(
      '#type' => 'hidden',
      '#value' => $project_name,
      '#id' => 'ap-project-select-field',
    );

    // Hidden field: Genus.
    $form['ap_genus_select_field'] = array(
      '#type' => 'hidden',
      '#value' => $project_genus,
      '#id' => 'ap-genus-select-field',
    );

    // Hidden field: File FID.
    $form['ap_file_fid_field'] = array(
      '#type' => 'hidden',
      '#value' => $data_file,
      '#id' => 'ap-file-fid-field',
    );

    $data_file = file_load($data_file);
    $file_contents = analyzedphenotypes_datafileprop('get_contents', array(
      'data_file' => $data_file,
    ));

    if ($file_contents) {
      ///
      $rows = explode("\n", $file_contents);
      unset($file_contents);

      $columns_row = $rows[0];
      $expected_columns = analyzedphenotypes_columnsprop();
      $trait_name = array_keys($expected_columns)[0];
      unset($expected_columns, $rows[0]);

      $c = str_getcsv($columns_row, "\t");
      $columns = array_map('trim', $c);
      $trait_name_index = array_search($trait_name, $columns);

      $arr_trait_describe = array();

      $main_fieldset = 'ap_trait_form_fieldset';

      $i = 0;

      foreach($rows as $j => $line) {
        if (!empty($line)) {
          $value = str_getcsv($line, "\t");
          $trait_name_value = $value[$trait_name_index];

          if (in_array($trait_name_value, $arr_trait_describe)) {
            continue;
          }
          else {
           ////
           $traitprop = array(
             'cvterm_id'      => '',
             'term'           => $trait_name_value,
             'name'           => '',
             'definition'     => '',
             'unit'           => '',
             'scale'          => '',
             'crop_ontology'  => '',
             'photos' => array(
               'photo_1'      => '',
               'photo_2'      => '',
             ),
           );

           $ap_fieldset = $main_fieldset . $i;
           $arr_trait_describe[] = $trait_name_value;

           $form['#ap_cvterm_hidden_field' . $i] = $trait_name_value;

           $name = analyzedphenotypes_traitprop('no_unit', array(
             'trait_name' => $trait_name_value,
           ));
           $traitprop['name'] = $name;

           // Match cvterm key.
           $similar_cvterm = analyzedphenotypes_cvprop('match_key', array(
             'cvterm_keyword' => $name,
           ));

           if ($similar_cvterm) {
             $form[$ap_fieldset]['ap_similar_trait_select_field' . $i] = array(
               '#type' => 'select',
               '#title' => t('Did you mean?'),
               '#options' => array(-1 => '- Select -', 0 => 'None of these apply') + $similar_cvterm,

               '#ajax' => array(
                 'event' => 'change',
                 'callback' => 'analyzedphenotypes_AJAX_callback',
                 'case' => 'describeform' . $i,
                 'contain' => $ap_fieldset,
                 'wrapper' => 'ap-AJAX-wrapper-describeform' . $i,
                 'progress' => array('type' => 'throbber', 'message' => 'Loading trait definition...'),
               ),
               '#description' => 'The system has detected a similar trait in the database.
                 It is recommended that you select a trait from the select box that best describes your data.
                 If trait is not listed, please select None of these apply option and use the form below to describe the trait.',
               '#attributes' => array('class' => array('ap-required')),
               '#element_validate' => array('analyzedphenotypes_validate_field_select_trait'),
               '#id' => 'ap-similar-header-select-field' . $i,
             );

             $describeform_show = 'no';
           }
           else {
             $describeform_show = 'yes';
           }

           if (isset($form_state['values']['ap_similar_trait_select_field' . $i])) {
             $select_trait = (int)$form_state['values']['ap_similar_trait_select_field' . $i];

             if ($select_trait >= 0) {
               if ($select_trait > 0) {
                 $selected_cvterm = $form_state['values']['ap_similar_trait_select_field' . $i];

                 $exists = analyzedphenotypes_cvprop('get_cvterm', array(
                   'cvterm_id' => $selected_cvterm,
                 ));

                 if ($exists) {
                   $traitprop['cvterm_id']     = $exists['cvterm_id'];
                   $traitprop['definition']    = $exists['definition'];
                   $traitprop['scale']         = '';
                   $traitprop['crop_ontology'] = $exists['crop_ontology_name'];
                   $traitprop['photos'] = analyzedphenotypes_cvprop('cvterm_get_photo', array(
                     'cvterm_id' => $exists['cvterm_id'],
                   ));
                 }
               }

               $describeform_show = 'yes';
             }
             else {
               $describeform_show = 'no';
             }
           }

           $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i] = array(
             '#prefix' => '<div id="ap-AJAX-wrapper-describeform' . $i . '">',
             '#suffix' => '</div>',
           );

           if ($describeform_show == 'yes') {
             $disabled = ($traitprop['cvterm_id']) ? TRUE : FALSE;
             $class = ($disabled) ? '' : 'ap-required';

             $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_group_about_the_trait' . $i] = array(
               '#type' => 'item',
               '#title' => t('&#9660; ABOUT THE TRAIT'),
             );

             // Text field: Trait name.
             $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_name_text_field' . $i] = array(
               '#type' => 'textfield',
               '#title' => t('Name'),
               '#disabled' => TRUE,
               '#default_value' => $traitprop['name'],
               '#description' => t('A Concise human-readable name or label for the trait.'),
               '#id' => 'ap-name-text-field' . $i,
             );

             $unit = analyzedphenotypes_traitprop('unit', array(
               'trait_name' => $trait_name_value,
             ));

             $traitprop['unit'] = $unit;

             // Text field: Unit.
             $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_unit_text_field' . $i] = array(
               '#type' => 'textfield',
               '#title' => t('Unit'),
               '#disabled' => TRUE,
               '#default_value' => $traitprop['unit'],
               '#description' => t('Unit used to measure this trait.'),
               '#id' => 'ap-unit-text-field' . $i,
             );

             // Textarea: Definition/description.
             $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_description_text_field' . $i] = array(
               '#type' => 'textarea',
               '#title' => t('Description'),
               '#disabled' => $disabled,
               '#default_value' => $traitprop['definition'],
               '#description' => t('Text definition or description of trait.'),
               '#attributes' => array('class' => array($class)),
               '#element_validate' => array('analyzedphenotypes_validate_field_description'),
               '#id' => 'ap-description-text-field' . $i,
             );

             // Field group title:
             $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_group_mapping_information' . $i] = array(
               '#type' => 'item',
               '#title' => t('&#9660; MAPPING INFORMATION'),
             );

             // Text field: Crop ontology.
             $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_crop_ontology_text_field' . $i] = array(
               '#title' => 'Crop Ontology',
               '#type' => 'textfield',
               '#disabled' => $disabled,
               '#default_value' => $traitprop['crop_ontology'],
               '#description' => t('Type in crop ontology or click one from suggestions below.'),
               '#attributes' => array('class' => array('ap-crop-ontology', $class)),
               '#element_validate' => array('analyzedphenotypes_validate_field_ontology'),
               '#id' => drupal_html_id('ap-crop-ontology-text-field' . $i),
             );

             if (!$disabled) {
               $ontology_suggestions = analyzedphenotypes_ontologyprop('suggest', array(
                 'trait_name' => $traitprop['name'],
                 'project_genus' => $project_genus,
               ));

               if ($ontology_suggestions) {
                 $a = array();
                 foreach($ontology_suggestions as $k => $term) {
                   $definition = empty($term['definition']) ? $term['name'] : $term['definition'];

                   $a[] = l($term['name'], '#', array('attributes' => array(
                     'alt' => $definition,
                     'title' => $definition,
                   )));
                 }

                 $list = theme('item_list', array(
                   'items' => $a,
                   'title' => '',
                   'attributes' => array('id' => 'ap-ontology-suggest-' . $i, 'class' => 'ap-item-list'),
                 ));

                 $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_ontology_suggestion_markup_field' . $i] = array(
                   '#markup' => $list,
                 );
               }
             }

             // Field group title:
             $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_group_photo_upload' . $i] = array(
               '#type' => 'item',
               '#title' => t('&#9660; PHOTO UPLOAD'),
             );

             $g = 1;
             $no_photo_count = 0;
             foreach($traitprop['photos'] as $seq => $photo_info) {
               if ($photo_info) {
                 $scale = analyzedphenotypes_scale_photo(array(
                   'height' => $photo_info['height'],
                   'width' => $photo_info['width'],
                   'scale_to' => 300,
                 ));

                 $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i][$seq . $i] = array(
                   '#type' => 'markup',
                   '#id' => 'ap-photo-1' . $i,
                   '#markup' => theme('image', array(
                     'path' => $photo_info['uri'],
                     'height' => $scale['height'],
                     'width' => $scale['width'],
                   )),
                 );
               }
               else {
                 if (!$disabled) {
                   $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i][$seq . '_file_field' . $i] = array(
                     '#type' => 'file',
                     '#title' => t('Image ' . $g . ' of 2'),
                     '#upload_validators' => array(
                       'file_validate_extensions' => array('jpg'),
                     ),
                     '#name' => 'files[' . $seq . $i . ']',
                     '#id' => 'ap-photo-'. $g .'-file-field' . $i,
                   );
                 }
                 else {
                   $no_photo_count++;
                 }
               }

               $g++;
             }

             if ($no_photo_count == count($traitprop['photos'])) {
               $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_not_available' . $i] = array(
                 '#markup' => 'Photo not available.',
               );
             }

             // Field group title:
             $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_group_summary_table' . $i] = array(
               '#type' => 'item',
               '#title' => t('&#9660; SUMMARY TABLE'),
             );

             $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_summary_table' . $i] = array(
               '#type' => 'markup',
             );

             $summary_data = analyzedphenotypes_table_values($rows, $columns, $traitprop['term']);

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

               $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_summary_table' . $i]['#markup'] = theme('table',
                 array(
                   'header' => array('Site-year', 'Minimum Value', 'Maximum Value', 'Mean Value', 'Standard Deviation'),
                   'rows' => $table_rows,
                   'attributes' => array('id' => 'ap_summary_table' . $i)
                 )
               );
             }
           }
           ////
          }
        }

        $i++;
      }

      $form['#ap_describe_trait'] = $arr_trait_describe;
      $form_state['multistep_values']['new_headers'] = array_keys($form['#ap_describe_trait']);
      $form['#ap_main_fieldset'] = $main_fieldset;
      ///
    }
  }


  return $form;
}


/**
 * # 4.
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
 function analyzedphenotypes_loader_form_save($form, &$form_state) {
  $vars = analyzedphenotypes_moduleprop('db_cv');
  $allow_new = variable_get($vars['ap_allow_new']);
  unset($vars);

  if ($allow_new) {
    $arr_trait_describe = $form_state['multistep_values']['new_headers'];

    if (count($arr_trait_describe) > 0) {
      $project_name = $form_state['values']['ap_project_select_field'];
      $project_genus = $form_state['values']['ap_genus_select_field'];
      $data_file = $form_state['values']['ap_file_fid_field'];

      foreach ($arr_trait_describe as $i => $trait) {
        $description = $form_state['values']['ap_description_text_field' . $i];

        $cvterm = analyzedphenotypes_cvprop('insert_cvterm', array(
          'cvterm_name'      => $trait,
          'definition' => $description,
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

          analyzedphenotypes_cvprop('cvterm_save_photo', $photo);
        }
      }
    }
  }

  // Register a job.
  $tripal_dataset = array(
    'description' => 'Save Analyzed Phenotype',
    'callback' => 'analyzedphenotypes_save_tsv_data',
    'dataset' => array(
      'project_name' => $project_name,
      'project_genus' => $project_genus,
      'data_file' => $data_file
    )
  );

  $job_id = analyzedphenotypes_tripaljobprop('new_job', $tripal_dataset);

  if ($job_id) {
    $base_admin = base_path() . 'admin/tripal/extension/analyzedphenotypes';

    $js_var = array(
      'stage'     => 'save',
      'job_id'    => $job_id,
      'path_JSON' => $base_admin . '/json/validate_jobstatus/',
      'path_VR'   => $base_admin . '/validation_result/data/',
    );

    drupal_add_js(array('analyzedphenotypes' => array('vars' => $js_var)), array('type' => 'setting'));
  }


  return $form;
}



///////



/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_file_validate($file) {
  $source = $file->source;

  if ($source == 'ap_dnd_field' || $source == 'ap_dnd_field_reupload') {
    if (isset($_POST['ap_project_select_field']) &&
        isset($_POST['ap_genus_select_field'])) {

      $project_name =  trim($_POST['ap_project_select_field']);
      $project_genus = trim($_POST['ap_genus_select_field']);

      // Validation is at file level.
      // Begin validation process.
      $status = analyzedphenotypes_validate_tsv_file(array(
        'project_name'  => trim($project_name),
        'project_genus' => trim($project_genus),
        'data_file' => $file,
      ));

      drupal_set_message(analyzedphenotypes_validationresult('file', $status), 'ap-result-panel');

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


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_file_insert($file) {
  $source = $file->source;

  if ($source == 'ap_dnd_field_reupload') {
    if (isset($_POST['ap_project_select_field']) &&
        isset($_POST['ap_genus_select_field'])) {

      $project_name = $_POST['ap_project_select_field'];
      $project_genus = $_POST['ap_genus_select_field'];
      $data_file = $file->fid;

      $tripal_dataset = array(
        'description' => 'Validate Analyzed Phenotypes',
        'callback'    => 'analyzedphenotypes_validate_tsv_data',
        'dataset' => array(
          'project_name'  => $project_name,
          'project_genus' => $project_genus,
          'data_file'     => $data_file,
        )
      );

      $job_id = analyzedphenotypes_tripaljobprop('new_job', $tripal_dataset);

      if ($job_id) {
        $tripal_dataset['dataset']['job_id'] = $job_id;
        $_SESSION['analyzedphenotypes']['projectprop'] = serialize($tripal_dataset['dataset']);

        $dir = base_path() . 'admin/tripal/extension/analyzedphenotypes/upload';
        drupal_add_js(array('analyzedphenotypes' => array('redirect' => $dir)), array('type' => 'setting'));
      }
    }
  }
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_validate_field_select_trait(&$element, &$form, &$form_state) {
  if ($element['#value'] == "") {
    drupal_add_library('system', 'ui.accordion');

    form_set_error($element['#name'], t('Please select an option.'));
  }
}

function analyzedphenotypes_validate_field_description(&$element, &$form, &$form_state) {
  if (empty($element['#value']) && $element['#disabled'] === FALSE) {
    drupal_add_library('system', 'ui.accordion');

    form_set_error($element['#name'], t('Please provide a description.'));
  }
}

function analyzedphenotypes_validate_field_ontology(&$element, &$form, &$form_state) {
  if (empty($element['#value'])) {
    drupal_add_library('system', 'ui.accordion');

    form_set_error($element['#name'], t('Please enter or select ontology.'));
  }
}

/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
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

    $form_state['new_stage'] = $next_stage;

    // Ensure the form state is saved and the form is rebuilt.
    $form_state['multistep_values']['form_build_id'] = $form_state['values']['form_build_id'];
    $form_state['stage'] = $form_state['new_stage'];
    $form_state['rebuild'] = TRUE;
  }
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_dragdrop($form, &$form_state, $source_name) {
  $file_extensions = analyzedphenotypes_datafileprop();

  // List file types allowed.
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

  // For JS to know the source.
  $settings = array(
    'source' => $source_id,
  );

  drupal_add_js(array('analyzedphenotypes' => array('vars' => $settings)), array('type' => 'setting'));


  return $form;
}




///////



/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_admin_settings($form, &$form_state, $source) {
  $vars = analyzedphenotypes_systemvars();
  $valid_source = array('crop_ontology', 'property_terms', 'database_cv');

  if (!isset($source)) {
    $source = null;
  }

  switch($source) {
    //
    case 'database_cv':
      // Database.
      $var_db = $vars['ap_database'];
      $var_db_value = variable_get($var_db);

      $dbs = analyzedphenotypes_dbprop();
      $db_options = array_map(function($d) {
        return $d['name'];
      }, $dbs);

      // AJAX callback wrapper.
      $form['ap_AJAX_wrapper_dbinfo'] = array(
        '#prefix' => '<div id="ap-AJAX-wrapper-dbinfo">',
        '#suffix' => '</div>',
      );

      $db_field_value = (isset($form_state['values'][$var_db]))
        ? $form_state['values'][$var_db] : 0;

      if ($db_field_value > 0) {
        $default_db = $db_field_value;
      }
      else {
        if ($var_db_value == 'not set' || $var_db_value == 0) {
          $default_db = 0;
          $db_options = array(0 => '- Select -') + $db_options;
        }
        else {
          $default_db = $var_db_value;
        }
      }

      $description_db = ($default_db == 0 || empty($dbs[$default_db]['description']))
       ? 'Description not available'
       : $dbs[$default_db]['description'];

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

      // CV and Genus, related (ontology).
      $var_cv = $vars['ap_cv'];
      $var_cv_value = variable_get($var_cv);

      $cvs = analyzedphenotypes_cvprop();
      $options = array_map(function($c) {
        return $c['name'];
      }, $cvs);

      $cv_options = $options;
      // AJAX callback wrapper.
      $form['ap_AJAX_wrapper_cvinfo'] = array(
        '#prefix' => '<div id="ap-AJAX-wrapper-cvinfo">',
        '#suffix' => '</div>',
      );

      $cv_field_value = (isset($form_state['values'][$var_cv]))
        ? $form_state['values'][$var_cv] : 0;

      if ($cv_field_value > 0) {
        $default_cv = $cv_field_value;
      }
      else {
        if ($var_cv_value == 'not set' || $var_cv_value == 0) {
          $default_cv = 0;
          $cv_options = array(0 => '- Select -') + $options;
        }
        else {
          $default_cv = $var_cv_value;
        }
      }

      $description_cv = ($default_cv == 0 || empty($cvs[$default_cv]['definition']))
        ? 'Description not available'
        : $cvs[$default_cv]['definition'];

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

      unset($cv_options);
      $cv_options = $options;

      $arr_cvterms = array(
        'ap_genus',
        'ap_related',
      );

      foreach($arr_cvterms as $k => $term) {
        $var = $vars[$term];
        $var_value = variable_get($var);
        $title = trim(str_replace('ap_', '', $term));

        if ($var_value == 'not set' || $var_value == 0) {
          $default_var_cv = 0;
          $cv_options = array(0 => '- Select -') + $options;
        }
        else {
          $varprop = analyzedphenotypes_cvprop('get_cvterm', array(
            'cvterm_id' => $var_value,
          ));

          $default_var_cv = $varprop['cv_id'];
        }

        $form['ap_cv_select_field' . $k] = array(
          '#type' => 'select',
          '#title' => t('Locate ' . $title . ' cvterm: Controlled Vocabulary'),
          '#required' => TRUE,
          '#options' => $cv_options,
          '#default_value' => $default_var_cv,
          //
          '#ajax' => array(
            'event' => 'change',
            'callback' => 'analyzedphenotypes_AJAX_callback',
            'case' => 'varcvterm' . $title,
            'wrapper' => 'ap-AJAX-wrapper-varcvterm' . $title,
            'progress' => array('type' => 'throbber', 'message' => 'Loading cvterms...'),
          ),
          '#id' => 'ap-cv-select-field' . $k,
        );

        $cvvar_field_value = (isset($form_state['values']['ap_cv_select_field' . $k]))
          ? $form_state['values']['ap_cv_select_field' . $k] : 0;

        if ($cvvar_field_value > 0) {
          $default_var_cv = $cvvar_field_value;
        }

        if ($default_var_cv > 0) {
          $default_cvterm = $var_value;
          $cvterms = analyzedphenotypes_cvprop('terms_in_cv', array(
            'cv_id' => $default_var_cv,
          ));

          $cvterm_options = array_map(function($t) {
            return $t['name'];
          }, $cvterms);
        }
        else {
          $default_cvterm = 0;
          $cvterm_options = array();
        }

        // AJAX callback wrapper.
        $form['ap_AJAX_wrapper_varcvterm' . $title] = array(
          '#prefix' => '<div id="ap-AJAX-wrapper-varcvterm' . $title . '">',
          '#suffix' => '</div>',
        );

        $form['ap_AJAX_wrapper_varcvterm' . $title][$var] = array(
          '#type' => 'select',
          '#title' => '',
          '#options' => $cvterm_options,
          '#default_value' => $default_cvterm,
          '#description' => t('Select ' . $title . ' cvterm.'),
          '#id' => $term,
        );
      }

      // Allow new traits?
      $var_allow_new = $vars['ap_allow_new'];
      $var_allow_new_value = variable_get($var_allow_new);
      $default_allow_new = ($var_allow_new_value == 'not set') ? 0 : $var_allow_new_value;

      $form[$var_allow_new] = array(
        '#type' => 'checkbox',
        '#title' => t('Allow new term to be added to the Controlled Vocabulary'),
        '#default_value' => $default_allow_new,
        '#id' => $var_allow_new,
      );

      break;

    //
    case 'property_terms':
      $vars = analyzedphenotypes_systemvars('plant_property');
      $cvs = analyzedphenotypes_cvprop();
      $cv_options = array_map(function($c) {
        return $c['name'];
      }, $cvs);

      foreach($vars as $i => $var) {
        $title = trim(str_replace('ap_', '', $i));
        $var_value = variable_get($var);

        if ($var_value == 'not set' || empty($var_value)) {
          $default_cv = 0;
          $cv_options = array(0 => '- Select- ') + $cv_options;
        }
        else {
          $cvtermprop = analyzedphenotypes_cvprop('get_cvterm', array(
            'cvterm_id' => $var_value,
          ));

          $default_cv = $cvtermprop['cv_id'];
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

        $cv_field_value = (isset($form_state['values'][$i . '_cv_select_field']))
          ? $form_state['values'][$i . '_cv_select_field'] : 0;

        if ($cv_field_value > 0) {
          $default_cv = $cv_field_value;
        }

        if ($default_cv > 0) {
          $default_cvterm = $var_genus_value;
          $cvterms = analyzedphenotypes_cvprop('terms_in_cv', array(
            'cv_id' => $default_cv,
          ));

          $cvterm_options = array_map(function($t) {
            return $t['name'];
          }, $cvterms);
        }
        else {
          $default_cvterm = 0;
          $cvterm_options = array();
        }

        $form['ap_AJAX_wrapper_getcvterms' . $title][$var] = array(
          '#type' => 'select',
          '#title' => '',
          '#options' => $cvterm_options,
          '#default_value' => $default_cvterm,
          '#description' => t('Select Controlled Vocabulary (CV) and CV Term.'),
          '#id' => $i . '_term_select_field',
        );
      }

      break;

    //
    case 'crop_ontology':
      $vars = analyzedphenotypes_systemvars('genus_ontology');
      $cvs = analyzedphenotypes_cvprop();
      $cv_options = array_map(function($c) {
        return $c['name'];
      }, $cvs);

      foreach($vars as $i => $var) {
        $title = trim(str_replace('ap_', '', $i));
        $var_value = variable_get($var);

        if ($var_value == 'not set' || empty($var_value)) {
          $cv_options = array(0 => '- Select -') + $cv_options;
          $default_cv = 0;
        }
        else {
          $default_cv = $var_value;
        }

        $form[$var] = array(
          '#type' => 'select',
          '#title' => 'Select ontology for genus ' . ucfirst($title),
          '#required' => TRUE,
          '#options' => $cv_options,
          '#default_value' => $default_cv,
          '#id' => $var,
        );
      }

      break;

      //
      // Define additional case here.

      //
      // default:
  }


  if (in_array($source, $valid_source)) {
    return system_settings_form($form);
  }
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
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
