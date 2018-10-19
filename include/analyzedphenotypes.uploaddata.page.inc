<?php
/**
 * @file
 * Contains administrative interfaces of this module.
 */



// Ontology.
module_load_include('inc', 'analyzedphenotypes', 'api/analyzedphenotypes.ontology.api');


/**
 * Implements hook_form().
 * Construct data uploader stages.
 */
function analyzedphenotypes_uploaddata_form($form, &$form_state) {
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
        $job_id = ap_get_tripaljob($projectprop['job_id']);

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
      $form = analyzedphenotypes_uploaddata_form_upload($form, $form_state);

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
      $form = analyzedphenotypes_uploaddata_form_validates($form, $form_state, $projectprop);

      break;

    //
    case 'describe':
      $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.stage3.css';
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.stage3.js';

      drupal_add_library('system', 'ui.accordion');

      // # 3.
      $form = analyzedphenotypes_uploaddata_form_describe($form, $form_state);

      break;

    //
    case 'save':
      $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.progress.css';
      $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.progress.js';

      drupal_add_js('misc/progress.js');

      // # 4.
      $form = analyzedphenotypes_uploaddata_form_save($form, $form_state);

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
function analyzedphenotypes_uploaddata_form_upload($form, &$form_state) {
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
      'callback' => 'ap_send_ajaxrequest',
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
function analyzedphenotypes_uploaddata_form_validates($form, &$form_state, $projectprop = null) {
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

    $job_id = ap_create_tripaljob($tripal_dataset);
  }
  elseif ($projectprop) {
    $project_name  = $projectprop['project_name'];
    $project_genus = $projectprop['project_genus'];
    $data_file     = $projectprop['data_file'];

    $job = ap_get_tripaljob($projectprop['job_id']);

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
function analyzedphenotypes_uploaddata_form_describe($form, &$form_state) {
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
    $file = drupal_realpath($data_file->uri);
    $handle = fopen($file, 'r');

    // Open the file and read the trait column header.
    // For each trait, the user has to describe before proceeding.

    if ($handle) {
      // Extract the headers column and extract the trait name index that contain
      // all traits to be described.
      $columns_row = fgets($handle);

      $expected_columns = ap_create_columnheaders('columns_datatype');
      $trait_name = array_keys($expected_columns)[0];

      unset($expected_columns);

      // Find the trait nane column from the file.
      $c = str_getcsv($columns_row, "\t");
      $columns = array_map('trim', $c);
      // Column # where the trait name column is.
      $trait_name_index = array_search($trait_name, $columns);

      // Line counter.
      $i = 0;

      // Array to hold each trait to describe.
      $arr_trait_describe = array();
      // Field id prefix.
      $main_fieldset = 'ap_trait_form_fieldset';

      while(!feof($handle)) {
        /// Each line.
        $row = fgets($handle);

        // Data row.
        if (!empty($row)) {
          // Individual cell.
          $value = str_getcsv($row, "\t");
          // Trait name value in the trait column.
          $trait_name_value = $value[ $trait_name_index ];

          if (in_array($trait_name_value, $arr_trait_describe)) {
            // Check if current trait name in array of traits to describe
            // and prevent it from being described more than once.
            continue;
          }
          else {
            // Prepare describe fieldset.
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

            // Prepare form fields.
            $ap_fieldset = $main_fieldset . $i;
            $arr_trait_describe[] = $trait_name_value;

            // Match cvterm key.
            // Try to match the trait name with existing name and suggest to user.
            // Limit the search to genus configuration of genus selected.
            // Cvterm Id passed on to callback.
            $name = ap_format_traitname($trait_name_value, 'no_unit');
            $traitprop['name'] = $name;

            $similar_cvterm = ap_get_cvterm(
              array('keyword' => $name, 'genus' => $project_genus),
              array('dataset' => 'cvtermidname')
            );

            // Suggest similar term to user.
            if (count($similar_cvterm) > 0) {

              $cvterm_options = array();
              // Create the options.
              foreach($similar_cvterm as $n => $cvterm) {
                $cvterm_options[ $cvterm['cvterm_id'] ] = $cvterm['name'];
              }

              // Select field to let user select matching traits to be reused.
              $form[$ap_fieldset]['ap_similar_trait_select_field' . $i] = array(
                '#type' => 'select',
                '#title' => t('Did you mean?'),
                '#options' => array(-1 => '- Select -', 0 => 'None of these apply') + $cvterm_options,

                '#ajax' => array(
                  'event' => 'change',
                  'callback' => 'ap_send_ajaxrequest',
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

              // Got some suggestions, no describe form until a trait has been selected.
              $describeform_show = 'no';
            }
            else {
              // Load describe form since no match found.
              $describeform_show = 'yes';
            }

            // User has picked a trait from suggestions.
            // With cvterm suggestion above, load and set fields default to term properties.
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
                    $name = ap_format_traitname($trait_name_value, 'no_unit');

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

                // Choice made, load the form with trait properties default values of fields.
                $describeform_show = 'yes';
              }
              else {
                // Select a trait opion (default) selected, no form.
                $describeform_show = 'no';
              }
            }

            // AJAX wrapper - to load prefilled describe form.
            $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i] = array(
              '#prefix' => '<div id="ap-AJAX-wrapper-describeform' . $i . '">',
              '#suffix' => '</div>',
            );

            // Describe form required.
            if ($describeform_show == 'yes') {
              ////
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

              // Text field: Unit.
              $unit = ap_format_traitname($trait_name_value, 'trait_unit');
              $traitprop['unit'] = $unit;

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

              // Default to not found.
              $sysvar_genus_ontology = 0;

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

                    foreach($ontology_suggestions as $k => $t) {
                      $definition = $t['name'];

                      $a[] = l($t['name'], '#', array('attributes' => array(
                        'alt' => $definition,
                        'title' => $definition,
                      )));
                    }

                    if (count($a) > 0) {
                      $list = theme('item_list', array(
                        'items' => $a,
                        'title' => '',
                        'attributes' => array('id' => 'ap-ontology-suggest-' . $i, 'class' => 'ap-item-list'),
                      ));
                    }
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
                '#autocomplete_path' => 'admin/tripal/extension/analyzedphenotypes/json/ontology/' . $project_genus,
                '#attributes' => array('class' => array('ap-crop-ontology', ap-no-required)),
                '#id' => drupal_html_id('ap-crop-ontology-text-field' . $i),
              );

              // Suggest ontology.
              if (isset($list) && !$disabled) {
                $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_ontology_suggestion_markup_field' . $i] = array(
                  '#markup' => $list . '<br />',
                );
              }

              // Field group title:
              // Photo.
              $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_group_photo_upload' . $i] = array(
                '#type' => 'item',
                '#title' => t('&#9660; PHOTO UPLOAD'),
              );

              $g = 1;
              $no_photo_count = 0;
              foreach($traitprop['photos'] as $seq => $photo_info) {
                if ($photo_info) {
                  $scale = ap_scale_photo($photo_info['height'], $photo_info['width'], 300);

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
                      '#attributes' => ($g == 1) ? array('class' => array('ap-no-required')) : array(),
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
              // Standard Deviation table.
              $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_group_summary_table' . $i] = array(
                '#type' => 'item',
                '#title' => t('&#9660; SUMMARY TABLE'),
              );

              $form[$ap_fieldset]['ap_AJAX_wrapper_describeform' . $i]['ap_summary_table' . $i] = array(
                '#type' => 'markup',
              );

              $summary_data = ap_create_summarytable($file, $columns, $traitprop['term']);

              if ($summary_data) {
                $siteyear = $summary_data['siteyear'];
                $syvalues = $summary_data['values'];

                $table_rows = array();

                foreach($siteyear as $sy) {
                  $max = max($syvalues[$sy]);
                  $min = min($syvalues[$sy]);
                  $avg = array_sum($syvalues[$sy]) / count($syvalues[$sy]);
                  $sdv = ap_calculate_standard_deviation($syvalues[$sy]);

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
              ////
            }
            // end prepare describe fieldset.
          }
        }

        // Next line.
        $i++;
        ///
      }

      fclose($handle);

      // Some values generated in this stage are required in the next stage.
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
function analyzedphenotypes_uploaddata_form_save($form, &$form_state) {
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
          array('return_inserted_id' => TRUE)
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
                array('name' => $ontology_value, 'genus' => $project_genus),
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

    // Create a new Tripal Job.
    $job_id = ap_create_tripaljob($tripal_dataset, array('return_jobid' => TRUE));

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


/**
 * Implements hook_file_validate().
 * Perform file level (is a valid file, project selected, etc.) validation.
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

      drupal_set_message(ap_report_validationresult('file', $status), 'ap-result-panel');

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
 * Perform data level (unit/value match, germplasm exists, etc.) validation.
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

      // Create a new Tripal Job.
      $job_id = ap_create_tripaljob($tripal_dataset);

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
 * Implements custom field validation.
 * Validate select trait form field.
 */
function analyzedphenotypes_validate_field_select_trait(&$element, &$form, &$form_state) {
  if ($element['#value'] == "") {
    drupal_add_library('system', 'ui.accordion');

    form_set_error($element['#name'], t('Please select an option.'));
  }
}


/**
 * Implements custom field validation.
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
    if (isset($form_state['values'][$field_name])) {
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
    }

    // File.
    // Image 1.
    $field_name = str_replace('ap_description_text_field', 'ap_photo_1', $element['#name']);
    $field_value = $_FILES['files']['name'][$field_name];
    if (!empty($field_value)) {
       $file_extension = pathinfo($field_value, PATHINFO_EXTENSION);

       if ($file_extension != 'jpg') {
         $error = TRUE;
         form_set_error($field_name, t('Photo 1 of 2 is not a .jpg file. Please provide a .jpg photo.'));
       }
    }

    // Image 2.
    $field_name = str_replace('ap_description_text_field', 'ap_photo_2', $element['#name']);
    $field_value = $_FILES['files']['name'][$field_name];
    if (!empty($field_value)) {
       $file_extension = pathinfo($field_value, PATHINFO_EXTENSION);

       if ($file_extension != 'jpg') {
         $error = TRUE;
         form_set_error($field_name, t('Photo 2 of 2 is not a .jpg file. Please provide a .jpg photo.'));
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
function analyzedphenotypes_uploaddata_form_submit($form, &$form_state) {
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
 * Helper function:
 * Construct Drag and Drop file dropzone area.
 */
function analyzedphenotypes_dragdrop($form, &$form_state, $source_name) {
  $file_extensions = ap_define_datafile_extensions('file_upload');

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
