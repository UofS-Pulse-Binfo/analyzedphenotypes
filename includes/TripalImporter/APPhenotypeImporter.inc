<?php

/**
 *
 * @see TripalImporter
 */
class APPhenotypeImporter extends TripalImporter {

  /**
   * The name of this loader.  This name will be presented to the site
   * user.
   */
  public static $name = 'Phenotypic Data Importer';

  /**
   * The machine name for this loader. This name will be used to construct
   * the URL for the loader.
   */
  public static $machine_name = 'APPhenotypeImporter';

  /**
   * A brief description for this loader.  This description will be
   * presented to the site user.
   */
  public static $description = 'Loads phenotypic data into the system.';

  /**
   * An array containing the extensions of allowed file types.
   */
  public static $file_types = ['txt', 'tsv'];

  /**
   * If the loader should require an analysis record.  To maintain provenance
   * we should always indiate where the data we are uploading comes from.
   * The method that Tripal attempts to use for this by associating upload files
   * with an analysis record.  The analysis record provides the details for
   * how the file was created or obtained. Set this to FALSE if the loader
   * should not require an analysis when loading. if $use_analysis is set to
   * true then the form values will have an 'analysis_id' key in the $form_state
   * array on submitted forms.
   */
  public static $use_analysis = FALSE;

  /**
   * If the $use_analysis value is set above then this value indicates if the
   * analysis should be required.
   */
  public static $require_analysis = FALSE;

  /**
   * We remove the button in order to control the label on it.
   */
  public static $use_button = FALSE;

  /**
   * We need the file to not be required so there is no validation error on
   * non-file containing form pages.
   */
  public static $file_required = FALSE;

  /**
   * Provides information to the user about the file upload.  Typically this
   * may include a description of the file types allowed.
   */
  public static $upload_description = 'This should be a tab-separated file
    with the following columns: <ol>
      <li><strong>Trait Name:</strong> The full name of the trait as you would
        like it to appear on a trait page. This should not be abbreviated
        (e.g. Days till one open flower).</li>
      <li><strong>Method Name:</strong> A short (<4 words) name describing the
        method. This should uniquely identify the method while being very
        succinct (e.g. 10% Plot at R1).</li>
      <li><strong>Unit:</strong> The unit the trait was measured with. In the
        case of a scale this column should defined the scale. (e.g. days)</li>
      <li><strong>Germplasm Accession:</strong> The stock.uniquename for the
        germplasm whose phenotype was measured. (e.g. ID:1234)</li>
      <li><strong>Germplasm Name:</strong> The stock.name for the germplasm
        whose phenotype was measured. (e.g. Variety ABC)</li>
      <li><strong>Year:</strong> The 4-digit year in which the measurement was
        taken. (e.g. 2020)</li>
      <li><strong>Location:</strong> The full name of the location either using
        “location name, country” or GPS coordinates (e.g. Saskatoon, Canada)</li>
      <li><strong>Replicate:</strong> The number for the replicate the current
        measurement is in. (e.g. 3)</li>
      <li><strong>Value:</strong> The measured phenotypic value. (e.g. 34)</li>
      <li><strong>Data Collector:</strong> The name of the person or
        organization which measured the phenotype.</li>
    </ol>
    <p>NOTE: The order of the above columns is important and your
      file <strong>must include a header</strong>!</p>';

  /**
   * Indicates the methods that the file uploader will support.
   */
  public static $methods = [
    // Allow the user to upload a file to the server.
    'file_upload' => FALSE,
    // Allow the user to provide the path on the Tripal server for the file.
    'file_local' => FALSE,
    // Allow the user to provide a remote URL for the file.
    'file_remote' => FALSE,
  ];

  // --------------------------------------------------------------------------
  // FORM: Form-proper.
  // --------------------------------------------------------------------------

  /**
   * @see TripalImporter:form().
   */
  public function form($form, &$form_state) {

    // Determine the current stage.
    $all_stages = array(
      'upload',   // Stage 1 - Select project, genus and upload data file.
      'validate', // Stage 2 - Validate data.
      'describe', // Stage 3 - Fully describe all traits.
    );
    if (isset($form_state['values'])) {
      // If the submit was clicked, iterate the stage.
      if ($form_state['triggering_element']['#type'] == 'submit') {
        $form_state['values']['stage_id']++;
      }
      $stage_id = $form_state['values']['stage_id'];
      $current_stage = $all_stages[$stage_id];
    }
    else {
      $stage_id = 0;
      $current_stage = 'upload';
    }

    // Add in the status header and any general JS/CSS.
    $form = $this->form_header($form, $form_state, $current_stage);

    // Add in hidden form elements to save values from pervious steps.
    $form = $this->save_stage_values($form, $form_state, $stage_id);

    // Based on the current stage, return the correct form.
    switch ($current_stage) {
      case 'upload':
        $form = $this->form_stage1($form, $form_state);
        break;
      case 'validate':
        $form = $this->form_stage2($form, $form_state);
        break;
      case 'describe':
        $form = $this->form_stage3($form, $form_state);
        break;
    }

    // Next stage submit button.
    $btn_text = ($current_stage == 'describe') ? 'Save' : 'Next Step';

    $form['ap_next_step_button'] = array(
      '#type' => 'submit',
      '#value' => $btn_text,
      '#weight' => 100,
      '#id' => 'ap-next-stage-submit-field',
    );


    // @debug dpm($form, '$form in Importer::form');
    // @debug dpm($form_state, '$form_state in Importer::form');

    return $form;
  }

  /**
   * Provides a status header to orientate the user which lists all stages
   * and indicates the current one.
   */
  public function form_header($form, &$form_state, $current_stage) {

    // Include CSS and JavaScript.
    //---------------------------------
    $path_theme = drupal_get_path('module', 'analyzedphenotypes') . '/theme/';

    $form['#attached']['css'] = array();
    $form['#attached']['js']  = array();

    // Main stylesheet.
    $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.global.css';

    // Based on the current stage, include the correct JS/CSS.
    // Based on the current stage, return the correct form.
    switch ($current_stage) {
      case 'upload':
        $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.stage1.js';
        break;
      case 'validate':
        $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.progress.css';
        $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.progress.js';
        $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.stage2.js';
        drupal_add_js('misc/progress.js');
        break;
      case 'describe':
        $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.stage3.css';
        $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.stage3.js';
        drupal_add_library('system', 'ui.accordion');
        break;
    }

    // Disable Import File button whichever is the stage.
    drupal_add_js("jQuery(document).ready(function () {
      if (jQuery('input[value=\"Import File\"]')) {
        jQuery('input[value=\"Import File\"]').hide();
      }
    })", 'inline');

