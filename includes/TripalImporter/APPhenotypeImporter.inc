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
      'save',     // Stage 4 - Save data and file.
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
      case 'save':
        $form = $this->form_stage4($form, $form_state);
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
        //$form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.stage1.js';
        break;
      case 'validate':
        $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.progress.css';
        $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.progress.js';
        $form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.stage2.js';
        drupal_add_js('misc/progress.js');
        break;
      case 'describe':
        //$form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.stage3.css';
        //$form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.stage3.js';
        //drupal_add_library('system', 'ui.accordion');
        break;
      case 'save':
        //$form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.loader.progress.css';
        //$form['#attached']['js'][]  = $path_theme . 'js/analyzedphenotypes.loader.progress.js';
        //drupal_add_js('misc/progress.js');
        break;
    }

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

    // @todo add in message regarding allow new traits.

    // Stage Indicator/Title.
    //---------------------------------
    $all_stage = array(
      'Upload',   // Stage 1 - Select project, genus and upload data file.
      'Validate', // Stage 2 - Validate data.
      'Describe', // Stage 3 - Fully describe all traits.
      'Save',     // Stage 4 - Save data and file.
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

    foreach($all_stage as $i => $stage) {
      // If this stage is before the current stage then it's complete.
      // Otherwise the user still needs to fill it out.
      $s = ($i <= $stage_id) ? 'stage' : 'todo';
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

      // When project is selected, autofill the genus field with corresponding project genus.
      /* @todo test this...
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
      */
      '#description' => t('Type in the experiment or project title your data is specific to.'),
      '#id' => 'ap-project-select-field'
    );

    // All genus.
    $arr_all_genus = array(0 => '- Select -');
    // Fetch all genus that have been configured in settings page...
    $genusprop = ap_get_activegenus();
    // and add them to the options array.
    if ($genusprop) {
      foreach($genusprop as $g) {
        $arr_all_genus[$g] = $g;
      }
    }

    // Project genus - autofill.
    if (isset($form_state['values']['project_name'])) {
      $project = trim($form_state['values']['project_name']);

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
        ['query' => ['genus' => $project_genus, 'project' => $project_name]]
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

    $form['status'] = [
      '#markup' => '<h3 color="green">THREE</h3>',
      '#weight' => -110,
    ];

    return $form;
  }

  /**
   * Provides the save data portion of the form.
   *
   * @stage 4
   */
  public function form_stage4 ($form,  &$form_state) {

    $form['status'] = [
      '#markup' => '<h3 color="green">FOUR</h3>',
      '#weight' => -110,
    ];

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
    // fid.
    // If this is the second stage...
    if (isset($form_state['values']['file_upload'])) {
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
            '#value' => $f->file_path,
          ],
          'fid' => [
            '#type' => 'hidden',
            '#value' => $f->fid,
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

    $stage_id = $form_state['values']['stage_id'];

    if ($stage_id === 1) {

      //...........................................
      // Validate that the genus is set in Stage 1.
      if ($form_state['values']['genus'] == '0') {
        form_set_error('genus', 'You must set the genus for which the data is associated.');
      }

      //...........................................
      // Validate that file is uploaded in Stage 1.
      // First check the upload element.
      if (empty($form_state['values']['file_upload'])) {
        // Then check the existing file element.
        if (empty($form_state['values']['file_upload_existing'])) {
          form_set_error('file', 'You must provide a file.');
        }
        else {
          $form_state['values']['fid'] = $form_state['values']['file_upload_existing'];
        }
      }
      else {
        $form_state['values']['fid'] = $form_state['values']['file_upload'];
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
    $form_state['rebuild'] = TRUE;
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

    // The path to the uploaded file is always made available using the
    // 'files' argument. The importer can support multiple files, therefore
    // this is an array of files, where each has a 'file_path' key specifying
    // where the file is located on the server.
    $file_path = $this->arguments['files'][0]['file_path'];

    $FILE = fopen($file_path, 'r');
    $linenum = 0;
    while (($data = fgetcsv($FILE, 100000, "\t")) !== FALSE) {
      if ($linenum !== 0) {
      }
      $linenum++;
    }
    fclose($FILE);
  }
}
