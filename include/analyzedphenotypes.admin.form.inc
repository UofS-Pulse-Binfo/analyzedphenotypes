<?php
/**
 * @file
 * Contains administrative interfaces of this module.
 */


module_load_include('inc', 'analyzedphenotypes', 'include/analyzedphenotypes.validators');
module_load_include('inc', 'analyzedphenotypes', 'api/analyzedphenotypes.api');


/**
 * FUNCTION CALLBACK
 * Construct data uploader stages.
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
 * FUNCTION CALLBACK
 * Construct data uploader stages #1.
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
    $project = trim($form_state['values']['ap_project_select_field']);

    if (!empty($project)) {
      $project_genus = analyzedphenotypes_genusprop('project_genus', array(
        'project_name' => $project,
      ));

      if ($project_genus) {
        $arr_project_genus = array($project_genus => $project_genus);
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
    '#description' => t('Select Genus. When experiment or project has genus set, a value will be selected.'),
    '#id' => 'ap-genus-select-field',
  );

  // Drag and Drop: Data file.
  $form = analyzedphenotypes_dragdrop($form, $form_state, 'ap_dnd_field');


  return $form;
}


/**
 * FUNCTION CALLBACK
 * Construct data uploader stages #2.
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
      'path_JSON' => $base_admin . '/json/jobstatus/',
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
 * FUNCTION CALLBACK
 * Construct data uploader stages #3.
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
             'cvterm_id'     => '',
             'term'          => $trait_name_value,
             'name'          => '',
             'definition'    => '',
             'unit'          => '',
             'scale'         => '',
             'crop_ontology' => '',
             'photos' => array(
               'ap_photo_1'  => '',
               'ap_photo_2'  => '',
             ),
           );

           $ap_fieldset = $main_fieldset . $i;
           $arr_trait_describe[] = $trait_name_value;

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
                   $trait_name_value = $exists['name'];
                   $name = analyzedphenotypes_traitprop('no_unit', array(
                     'trait_name' => $trait_name_value,
                   ));
                   $traitprop['name']          = $name;
                   $traitprop['cvterm_id']     = $exists['cvterm_id'];
                   $traitprop['definition']    = $exists['definition'];
                   $traitprop['scale']         = '';
                   $traitprop['crop_ontology'] = $exists['crop_ontology'];

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
               '#element_validate' => array('analyzedphenotypes_validate_field_required'),
               '#attributes' => array('class' => array($class)),
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
               '#autocomplete_path' => 'admin/tripal/extension/analyzedphenotypes/json/ontology/' . $project_genus . ':' . $traitprop['name'],
               '#attributes' => array('class' => array('ap-crop-ontology', $class)),
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
                     '#name' => 'files[' . $seq . $i . ']',
                     '#attributes' => ($g == 1) ? array('class' => array('ap-required')) : array(),
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

            $i++;
           ////
          }
        }
      }

      $form['#ap_describe_trait'] = $arr_trait_describe;
      $form_state['multistep_values']['traits'] = $form['#ap_describe_trait'];
      $form['#ap_main_fieldset'] = $main_fieldset;
      ///
    }
  }


  return $form;
}


/**
 * FUNCTION CALLBACK
 * Construct data uploader stages #4.
 */