    // Warnings.
    //---------------------------------
    $form['warning'] = [
      '#type' => 'markup',
      '#prefix' => '<div class="messages warning ap-messages">',
      '#suffix' => '</div>',
      '#markup' => 'Phenotypic data should be <strong>filtered for outliers
        and mis-entries</strong> before being uploaded here. Do not upload
        data that should not be used in the final analysis for a scientific
        article. Furthermore, data should <strong>NOT be averaged across
        replicates or site-year.</strong>',
    ];

    $allow_new = FALSE;
    // Allow new terms to be upload. Warn admin that the module is set to not allow
    // new trait be uploaded.
    $sysvars = ap_get_variablenames(
      array('variablename' => 'varset'),
      array('set' => 'options')
    );

    foreach($sysvars as $i => $var) {
      $sysvar_options = variable_get($var);

      if ($i == 'ap_allownew' && $sysvar_options == 1) {
        $allow_new = TRUE;
      }
    }
    $form['allow_new'] = [
      '#type' => 'hidden',
      '#value' => $allow_new,
    ];
    if (!$allow_new) {
      $form['allow_new_warning'] = [
        '#type' => 'markup',
        '#prefix' => '<div class="messages warning ap-messages">',
        '#suffix' => '</div>',
        '#markup' => 'This module is set to NOT allow new traits to be added
          into the system. Please contact the administrator of this website
          before loading your analyzed phenotypes.',
      ];
    }

    // Stage Indicator/Title.
    //---------------------------------
    $all_stage = array(
      'Upload',   // Stage 1 - Select project, genus and upload data file.
      'Validate', // Stage 2 - Validate data.
      'Describe', // Stage 3 - Fully describe all traits.
    );
    // Determine which steps still need to be completed.
    $active_stage = ucfirst($current_stage);
    $stage_id = array_search($active_stage, $all_stage);

    // Save the current stage in the form.
    $form['stage'] = [
      '#type' => 'hidden',
      '#value' => $current_stage
    ];
    $form['stage_id'] = [
      '#type' => 'hidden',
      '#value' => $stage_id,
    ];

    foreach($all_stage as $current_stage_id => $stage) {
      // If this stage is before the current stage then it's complete.
      // Otherwise the user still needs to fill it out.
      $s = ($current_stage_id <= $stage_id) ? 'stage' : 'todo';
      $stage_indicators[$stage] = $s;
    }
    $stage_title = 'Stage ' . ($stage_id + 1) . ' of ' . count($all_stage) . ' - ' . $all_stage[$stage_id];

    // Stage title.
    $form['title'] = [
      '#type' => 'markup',
      '#markup' => '<h3>' . strtoupper($stage_title) . '</h3>',
      '#weight' => -100,
    ];

    // Now add in the header showing the various stages.
    $form['stage_indicator'] = [
      '#type' => 'markup',
      '#markup' => '',
      '#prefix' => '<div id="ap-stage-indicators-container">',
      '#suffix' => '</div>' . '<div class="ap-clear-both">&nbsp;</div>',
    ];
    $i = 1;
    foreach($stage_indicators as $stage => $status) {
      $class = ($status == 'todo') ? 'ap-progress-stage-todo' : '';
      $form['stage_indicator']['#markup'] .= '<div class="' . $class . ' ap-progress-stage" title="' . $stage . '">
        <span>&nbsp;</span>' . $i . '. ' . $stage . '<span>&nbsp;</span>
        </div>';
      $i++;
    }

