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
function analyzedphenotypes_admin_settings($form, &$form_state, $source = null) {
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
        'ap_method',
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
          $default_cvterm = $var_value;
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
        $var_value = variable_get($var['var']);
        $var_value_default = variable_get($var['def']);
  
        if ($var_value == 'not set' || empty($var_value)) {
          $cv_options = array(0 => '- Select -') + $cv_options;
          $default_cv = 0;
        }
        else {
          $default_cv = $var_value;
        }

        $form[$var['var']] = array(
          '#type' => 'select',
          '#title' => 'Select ontology controlled vocabulary for genus ' . ucfirst($title),
          '#required' => TRUE,
          '#options' => $cv_options,
          '#default_value' => $default_cv,
          '#id' => $var,
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
        
        $cv_field_value = (isset($form_state['values'][$var['var']]))
          ? $form_state['values'][$var['var']] : 0;

        if ($cv_field_value > 0) {
          $default_cv = $cv_field_value;
        }

        if ($default_cv > 0) {
          $default_cvterm = $var_value_default;
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

        $form['ap_AJAX_wrapper_getcvterms' . $title][$var['def']] = array(
          '#type' => 'select',
          '#title' => 'Select default term',
          '#options' => $cvterm_options,
          '#default_value' => $default_cvterm,
          '#description' => t('Select Controlled Vocabulary (CV) and CV Term.'),
          '#id' => $i . '_term_select_field',
        );
      }

      break;

      //
      // Define additional case here.

      //
      default:
        $base_admin = 'admin/tripal/extension/analyzedphenotypes';

        $header = array('Section', 'Information', 'View');
        $rows[] = array('Crop Ontology', 'Map a Genus to a Crop Ontology Vocabularies.', l('view', $base_admin . '/settings/crop_ontology'));
        $rows[] = array('Database and CV Terms', 'Set the default Database and Controlled Vocabulary.', l('view', $base_admin . '/settings/database_cv'));
        $rows[] = array('Plant Property Traits', 'Define Controlled Vocabulary for Plant Property Traits location, Replicate, Year and Data Collector.', l('view', $base_admin . '/settings/property_terms'));

        $form['ap_table'] = array(
          '#markup' => theme('table', array(
            'header' => $header,
            'rows' => $rows,
            'attributes' => array(
              'id' => 'ap-settings-info-table',
              'sticky' => FALSE,
            ),
          )),
        );

        return $form;
  }

  if (in_array($source, $valid_source)) {
    // Update materialized view to use terms Location and Year.
    
    /*
    if ($source == 'property_terms' && isset($form_state['triggering_element']) && $form_state['triggering_element'] !=  'select') {
      // Find the materialized view.
      $mview_id = tripal_get_mview_id('mview_phenotype');
      if ($mview_id) {
        $term_id_location = variable_get($vars['ap_location']);
        $term_id_year = variable_get($vars['ap_year']);
      
        $update_mview = analyzedphenotypes_mview('view_phenotypes', array(
          'term_id_location' => $term_id_location,
          'term_id_year' => $term_id_year,
        ));
        
        tripal_edit_mview($mview_id, 'mview_phenotype', 'analyzedphenotypes', null, null, null, $update_mview['query']);
      }
    }
    */
    
    //return system_settings_form($form);
  }
  
  
  $form = system_settings_form($form);
  $form['source'] = array(
    '#type' => 'value',
    '#value' => $source,
  );
  
  $form['#submit'][] = 'analyzedphenotypes_updatemview';
  
  return $form;
}


/**
 * FUNCTION CALLBACK
 * Update materialized view used in Phenotype Visualizer.
 */
function analyzedphenotypes_updatemview(&$form, &$form_state) {
  $souce = $form_state['values']['source'];
  
  if ($souce == 'property_terms') {
    // Update materialized views to use the terms location and year selected in this form.
    $mview_phenotype = 'mview_phenotype';
    $mview_id = tripal_get_mview_id($mview_phenotype);

    if ($mview_id) {
      $vars = analyzedphenotypes_systemvars('plant_property');
  
      $term_id_location = variable_get($vars['ap_location']);
      $term_id_year = variable_get($vars['ap_year']);

      $update_mview = analyzedphenotypes_mview('view_phenotypes', array(
        'term_id_location' => $term_id_location,
        'term_id_year' => $term_id_year,
      ));
      
      // This works but it deletes the schema and brakes the view.
      // tripal_edit_mview($mview_id, $mview_phenotype, 'analyzedphenotypes', '', '', 0, $update_mview['query'], '');
      
      chado_query("UPDATE tripal_mviews SET query = :mview_query  WHERE mview_id = :mview_id", array(
        ':mview_query' => $update_mview['query'],
        ':mview_id' => $mview_id,
      ));
    }
  }
}


/**
 * FUNCTION CALLBACK
 * Main directory page.
 */
function analyzedphenotypes_admin_page_directory($form, &$form_state) {
  // Create a directory of links in the module.
  // Array to hold administrative page directory in this module.
  // Key - page id and page info.
  $arr_directory = array(
    'data loader' => array('page_id' => 'upload', 'info' => 'Upload Analyzed Phenotypic Data'),
  );

  $form['#ap_admin_directory'] = $arr_directory;


  return $form;
}



///////



/**
 * FUNCTION CALLBACK
 * Construct data downloader form
 */