function analyzedphenotypes_loader_form_save($form, &$form_state) {
  $traits = $form_state['multistep_values']['traits'];

  if (count($traits) > 0) {
    $project_name  = $form_state['values']['ap_project_select_field'];
    $project_genus = $form_state['values']['ap_genus_select_field'];
    $data_file = $form_state['values']['ap_file_fid_field'];

    $vars = analyzedphenotypes_systemvars('genus_ontology');
    $var_name = 'ap_' . strtolower(str_replace(' ', '_', $project_genus));
    $cv_id = variable_get($vars[$var_name]['var']);

    foreach($traits as $i => $trait) {
      // Check the select similar trait value.
      if (isset($form_state['values']['ap_similar_trait_select_field' . $i]) &&
          $form_state['values']['ap_similar_trait_select_field' . $i] > 0) {

        $cvterm_id = $form_state['values']['ap_similar_trait_select_field' . $i];
      }
      else {
        $description = $form_state['values']['ap_description_text_field' . $i];

        $cvterm = analyzedphenotypes_cvprop('insert_cvterm', array(
          'name' => $trait,
          'definition' => $description,
        ));

        if ($cvterm) {
          $cvterm_id = $cvterm;

          // Unit.
          // Scale.
          // Ontology.
          $ontology_value = $form_state['values']['ap_crop_ontology_text_field' . $i];

          $ontologyprop = analyzedphenotypes_cvprop('get_cvterm', array(
            'name' => $ontology_value,
            'cv_id' => $cv_id,
          ));

          if ($ontologyprop) {
            // Create relationship.
            analyzedphenotypes_ontologyprop('map_ontology_trait', array(
              'ontology_cvterm_id' => $ontologyprop['cvterm_id'],
              'trait_cvterm_id' => $cvterm_id,
            ));
          }

          // Photo.
          $photo_1 = 'ap_photo_1' . $i;
          $photo_2 = 'ap_photo_2' . $i;

          analyzedphenotypes_cvprop('cvterm_save_photo', array(
            'cvterm_id' => $cvterm_id,
            'photo' => array($photo_1, $photo_2),
          ));
        }
      }

      $arr_traitname_cvtermid[$trait] = $cvterm_id;
    }

    // Save genus and project.
    // Does project have a genus?
    $projectprop = analyzedphenotypes_genusprop('project_genus', array(
      'project_name' => $project_name,
    ));

    if (!$projectprop) {
      $projectprop = analyzedphenotypes_projectprop('match_name', array(
        'project_name' => $project_name,
      ));

      analyzedphenotypes_genusprop('assign_genus', array(
        'project_genus' => $project_genus,
        'project_id' => $projectprop['project_id'],
      ));
    }

    // Register a job.
    $tripal_dataset = array(
      'description' => 'Save Analyzed Phenotype',
      'callback' => 'analyzedphenotypes_save_tsv_data',
      'dataset' => array(
        'project_name' => $project_name,
        'project_genus' => $project_genus,
        'data_file' => $data_file,
        'trait_cvterm' => $arr_traitname_cvtermid,
      )
    );

    $job_id = analyzedphenotypes_tripaljobprop('new_job', $tripal_dataset);

    if ($job_id) {
      $base_admin = base_path() . 'admin/tripal/extension/analyzedphenotypes';

      $js_var = array(
        'stage'     => 'save',
        'job_id'    => $job_id,
        'path_JSON' => $base_admin . '/json/jobstatus/',
        'path_VR'   => $base_admin . '/validation_result/data/',
      );

      drupal_add_js(array('analyzedphenotypes' => array('vars' => $js_var)), array('type' => 'setting'));
    }
  }


  return $form;
}



///////



/**
 * Implements hook_file_validate().
 * Perform file level validation.
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
 * Implements hook_file_insert().
 * Perform data level file validation.
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
 * FUNCTION CALLBACK
 * Validate select trait form field.
 */
function analyzedphenotypes_validate_field_select_trait(&$element, &$form, &$form_state) {
  if ($element['#value'] == "") {
    drupal_add_library('system', 'ui.accordion');

    form_set_error($element['#name'], t('Please select an option.'));
  }
}


/**
 * FUNCTION CALLBACK
 * Validate required fields: trait description, ontology and photo.
 */