    return $form;
  }

  /**
   * Provides the upload file portion of the form.
   *
   * @stage 1
   */
  public function form_stage1 ($form,  &$form_state) {
    $class = get_called_class();
    global $user;
    // Pull defaults from the path if they are available.
    $query = drupal_get_query_parameters();
    $default_project = $default_genus = NULL;
    if (isset($query['project'])) { $default_project = $query['project']; }
    if (isset($query['genus'])) { $default_genus = $query['genus']; }
    // @debug dpm($query, '$query in Importer::stage1');
    // Autocomplete search field: Experiment/project title.
    $form['project_name'] = array(
      '#title' => 'Experiment',
      '#type' => 'textfield',
      '#maxlength' => 200,
      '#size' => 100,
      '#required' => TRUE,
      '#default_value' => $default_project,
      // @see analyzedphenotypes_data_json() in functions.inc where $dataype = projects.
      '#autocomplete_path' => 'admin/tripal/extension/analyzedphenotypes/json/projects/experiment',
      '#description' => t('Type in the experiment or project title your data is specific to.'),
      '#id' => 'ap-project-select-field'
    );

    // All genus.
    $arr_all_genus = array(0 => '- Select -');
    // Fetch all genus that have been configured in settings page...
    $genusprop = ap_get_activegenus();

    if ($genusprop) {
      foreach($genusprop as $g) {
        $arr_all_genus[$g] = $g;
      }

      // Register source url to fetch project genus.
      drupal_add_js(array('analyzedphenotypes' => array('vars' => array(
        'project_genus' =>  base_path() . 'admin/tripal/extension/analyzedphenotypes/json/projectgenus/'
      ))), array('type' => 'setting'));
    }
    else {
      drupal_set_message(t('This module is <strong>NOT configured</strong> to import analyzed phenotypes.'), 'warning');

      $form['genus_not_configured_warning'] = array(
        '#markup' => tripal_set_message('Administrators, check to ensure that you have a genus configured to accept data. Please click '
          . l('Set Module Ontologies', $GLOBALS['base_url'] . '/admin/tripal/extension/analyzedphenotypes/setup')
          . ' to configure.', TRIPAL_INFO, array('return_html' => TRUE)),
      );
    }

    $genus_options = $arr_all_genus;
    // AJAX callback wrapper.
    $form['ap_AJAX_wrapper_autofillgenus'] = array(
      '#prefix' => '<div id="ap-AJAX-wrapper-autofillgenus">',
      '#suffix' => '</div>',
    );
    if (isset($form_state['values']['genus']) &&
       !empty($form_state['values']['genus'])) {
      $default_genus = $form_state['values']['genus'];
    }
    elseif ($default_genus === NULL) {
      $default_genus = array_keys($genus_options)[0];
    }

    // Select field: Genus.
    $form['ap_AJAX_wrapper_autofillgenus']['genus'] = array(
      '#type' => 'select',
      '#title' => t('Genus'),
      '#required' => TRUE,
      '#options' => $genus_options,
      '#default_value' => $default_genus,
      '#description' => t('Select Genus. When experiment or project has genus set, a value will be selected.'),
      '#id' => 'ap-genus-select-field',
    );

    // Now to add the file + description based on our class.
    $form['file'] = [
      '#type' => 'fieldset',
      '#title' => t($class::$upload_title),
      '#description' => t($class::$upload_description),
    ];

    $existing_files = tripal_get_user_uploads($user->uid, $class::$file_types);
    if (count($existing_files) > 0) {
      $fids = [0 => '--Select a file--'];
      foreach ($existing_files as $fid => $file) {
        $fids[$fid] = $file->filename . ' (' . tripal_format_bytes($file->filesize) . ') ';
      }
      $form['file']['file_upload_existing'] = [
        '#type' => 'select',
        '#title' => t('Existing Files'),
        '#description' => t('You may select a file that is already uploaded.'),
        '#options' => $fids,
      ];

      $form['file']['file_upload_existing']['#element_validate'][] = 'APPhenotypeImporter_file_validate';
    }

    $form['file']['file_upload'] = [
      '#type' => 'html5_file',
      '#title' => '',
      '#description' => 'Remember to click the "Upload" button below to send ' .
        'your file to the server.  This interface is capable of uploading very ' .
        'large files.  If you are disconnected you can return, reload the file and it ' .
        'will resume where it left off.  Once the file is uploaded the "Upload ' .
        'Progress" will indicate "Complete".  If the file is already present on the server ' .
        'then the status will quickly update to "Complete".',
      '#usage_type' => 'tripal_importer',
      '#usage_id' => 0,
      '#allowed_types' => $class::$file_types,
      '#cardinality' => $class::$cardinality,
    ];

    $form['file']['file_upload']['#element_validate'][] = 'APPhenotypeImporter_file_validate';

    $form['file']['file_local'] = [
      '#title' => t('Server path'),
      '#type' => 'textfield',
      '#maxlength' => 5120,
      '#description' => t('If the file is local to the Tripal server please provide the full path here.'),
    ];

    $form['file']['file_local']['#element_validate'][] = 'APPhenotypeImporter_file_validate';


    return $form;
  }

  /**
   * Provides the validate file portion of the form.
   *
   * @stage 2
   */
  public function form_stage2 ($form,  &$form_state) {

    // @debug dpm($form_state, '$form_state in Importer::form_stage2');
    $job_id = NULL;
    if (isset($form_state['values']['project_name']) &&
        isset($form_state['values']['genus'])   &&
        isset($form_state['values']['files'])) {

      $project_name  = $form_state['values']['project_name'];
      $project_genus = $form_state['values']['genus'];
      $data_file     = $form_state['values']['files'][0]['fid'];

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

    // Add in neccessary markup.
    $form['context_display'] = [
      '#type' => 'item',
      '#title' => 'Experiment (Genus)',
      '#markup' => $project_name . ' (' . $project_genus . ')',
    ];

    $form['validating_container'] = [
      '#type' => 'markup',
      '#markup' => '<div class="ap-tsv-file-form-element"><label>TSV Data File</label><span>Validating...</span></div>'
        . '<div id="ap-progress-container" class="ap-tsv-file-form-element">
            <div class="ap-navy-spinner">Validating data... Please wait.</div>
            <!-- Apply progress bar - disable display rule to use progress bar instead of spinner above !-->
            <div class="ap-progress-wrapper" style="display: none">
              <div class="progress-pane"></div>
            </div>
          </div>',
    ];
    // NOTE: The above markup element will be replaced with the validation
    // result once the job completes. This is achomplished through AJAX in
    // the stage2 js file.

    $form['back'] = [
      '#type' => 'markup',
      '#markup' => l(
        'Re-upload File',
        '/admin/tripal/loaders/APPhenotypeImporter',
        ['query' => ['genus' => $project_genus, 'project' => $project_name],
        'attributes' => ['class' => ['ap-option-link-file-reupload']]]
      ),
    ];

    return $form;
  }

  /**
   * Provides the describe traits portion of the form.
   *
   * @stage 3
   */
  public function form_stage3 ($form,  &$form_state) {

    $project_name = $form_state['values']['project_name'];
    $project_genus = $form_state['values']['genus'];
    $data_file = $form_state['values']['files'][0]['file_path'];

    // Retrieve list of traits from file.
    // First determine the column headers for trait/method/unit.
    $expected_columns = ap_create_columnheaders('columns_datatype');
    $trait_name = array_keys($expected_columns)[0];
    $method_name = array_keys($expected_columns)[1];
    $unit_name = array_keys($expected_columns)[2];
    // @debug dpm([$trait_name, $method_name, $unit_name], 'Header Names of Interest');
    unset($expected_columns);

    // Now determine the indicies in the current file based on the header.
    $header_raw =  `head -n 1 $data_file`;
    $header = array_map('trim', str_getcsv($header_raw, "\t"));
    // @debug dpm($header, 'header');
    // Column # where the trait name column is.
    $trait_name_index = array_search($trait_name, $header);
    $trait_cut_i = $trait_name_index + 1;
    // Column # where the method name column is.
    $method_name_index = array_search($method_name, $header);
    $method_cut_i = $method_name_index + 1;
    // Column # where the unit column is.
    $unit_name_index = array_search($unit_name, $header);
    $unit_cut_i = $unit_name_index + 1;
    // @debug dpm([$trait_name_index, $method_name_index, $unit_name_index], 'indicies');

    // Now grab the unique combination of trait/method/name from the file
    // and save it into an array.
    $traits_raw = `cut -f $trait_cut_i,$method_cut_i,$unit_cut_i $data_file | sort -u`;
    $traits = [];
    foreach (explode("\n", $traits_raw) as $trait_line) {
      $trait_combo = explode("\t", trim($trait_line));
      if ((sizeof($trait_combo) == 3) and ($trait_combo[0] != $trait_name)) {
        $traits[] = [
          'trait' => $trait_combo[0],
          'method' => $trait_combo[1],
          'unit' => $trait_combo[2],
        ];
      }
    }
    // @debug dpm($traits, 'traits');

    // Field id prefix.
    $main_fieldset = 'ap_trait_form_fieldset';

    // Header form elements.
    $form['msg'] = [
      '#type' => 'markup',
      '#markup' => '
        <div class="messages warning">
          Please fully describe the following traits before clicking the next step button.
        </div>',
    ];

    $form['traits'] = [
      '#prefix' => '<div id="ap-describe-main-form-container">
        <div id="accordion">',
      '#suffix' => '</div></div><br />',
      '#tree' => TRUE,
    ];

    // For each unique trait combination, add form elements to describe it.
    //.......................................................................
    foreach ($traits as $i => $trait_details) {
      $trait_name_value = implode('; ',$trait_details);

      // Strip all quotes and spaces from this point for both Trait Details.
      // # Detail.
      foreach($trait_details as $detail_key => $detail_value) {
        $trait_details[ $detail_key ] = trim(str_replace('"', '', $detail_value));
      }

      // Prepare describe fieldset.
      $traitprop = array(
        'cvterm_id'     => '',
        'term'          => $trait_name_value,
        'name'          => $trait_details['trait'],
        'definition'    => '',
        'method'        => $trait_details['method'],
        'unit'          => $trait_details['unit'],
        'scale'         => '',
        'crop_ontology' => '',
        'photos' => array(
          'ap_photo_1'  => '',
          'ap_photo_2'  => '',
        ),
      );

      // trait definition.
      $trait_select = ap_get_trait($project_genus, $trait_details['trait']);
      // trait ontology var passed to the form api.
      $trait_crop_ontology = null;

      if (!empty($trait_select)) {
        $traitprop['definition'] = $trait_select->definition;
        $traitprop['cvterm_id'] = $trait_select->cvterm_id;

        // Pre-existing trait, inspect if this trait has been set to
        // contain an crop ontology information.
        $trait_ontology = ap_get_cvterm(
          array('cvterm_id' => $trait_select->cvterm_id),
          array('dataset' => 'fullset')
        );

        if ($trait_details) {
          $trait_crop_ontology = $trait_ontology[0]['crop_ontology'];
        }

        unset($trait_ontology);
      }

      $form['traits'][$trait_name_value] = array(
        '#type' => 'markup',
        '#prefix' => '<h3>#' . ($i+1) . '. ' . $trait_name_value . '</h3>'
          . '<div id="ap-accordion-container' . $i . '" class="ap-form-describe-trait">',
        '#suffix'  => '</div>',
      );

      $disabled = ($traitprop['cvterm_id']) ? TRUE : FALSE;
      $class = ($disabled) ? '' : 'ap-required';

      // Strip all quotes and spaces from this point for both Trait Property.
      // # Property.
      foreach($traitprop as $property_key => $property_value) {
        if ($property_key != 'photos') {
          $traitprop[ $property_key ] = trim(str_replace('"', '', $property_value));
        }
      }

      $form['traits'][$trait_name_value]['ap_group_about_the_trait'] = array(
        '#type' => 'item',
        '#title' => t('&#9660; ABOUT THE TRAIT'),
      );

      // -- Trait name
      $form['traits'][$trait_name_value]['trait_name'] = array(
        '#type' => 'textfield',
        '#title' => t('Trait Name'),
        '#disabled' => TRUE,
        '#default_value' => trim($trait_details['trait']),
        '#description' => t('A Concise human-readable label for the trait. This should not include abbreviations.'),
        '#id' => 'ap-name-text-field' . $i,
      );

      $form['traits'][$trait_name_value]['trait_description'] = array(
        '#type' => 'textarea',
        '#title' => t('Trait Description'),
        '#disabled' => $disabled,
        '#default_value' => $traitprop['definition'],
        '#description' => t('Describe the trait. For example, for "Days to Flower", the description might be "The number of days from emergence of the plant until it flowers". <strong> This should not include the collection method or units.</strong>'),
        '#attributes' => array('class' => array($class)),
        '#id' => 'ap-description-text-field' . $i,
      );

      // -- Method.
      // Save the method name from the file.
      $form['traits'][$trait_name_value]['method_name'] = array(
        '#type' => 'textfield',
        '#title' => 'Collection Method Name',
        '#disabled' => TRUE,
        '#default_value' => trim($trait_details['method']),
        '#description' => 'The method name is set based on your file.',
      );

      // If the method already exists then supply the full details here.
      $method = ap_get_method($project_genus, trim($trait_details['method']));
      $traitprop['method'] = (is_object($method)) ? $method->definition : '';
      $form['traits'][$trait_name_value]['method_description'] = array(
        '#type' => 'textarea',
        '#title' => t('Collection Method Description'),
        '#disabled' => (empty($traitprop['method'])) ? FALSE : TRUE,
        '#default_value' => $traitprop['method'],
        '#description' => t('Describe how you collected the data. Ensure you include all information needed for an unskilled labourer to reproduce your work.'),
        '#attributes' => array('class' => array($class)),
        '#id' => 'ap-method-text-field' . $i,
      );

      // -- Unit.
      $form['traits'][$trait_name_value]['unit'] = array(
        '#type' => 'textfield',
        '#title' => t('Unit'),
        '#disabled' => TRUE,
        '#default_value' => trim($trait_details['unit']),
        '#description' => t('The <strong>full name</strong> of the unit used to measure this trait. For scales, please enter "Scale [description of scale members]" (e.g. "scale 1-5, where 1 is upright and 5 is fully lodged", "scale; R:red, P:pink, W:white")'),
        '#autocomplete_path' => 'admin/tripal/extension/analyzedphenotypes/json/traitunit/units',
        '#attributes' => array('class' => array('ap-allow-revert')),
        '#id' => 'ap-unit-text-field' . $i,
      );

      // -- Crop ontology.
      // When genus ontology set:
      //   Use the words in the trait name and attempt to match term(s) in the ontology system variable.
      //   When no terms matched, let user search for terms in all ontology terms.
      // When genus ontology not set:
      //  This field should not be available.
      $sysvar = ap_get_variablenames(
        array('variablename' => $project_genus),
        array('set' => 'cvdbon', 'suffix' => 'ontology')
      );
      $sysvar_genus_ontology = variable_get($sysvar);

      // Construct ontologies suggestions:
      // Var to hold ontology term suggestions.
      $list = '';

      if ($sysvar_genus_ontology) {
        // Ontology set.

        if ($disabled) {
          // Field is disabled to load fetch ontology value for a trait
          // selected from suggested traits.

          // Additional check to see if there is a value at all.
          if (empty($traitprop['crop_ontology'])) {
            // Not set, fieldset should be hidden.
            $fieldset_disable = TRUE;
          }
          else {
            // A value in there, show the field but disabled.
            $fieldset_disable = FALSE;
            $ontology_disable = TRUE;
          }
        }
        else {
          // Regular page load with properly set ontoloy configuration
          // Begin suggestions.

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

          $fieldset_disable = FALSE;
        }
      }
      else {
        // Not set, make ontology fieldset unavailable.
        $fieldset_disable = TRUE;
      }

      $traitprop['crop_ontology'] = $trait_crop_ontology ?? '';
      $ontology_description = ($trait_crop_ontology)
        ? ''
        : t('Type in a term name or click one from suggestions below. The term choosen should be generic but as similar as possible to your trait.');

      if (!$fieldset_disable || $traitprop['crop_ontology']) {
        // Field group title:
        $form['traits'][$trait_name_value]['ap_group_mapping_information'] = array(
          '#type' => 'item',
          '#title' => t('&#9660; RELATED ONTOLOGY TERMS'),
        );

        $form['traits'][$trait_name_value]['ap_crop_ontology_text_field'] = array(
          '#title' => 'Crop Ontology',
          '#type' => 'textfield',
          '#disabled' => (isset($ontology_disable)) ? $ontology_disable : $disabled,
          '#default_value' => $traitprop['crop_ontology'],
          '#description' => $ontology_description,
          '#autocomplete_path' => 'admin/tripal/extension/analyzedphenotypes/json/ontology/' . $project_genus,
          '#attributes' => array('class' => array('ap-crop-ontology', 'ap-no-required')),
          '#id' => drupal_html_id('ap-crop-ontology-text-field' . $i),
        );

        // Suggest ontology.
        if (isset($list) && !$disabled) {
          $form['traits'][$trait_name_value]['ap_ontology_suggestion_markup_field'] = array(
            '#markup' => $list . '<br />',
          );
        }
      }

      // -- Photo.
      // When fields are disabled it is therefore using a pre-existing trait.
      // Fetch image set for this trait. Allow description to be relevant when image is
      // available or user is asked to upload one.
      if ($disabled) {
        $traitprop['photos'] = ap_download_cvtermphoto($traitprop['cvterm_id']);
        $photo_description = '';
      }
      else {
        // Trait photo set is the one set above - which is an empty pair.
        $photo_description = '<br />If possible, please upload pictures of your trait. For example, for plant height, you could take a picture of the plant with the ruler, and for disease rating you could take a picture of the various levels of disease.';
      }

      $form['traits'][$trait_name_value]['ap_group_photo_upload'] = array(
        '#type' => 'item',
        '#title' => t('&#9660; PHOTO UPLOAD'),
        '#description' => $photo_description,
      );

      $g = 1;
      $no_photo_count = 0;

      foreach($traitprop['photos'] as $seq => $photo_info) {
        if ($photo_info) {
          $scale = ap_scale_photo($photo_info['height'], $photo_info['width'], 300);

          $form['traits'][$trait_name_value][$seq] = array(
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
            $form['traits'][$trait_name_value][$seq . '_file_field' . $i] = array(
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


      // While showing a pre-existing trait:
      // Inform user no image was uploaded. They can't upload image at this point.
      if ($disabled && $no_photo_count == count($traitprop['photos'])) {
        $form['traits'][$trait_name_value]['ap_not_available'] = array(
          '#markup' => 'Photo not available.',
        );
      }

      // -- Standard Deviation table.
      $form['traits'][$trait_name_value]['ap_group_summary_table' . $i] = array(
        '#type' => 'item',
        '#title' => t('&#9660; SUMMARY TABLE'),
        '#description' => '<br />The following table summarizes your data, broken down by site year. Please take this opprotunity to <strong>ensure your data has been filtered for outliers</strong>.',
      );

      $form['traits'][$trait_name_value]['ap_summary_table' . $i] = array(
        '#type' => 'markup',
      );

      $summary_data = ap_create_summarytable($data_file, $header, $traitprop['term']);

      if ($summary_data) {
        $siteyear = $summary_data['siteyear'];
        $syvalues = $summary_data['values'];

        $table_rows = array();

        foreach($siteyear as $sy) {
          $max = max($syvalues[$sy]);
          $min = min($syvalues[$sy]);
          $avg = array_sum($syvalues[$sy]) / count($syvalues[$sy]);
          $sdv = ap_calculate_standard_deviation($syvalues[$sy]);

          if (!is_numeric($syvalues[$sy][0]) OR preg_match('/[Ss]cale/',$traitprop['unit'])) {
            $avg = 'n/a';
            $sdv = 'n/a';
          }
          else {
            $avg = round($avg, 2);
            $sdv = round($sdv, 2);
          }

          $table_rows[] = array($sy, $min, $max, $avg, $sdv);
        }

        $form['traits'][$trait_name_value]['ap_summary_table' . $i]['#markup'] = theme('table',
          array(
            'header' => array('Site-year', 'Minimum Value', 'Maximum Value', 'Mean Value', 'Standard Deviation'),
            'rows' => $table_rows,
            'attributes' => array('id' => 'ap_summary_table' . $i)
          )
        );
      }
    }

    // Hidden field: Genus.
    // Make this information available to hooks (ie validate).
    $form['ap_genus_select_field'] = array(
      '#type' => 'hidden',
      '#value' => $project_genus,
      '#id' => 'ap-genus-select-field',
    );

    // Support for file/image.
    $form['#attributes']['enctype'] = 'multipart/form-data';
    return $form;
  }

  /**
   * This function will ensure values from other steps are saved through to
   * the last step. This allows our TripalImporter::run() to act as through
   * it's a single page form!
   */
  public function save_stage_values($form, &$form_state, $stage_id) {

    //...........................................
    // STAGE 1:
    // project_name.
    if (isset($form_state['values']['project_name'])) {
      $form['project_name'] = [
        '#type' => 'hidden',
        '#value' => $form_state['values']['project_name'],
      ];
    }
    // genus.
    if (isset($form_state['values']['genus'])) {
      $form['genus'] = [
        '#type' => 'hidden',
        '#value' => $form_state['values']['genus'],
      ];
    }
    if (isset($form_state['values']['allow_new'])) {
      $form['allow_new'] = [
        '#type' => 'hidden',
        '#value' => $form_state['values']['allow_new'],
      ];
    }

    // File through server path.
    if (isset($form_state['values']['file_local']) && !empty($form_state['values']['file_local'])) {
      // An array containing the file id and file path.
      // @see APPhenotypeImporter_file_validate().
      $local_file = $form_state['values']['file_local'];

      $form['files'] = [
        '#tree' => TRUE,
      ];

      $form_state['values']['files'][0]['fid'] = $local_file['fid'];
      $form['files'][0] = [
        'file_path' => [
          '#type' => 'hidden',
          '#value' => $local_file['path'],
        ],
        'fid' => [
          '#type' => 'hidden',
          '#value' => $local_file['fid'],
        ]
      ];
    }
    // fid.
    // If this is the second stage...
    elseif (isset($form_state['values']['file_upload'])) {
      // Then we want to reformat the results and save them for future use.
      // The fid will be in either file_upload or file_upload_existing for
      // if the user uploaded a file or picked an existing one.
      $fid = !empty($form_state['values']['file_upload']) ? $form_state['values']['file_upload'] : $form_state['values']['file_upload_existing'];
      // @debug dpm($fid, '$fid in Importer::save_stage_values');

      $form['files'] = [
        '#tree' => TRUE,
      ];

      // Handle multiple file uploads.
      if (preg_match('/\|/', $fid)) {
        $fids = explode('|', $fid);
        foreach ($fids as $k => $id) {
          $file = file_load($id);
          $form_state['values']['files'][$k]['fid'] = $id;
          $form['files'][] = [
            'file_path' => [
              '#type' => 'hidden',
              '#value' => drupal_realpath($file->uri),
            ],
            'fid' => [
              '#type' => 'hidden',
              '#value' => $id,
            ]
          ];
        }
      }
      // Handle a single file.
      else {
        $file = file_load($fid);
        // @debug dpm($file, '$file in Importer::save_stage_values');
        $form_state['values']['files'][0]['fid'] = $fid;
        $form['files'][] = [
          'file_path' => [
            '#type' => 'hidden',
            '#value' => drupal_realpath($file->uri),
          ],
          'fid' => [
            '#type' => 'hidden',
            '#value' => $fid,
          ]
        ];
      }
    }
    // Otherwise we have already formatted the results and just need
    // to continue to pass them through...
    elseif (isset($form_state['values']['files'])) {

      $form['files'] = [
        '#tree' => TRUE,
      ];
      foreach($form_state['values']['files'] as $k => $f) {
        $form['files'][$k] = [
          'file_path' => [
            '#type' => 'hidden',
            '#value' => $f['file_path'],
          ],
          'fid' => [
            '#type' => 'hidden',
            '#value' => $f['fid'],
          ]
        ];
      }
    }

    //...........................................
    // STAGE 2:

    return $form;
  }

  // --------------------------------------------------------------------------
  // FORM: Form validation.
  // --------------------------------------------------------------------------

  /**
   * @see TripalImporter:formValidate().
   */
  public function formValidate($form, &$form_state) {

    // @debug dpm($form_state, '$form_state in Importer::validate.');
    // @debug dpm($form, 'form');

    // @todo why does the theme change when a validation error is encountered.
    //  Check when the CSS/JS is applied.

    $stage_id = $form_state['values']['stage_id'];

    // FILE.
    if ($stage_id === 0) {
      // Validate that the genus is set in Stage 1.
      if ($form_state['values']['genus'] == '0') {
        form_set_error('genus', 'You must set the genus for which the data is associated.');
      }

      if (empty($form_state['values']['file_upload'])) {
        // Main file upload field - Check the next field, existing file field.
        if (empty($form_state['values']['file_upload_existing'])) {
          // Secondary file upload field - Check next field, server path field.
          if (empty($form_state['values']['file_local'])) {
            // Finally, none provided.
            form_set_error('file', 'You must provide a file.');
          }
          else {
            $form_state['values']['fid'] = $form_state['values']['file_local']['fid'];
          }
        }
        else {
          $form_state['values']['fid'] = $form_state['values']['file_upload_existing'];
        }
      }
      else {
        $form_state['values']['fid'] = $form_state['values']['file_upload'];
      }
    }

    // DESCRIBE.
    if ($stage_id === 2) {
      // Set error to False and prove otherwise.
      $error = FALSE;

      // Check required fields for each trait.
      $required_fields = [
          'trait_name' => 'Trait Name',
          'trait_description' => 'Trait Description',
          'method_name' => 'Collection Method Name',
          'method_description' => 'Collection Method Description',
          'unit' => 'Unit',
        ];

      // Image index.
      $file_index = 0;
      foreach ($form_state['values']['traits'] as $trait_name_value => $values) {
        foreach ($required_fields as $req_field_name => $field_title) {
          if (empty($values[$req_field_name])) {
            $error = TRUE;
            form_set_error("traits][$trait_name_value][$req_field_name", "The $field_title for $trait_name_value is required.");
          }
        }

        // Check that all traits are disabled (i.e. already existed).
        if ($form_state['values']['allow_new'] === FALSE) {
          if ($form['traits'][$trait_name_value]['trait_description']['#disabled'] === FALSE) {
            $error = TRUE;
            form_set_error("traits][$trait_name_value][trait_name",
              t("The trait (:trait) must already exist before loading data due to
              configuration. Please contact an administrator.",
              [':trait' => $trait_name_value]));
          }
        }

        // Check crop ontology.
        if (isset($form_state['values']['traits'][ $trait_name_value ]['ap_crop_ontology_text_field'])) {
          $field_value = $form_state['values']['traits'][ $trait_name_value ]['ap_crop_ontology_text_field'];

          if (!empty($field_value)) {
            $project_genus = $form_state['values']['ap_genus_select_field'];

            // Check if a trait ontology matched a record.
            $exists = ap_match_ontology($project_genus, $field_value);

            if (!$exists) {
              $error = TRUE;
              form_set_error("traits][$trait_name_value][ap_crop_ontology_text_field", t('Then Crop ontology for ' . $trait_name_value . ' is not recognized. Please enter or select crop ontology.'));
            }
          }
        }

        // File photo.
        // Image 1.
        $field_name = 'ap_photo_1' . $file_index;
        if (isset($_FILES['files']['name'][$field_name])) {
          $field_value = $_FILES['files']['name'][$field_name];
          if (!empty($field_value)) {
             $file_extension = pathinfo($field_value, PATHINFO_EXTENSION);

             if ($file_extension != 'jpg') {
               $error = TRUE;
               form_set_error("traits][$trait_name_value][ap_photo_1_file_field$file_index",
                 t('Photo #1 for ' . $trait_name_value . ', is not a .jpg file. Please provide a .jpg photo.'));
             }
          }
        }

        // Image 2.
        $field_name = 'ap_photo_2' . $file_index;
        if (isset($_FILES['files']['name'][$field_name])) {
          $field_value = $_FILES['files']['name'][$field_name];
          if (!empty($field_value)) {
             $file_extension = pathinfo($field_value, PATHINFO_EXTENSION);

             if ($file_extension != 'jpg') {
               $error = TRUE;
               form_set_error("traits][$trait_name_value][ap_photo_2_file_field$file_index",
                 t('Photo #2 for ' . $trait_name_value . ', is not a .jpg file. Please provide a .jpg photo.'));
             }
          }
        }

        $file_index++;
      }

      if ($error) {
        // On error the accordion breaks, this will reinstantiate the library.
        drupal_add_library('system', 'ui.accordion');
      }
    }
  }

  // --------------------------------------------------------------------------
  // FORM: Form submit.
  // --------------------------------------------------------------------------

  /**
   * @see TripalImporter:formSubmit().
   */
  public function formSubmit($form, &$form_state) {

    $stage_id = $form_state['values']['stage_id'];

    if ($stage_id < 3) {
      $form_state['rebuild'] = TRUE;
    }

    if ($stage_id == 2) {
      $form_state['rebuild'] = FALSE;

      // Save the project genus.
      $project_name = $form_state['values']['project_name'];
      $project_genus = $form_state['values']['genus'];

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

      // These arrays will save the trait details for use in the importer::run.
      $arr_traitname_cvtermid = [];
      $arr_method_cvtermid = [];
      $arr_unit_cvtermid = [];

      // System variables: Genus ontology.
      // Fetch genus ontology settings for this project genus.
      $sysvar = ap_get_variablenames(
        array('variablename' => $project_genus),
        array('set' => 'cvdbon', 'suffix' => 'ontology')
      );

      $sysvar_genus_ontology = variable_get($sysvar);


      // We want to save the trait.
      // Photo index. Each trait has photo field, this counter assigns a
      // unique sequencial number to each field.
      $file_index = 0;
      foreach ($form_state['values']['traits'] as $k => $trait_details) {

        // Determine whether this trait is qualitative or quantitative.
        $type = 'quant';
        if (preg_match('/[Ss]cale/', $trait_details['unit'])) {
          $type = 'qual';
        }

        // Insert the trait.
        $result = ap_insert_trait([
          'genus' => $form_state['values']['genus'],
          'name' => $trait_details['trait_name'],
          'description' => $trait_details['trait_description'],
          'method_title' => $trait_details['method_name'],
          'method' => $trait_details['method_description'],
          'unit' => $trait_details['unit'],
          'type' => $type,
        ]);

        // Save the trait details for use in importer::run().
        $form_state['values']['traits'][$k]['trait_id'] = $result['trait']->cvterm_id;
        $form_state['values']['traits'][$k]['method_id'] = $result['method']->cvterm_id;
        $form_state['values']['traits'][$k]['unit_id'] = $result['unit']->cvterm_id;

        if ($result) {
          if ($sysvar_genus_ontology > 0 && isset($form_state['values']['traits'][$k]['ap_crop_ontology_text_field'])) {
            // Create plant ontology relationship only when genus ontology (on)
            // is set in congiguration file.
            // Unit. Scale. Ontology.
            $ontology_value = $form_state['values']['traits'][$k]['ap_crop_ontology_text_field'];

            if (!empty($ontology_value)) {
              // Fetch cvterm/ontology term limit search in genus configuration of project genus.
              $ontologyprop = ap_get_cvterm(
                array('name' => $ontology_value, array('genus' => $project_genus)),
                array('dataset' => 'cvtermid')
              );

              if ($ontologyprop) {
                // Create relationship.
                // Set the ontology for term to crop ontology selected.
                // Replace existing ontology-trait relationship set to FALSE.
                ap_set_ontology(
                  array('trait_cvterm_id' => $result['trait']->cvterm_id, 'ontology_cvterm_id' => $ontologyprop[0]['cvterm_id']),
                  array('replace' => FALSE)
                );
              }
            }
          }

          // Upload photo's.
          // Photo.
          $photo_1 = 'ap_photo_1' . $file_index;
          $photo_2 = 'ap_photo_2' . $file_index;

          // Save cvterm image.
          ap_upload_cvtermphoto(
            array('cvterm_id' => $result['trait']->cvterm_id, 'photo' => array($photo_1, $photo_2)),
            array('scaleto' => 400)
          );

          $file_index++;
        }

        // Save the ontology term.
        // @todo save ontology term

      }
    }
  }

  // --------------------------------------------------------------------------
  // FORM: Import Data.
  // --------------------------------------------------------------------------

  /**
   * @see TripalImporter::run()
   */
  public function run() {

    // All values provided by the user in the Importer's form widgets are
    // made available to us here by the Class' arguments member variable.
    $arguments = $this->arguments['run_args'];

    // We are using a separate function to load the file. It requires the fid.
    $fid = $arguments['files'][0]['fid'];

    // Look up the project_id.
    $projectprop = ap_match_projectname(
      array('name' => $arguments['project_name']),
      array('fullmatch' => TRUE, 'limitrows' => 1)
    );
    $project_id = ($projectprop) ? $projectprop['project_id'] : 0;

    // The function also requires the trat/method/unit IDs.
    $trait_ids = array();
    $method_ids = array();
    $unit_ids = array();

    // Inspect the composite key if it exists, ignore and move on to next in line
    // otherwise, insert into collections table. Needs only one to satisfy the condition.
    $sql_collection = "SELECT collection_id FROM analyzedphenotypes_collections
      WHERE (trait_id, method_id, unit_id, project_id) = (:trait_id, :method_id, :unit_id, :project_id)
      LIMIT 1";

    foreach ($arguments['traits'] as $result) {
      $trait_ids[ $result['trait_name'] ] = $result['trait_id'];
      $method_ids[ $result['method_name'] ] = $result['method_id'];
      $unit_ids[ $result['unit'] ] = $result['unit_id'];

      // Determine the default chart type based on unit name.
      $chart_type = 'violin';
      if (preg_match('([Ss]cale|\d+\s*-\s*\d+)', $result['method_name'])) {
        $chart_type = 'multibar';
      }

      $result['project_id'] = $result['project_id'] ?? $project_id;

      // Fetch key.
      $key_exists = chado_query($sql_collection, array(
        ':trait_id'   => $result['trait_id'],
        ':method_id'  => $result['method_id'],
        ':unit_id'    => $result['unit_id'],
        ':project_id' => $result['project_id']))
      ->rowCount();

      // Only if key combination does not exist, we add a row.
      if (!$key_exists || $key_exists <= 0) {
        // Add the data collection with default chart type.
        db_insert('analyzedphenotypes_collections')->fields([
          'genus' => $arguments['genus'],
          'project_id' => $project_id,
          'trait_id' => $result['trait_id'],
          'method_id' => $result['method_id'],
          'unit_id' => $result['unit_id'],
          'chart_type' => $chart_type,
        ])->execute();
      }
    }

    $dataset = [
      'project_name' => $arguments['project_name'],
      'project_genus' => $arguments['genus'],
      'trait_cvterm' => $trait_ids,
      'method_cvterm' => $method_ids,
      'unit_cvterm' => $unit_ids,
      'data_file' => $fid,
    ];


    // Now pass the dataset off to the original loading function.
    $job_id = $this->job->getJobID();
    analyzedphenotypes_save_tsv_data(serialize($dataset), $job_id);
  }
}


/**
 * Custom validation for the file_upload element in stage 1.
 */
function APPhenotypeImporter_file_validate($element, &$form_state, $form) {

  // If existing
  if ($element['#type'] == 'select') {
    $element_key = 'file][file_upload_existing';
    $fid = $form_state['values']['file_upload_existing'];
  }
  // If uploaded.
  elseif ($element['#type'] == 'html5_file') {
    $element_key = 'file][file_upload';
    $fid = $form_state['values']['file_upload'];
  }
  // If local server file.
  elseif ($element['#type'] == 'textfield') {
    $element_key = 'file][file_local';

    if (file_exists($form_state['values']['file_local'])) {
      global $user;

      // Field value which is the local path to file.
      $file_name = basename($form_state['values']['file_local']);
      // We want to move the server file to user directory to serve
      // as a backup copy of the original file and reference this file in succeeding stages.
      $f = fopen($form_state['values']['file_local'], 'r');
      $destination_file = 'public://' . 'tripal/users/' . $user->uid . '/' . $file_name;
      $file_copy = file_save_data($f, $destination_file, FILE_EXISTS_RENAME);
      fclose($f);

      $fid = $file_copy->fid;

      // Since the the value in the field is not file object but a text value we need
      // transfer the file in the file system designed for this module while generating
      // a file id in the process.
      $form_state['values']['file_local'] = array(
        'fid' => $fid,
        // The new file, local in users folder not the original file.
        'path' => drupal_realpath($file_copy->uri),
      );
    }
    else {
      $fid = null;
    }
  }

  if (!empty($fid)) {
    $file = file_load($fid);
    $project_name =  trim($form_state['values']['project_name']);
    $project_genus = trim($form_state['values']['genus']);

    // Validation is at file level.
    // Begin validation process.
    $status = analyzedphenotypes_validate_tsv_file(array(
      'project_name'  => trim($project_name),
      'project_genus' => trim($project_genus),
      'data_file' => $file,
    ));

    $all_results = array_values($status);
    $error = FALSE;
    foreach($all_results as $result) {
      if ($result['type'] == 'failed') {
        form_set_error($element_key, 'The specified file could not be uploaded. See Validation Result for more information about the error.');
        $error = TRUE;
      }
    }

    if ($error) {
      drupal_set_message(ap_report_validationresult('file', $status), 'ap-result-panel');
      return FALSE;
    }
    else {
      drupal_set_message('Your file uploaded successfully. Please click "Next Step" to continue.');
      return TRUE;
    }
  }
}