function analyzedphenotypes_data_download($form, &$form_state) {
  // Include CSS and JavaScript.
  $path_theme = drupal_get_path('module', 'analyzedphenotypes') . '/theme/';
  $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.downloader.global.css';
  $form['#attached']['js'] [] = $path_theme . 'js/analyzedphenotypes.downloader.script.js';
  
  // Field names.
  // Same name will be used for hooks, js etc.
  $fldname = analyzedphenotypes_fieldnames();
  
  // F. 1.
  $fld_experiment      = $fldname['experiment']['base']; 
  $fld_genus           = $fldname['genus']['base'];
  $fld_species         = $fldname['species']['base'];
  $fld_traits          = $fldname['traits']['base'];
  // F. 2.
  $fld_year            = $fldname['year']['base'];
  $fld_location        = $fldname['location']['base'];
  $fld_germplasmtype   = $fldname['germplasmtype']['base'];
  $fld_germplasm       = $fldname['germplasm']['base']; 
  $fld_missingdata     = $fldname['missingdata']['base']; 
  // F. 3.
  $fld_filetype        = $fldname['filetype']['base'];
  $fld_averagerep      = $fldname['averagerep']['base'];
  $fld_rfriendly       = $fldname['rfriendly']['base']; 
  $fld_columnheaders   = $fldname['columnheaders']['base']; 

  // Field form properties.
  $field_prop = array(
    //
    $fld_experiment    => array('options'),
    $fld_genus         => array('disabled' => TRUE, 'options' => array(), 'empty'   => t('- Select -')),
    $fld_species       => array('disabled' => TRUE, 'options' => array(), 'empty'   => t('- Select -')),
    $fld_traits        => array('disabled' => TRUE, 'options' => array(), 'default' => array(0)),
    //
    $fld_year          => array('disabled' => TRUE, 'options' => array(), 'default' => array(0)),
    $fld_location      => array('disabled' => TRUE, 'options' => array(), 'default' => array(0)),
    $fld_germplasmtype => array('disabled' => TRUE, 'options' => array(), 'default' => array(0)),
    $fld_germplasm     => array('disabled' => TRUE, 'options' => array(), 'default' => ''),
    $fld_missingdata   => array('disabled' => TRUE, 'options' => array(), 'default' => '100%'),
    //
    $fld_filetype      => array('disabled' => TRUE),
    $fld_averagerep    => array('disabled' => TRUE),
    $fld_rfriendly     => array('disabled' => TRUE),
  );

  // Main AJAX Wrapper.
  $AJAX_wrapper_id = 'ap-AJAX-wrapper-main';
  // Use this character to separate entries in a string.
  $delimiter = '#';
  
  // Global AJAX settings.
  // This array element will add AJAX to any field.
  $AJAX_callback['#ajax'] = array(
    'event' => 'change',
    'callback' => 'analyzedphenotypes_AJAX_callback',
    'case' => 'main',
    'wrapper' => $AJAX_wrapper_id,
    'progress' => array('type' => 'none', 'message' => ''),
  );
  
  // Main AJAX wrapper. Any field wrapped in this container
  // will be processed by AJAX request.
  $AJAX_wrapper = 'ap_AJAX_wrapper_main';
  $form[$AJAX_wrapper] = array(
    '#prefix' => '<div id="' . $AJAX_wrapper_id . '">',
    '#suffix' => '</div>'
  );

  // HANDLE AJAX:
  // Initialize form field value and state.
  
  
  // Create list of default and optional column headers.
  // Will return Experiment, Trait Name, Germplasm Name, Year, Location, Replicate, Data Collector, Country of Origin.
  $headers = analyzedphenotypes_columnsprop('downloadable');
  $default_headers = array_combine($headers, $headers);
  $optional_headers = array();
    
  // Move Data Collector and Origin to optional column headers.
  $optional_headers['Data Collector'] = 'Data Collector';
  unset($default_headers['Data Collector']);
       
  $optional_headers['Origin'] = 'Country of Origin';
  unset($default_headers['Country of Origin']);
    
  // By default form averages rep - replicate is not required.
  unset($default_headers['Replicate']);
  
  // Contain all field values.
  $field_values = null;
  // Contain all cache values of fields.
  $cache_values = null;
  
  // Submit button. This ensure the submit button gets enabled
  // when this variable is set to 1.
  $btn_ok = 0;
  
  
  // Handle AJAX. form state values has values.
  if (isset($form_state['values'])) {
    //
    if ($form_state['values'][ $fldname[$fld_experiment]['field'] ] != '') {
      /// Experiment selected. 
      $e = $form_state['values'][ $fldname[$fld_experiment]['field'] ];
      $field_values[$fld_experiment] = analyzedphenotypes_checkboxesvalue($e);

      // Enable avaerage rep and r friendly options.
      if (count($field_values[$fld_experiment])) {
        $field_prop[$fld_filetype]  ['disabled'] = FALSE;
        $field_prop[$fld_averagerep]['disabled'] = FALSE;
        $field_prop[$fld_rfriendly] ['disabled'] = FALSE;
      }
      
      // Project selected. Test if multiple projects selected.
      if (count($field_values[$fld_experiment]) > 1) {
        // MANY EXPERIMENTS.
        
        // Enable submit button.
        $btn_ok = 1;
        
        // #Genus.
        $field_values[$fld_genus] = 'all';

        $field_prop[$fld_genus]['options'] = array($field_values[$fld_genus] => 'All Genus');
        $field_prop[$fld_genus]['disabled'] = FALSE;
        $field_prop[$fld_genus]['empty'] = null;

        // #Species.
        $field_values[$fld_species] = 'all';

        $field_prop[$fld_species]['options'] = array($field_values[$fld_genus] => 'All Species');
        $field_prop[$fld_species]['disabled'] = FALSE;
        $field_prop[$fld_species]['empty'] = null;

        // #Traits.
        $options = analyzedphenotypes_populatefield('field_traits', array(
          'project_id' => $field_values[$fld_experiment],
        ));

        if ($options) {
          $field_values[$fld_traits] = array_keys($options);

          $field_prop[$fld_traits]['default'] = $field_values[$fld_traits];
          $field_prop[$fld_traits]['options'] = $options;
          $field_prop[$fld_traits]['disabled'] = FALSE;
        
          // #Year.
          unset($options);
          $options = analyzedphenotypes_populatefield('field_year', array(
            'project_id' => $field_values[$fld_experiment],
            'traits' => $field_values[$fld_traits],
          ));
          
          if ($options) {
            $field_values[$fld_year] = array_keys($options);
            
            $field_prop[$fld_year]['default'] = $field_values[$fld_year];
            $field_prop[$fld_year]['options'] = $options;
            $field_prop[$fld_year]['disabled'] = FALSE;
            
            // #Location.
            unset($options);
            $options = analyzedphenotypes_populatefield('field_location', array(
              'project_id' => $field_values[$fld_experiment],
              'traits' => $field_prop[$fld_traits]['default'],
              'year' => $field_prop[$fld_year]['default'],
            ));
            
            if ($options) {
              $field_values[$fld_location] = array_keys($options);
              
              $field_prop[$fld_location]['default'] = $field_values[$fld_location];
              $field_prop[$fld_location]['options'] = $options;
              $field_prop[$fld_location]['disabled'] = FALSE;
            
              // #Germplasm Type.
              unset($options);
              $options = analyzedphenotypes_populatefield('field_germplasmtype', array(
                'project_id' => $field_values[$fld_experiment],
                'traits' => $field_prop[$fld_traits]['default'],
                'year' => $field_prop[$fld_year]['default'],
                'location' => $field_prop[$fld_location]['default'],
              ));
              
              if ($options) {
                $field_values[$fld_germplasmtype] = array_keys($options);
                
                $field_prop[$fld_germplasmtype]['default'] = $field_values[$fld_germplasmtype];
                $field_prop[$fld_germplasmtype]['options'] = $options;
                $field_prop[$fld_germplasmtype]['disabled'] = FALSE;
                
                // Germplasm.
                unset($options);
                $options = analyzedphenotypes_populatefield('field_germplasm', array(
                  'project_id' => $field_values[$fld_experiment],
                ));
                
                if ($options) {
                  $field_prop[$fld_germplasm]['default'] = '';
                  $field_prop[$fld_germplasm]['options'] = $options;
                  $field_prop[$fld_germplasm]['disabled'] = TRUE;
                  
                  $field_prop[$fld_missingdata]['default'] = '';
                  $field_prop[$fld_missingdata]['disabled'] = TRUE;
                // end Germplasm.  
                }
              // end Germplasm Type.  
              }
            // end Location.  
            }
          // end Year.  
          }
        // end Traits.  
        }
        
      // END MANY EXPERIMENTS.
      }
      else {
        // ONE EXPERIMENT.

        $field_values[$fld_experiment] = current($field_values[$fld_experiment]);
        
        // Experiment is optional with single project.
        if ($field_values[$fld_experiment] > 0) {
          unset($default_headers['Experiment']);
          $optional_headers['Experiment'] = 'Experiment';
        }
        
        // #Genus.
        $options = analyzedphenotypes_populatefield('field_genus', array(
          'project_id' => $field_values[$fld_experiment],
        ));
        
        if ($options) { 
          $field_prop[$fld_genus]['options'] = $options;
          $field_prop[$fld_genus]['disabled'] = FALSE;
          $field_prop[$fld_genus]['empty'] = null;

          $field_values[$fld_genus] = current($options);
          
          // #Species
          unset($options);
          $options = analyzedphenotypes_populatefield('field_species', array(
            'project_id' => $field_values[$fld_experiment],
            'genus' => $field_values[$fld_genus],
          ));

          if ($options) {
            $field_prop[$fld_species]['disabled'] = FALSE;
            $field_prop[$fld_species]['options'] = $options;
          }
          
          unset($options);
        }
        
        // Cache value selected. This will allow us to compare cache values
        // with current values returned from selecting different filters in the form.
        // NOTE: _cachevalue() function returns string as a result of imploding the array.
        $form['values'][ $fldname[$fld_experiment]['cache'] ] = analyzedphenotypes_cachevalue($field_values[$fld_experiment]);
        $form['values'][ $fldname[$fld_genus]['cache'] ] = analyzedphenotypes_cachevalue($field_values[$fld_genus]);
        
        $cache_values[$fld_experiment] = $form_state['values'][ $fldname[$fld_experiment]['cache'] ];
        $cache_values[$fld_genus] = $form_state['values'][ $fldname[$fld_genus]['cache'] ];
        
        // AJAX - same experiment and genus.
        if ($cache_values[$fld_experiment] == $field_values[$fld_experiment] && 
            $cache_values[$fld_genus] == $field_values[$fld_genus]) {
          
          /// Species selected - Create traits options.
          if ($form_state['values'][ $fldname[$fld_species]['field'] ] != '') {
            $field_values[$fld_species] = $form_state['values'][ $fldname[$fld_species]['field'] ];
            
            $options = analyzedphenotypes_populatefield('field_traits', array(
              'project_id' => $field_values[$fld_experiment],
              'genus' => $field_values[$fld_genus],
              'species' => $field_values[$fld_species],
            ));
            
            // #Traits.
            if ($options) {
              $field_prop[$fld_traits]['options'] = $options;
              $field_prop[$fld_traits]['disabled'] = FALSE;
            }
            
            unset($options);
            
            // Cache value selected.
            $form['values'][ $fldname[$fld_species]['cache'] ] = analyzedphenotypes_cachevalue($field_values[$fld_species]);
            $cache_values[$fld_species] = $form_state['values'][ $fldname[$fld_species]['cache'] ];
            
            // AJAX - same experiment, genus and species.
            if ($cache_values[$fld_species] == $field_values[$fld_species]) {
              
              ///// Traits selected - Create year options.
              if ($form_state['values'][ $fldname[$fld_traits]['field'] ] != '') {
                $t = $form_state['values'][ $fldname[$fld_traits]['field'] ];
                $field_values[$fld_traits] = analyzedphenotypes_checkboxesvalue($t);
                
                if (count($field_values[$fld_traits]) > 0) {
                  // User selected some traits, enable submit button.
                  $btn_ok = 1;
                  
                  $options = analyzedphenotypes_populatefield('field_year', array(
                    'project_id' => $field_values[$fld_experiment],
                    'traits' => $field_values[$fld_traits],
                  ));
                  
                  if ($options) {
                    $field_prop[$fld_year]['options'] = $options;
                    $field_prop[$fld_year]['disabled'] = FALSE;
                    
                    // Year, Location and Germplasm Type.
                    if (count($field_values[$fld_traits]) > 1) {
                      // Many traits selected.
                      $field_values[$fld_year] = array_keys($options);
                      
                      $field_prop[$fld_year]['default'] = $field_values[$fld_year];
                      $options = analyzedphenotypes_populatefield('field_location', array(
                        'project_id' => $field_values[$fld_experiment],
                        'genus' => $field_values[$fld_genus],
                        'species' => $field_values[$fld_species],
                        'traits' => $field_values[$fld_traits],
                        'year' => $field_values[$fld_year],
                      ));
                      
                      if ($options) {
                        $field_values[$fld_location] = array_keys($options);
              
                        $field_prop[$fld_location]['default'] = $field_values[$fld_location];
                        $field_prop[$fld_location]['options'] = $options;
                        $field_prop[$fld_location]['disabled'] = FALSE;
                      
                        $options = analyzedphenotypes_populatefield('field_germplasmtype', array(
                          'project_id' => $field_values[$fld_experiment],
                          'genus' => $field_values[$fld_genus],
                          'species' => $field_values[$fld_species],
                          'traits' => $field_values[$fld_traits],
                          'year' => $field_values[$fld_year],
                          'location' => $field_values[$fld_location],
                        ));
                        
                        if ($options) {
                          $field_values[$fld_germplasmtype] = array_keys($options);
                
                          $field_prop[$fld_germplasmtype]['default'] = $field_values[$fld_germplasmtype];
                          $field_prop[$fld_germplasmtype]['options'] = $options;
                          $field_prop[$fld_germplasmtype]['disabled'] = FALSE;
                        }
                      }
                    }
                  }
                  
                  unset($options);
                  
                  // Cache value selected.
                  $form['values'][ $fldname[$fld_traits]['cache'] ] = analyzedphenotypes_cachevalue($field_values[$fld_traits]);
                  $cache_values[$fld_traits] = $form_state['values'][ $fldname[$fld_traits]['cache'] ];
                  
                  // AJAX - same experiment, genus, species and traits.
                  if ($cache_values[$fld_traits] == implode('_', $field_values[$fld_traits])) {
                    
                    ////// Year selected - Create location options.
                    if ($form_state['values'][ $fldname[$fld_year]['field'] ] != '') {
                      $y = $form_state['values'][ $fldname[$fld_year]['field'] ];
                      $field_values[$fld_year] = analyzedphenotypes_checkboxesvalue($y);
                      
                      if (count($field_values[$fld_year]) > 0) {
                        $options = analyzedphenotypes_populatefield('field_location', array(
                          'project_id' => $field_values[$fld_experiment],
                          'genus' => $field_values[$fld_genus],
                          'species' => $field_values[$fld_species],
                          'traits' => $field_values[$fld_traits],
                          'year' => $field_values[$fld_year],
                        ));
                        
                        if ($options) {
                          $field_prop[$fld_location]['options'] = $options;
                          $field_prop[$fld_location]['disabled'] = FALSE;
                          
                          // Location and Germplasm Type.
                          if (count($field_values[$fld_year]) > 1) {
                            // Many years selected.
                            $field_values[$fld_location] = array_keys($options);
              
                            $field_prop[$fld_location]['default'] = $field_values[$fld_location];
                            $options = analyzedphenotypes_populatefield('field_germplasmtype', array(
                              'project_id' => $field_values[$fld_experiment],
                              'genus' => $field_values[$fld_genus],
                              'species' => $field_values[$fld_species],
                              'traits' => $field_values[$fld_traits],
                              'year' => $field_values[$fld_year],
                              'location' => $field_values[$fld_location],
                            ));
                            
                            if ($options) {
                              $field_values[$fld_germplasmtype] = array_keys($options);
                
                              $field_prop[$fld_germplasmtype]['default'] = $field_values[$fld_germplasmtype];
                              $field_prop[$fld_germplasmtype]['options'] = $options;
                              $field_prop[$fld_germplasmtype]['disabled'] = FALSE;
                            }
                          }
                        }
                        
                        unset($options);
                        
                        // Cache value selected.
                        $form['values'][ $fldname[$fld_year]['cache'] ] = analyzedphenotypes_cachevalue($field_values[$fld_year]);
                        $cache_values[$fld_year] = $form_state['values'][ $fldname[$fld_year]['cache'] ];

                        // AJAX - same experiment, genus, species, traits and year.
                        if ($cache_values[$fld_year] == implode('_', $field_values[$fld_year])) {

                          /////// Location selected - Create germplasm type options.
                          if ($form_state['values']['ap_' . $fld_location . '_field'] != '') {
                            $l = $form_state['values']['ap_' . $fld_location . '_field'];
                            $field_values[$fld_location] = analyzedphenotypes_checkboxesvalue($l);
                            
                            if (count($field_values[$fld_location]) > 0) {
                              $options = analyzedphenotypes_populatefield('field_germplasmtype', array(
                                'project_id' => $field_values[$fld_experiment],
                                'genus' => $field_values[$fld_genus],
                                'species' => $field_values[$fld_species],
                                'traits' => $field_values[$fld_traits],
                                'year' => $field_values[$fld_year],
                                'location' => $field_values[$fld_location],
                              ));

                              if ($options) {
                                $field_prop[$fld_germplasmtype]['options'] = $options;
                                $field_prop[$fld_germplasmtype]['disabled'] = FALSE;
                              
                                // Germplasm Type.
                                if (count($field_values[$fld_location]) > 1) {
                                  // Many locations selected.
                                  $field_values[$fld_germplasmtype] = array_keys($options);
                                  
                                  $field_prop[$fld_germplasmtype]['default'] = $field_values[$fld_germplasmtype];
                                  $options = analyzedphenotypes_populatefield('field_germplasmtype', array(
                                    'project_id' => $field_values[$fld_experiment],
                                    'genus' => $field_values[$fld_genus],
                                    'species' => $field_values[$fld_species],
                                    'traits' => $field_values[$fld_traits],
                                    'year' => $field_values[$fld_year],
                                    'location' => $field_values[$fld_location],
                                  ));
                                  
                                  if ($options) {
                                    $field_values[$fld_germplasmtype] = array_keys($options);
                
                                    $field_prop[$fld_germplasmtype]['default'] = $field_values[$fld_germplasmtype];
                                    $field_prop[$fld_germplasmtype]['options'] = $options;
                                    $field_prop[$fld_germplasmtype]['disabled'] = FALSE;
                                  }
                                }
                              }
                              
                              // Cache value selected.
                              $form['values'][ $fldname[$fld_location]['cache'] ] = analyzedphenotypes_cachevalue($field_values[$fld_location]);
                              $cache_values[$fld_location] = $form_state['values'][ $fldname[$fld_location]['cache'] ];
                              
                              // AJAX - same experiment, genus, species, traits, year and location.
                              if ($cache_values[$fld_location] == implode('_', $field_values[$fld_location])) {
                                
                                //////// Germplasm type selected - Update germplasm list.
                                if ($form_state['values'][ $fldname[$fld_germplasmtype]['field'] ] != '') {
                                  $g = $form_state['values'][ $fldname[$fld_germplasmtype]['field'] ];
                                  $field_values[$fld_germplasmtype] = analyzedphenotypes_checkboxesvalue($g);
                                ////////  
                                }
                              // end AJAX - same experiment, genus, species, traits, year and location.  
                              }
                              else {
                                if (isset($form_state['input'][ $fldname[$fld_germplasmtype]['field'] ])) { 
                                  $input = $form_state['input'][ $fldname[$fld_germplasmtype]['field'] ];
                                  foreach($input as $i => $v) {
                                    if (!empty($v)) {
                                      unset($form_state['input'][ $fldname[$fld_germplasmtype]['field'] ]);
                                      break;
                                    }
                                  }
                                }
                              // End else ^  
                              }
                            }
                          ///////  
                          }
                        // end AJAX - same experiment, genus, species, traits and year.  
                        }
                        else {
                          if (isset($form_state['input'][ $fldname[$fld_location]['field'] ])) { 
                            $input = $form_state['input'][ $fldname[$fld_location]['field'] ];
                            foreach($input as $i => $v) {
                              if (!empty($v)) {
                                unset($form_state['input'][ $fldname[$fld_location]['field'] ]);
                                break;
                              }
                            }
                          }
                        // End else ^  
                        }
                      }
                    //////  
                    }
                  // end AJAX - same experiment, genus, species and traits.  
                  }
                  else {
                    if (isset($form_state['input'][ $fldname[$fld_year]['field'] ])) {
                      $input = $form_state['input'][ $fldname[$fld_year]['field'] ];
                      foreach($input as $i => $v) {
                        if ($v > 0) {
                          unset($form_state['input'][ $fldname[$fld_year]['field'] ]);
                          break;
                        }
                      }
                    }
                  // End else ^  
                  }
                }
              /////  
              }
            // end AJAX - same experiment, genus and species.  
            }
          ///  
          }
        // end AJAX - same experiment and genus.
        }
        
        // Update germplasm list.
        $values = array(
          'project_id' => $field_values[$fld_experiment],
          'genus' => $field_values[$fld_genus],
          'species' => $field_values[$fld_species],
          'traits' => $field_values[$fld_traits],
          'year' => $field_values[$fld_year],
          'location' => $field_values[$fld_location],
          'germplasmtype' => $field_values[$fld_germplasmtype],
        );

        $options = analyzedphenotypes_populatefield('field_germplasm', $values);

        if ($options) {
          $field_prop[$fld_germplasm]['disabled'] = FALSE;
          $field_prop[$fld_germplasm]['options'] = $options;
          $field_prop[$fld_germplasm]['default'] = '';
           
          $field_prop[$fld_missingdata]['disabled'] = FALSE;
        }
      // END ONE EXPERIMENT.
      }

      // Replace Trait name to Traits selected.
      // eg. Plant Height (cm), Planting Data ....
      if (count($field_prop[$fld_traits]['options']) > 0) {
        $s = array();
        foreach($field_prop[$fld_traits]['options'] as $id => $name) {
          if (in_array($id, $field_values[$fld_traits])) {
            $s[] = $name;
          }
        }
        
        $traits_list = implode(', ', $s);
        
        $default_headers['Trait Name'] = $traits_list;
      }
      
      // Add Replicate to Default headers when user does not want to average data.
      if ($form_state['values'][ $fldname[$fld_averagerep]['field'] ] == 0) {
        $default_headers['Replicate'] = 'Replicate';
      }
    ///  
    }
  //
  }
  
  
  //
  //
  // Fieldset: #1. About the traits.
  $ap_fieldset_1 = 'ap_about_trait_fieldset';
  $form[$AJAX_wrapper][$ap_fieldset_1] = array(
    '#type' => 'fieldset',   
    '#title' => 'Select Trait.',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#id' => 'ap-about-traits-fieldset',
  );
    
    $form[$AJAX_wrapper][$ap_fieldset_1]['ap_fieldset1_description'] = array(
      '#type' => 'item',
      '#description' => t('Indicate the trait you would like phenotypic data for by selecting 
        experiment and the genus of the crop, as well as the name of the trait below. Field marked with a * means field is required and must have a value before proceeding.<br /><br /><strong>It is highly 
        recommended</strong> to restrict the dataset to a specific experiment. This can be done by entering the name of 
        the experiment below (the name will autocomplete as you type). You can further filter by year and location if desired.'),
      '#id' => 'ap-fieldset1-description',
    );
    
    
    //
    // FIELD: EXPERIMENT: 
    $exp = analyzedphenotypes_populatefield('field_project');
    $field_prop[$fld_experiment]['options'] = ($exp) ? $exp : array();
    $experiment_count = count($field_prop[$fld_experiment]['options']);
    
    $form[$AJAX_wrapper][$ap_fieldset_1][$fld_experiment]['title'] = array(
      '#title' => t('Experiment'),
      '#type' => 'item',
      '#required' => TRUE,
    );

    $h = ($experiment_count < 2) ? 35 : 25;
    $form[$AJAX_wrapper][$ap_fieldset_1][ $fldname[$fld_experiment]['field'] ] = array(
      '#type' => 'checkboxes',
      '#options' => $field_prop[$fld_experiment]['options'],

      // Class to indicate field is evaluated when generating a set of headers.
      '#attributes' => array('class' => array('ap-field-header')),
      '#id' => $fldname[$fld_experiment]['field-id'],
    )
    + $AJAX_callback
    // Always reveal all available experiment.
    + analyzedphenotypes_wrapcheckbox($h * $experiment_count, $experiment_count);

    // Count data points, # of traits and # of stocks.
    $summary = analyzedphenotypes_populatefield('field_project_summary');
    
    // Show a short summary (# of data available) on mouse over.
    if ($summary) {
      $markup = '';
      
      foreach($summary as $id => $info) {
        $markup .= '<div id="ap-info-experiment' . $id . '" class="ap-tooltip">' . $info . '</div>';
      }
      
      $form[$AJAX_wrapper][$ap_fieldset_1][$fld_experiment]['tooltip'] = array(
        '#markup' => $markup,
      );
    }
    ///////
    
    
    //
    // FIELD: GENUS AND SPECIES:
    $form[$AJAX_wrapper][$ap_fieldset_1][ $fldname[$fld_genus]['field'] ] = array(
      '#type' => 'select',
      '#title' => t('Genus'),
      '#required' => TRUE,
      '#default_value' => 0,
      '#empty_option'  => $field_prop[$fld_genus]['empty'],
      '#options'       => $field_prop[$fld_genus]['options'],
      '#disabled'      => $field_prop[$fld_genus]['disabled'],
      
      // Class to indicate field aligned next to another.
      '#prefix' => '<div class="ap-field-inline-container">',
      '#suffix' => '</div>',
      '#id' => $fldname[$fld_genus]['field-id'],
    ) 
    + $AJAX_callback;
    
    //
    $form[$AJAX_wrapper][$ap_fieldset_1]['ap_' . $fld_species . '_field'] = array(
      '#title' => t('Species'),
      '#type' => 'select',
      '#required' => TRUE,
      '#default_value' => 0,
      '#empty_option'  => $field_prop[$fld_species]['empty'],
      '#options'       => $field_prop[$fld_species]['options'],
      '#disabled'      => $field_prop[$fld_species]['disabled'],
      
      // Class to indicate field aligned next to another.
      '#prefix' => '<div class="ap-field-inline-container">',
      '#suffix' => '</div>
                    <div class="ap-clear-float"></div>',
      '#id' => $fldname[$fld_species]['field-id'],
    ) 
    + $AJAX_callback;

    $form[$AJAX_wrapper][$ap_fieldset_1]['ap_genus_species_description'] = array(
      '#type' => 'item',
      '#description' => t('Select the genus of the crop you would like phenotypic data for. Additionally, the species 
        can be indicated to further restrict the germplasm phenotypic data is exported for.'),
      
      '#id' => 'ap-genus-species-description',
    );
    ///////
    
    
    //
    // FIELD: TRAIT NAME.
    $form[$AJAX_wrapper][$ap_fieldset_1][$fld_traits]['title'] = array(
      '#title' => t('Trait Name'),
      '#type' => 'item',
      '#required' => TRUE,
    );
    
    $form[$AJAX_wrapper][$ap_fieldset_1][ $fldname[$fld_traits]['field'] ] = array(
      '#type' => 'checkboxes',
      '#default_value' => $field_prop[$fld_traits]['default'],
      '#options'       => $field_prop[$fld_traits]['options'],
      '#disabled'      => $field_prop[$fld_traits]['disabled'],

      // Class to indicate field is evaluated when generating a set of headers.
      '#attributes' => array('class' => array('ap-field-header')),
      '#id' => $fldname[$fld_traits]['field-id'],
    )
    + $AJAX_callback
    + analyzedphenotypes_wrapcheckbox(115, count($field_prop[$fld_traits]['options']));
    
    // Select all options when many experiments selected.
    if ($field_prop[$fld_traits]['default'][0] > 0 || !empty($field_prop[$fld_traits]['default'][0])) {
      foreach($field_prop[$fld_traits]['default'] as $o) {
        $form[$AJAX_wrapper][$ap_fieldset_1][ $fldname[$fld_traits]['field'] ][$o]['#attributes'] 
          = array('checked' => 'checked');
      }   
    }
    ///////
    
    
    
  //
  //
  // Fieldset: #2. Additional filter criteria.
  $ap_fieldset_2 = 'ap_additional_filter_fieldset';
  $form[$AJAX_wrapper][$ap_fieldset_2] = array(
    '#title' => t('Additional filter criteria (Optional).'),
    '#type' => 'fieldset',   
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );  
    
    $form[$AJAX_wrapper][$ap_fieldset_2]['ap_fieldset2_description'] = array(
      '#type' => 'item',
      '#description' => t('We recommend you fill out as many of the following filters as possible to narrow the phenotype set to those you are most interested in.'),
      '#id' => 'ap-fieldset2-description',
    );
 
 
    //
    // FIELD: YEAR.
    $form[$AJAX_wrapper][$ap_fieldset_2][$fld_year]['title'] = array(
      '#title' => t('Year'),
      '#type' => 'item',
    );

    $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_year]['field'] ] = array(
      '#type' => 'checkboxes',
      '#default_value' => $field_prop[$fld_year]['default'],
      '#options'       => $field_prop[$fld_year]['options'],
      '#disabled'      => $field_prop[$fld_year]['disabled'],

      // Class to indicate field is evaluated when generating a set of headers.
      '#attributes' => array('class' => array('ap-field-header')),
      '#id' => $fldname[$fld_year]['field-id'],
    )
    + $AJAX_callback
    + analyzedphenotypes_wrapcheckbox(70, count($field_prop[$fld_year]['options']));
   
    // Select all options when many experiments selected.
    if ($field_prop[$fld_year]['default'][0] > 0 || !empty($field_prop[$fld_year]['default'][0])) {
      foreach($field_prop[$fld_year]['default'] as $o) {
        $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_year]['field'] ][$o]['#attributes'] 
          = array('checked' => 'checked');
      }   
    }
    
    
    // 
    // FIELD: LOCATION.
    $form[$AJAX_wrapper][$ap_fieldset_2][$fld_location]['title'] = array(
      '#title' => t('Location'),
      '#type' => 'item',
    );

    $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_location]['field'] ] = array(
      '#type' => 'checkboxes',
      '#default_value' => $field_prop[$fld_location]['default'],
      '#options'       => $field_prop[$fld_location]['options'],
      '#disabled'      => $field_prop[$fld_location]['disabled'],

      // Class to indicate field is evaluated when generating a set of headers.
      '#attributes' => array('class' => array('ap-field-header')),
      '#id' => $fldname[$fld_location]['field-id'],
    )
    + $AJAX_callback
    + analyzedphenotypes_wrapcheckbox(70, count($field_prop[$fld_location]['options']));
   
    // Select all options when many experiments selected.
    if ($field_prop[$fld_location]['default'][0] > 0 || !empty($field_prop[$fld_location]['default'][0])) {
      foreach($field_prop[$fld_location]['default'] as $o) {
        $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_location]['field'] ][$o]['#attributes'] 
          = array('checked' => 'checked');
      }   
    }
    

    // 
    // FIELD: GERMPLASM TYPE.
    $form[$AJAX_wrapper][$ap_fieldset_2][$fld_germplasmtype]['title'] = array(
      '#title' => t('Germplasm Type'),
      '#type' => 'item',
    );

    $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_germplasmtype]['field'] ] = array(
      '#type' => 'checkboxes',
      '#default_value' => $field_prop[$fld_germplasmtype]['default'],
      '#options'       => $field_prop[$fld_germplasmtype]['options'],
      '#disabled'      => $field_prop[$fld_germplasmtype]['disabled'],

      // Class to indicate field is evaluated when generating a set of headers.
      '#attributes' => array('class' => array('ap-field-header')),
      '#id' => $fldname[$fld_germplasmtype]['field-id'],
    )
    + $AJAX_callback
    + analyzedphenotypes_wrapcheckbox(70, count($field_prop[$fld_germplasmtype]['options']));
   
    // Select all options when many experiments selected.
    if ($field_prop[$fld_germplasmtype]['default'][0] > 0 || !empty($field_prop[$fld_germplasmtype]['default'][0])) {
      foreach($field_prop[$fld_germplasmtype]['default'] as $o) {
        $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_germplasmtype]['field'] ][$o]['#attributes'] 
          = array('checked' => 'checked');
      }   
    }
    
    
    //
    // FIELD: GERMPLASM.
    $form[$AJAX_wrapper][$ap_fieldset_2][$fld_germplasm]['title'] = array(
      '#title' => t('Germplasm'),
      '#type' => 'item',
    );
    
    // GERMPLASM BAR
    // Suggest all germplasm found given the combination of fiters above.
    if ($field_prop[$fld_germplasm]['options']) {
      $germplasm_found = $field_prop[$fld_germplasm]['options'];
      
      // Indicate the number of germplasm matched given set of filters. Provide filter option to enter specific germplasm.
      $germplasm_text = '<strong>' . count($germplasm_found)  . ' germplasm</strong> found based on the filters above.';
      $germplasm_class = 'ap-field-enabled';
    } 
    else {
      // Nothing found.
      $germplasm_found = array();
      $germplasm_text = '';
      $germplasm_class = 'ap-field-disabled';
    }
    
    // Germplasm/Stock Name control bar.
    $form[$AJAX_wrapper][$ap_fieldset_2]['ap_filter_germplasm_bar'] = array(
      '#markup' => '<div id="ap-filter-germplasm-bar" class="' . $germplasm_class . '">' . $germplasm_text . '</div>',
    );
    
    // Hidden fields required.
    $form[$AJAX_wrapper][$ap_fieldset_2]['ap_field_hidden'] = array(
      '#prefix' => '<div id="ap-field-hidden">',
      '#suffix' => '</div>',
    ); 
      
      // A set of germplasm for a given filters.
      $form[$AJAX_wrapper][$ap_fieldset_2]['ap_field_hidden'][ $fldname[$fld_germplasm]['field_hidden'] ] = array(
        '#type' => 'hidden',
        '#value' => implode($delimiter, $germplasm_found),
        '#attributes' => array('id' => 'ap-germplasm-found-cache'),
      );
      
      // Store values enetered in each newly added germplasm field.
      $form[$AJAX_wrapper][$ap_fieldset_2]['ap_field_hidden']['ap_autofield_cache'] = array(
        '#type' => 'hidden',
        '#default_value' => '',
        '#attributes' => array('id' => 'ap-autofield-cache'),
      );


    // Add germplasm filter bar and list germplasm/stock name found.
    if (count($germplasm_found) > 0) {
      // Matching germplasm based on the filters above.
      if (isset($germplasm_found) && is_array($germplasm_found)) {
        $list = '';
      
        foreach($germplasm_found as $g) {
          $list .= '<li>' . $g . '</li>';
        }
      
        $form[$AJAX_wrapper][$ap_fieldset_2]['ap_filter_germplasm_found'] = array(
          '#markup' => '<div id="ap-filter-germplasm-found"><ul>' . $list . ' </ul></div> 
                        <div id="ap-filter-germplasm-reveal">
                          <div class="ap-filter-germplasm-reveal-on" title="Show/Hide ' . count($germplasm_found) . ' Germplasm/Stock Name">&nbsp;</div>
                        </div>',
        );
      }

      // Fields as filters.
      $form[$AJAX_wrapper][$ap_fieldset_2]['ap_filter_germplasm_by_field'] = array(
        '#prefix' => '<div id="ap-filter-germplasm-by-field">
                        Phenotype for Specific Germplasm<br />
                        <small>If you are interested in phenotypes for specific germplasm, you can add them individually by clicking add button 
                        . To retrive all germplasm based on your other filter criteria, proceed to the next filter.</small>',
        '#suffix' => '</div>',
      );

      // Autofield - Filter by specific germplasm name(s).
      $ap_autofield = 'ap_autofield';
    
      // Element wrapper, fields will be inserted to this container.
      $form[$AJAX_wrapper][$ap_fieldset_2]['ap_filter_germplasm_by_field'][$ap_autofield] = array(
        '#prefix' => '<div id="ap-autofield">',
        '#suffix' => '</div>',
      );
      
      // Store values selected.
      $ap_autofield_cache = '';
      if (isset($form_state['values']['ap_autofield_cache'])) {
        $ap_autofield_cache = $form_state['values']['ap_autofield_cache'];
      }
    
      $form[$AJAX_wrapper][$ap_fieldset_2]['ap_field_hidden']['ap_autofield_cache']['#default_value'] = $ap_autofield_cache;
      
      // Since fields are added through AJAX, form has to reconstruct fields using
      // the values in the cache.
      $single_field = 0;
      
      if (isset($form_state['values']) && !empty($ap_autofield_cache)) {
        $stock_names = explode('#', $ap_autofield_cache);
        
        if (count($stock_names) > 0) {
          foreach($stock_names as $name) {
            // Add autofield default to germplasm name.
            if (in_array($name, $germplasm_found)) {
              $form[$AJAX_wrapper][$ap_fieldset_2]['ap_filter_germplasm_by_field'][$ap_autofield][] 
                = analyzedphenotypes_autofieldprop($name);   
            }
            else {
              $single_field++;
            }
          }  
        }
        else {
          $single_field = 1;
        }
      }
      else {
        // Add one.
        $single_field = 1;
      }
      
      if ($single_field) {
        // Add one field.
        $form[$AJAX_wrapper][$ap_fieldset_2]['ap_filter_germplasm_by_field'][$ap_autofield][] 
          = analyzedphenotypes_autofieldprop();   
      }
    }
    ///////
    
    
    //
    // FIELD: MAX MISSING DATA.
    $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_missingdata]['field'] ] = array(
      '#title' => t('Maximum Allowed Missing Data'),
      '#type' => 'textfield',
      '#default_value' => $field_prop[$fld_missingdata]['default'],
      '#disabled'      => $field_prop[$fld_missingdata]['disabled'],
      '#size' => 10,
      
      '#description' => t('Enter the percent (%) missing data per germplasm that you would like to allow. For example, a value of 20% will ensure that all germplasm exported have values for at least 20% of site-years this trait was observed in. If you further restrict the site-year exported using other filter criteria, this filter will be applied to the restricted dataset.'),
      '#id' => $fldname[$fld_missingdata]['field-id'],
    );


  
  //
  //
  // Fieldset: #3. Choose your file.
  $ap_fieldset_3 = 'ap_file_type_fieldset';
  $form[$AJAX_wrapper][$ap_fieldset_3] = array(
    '#type' => 'fieldset',   
    '#title' => 'Choose your output file.',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  
    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_fieldset3_description'] = array(
      '#type' => 'item',
      '#description' => t('Select the file format, column headers and summary options you would like the data exported in below.'),
      '#id' => 'ap-fieldset3-description',
    );

    
    // FILE TYPE:
    $download_type = analyzedphenotypes_datafileprop('download_type');
    $form[$AJAX_wrapper][$ap_fieldset_3][ $fldname[$fld_filetype]['field'] ] = array(
      '#title' => t('File Type'),
      '#type' => 'select',
      '#disabled' => $field_prop[$fld_filetype]['disabled'],
      '#options'  => $download_type,
      '#description' => t('Select the format you would like the data exported.'),
      
      // Class to inidcate the field does not require AJAX.      
      '#attributes' => array('class' => array('ap-skip-loading')),
      '#id' => $fldname[$fld_filetype]['field-id'],
    );
    
    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_filetype_warning'] = array(
      '#type' => 'item',
      '#markup' => '<div>Please note that some germplasm names may have converted into dates in Microsoft Excel.</div>',
      '#id' => 'ap-filetype-warning',
    );


    //
    // INCLUDE ALL REPLICATES
    $form[$AJAX_wrapper][$ap_fieldset_3][ $fldname[$fld_averagerep]['field'] ] = array(
      '#title' => t('Average Replicates per Site-Year'),
      '#type' => 'checkbox',
      '#default_value' => 1,
      '#disabled' => $field_prop[$fld_averagerep]['disabled'],
      '#description' => t('By default, all replicates in a single site-year will be averaged. Please specify if you would like all replicates to be included. To export the value of each replicate, uncheck this option.'),
      '#id' => $fldname[$fld_averagerep]['field-id'],
    )
    + $AJAX_callback;

    //
    // RFRIENDLY HEADERS.
    $form[$AJAX_wrapper][$ap_fieldset_3][ $fldname[$fld_rfriendly]['field'] ] = array(
      '#title' => t('Make Column Headers R Friendly'),
      '#type' => 'checkbox',
      '#disabled' => $field_prop[$fld_rfriendly]['disabled'],
      '#prefix' => '<div id="ap-chk-r">',
      '#suffix' => '</div>',
      
      // Class to inidcate the field does not require AJAX.      
      '#attributes' => array('class' => array('ap-skip-loading')),
      '#id' => $fldname[$fld_rfriendly]['field-id'],
    );


    //
    // COLUMN HEADER PICKER.
    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_column_header_picker_container']['ap_table_title'] = array(
      '#type' => 'item',
      '#title' => 'Column Headers',
    );

    $header = array();
    $rows   = array();

    // HEADERS:

    // Default Headers.
    $i = 1;
    foreach($default_headers as $def) {
      $id = 'ap-default-row-' . $i;
      
      // Load the RFriendly version of headers. 
      $rfriendly = analyzedphenotypes_columnsprop('rfriendly', array(
        'column_header' => $def,
      ));
        
      $rfriendly = '<em>' . ucwords(str_replace(',.', ', ', $rfriendly))  . '</em>';

      $rows[] = array(
        'number'   => array('title' => t('Order #'), 'id' => $id . 'seqn', 'class' => array('ap-seq-no'), 'data' => '#' . $i),
        'column'   => array('title' => t('Click to move one level up...'), 'id' => $id . 'cols', 'class' => array('ap-column'), 'data' => '<input type="checkbox" checked="checked" disabled="disabled"> <small>(default)</small> &nbsp;- ' . $def . $rfriendly),
        'row-up'   => array('title' => t('Click to move one level up...'), 'id' => $id . 'goup', 'class' => array('ap-row-up'), 'data' => '&nbsp;'),
        'row-down' => array('title' => t('Click to move one level down...'), 'id' => $id . 'down', 'class' => array('ap-row-down'), 'data' => '&nbsp;')
      );
      
      $i++;
    }
    
    //
    // DEFAULT COLUMN HEADERS TABLE.
    $tbl1 = theme('table', array(
      'header' => $header, 
      'rows'   => $rows, 
      'attributes' => array('id' => 'ap-table-default-headers')
    ));

    // Preview Headers.
    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_column_header_picker_container']['ap_table_default_headers'] = array(
      '#markup' => $tbl1
        . l('Preview headers', '#', array('attributes' => array('id' => 'ap-preview-link')))
        . '<div class="ap-clear-float"></div>',
    );
    
    
    //
    // PREIVEW COLUMN HEADERS TABLE.
    $tbl3 = theme('table', array(
      'header' => array(), 
      'rows'   => array(), 
      'attributes' => array('id' => 'ap-table-preview-headers'),
      'sticky' => FALSE,
    ));
    
    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_column_header_picker_container']['ap_table_preview_headers'] = array(
      '#markup' => '<div id="ap-table-preview-headers-container">
        <br />This is a sample data to show a preview of the otput file based on the column headers chosen above.' 
        . $tbl3 . '<span>&nbsp;</span></div>',
    );
  
    //
    // OPTIONAL COLUMN HEADERS TABLE.
    $rows = array();
    $i = 1;
    
    foreach($optional_headers as $opt) {
      $id = 'ap-optional-row' . $i;
      
      $rfriendly = analyzedphenotypes_columnsprop('rfriendly', array(
        'column_header' => $opt,
      ));
        
      $rfriendly = '<em>' . $rfriendly  . '</em>';
      
      $rows[] = array(
        'number'   => array('title' => t('Check header to include/exclude in the output file.'), 'id' => $id . 'seqn', 'class' => array(), 'data' => '+'),
        'column'   => array('title' => t('Check header to include/exclude in the output file.'), 'id' => $id . 'cols', 'class' => array(), 'data' => '<input type="checkbox" id="' . $id  . 'checkbox" class="ap-optional-traits ap-skip-loading"> &nbsp;- ' . $opt . $rfriendly),
        'row-up'   => array('title' => t('Check header to include/exclude in the output file.'), 'id' => $id . 'goup', 'class' => array(), 'data' => '&nbsp;'),
        'row-down' => array('title' => t('Check header to include/exclude in the output file.'), 'id' => $id . 'down', 'class' => array(), 'data' => '&nbsp;')
      );
    
      $i++;
    }

    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_column_header_picker_container']['ap_optional_table_title'] = array(
      '#type' => 'item',
      '#title' => 'Optional Headers',
    );

    $tbl2 = theme('table', array(
      'header' => $header, 
      'rows'   => $rows, 
      'attributes' => array('id' => 'ap-table-optional-headers')
    ));

    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_column_header_picker_container']['ap_table_optional_headers'] = array(
      '#markup' => $tbl2,
    );

    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_column_header_picker_container']['ap_table_description'] = array(
      '#type' => 'item',
      '#description' => t('Check the column headers you want to include in the output file. Use the up and down arrow buttons to change the order they appear in the file. Click Preview headers to preview selected column headers.'),
    );
    
    
    //
    // STORE VALUES OF COLUMN HEADERS.
    if (isset($form_state['values'])) {
      $headers = $form_state['values'][ $fldname[$fld_columnheaders]['field'] ];
    }
    else {
      $headers = implode($delimiter, $default_headers);
    }
    
    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_column_header_picker_container'][ $fldname[$fld_columnheaders]['field'] ] = array(
     '#type' => 'hidden',
     '#default_value' => $headers,
     '#attributes' => array('id' => $fldname[$fld_columnheaders]['field-id']),
    );


  //
  // SUBMIT BUTTON.
  drupal_add_js(array('analyzedphenotypes' => array('btn' => $btn_ok)), array('type' => 'setting'));

  $form['ap_download_submit'] = array(
    '#type' => 'submit',
    '#value' => 'Download',
    '#weight' => 100,
    
    // Disabled by default.
    '#attributes' => array(
      'class' => array('form-button-disabled'),
      'disabled' => 'disabled',
    ),
    '#id' => 'ap-download-submit-field',
  );
  
  
  return $form;
}