function analyzedphenotypes_validate_field_required(&$element, &$form, &$form_state) {
  if ($element['#disabled'] === FALSE) {
    $error = FALSE;

    // Description.
    $description_field_value = $element['#value'];
    if (empty($description_field_value)) {
      $error = TRUE;
      form_set_error($element['#name'], t('Trait description field is empty. Please provide trait description.'));
    }

    // Crop ontology.
    $field_name = str_replace('ap_description_text_field', 'ap_crop_ontology_text_field', $element['#name']);
    $field_value = $form['values'][$field_name];
    if (empty($field_value)) {
      $error = TRUE;
      form_set_error($field_name, t('Crop ontology field is empty. Please enter or select crop ontology.'));
    }
    else {
      $project_genus = $form['values']['ap_genus_select_field'];
      $exists = analyzedphenotypes_ontologyprop('match_ontology', array(
        'project_genus' => $project_genus,
        'ontology' => $field_value,
      ));

      if (!$exists) {
        $error = TRUE;
        form_set_error($field_name, t('Crop ontology entered is not recognized. Please enter or select crop ontology.'));
      }
    }

    // File.
    $field_name = str_replace('ap_description_text_field', 'ap_photo_1', $element['#name']);
    $field_value = $_FILES['files']['name'][$field_name];
    if (empty($field_value)) {
      $error = TRUE;
      form_set_error($field_name, t('Photo 1 of 2 is empty. Please provide a photo.'));
    }
    else {
       $file_extension = pathinfo($field_value, PATHINFO_EXTENSION);

       if ($file_extension != 'jpg') {
         $error = TRUE;
         form_set_error($field_name, t('Photo 1 of 2 is not a .jpg file. Please provide a .jpg photo.'));
       }
    }

    if ($error) {
      drupal_add_library('system', 'ui.accordion');
    }
  }
}




/**
 * Implements hook_submit().
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
 * FUNCTION CALLBACK
 * Construct Drag and Drop file dropzone area.
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
 * FUNCTION CALLBACK
 * Construct administrative pages.
 */
