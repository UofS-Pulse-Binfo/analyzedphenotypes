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
  // Fetch all genus that have been configured in settings page.
  $genusprop = ap_get_activegenus();

  if ($genusprop) {
    foreach($genusprop as $g) {
      $arr_all_genus[$g] = $g;
    }
  }

  // Project genus - autofill.
  if (isset($form_state['values']['ap_project_select_field'])) {
    $project = trim($form_state['values']['ap_project_select_field']);

    if (!empty($project)) {
      // Fetch genus assigned to a project.
      $project_genus = ap_get_projectgenus(
        array('project_name' => $project)
      );

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
           // Try to match the trait name with existing name and suggest to user.
           // Limit the search to genus configuration of genus selected. 
           // Cvterm Id passed on to callback.
           $similar_cvterm = ap_get_cvterm(
             array('keyword' => $name, 'genus' => $project_genus),
             array('dataset' => 'cvtermidname')
           );
           
           $similar_cvterm = array_values($similar_cvterm);
           
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

           // From cvterm suggestion above, load and set fields default to term properties.
           if (isset($form_state['values']['ap_similar_trait_select_field' . $i])) {
             $select_trait = (int)$form_state['values']['ap_similar_trait_select_field' . $i];

             if ($select_trait >= 0) {
               if ($select_trait > 0) {
                 $selected_cvterm = $form_state['values']['ap_similar_trait_select_field' . $i];

                 // Fetch cvterm and all properties to be used as default value in fields.
                 $exists = ap_get_cvterm(
                   array('cvterm_id' => $selected_cvterm),
                   array('dataset' => 'fullset')
                 );

                 if ($exists) {
                   $trait_name_value = $exists[0]['name'];
                   $name = analyzedphenotypes_traitprop('no_unit', array(
                     'trait_name' => $trait_name_value,
                   ));
                   
                   // Term properties.
                   $traitprop['name']          = $name;
                   $traitprop['cvterm_id']     = $exists[0]['cvterm_id'];
                   $traitprop['definition']    = $exists[0]['definition'];
                   $traitprop['scale']         = '';
                   $traitprop['crop_ontology'] = $exists[0]['crop_ontology'];
                   // Term photo.  
                   $traitprop['photos'] = ap_download_cvtermphoto($exists[0]['cvterm_id']);
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
             // When genus ontology set:
             //   Use the words in the trait name and attempt to match term(s) in the ontology system variable.
             //   When no terms matched, let user search for terms in all ontology terms.
             // When genus ontology not set:
             //  This field should not be available.
             if (!$disabled) {
               // When field is enabled. Disabled might mean the field is used to load previously added record.
               // Fetch ontology settings for this project genus.
               $sysvar = ap_get_variablenames(
                 array('variablename' => $project_genus),
                 array('set' => 'cvdbon', 'suffix' => 'ontology')
               );

               $sysvar_genus_ontology = variable_get($sysvar);

               if ($sysvar_genus_ontology) {
                 // Attempt suggest term.
                 $ontology_suggestions = ap_suggest_ontology(
                   array('trait' => $traitprop['name'], 'genus' => $project_genus),
                   array('limitrows' => 10)
                 );

                 if ($ontology_suggestions) {
                   // Found term to suggest.
                   $a = array();

                   foreach($ontology_suggestions as $k => $term) {
                     $definition = $term['name'];

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
                 }

                 // Else.
                 // Nope. Let user find the term but limit within the ontology system variable.
               }
               else {
                 // Disable the field.
                 $ontology_disable = TRUE;
               }
             }

             $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_crop_ontology_text_field' . $i] = array(
               '#title' => 'Crop Ontology',
               '#type' => 'textfield',
               '#disabled' => (isset($ontology_disable)) ? $ontology_disable : $disabled,
               '#default_value' => $traitprop['crop_ontology'],
               '#description' => t('Type in crop ontology or click one from suggestions below.'),
               '#autocomplete_path' => 'admin/tripal/extension/analyzedphenotypes/json/ontology/' . $sysvar_genus_ontology,
               '#attributes' => array('class' => array('ap-crop-ontology', $class)),
               '#id' => drupal_html_id('ap-crop-ontology-text-field' . $i),
             );

             // Suggest ontology.
             if (isset($list) && !$disabled) {
               $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_ontology_suggestion_markup_field' . $i] = array(
                 '#markup' => $list,
               );
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
                     '#description' => t('JPG file type only.'),
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

    // System variables: Genus ontology.
    // Fetch genus ontology settings for this project genus.
    $sysvar = ap_get_variablenames(
      array('variablename' => $project_genus),
      array('set' => 'cvdbon', 'suffix' => 'ontology')
    );
    
    $sysvar_genus_ontology = variable_get($sysvar);

    foreach($traits as $i => $trait) {
      // Check the select similar trait value.
      if (isset($form_state['values']['ap_similar_trait_select_field' . $i]) &&
          $form_state['values']['ap_similar_trait_select_field' . $i] > 0) {

        $cvterm_id = $form_state['values']['ap_similar_trait_select_field' . $i];
      }
      else {
        $description = $form_state['values']['ap_description_text_field' . $i];

        // Insert new term. Inserted term id is required.
        $cvterm = ap_insert_cvterm(
          array(
            'name' => $trait,
            'definition' => $description,
            'genus' => $project_genus,
          ),
          array('return_insert_id' => TRUE)
        );

        if ($cvterm) {
          $cvterm_id = $cvterm;

          if ($sysvar_genus_ontology > 0) {
            // Create plant ontology relationship only when genus ontology (on)
            // is set in congiguration file.
            // Unit.
            // Scale.
            // Ontology.
            $ontology_value = $form_state['values']['ap_crop_ontology_text_field' . $i];

            if (!empty($ontology_value)) {
              // Fetch cvterm/ontology term limit search in genus configuration of project genus.
              $ontologyprop = ap_get_cvterm(
                array('name' => $ontology_value, 'genus' => $sysvar_genus_ontology),
                array('dataset' => 'cvtermid')
              );

              if ($ontologyprop) {
                // Create relationship.
                // Set the ontology for term to crop ontology selected.
                // Replace existing ontology-trait relationship set to FALSE.
                ap_set_ontology(
                  array('trait_cvterm_id' => $cvterm_id, 'ontology_cvterm_id' => $ontologyprop[0]['cvterm_id']),
                  array('replace' => FALSE)
                );
              }
            }
          }

          // Photo.
          $photo_1 = 'ap_photo_1' . $i;
          $photo_2 = 'ap_photo_2' . $i;
          
          // Save cvterm image.
          ap_upload_cvtermphoto(
            array('cvterm_id' => $cvterm_id, 'photo' => array($photo_1, $photo_2)),
            array('scaleto' => 400)
          ); 
        }
      }

      $arr_traitname_cvtermid[$trait] = $cvterm_id;
    }

    // Save genus and project.
    // Does project have a genus?
    $projectprop = ap_get_projectgenus(
      array('project_name' => $project_name)
    );
    
    if (!$projectprop) {
      // No previous genus set for the project, create one.
      // Match (full match) a project name.
      $projectprop = ap_match_projectname(
        array('name' => $project_name),
        array('fullmatch' => TRUE, 'limitrows' => 1)
      );

      // Set new genus to project.
      ap_set_projectgenus(
        array('project_id' => $projectprop['project_id'], 'genus' => $project_genus),
        array('replace' => FALSE)
      );
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
    $field_value = $form_state['values'][$field_name];
    if (!empty($field_value)) {
      $project_genus = $form_state['values']['ap_genus_select_field'];
      
      // Check if a trait ontology matched a record.
      $exists = ap_match_ontology($project_genus, $field_value);
      
      if (!$exists) {
        $error = TRUE;
        form_set_error($field_name, t('Crop ontology entered is not recognized. Please enter or select crop ontology.'));
      }
    }

    // File.
    // @TODO: this image field should be optinal in any case.
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

  // Fetch spreadsheet/ directory.
  $directories = ap_construct_datafile_destinationpath();

  $text = implode(', ', $text) . ' only';
  $source_id = str_replace('_', '-', $source_name);

  // Drag and Drop:
  $form[$source_name] = array(
    '#type' => 'dragndrop_upload',
    '#title' => t('TSV Data File'),
    '#file_upload_max_size' => '10M',
    '#upload_location' => $directories['spreadsheet'],
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
  $genus = ap_get_genus();

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
  $cvs = ap_get_cv();
  $cv_options = array_map(function($c) {
    return $c['name'];
  }, $cvs);

  $field_default = 0;
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
  // Fetch terms configuration used by AP.
  $term_vars = ap_get_variablenames(
    array('variablename' => 'varset'),
    array('set' => 'terms')
  );

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
      
      // Fetch the cvterm name (cv name) of a cvterm id number.
      $default_field = ap_get_cvterm(
        array('cvterm_id' => $var_id),
        array('dataset' => 'namecvname_format')
      );

      $default_value = ($default_field) ? reset($default_field) : '';

      // Otherwise a autocomplete field.
      $form[$frameset_cv][ $prop['field_name'] ] = array(
        '#type' => 'textfield',
        '#title' => t($term),
        '#description' => t($prop['description']),
        '#default_value' => $default_value['namecvname'],
        '#attributes' => array('class' => array('ap-autocomplete-field-terms'),
          'title' => array($default_value['namecvname'])),
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
  $cvdbon = ap_get_variablenames(
    array('variablename' => 'varset'),
    array('set' => 'cvdbon')
  );

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
  // All term configuration used by AP.
  $terms = ap_get_variablenames(
    array('variablename' => 'varset'),
    array('set' => 'terms')
  );

  // Test if value entered (field for terms are autocomplete).
  foreach($terms as $term => $t) {
    $field_term = $form_state['values'][$t];
    $var_key = str_replace('ap_', '', $term);

    if (empty($field_term)) {
      // Field is empty. Mark field.
      form_set_error($t, 'You must set the controlled vocabulary term for ' . $var_key . '.');
    }
    else {
      // Term entered a combination of cvterm name and cv name.
      // Checking to see if it matched a record.
      $is_term = ap_get_cvterm(
        array('cvtermcv' => $field_term)
      );

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
  // All system variables, groupped by options, terms and cvdbon.
  $vars = ap_construct_variablenames();
  
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

        $fld_value_id = ap_get_cvterm(
          array('cvtermcv' => $fld_value)
        );

        // Term entered a combination of cvterm name and cv name.
        // Checking to see if it matcheed a record before saving it to a system var.
        variable_set($v, $fld_value_id[0]['cvterm_id']);
      }
      
      // Update materialized view to account for terms location and year.
      ap_update_mview_phenotypesummary();
    }
  }

  drupal_set_message('Configuration saved.', 'status');
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

  drupal_add_library('system', 'ui.autocomplete');

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
  $delimiter = '~';

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
            'genus' => $field_values[$fld_genus],
            'species' => $field_values[$fld_species],
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
                    'genus' => $field_values[$fld_genus],
                    'species' => $field_values[$fld_species],
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

                      // Many traits, use all year. Otherwise, use what user checked.
                      if (count($field_values[$fld_traits]) > 1) {
                        $field_values[$fld_year] = $field_values[$fld_year];
                      }
                      else {
                        $field_values[$fld_year] = analyzedphenotypes_checkboxesvalue($y);
                      }

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

                            // Many year, use all location. Otherwise, use what user checked.
                            if (count($field_values[$fld_year]) > 1) {
                              $field_values[$fld_location] = $field_values[$fld_location];
                            }
                            else {
                              $field_values[$fld_location] = analyzedphenotypes_checkboxesvalue($l);
                            }

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

                                  // Many year, use all location. Otherwise, use what user checked.
                                  if (count($field_values[$fld_location]) > 1) {
                                    $field_values[$fld_germplasmtype] = $field_values[$fld_germplasmtype];
                                  }
                                  else {
                                    $field_values[$fld_germplasmtype] = analyzedphenotypes_checkboxesvalue($g);
                                  }
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
            else {
              // Reset trait.
              $reset_traits = 1;
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
        recommended</strong> to restrict the dataset to a specific experiment. This can be done by clicking on the name of
        the experiment below. You can further filter by year and location if desired.'),
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

    $form[$AJAX_wrapper]['reset_traits'] = array(
      '#type' => 'hidden',
      '#value' => (isset($reset_traits)) ? 1 : 0,
      '#attributes' => array('id' => 'ap-reset-traits'),
    );
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

      $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_year]['field'] ]['#attributes']['class'][] = 'ap-no-mouse-event';
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

      $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_location]['field'] ]['#attributes']['class'][] = 'ap-no-mouse-event';
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

      $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_germplasmtype]['field'] ]['#attributes']['class'][] = 'ap-no-mouse-event';
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

    // Count germplasm names found.
    $germplasm_count = (isset($germplasm_found)) ? count($germplasm_found) : 0;

    if ($germplasm_count > 0) {
      // Define AJAX element wrapper.
      $field_container = 'ap_AJAX_germplasmname_field_wrapper';
      
      $form[$AJAX_wrapper][$ap_fieldset_2][$field_container] = array(
        '#prefix' => '<div id="ap-AJAX-germplasmname-field-wrapper">',
        '#suffix' => '</div>',
        // Nest elements inserted, this important or else 
        // everything will be wobbly.
        '#tree' => TRUE,
      );
      
      $form[$AJAX_wrapper][$ap_fieldset_2][$field_container][ $fldname[$fld_germplasm]['field'] ] = array(
        '#type' => 'checkboxes',
        '#options' => drupal_map_assoc($germplasm_found),
        '#ajax' => array(
          'callback' => 'ap_AJAX_germplasmfield_callback',
          'command' => 'germplasm_add_name',
          'wrapper' => 'ap-AJAX-germplasmname-field-wrapper',
          'progress' => array('type' => 'none'),
        ),
        '#prefix' => '<div id="ap-filter-germplasm-found">',
        '#suffix' => '</div>',
        //
        '#id' => 'ap-field-germplasm-names',
      );

      $form[$AJAX_wrapper][$ap_fieldset_2][$field_container]['title'] = array(
        '#markup' => '<div id="ap-filter-germplasm-reveal">
          <div class="ap-filter-germplasm-reveal-on" title="Show/Hide ' . count($germplasm_found) . ' Germplasm/Stock Name">&nbsp;</div>
          </div>
          
          <div id="ap-filter-germplasm-by-field">
            Phenotype for Specific Germplasm<br />
            <small>If you are interested in phenotypes for specific germplasm, you can add them individually by clicking add button or germplasm names.
            To retrive all germplasm based on your other filter criteria, proceed to the next filter.</small>
          </div>',
      );

      // Field names.
      $field = array(
        'field_hidden_values' => 'ap_field_hidden_values',
        'field_textfield'     => 'ap_field_textfield_', // suffix unique id.
        'field_button_add'    => 'ap_field_button_add',
        'field_button_rem'    => 'ap_field_button_rem_', // suffix unique id.
      );
      
      
      // Field array.
      $field_textfield = array(
        '#type' => 'textfield',
        '#title' => 'Type Germplasm Name/Stock Name',
        '#title_display' => 'invisible',
        '#size' => 65,
        '#ajax' => array(
          'event' => 'autocompletechange',
          'callback' => 'ap_AJAX_germplasmfield_callback',
          'command' => 'germplasm_update_element',
          'wrapper' => 'ap-AJAX-germplasmname-field-wrapper',
          'progress' => array('type' => 'none'),
        ),
        '#attributes' => array('placeholder' => 'Type Germplasm Name/Stock Name', 'class' => array('ap-textfield-germplasmname')),
      );
      
      $field_button_rem = array(
        '#type' => 'button',
        '#value' => '',
        '#ajax' => array(
          'callback' => 'ap_AJAX_germplasmfield_callback',
          'command' => 'germplasm_remove_element',
          'wrapper' => 'ap-AJAX-germplasmname-field-wrapper',
          'progress' => array('type' => 'none'),
        ),
        '#attributes' => array('class' => array('remove-btn')),
      );
    
      // Each field inserted should contain a form of unique identifiers.
      $germplasm_field_id = uniqid();
      // Pass on to this variable each time a new field is added.
      $new_element = null;
      
      // Load values array (names pre-existing) by either buttons or 
      // start wit a single empty field on initial page load.
      $values_array = (isset($form_state['triggering_element']))
        ? $form_state['values'][ $field['field_hidden_values']  ]
        : array($germplasm_field_id => '');

      // Append the one and only add button.
      $form[$AJAX_wrapper][$ap_fieldset_2][$field_container][ $field['field_button_add'] ] = array(
        '#type' => 'button',
        '#value' => '+',
        '#ajax' => array(
          'callback' => 'ap_AJAX_germplasmfield_callback',
          'command' => 'germplasm_add_blank',
          'wrapper' => 'ap-AJAX-germplasmname-field-wrapper',
          'progress' => array('type' => 'none'),
        ),
        '#attributes' => array('class' => array('add-btn')),
      );
      
      // Execute append action only when field count is less than the number
      // of germplasm and/or if added by link, germplasm is not previously added.
      
      if (isset($form_state['triggering_element'])) {
        // Action required and value.
        $triggering_element = array(
          'command' => $form_state['triggering_element']['#ajax']['command'],
          'value' => $form_state['triggering_element']['#value'],
        );
       
        // Execute command.
        // Save error - invalid/not found germplasm name.
        $error = array();
        
        // Flag if form has been submitted for the first time.
        // This will indicate the stage to add the first set of elements.
        $form['ap_form_submitted'] = array(
          '#type' => 'hidden',
          '#value' => isset($form_state['values']['ap_form_submitted']) ? TRUE : FALSE,
        );
        
        if ($triggering_element['command'] == 'germplasm_add_blank') {
          // ADD - add a blank field.
          // To add, check values array and inspect the number of empty
          // fields, if it exceeded 1, then don't allow any more blank fields.
          // Field is default to: Type Germplasm Name/Stock Name.
          $blanks = 0;
          
          foreach($values_array as $v) {
            if (empty($v)) {
              $blanks++;
              if ($blanks > 1) break;
            }
          }
          
          if ($blanks < 1) {
            // Can't add more than 1 empty field.
            $values_array[$germplasm_field_id] = '';
          }
          
          // See JS for autocomplete support in fields.
        }
        elseif ($triggering_element['command'] == 'germplasm_add_name') {
          // ADD - add field by checking a germplasm name.
          // To add, see if action required is to include or exclude a name.
          
          // Checking or uncheck-ing.
          if ($triggering_element['value'] === 0) {
            // To uncheck - Find the value (stock name) of the triggering element
            // to extract the unique id number then use it to reset the value in that key.
            $fid = array_search($form_state['triggering_element']['#return_value'], $values_array);
            unset($values_array[$fid]);
          }
          else {
            // Tick off.
            // Add a field and set the value to matching germplasm name.
            $values_array[$germplasm_field_id] = $triggering_element['value'];
          }
        }
        elseif ($triggering_element['command'] == 'germplasm_update_element') {
          // ADD/REM - interacting with text field.
          // To add/rem, see if action required is to add or remove.
          $id =  $form_state['triggering_element']['#id'];
          $id_value = $form_state['input'][$field_container][ $field['field_textfield'] . $id ];
          
          if ($id_value) {
            // User typed in a germplasm name.
            // Double check that value entered matches a name in germplasm names.
            if (in_array($id_value, $germplasm_found)) {
              $values_array[$id] = $id_value;
            }
            else {
              // Bad name, does not exists.
              $error[$id] = '<small class="ap-germplasmfield-error">*The germplasm name/stock name you entered is invalid or does not exists.</small>';
            }
          }
          else {
            // User hit del/backspace key to remove germplasm name.
            // Cache the value removed, since this will be the only record of the
            // value and unchecking of name requres a reference to this value.
            $form['ap_cache_germplasmname'] = array(
              '#type' => 'hidden',
              '#value' => $values_array[$id],
            );
            
            // With value cached, we can leave out this item.
            $values_array[$id] = '';
          }
        }
        elseif ($triggering_element['command'] == 'germplasm_remove_element') {
          // REMOVE - hit x / remove button ajacent to the field.
          // Remove field.
          unset($values_array[ $triggering_element['value'] ]);
          
          // Uncheck matching germplasm name.
          // see callback.
        }
        else {
          // ADD - the first set of element that gets added.
          // A blank field and add button.
          if ($form_state['values']['ap_form_submitted'] == FALSE) {
            $values_array[$germplasm_field_id] = '';
            $form['ap_form_submitted']['#value'] = TRUE;
          }
        }
      }

      // Render values array.
      foreach($values_array as $key => $value) {
        // Text field.
        $field_textfield['#value'] = $value;
        $field_textfield['#id'] = $key;
        $field_textfield['#suffix'] = (isset($error[$key])) ? $error[$key] : '';
        $form[$AJAX_wrapper][$ap_fieldset_2][$field_container][ $field['field_textfield'] . $key ] = $field_textfield;
        
        // Remove button.
        if (count($values_array) > 1) {
          $field_button_rem['#value'] = $key;
          $form[$AJAX_wrapper][$ap_fieldset_2][$field_container][ $field['field_button_rem'] . $key ] = $field_button_rem;
        }
      }

      // Record keeping.
      $form[ $field['field_hidden_values'] ] = array(
        '#type' => 'hidden',
        '#value' => $values_array,
      );
    
      // Construct suggestion array - in autocomplete field by excluding 
      // name that had been checked.
      $suggest_germplasmname = $germplasm_found;
      foreach($values_array as $element_id => $element_value) {
        if (!empty($element_value)) {
          // A germplasm name in this field, remove it from suggestions.
          $o = array_search($element_value, $suggest_germplasmname);
          unset($suggest_germplasmname[$o]);
        }
      } 

      drupal_add_js(array('analyzedphenotypes' => array('suggest' => array_values($suggest_germplasmname))), array('type' => 'setting'));
    // End if germplasm_count.  
    }
    
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
    $form[$AJAX_wrapper][$ap_fieldset_3][ $fldname[$fld_averagerep ]['field'] ] = array(
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

      $rfriendly = ucwords(str_replace(',.', ', ', $rfriendly));

      $rows[] = array(
        'number'   => array('title' => t('Order #'), 'id' => $id . 'seqn', 'class' => array('ap-seq-no'), 'data' => '#' . $i),
        'column'   => array('title' => t('Click to move one level up...'), 'id' => $id . 'cols', 'class' => array('ap-column'), 'data' => '<input type="checkbox" checked="checked" disabled="disabled"> <small>(default)</small> &nbsp;-  <span>' . $def . '</span><em>' . $rfriendly . '</em>'),
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
        'column'   => array('title' => t('Check header to include/exclude in the output file.'), 'id' => $id . 'cols', 'class' => array('ap-column'), 'data' => '<input type="checkbox" id="' . $id  . 'checkbox" class="ap-optional-traits ap-skip-loading"> &nbsp;- <span>' . $opt . '</span><em>' . $rfriendly . '</em>'),
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
  $delimiter = '~';
  $field_values = array();

  // Field names.
  $fldname = analyzedphenotypes_fieldnames();

  foreach($fldname as $f) {
    if (isset($form_state['values'][ $f['field'] ]) || 
      isset($form_state['values']['ap_AJAX_germplasmname_field_wrapper'][ $f['field'] ])) {
      // NOTE: germplasm name had #tree option - check value with this option on.
      
      $val = (isset($form_state['values'][ $f['field'] ])) 
        ? $form_state['values'][ $f['field'] ]
        : $form_state['values']['ap_AJAX_germplasmname_field_wrapper'][ $f['field'] ];

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
  
  // Prepare query string.  
  $querystring = implode('&', $field_values);
  $url = base64_encode($querystring);

  $form_state['redirect'] = array(
    '/phenotype/download/analyzed_phenotypic_data',
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