/**
 * Implements hook_submit().
 */
function analyzedphenotypes_data_download_submit($form, &$form_state) {
  // Use this character to separate entries in a string.
  $delimiter = '#';
  $field_values = array();
  
  // Field names.
  $fldname = analyzedphenotypes_fieldnames();
  
  foreach($fldname as $f) {
    if (isset($form_state['values'][ $f['field'] ])) {
      $val = $form_state['values'][ $f['field'] ];
      
      if (is_array($val)) {
        // Field checkboxes.
        $v = analyzedphenotypes_checkboxesvalue($val);
        $entered_value = ($v) ? implode($delimiter, $v) : 'all';
      }
      else {
        $entered_value = trim(strip_tags($val));  
      }
      
      $field_values[ $f['base'] ] = $f['base'] . '=' . $entered_value;
    }
  }
  
  // Germplasm.
  $autofield = $form_state['values']['ap_autofield_cache'];
  $germplasm = (empty($autofield)) ? 'all' : $autofield;
  $field_values[ $fldname['germplasm']['base'] ] = $fldname['germplasm']['base'] . '=' . $germplasm;
  
  $querystring = implode('&', $field_values);
  $url = base64_encode($querystring);

  $form_state['redirect'] = array(
    '/download/analyzed_phenotypic_data',
    array(
      'query' => array('code' => $url),
    ),
  );
}


// HELPER FUNCTIONS:

/**
 * Function wrap a checkbox to a div and stlye it to look like a multi select box.
 * 
 * @param $height
 *   An integer, height value of the container element.
 * @param $has_options
 *   An integer, the number of options available.
 */
function analyzedphenotypes_wrapcheckbox($height, $has_options) {
  // Set field to disabled when checkbox has no options available.
  $class = ($has_options > 0) ? '' : ' ap-field-disabled';

  return array(
    '#prefix' => '<div class="ap-checkboxes-container' . $class . '" style="height: '. $height .'px">
                    <div class="ap-checkbox-wrapper">',
    '#suffix' => '  </div>
                  </div>',
  );
}


/**
 * Function get checked values from field checkboxes.
 * 
 * @param $value
 *   An array of values returned when check a checkbox.
 */
function analyzedphenotypes_checkboxesvalue($value) {
  $checkboxes_value = array();
  
  if ($value) {
    // Sort values selected first to take into account
    // the order options are selected/checked.
    asort($value);
  
    foreach($value as $v) {
      if ($v > 0 || $v != '') {
        $checkboxes_value[] = $v;
      }
    }
  }
  
  return $checkboxes_value;
}