function analyzedphenotypes_admin_settings($form, &$form_state) {
  //
  // #FRAMESET TRAIT ONTOLOGIES.
  $frameset_to = 'ap_frameset_ontology';

  // Title.
  $form[$frameset_to] = array(
    '#type' => 'fieldset',
    '#title' => t('Trait Ontologies'),
    '#description' => t('This module requires that phenotypic traits be part of a controlled vocabulary.'),
  );

  // Instructions.
  $items = array(
    '<u>Trait Vocabulary</u>:' .
      '<br />A container of terms where each term is a phenotypic trait that can be measured in your species of interest. This controlled vocabulary should be specific to a given genus and each term will become a trait page on your Tripal site. If you do not already have a trait vocabulary, you can create it ' . l('here', 'admin/tripal/loaders/chado_vocabs/chado_cv/add', array('attributes' => array('target'=>'_blank'))) . ' and add terms upfront and/or automatically on upload of phenotypic data.',
    '<u>Associated Database</u>:' .
      '<br />Chado requires a "database" container to be associated with all controlled vocabularies. Please select the "database" container you would like to be associated with your trait vocabulary. If needed, create one '. l('here', 'http://knowpulse.usask.ca/dev/fresh/admin/tripal/loaders/chado_db/add', array('attributes' => array('target'=>'_blank'))).'.',
    '<u>Crop Ontology</u>:' .
      '<br />Our experience with breeders has led us to recommend using the trait names your breeder(s) already use in the Trait Vocabulary and then linking them to a more generic crop ontology such as those provided by <u><em>' . l('cropontology.org', 'http://cropontology.org') . '</em></u> to facilitate sharing. If you decide to go this route, you can set the species specific crop ontology here and on upload suitable terms will be suggested based on pattern matching.',
  );

  $form[$frameset_to]['config_instructions'] = array(
    '#theme' => 'item_list',
    '#items' => $items,
  );

  // Instructions - select.
  $form[$frameset_to]['config_select'] = array(
    '#type' => 'item',
    '#title' => t('Please select the appropriate vocabulary for each genus you intend to support phenotypic data for.'),
  );

  // Table Field.
  // Fetch all available genus.
  $genus = analyzedphenotypes_genusprop();

  // Create fields.
  $fields = array();
  // System vars basename.
  $basename = 'analyzedphenotypes_systemvar_';
  $var_token = array(
    1 => array('cv', 'Select Vocabulary'),
    2 => array('db', 'Select Database'),
    3 => array('ontology', 'Select Crop Ontology'),
  );

  // Fetch all Controlled Vocabulary.
  $cvs = analyzedphenotypes_cvprop();
  $cv_options = array_map(function($c) {
    return $c['name'];
  }, $cvs);

  // Fetch all Database.
  $dbs = analyzedphenotypes_dbprop();
  $db_options = array_map(function($d) {
    return $d['name'];
  }, $dbs);

  // Construct table form field.
  foreach($genus as $i => $g) {
    $fields[ 'ap_field_row_' . $i ] = array();
    $genus_key = strtolower(str_replace(' ', '_', $g));

    // Each genus requires 4 columns - genus title, cv, db and ontology.
    $j = 0;
    while ($j < 4) {
      if ($j == 0) {
        // If 0 or column genus of the table - print the genus text.
        $field_prop = array(
          '#type' => 'item',
          '#title' => t(ucfirst($g)),
        );

        $field_name = $genus_key . '_title';
      }
      else {
        // Else, construct a select box.
        $field_options = ($j == 2) ? $db_options : $cv_options;

        // Find if a config has been set.

        $var_value = variable_get($basename . $genus_key . '_' . $var_token[$j][0]);
        if (isset($var_value)) {
          $field_default = $var_value;
        }

        $field_prop = array(
          '#type' => 'select',
          '#title' => '',
          '#options' => array(0 => $var_token[$j][1]) + $field_options,
          '#attributes' => array('class' => array('ap-table-select-field')),
          '#default_value' => $field_default,
        );

        $field_name = $basename . $genus_key . '_' . $var_token[$j][0];
      }

      $fields[ 'ap_field_row_' . $i ][$field_name] = $field_prop;

      $j++;
    }
  }

  // This will flex select fields.
  drupal_add_css('.ap-table-select-field { width: 100%; }', 'inline');

  $form[$frameset_to]['ap_formfield_table'] = array(
    '#theme' => 'analyzedphenotypes_form_table',
    '#header' => array(t('<b>Genus</b>'), t('Trait Vocabulary'), t('Associated Database'), t('Crop Ontology')),
    'rows' => array(
      '#tree' => FALSE,
    ) + $fields,
  );

  // Allow add new traits.
  $field_default = variable_get($basename . 'allownew');
  $form[$frameset_to][ $basename . 'allownew' ] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow new terms to be added to the Controlled Vocabulary during upload.'),
    '#description' => t('This applies to all organism listed above.'),
    '#default_value' => ($field_default == 1) ? 1 : FALSE,
  );

  // Warning.
  $form[$frameset_to]['ap_warning_text'] = array(
    '#markup' => '<div class="messages  status">
      Once phenotypic data has been uploaded for a genus, these vocabularies cannot be changed!
      Please take the time to read the description above and if you have questions,
      submit a ticket to <u>' . l('Github: UofS-Pulse-Binfo', 'https://github.com/UofS-Pulse-Binfo/analyzedphenotypes/issues') . '</u>.
      </div> <br />',
  );

  // Submit.
  $form[$frameset_to]['ap_saveontology_submit_button'] = array(
    '#type' => 'submit',
    '#value' => 'Save Ontology Configuration',
    '#validate' => array('system_settings_form_validate'),
    '#submit' => array('system_settings_form_config_submit'),
  );


  //
  // #FRAMESET CONTROLLED VOCABULARY TERMS.
  $frameset_cv = 'ap_frameset_cv';

  // Title.
  $form[$frameset_cv] = array(
    '#type' => 'fieldset',
    '#title' => t('Controlled Vocabulary Terms'),
    '#description' => t('Chado uses controlled vocabularies extensively to allow for flexible storing of data.
      As such, this module supports that flexibility to ensure that you have the ability to choose the terms that best support your data.'),
  );

  // Warning.
  $form[$frameset_cv]['ap_warning_text'] = array(
    '#markup' => '<div class="messages  status">
      We have helpfully selected what we think are the best ontology terms below.
      Thus the following configuration is completely optional, although I do recommend you review our choices.
      </div>',
  );

  // Array for property terms.
  $term_vars = analyzedphenotypes_systemvariables('terms');

  // Property terms.
  $terms_prop = array(
    'Title' => 'Please indicate the term we should use to indicate the property/relationship types specified below.',
    'Genus' => array(
      'title' => 'Genus',
      'default' => '',
      'field_name' => $term_vars['ap_genus'],
      'description'  => t('This term indicates that a given property is the associated "genus". For example, this module
        saves experiments producing phenotypic data as projects and the organism that experiment assays is indicated by
        adding a "genus" property to it.'),
    ),
    'Method' => array(
      'title' => 'Method',
      'default' => '',
      'field_name' => $term_vars['ap_method'],
      'description'  => t('This term indicates that a given trait propery describes the method with which the data was taken.'),
    ),
    'Related' => array(
      'title' => 'Related',
      'default' => '',
      'field_name' => $term_vars['ap_related'],
      'description'  => t('This term describes the relationship between your trait vocabulary term and the chosen equivalent crop ontolog term.'),
    ),
  );

  // Meta data terms.
  $terms_meta = array(
    'Title' => 'Please indicate the terms we should use for the metadata of each phenotypic measurement.',
    'Year' => array(
      'title' => 'Year',
      'default' => '',
      'field_name' => $term_vars['ap_year'],
      'description'  => t('Metadata tagged with this term indicates the year in which a phenotypic datapoint was collected'),
    ),
    'Location' => array(
      'title' => 'Location',
      'default' => '',
      'field_name' => $term_vars['ap_location'],
      'description'  => t('Metadata tagged with this term indicates the physical location of the environment giving rise to the phenotype.'),
    ),
    'Replicate' => array(
      'title' => 'Replicate',
      'default' => '',
      'field_name' => $term_vars['ap_replicate'],
      'description'  => t('Metadata tagged with this term indicates the unique identifier for the replicate the current datapoint is part of.'),
    ),
    'Data Collector' => array(
      'title' => 'Data Collector',
      'default' => '',
      'field_name' => $term_vars['ap_collector'],
      'description'  => t('Metadata tagged with this term indicates the name of the person who collected the phenotypic measurement.'),
    ),
  );

  $i = 0;
  while($i < 2) {
    // Which array to process.
    $all_terms = ($i == 0) ? $terms_prop : $terms_meta;

    // Write the render array.
    foreach($all_terms as $term => $prop) {
      if ($term == 'Title') {
        // Title - show as item and title.
        $form[$frameset_cv]['title_' . $i] = array(
          '#type' => 'item',
          '#title' => t($prop),
        );

        continue;
      }

      // Get default.
      // Since what is saved is cvterm_id into the variable, we need to convert the id
      // to something human readbale.
      $var_id = variable_get($prop['field_name']);
      $default_field = analyzedphenotypes_cvprop('term_id', array(
        'term_id' => $var_id,
      ));

      $default_value = (empty($default_field)) ? '' : $default_field;

      // Otherwise a autocomplete field.
      $form[$frameset_cv][ $prop['field_name'] ] = array(
        '#type' => 'textfield',
        '#title' => t($term),
        '#description' => t($prop['description']),
        '#default_value' => $default_value,
        '#attributes' => array('class' => array('ap-autocomplete-field-terms'),
          'title' => array($default_value)),
        //
        '#autocomplete_path' => 'admin/tripal/extension/analyzedphenotypes/json/cvterms/cv',
      );
    }

    $i++;
  }

  // Select field value when clicked to ease typing in new value.
  drupal_add_js('jQuery(document).ready(function() {
    jQuery(".ap-autocomplete-field-terms").focusin(function(){
      jQuery(this).select();
    });
  })', 'inline');

  // Warning.
  $form[$frameset_cv]['warning'] = array(
    '#markup' => '<div class="messages  status">
      Once phenotypic data has been uploaded for a genus, these vocabularies cannot be changed!
      Please take the time to read the description above and if you have questions,
      submit a ticket to <u>' . l('Github: UofS-Pulse-Binfo', 'https://github.com/UofS-Pulse-Binfo/analyzedphenotypes/issues') . '</u>.
      </div> <br />',
  );

  // Submit
  $form[$frameset_cv]['ap_savevc_submit_button'] = array(
    '#type' => 'submit',
    '#value' => 'Save Term Configuration',
    '#validate' => array('system_settings_form_validate'),
    '#submit' => array('system_settings_form_config_submit'),
  );


  return $form;
}


/**
 * FUNCTION CALLBACK:
 * Validate configuration page.
 */
function system_settings_form_validate($form, &$form_state) {
  // CHECK GENUS CV, DB AND ONTOLOGY:
  // For a give genus, if one field was altered then user is trying
  // to set a value and this validate should ensure that both
  // controlled vocabulary and database configuration must have a value selected.
  $cvdbon = analyzedphenotypes_systemvariables('cvdbon');

  foreach($cvdbon as $i => $vars) {
    $var_ctr = 0;
    $var_key = str_replace('ap_', '', $i);

    // Inspect cv, db and ontology field for each genus and see
    // if a field has been selected. If a field has been altered
    // check cv and databse field and ensure both are set.
    foreach($vars as $type => $v) {
      if ((int)$form_state['values'][$v] > 0) {
        $var_ctr++;
      }
    }

    if ($var_ctr > 0) {
      // Field set.
      if ($form_state['values'][$vars['cv']] <= 0) {
        // Controlled vocabulary.
        form_set_error($vars['cv'], 'Please select Controlled Vocabulary for ' . $var_key . ' genus.');
      }

      if ($form_state['values'][$vars['db']] <= 0) {
        // Associated databse.
        form_set_error($vars['db'], 'Please select Associated Database for ' . $var_key . ' genus.');
      }
    }
  }

  // CHECK CONTROLLED VOCABULARY TERMS:
  $terms = analyzedphenotypes_systemvariables('terms');

  // Test if value entered (field for terms are autocomplete).
  foreach($terms as $term => $t) {
    $field_term = $form_state['values'][$t];
    $var_key = str_replace('ap_', '', $term);

    if (empty($field_term)) {
      // Field is empty. Mark field.
      form_set_error($t, 'You must set the controlled vocabulary term for ' . $var_key . '.');
    }
    else {
      // Check if term in cvterm table.
      $is_term = analyzedphenotypes_cvprop('term_cv', array(
        'term_cv' => $field_term,
      ));

      if (is_null($is_term)) {
        // Invalid term. Mark field.
        form_set_error($t, $field_term . ' is not a valid controlled vocabulary term for term ' . $var_key . '. Please confirm that it already exists keeping in mind that case and whitespace are important.');
      }
    }
  }
}


/**
 * FUNCTION CALLBACK:
 * Save congfiguration variables.
 */
function system_settings_form_config_submit($form, &$form_state) {
  $vars = analyzedphenotypes_systemvariables();

  foreach($vars as $i => $var) {
    // Convert the values selected from the interface to id number (cvterm or cv id number).
    if ($i == 'cvdbon') {
      // Genus cv, db and ontology.
      // Select field returns the id number, no processing required.
      foreach ($var as $v) {
        foreach($v as $k) {
          $fld_value = $form_state['values'][$k];
          variable_set($k, $fld_value);
        }
      }
    }
    elseif ($i == 'options') {
      // Options. Allow new traits added to upload.
      // 1 or 0 value. No processing required,
      foreach ($var as $v) {
        $fld_value = $form_state['values'][$v];

        variable_set($v, $fld_value);
      }
    }
    elseif ($i == 'terms') {
      // Controlled vocabulary term.
      // Autocomplete returns cvterm name (cv term) format, requires cvterm_id number.
      foreach ($var as $v) {
        $fld_value = $form_state['values'][$v];

        $fld_value_id = analyzedphenotypes_cvprop('term_cv', array(
          'term_cv' => $fld_value,
        ));

        variable_set($v, $fld_value_id);
      }
    }
  }

  drupal_set_message('Configuration saved.', 'status');
}
