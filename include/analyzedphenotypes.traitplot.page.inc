<?php
/**
 * @file
 * Contains functions related to rendering the violin plots.
 */

/**
 * Trait Distribution Single-Page Application.
 *
 * This page allows users to select an experiment + trait-method-unit
 * combination and visualize the data in violin plot form.
 *
 * @param $form
 *   The initialized form.
 * @param $form_state
 *   The current state of the form.
 * @return
 *   The completed form.
 */
function analyzedphenotypes_traitplot_app_form($form, $form_state) {

  // Add CSS for the chart.
  $module_path = drupal_get_path('module','analyzedphenotypes');
  $form['#attached']['css'][] = $module_path . '/theme/css/violinplot.chart.css';

  // Stage - Indicate when page is at Trait distribution summary or chart.
  // We would like ot show more intuitive form elements for the the first
  // time people are choosing a trait while still cleanly including the form
  // elements once the chart is drawn to allow users to change them quickly.
  // We are going to achomplish this by having two "stages".
  $experiment_id = 0;
  $trait_id = 0;
  $method_id = 0;
  $unit_id = 0;
  if (isset($form_state['build_info']['args'][0])
    AND is_numeric($form_state['build_info']['args'][0])) {
      $experiment_id = $form_state['build_info']['args'][0];
  }
  if (isset($form_state['build_info']['args'][1])
    AND is_numeric($form_state['build_info']['args'][1])) {
      $trait_id = $form_state['build_info']['args'][1];
  }
  if (isset($form_state['build_info']['args'][2])
    AND is_numeric($form_state['build_info']['args'][2])) {
      $method_id = $form_state['build_info']['args'][2];
  }
  if (isset($form_state['build_info']['args'][3])
    AND is_numeric($form_state['build_info']['args'][3])) {
      $unit_id = $form_state['build_info']['args'][3];
  }

  $stage = ($experiment_id && $trait_id && $method_id && $unit_id) ? 2 : 1;

  if ($stage == 1) {
    analyzedphenotypes_traitplot_app_filter_form($form, $form_state,
      $trait_id, $experiment_id, $method_id, $unit_id);
  }
  else {

    $form['link_back'] = array(
      '#type' => 'item',
      '#prefix' => '<span class="trait-app back-link">',
      '#markup' => l('Select a different trait', '/phenotypes/trait-distribution'),
      '#suffix' => '</span>',
    );

    analyzedphenotypes_traitplot_form($form, $form_state,
      $trait_id, $experiment_id, $method_id, $unit_id);
  }

  return $form;
}

/**
 * Trait Distribution Single-Page Application: Filter Form.
 *
 * @param $form
 *   An initialized form.
 * @param $form_state
 *   The current state of the form. Used to determine defaults.
 * @param $trait_id
 *   The cvterm_id for the trait to show.
 * @param $experiment_id
 *   The project_id for the experiment.
 * @param $method_id
 *   The cvterm_id for the method.
 * @param $unit_id
 *   The cvterm_id for the unit.
 */
function analyzedphenotypes_traitplot_app_filter_form(&$form, &$form_state,
  $trait_id, $experiment_id, $method_id, $unit_id) {

  // Select Dataset form.
  //----------------------------------------
  $AJAX_wrapper = 'nav';

  // Determine Defaults based on form_state.
  // @debug dpm($form_state, 'form state');
  $selection_path = 'none';
  if (isset($form_state['values']) AND !empty($form_state['values'])) {
    $selection_path = $form_state['values']['selection_path'];
    $experiment_id = $form_state['values']['experiment_id'];
    $trait_id = $form_state['values']['trait_id'];
    $method_id = (isset($form_state['values']['method_id'])) ? $form_state['values']['method_id'] : 0;
    $unit_id = (isset($form_state['values']['unit_id'])) ? $form_state['values']['unit_id'] : 0;
  }

  // @debug dpm(['exp' => $experiment_id, 'trait' => $trait_id, 'method' => $method_id, 'unit' => $unit_id], 'values');

  // AJAX Wrapper.
  $form[$AJAX_wrapper] = array(
    '#type' => 'fieldset',
    '#title' => 'Select Dataset',
    '#prefix' => '<div id="traitplot-nav" class="traitplot-nav expanded">',
    '#suffix' => '</div>',
    '#description' => 'This chart is meant to summarize the observed values for a specific trait in a particular phenotyping experiment.
      Since it is not always statistically correct to combine data across sites and years, this summary keeps the data seperate.
      This allows you to compare values across site-years.',
    '#weight' => 10,
  );

  $form[$AJAX_wrapper]['selection_path'] = array(
    '#type' => 'radios',
    '#title' => 'Select Dataset by',
    '#description' => 'The option select above will determine which options you have available for selecting a dataset.',
    '#options' => array(
      'experiment' => '<strong>Experiment:</strong> Browse various traits produced in a single experiment.',
      'trait' => '<strong>Trait:</strong> Browse various experiments in which a given trait was measured.',
    ),
    '#ajax' => array(
      'callback' => 'ajax_analyzedphenotypes_traitplot_page_nav_callback',
      'wrapper' => 'traitplot-nav',
      'method' => 'replace',
    ),
  );
  if ($selection_path == 'none') {
    $form[$AJAX_wrapper]['selection_path']['#prefix'] = '<span class="now-this">';
    $form[$AJAX_wrapper]['selection_path']['#suffix'] = '</span>';
  }

  // Experiments.
  // Retrieve options for Experiment (Project).
  $experiments = array();
  if ($selection_path == 'experiment') {
    // Fetch all experiments available in chado.phenotype.
    $experiments = chado_query("SELECT t1.project_id, t1.name
      FROM {project} AS t1 INNER JOIN {phenotype} USING(project_id)
      GROUP BY t1.project_id, t1.name ORDER BY t1.name ASC")->fetchAllKeyed();
  }
  elseif (($selection_path == 'trait') AND ($trait_id > 0)) {
    // Fetch only experiments measuring the given trait.
    $experiments = chado_query("SELECT t1.project_id, t1.name
      FROM {project} AS t1 INNER JOIN {phenotype} USING(project_id)
      WHERE phenotype.attr_id=:trait_id
      GROUP BY t1.project_id, t1.name ORDER BY t1.name ASC",
      array(':trait_id' => $trait_id))->fetchAllKeyed();
  }
  $form[$AJAX_wrapper]['experiment_id'] = array(
    '#type' => 'select',
    '#title' => 'Experiment',
    '#required' => TRUE,
    '#options' => $experiments,
    '#empty_option' => '- Select -',
    '#default_value' => $experiment_id,
    '#description' => 'Show only phenotypic data from the selected project.',
    '#ajax' => array(
      'callback' => 'ajax_analyzedphenotypes_traitplot_page_nav_callback',
      'wrapper' => 'traitplot-nav',
      'method' => 'replace',
    ),
    '#disabled' => TRUE,
  );
  if ($selection_path == 'trait' AND $trait_id) {
    $form[$AJAX_wrapper]['experiment_id']['#disabled'] = FALSE;
    $form[$AJAX_wrapper]['experiment_id']['#prefix'] = '<span class="now-this">';
    $form[$AJAX_wrapper]['experiment_id']['#suffix'] = '</span>';
  }
  elseif (($selection_path == 'experiment') AND !$experiment_id) {
    $form[$AJAX_wrapper]['experiment_id']['#disabled'] = FALSE;
    $form[$AJAX_wrapper]['experiment_id']['#prefix'] = '<span class="now-this">';
    $form[$AJAX_wrapper]['experiment_id']['#suffix'] = '</span>';
  }
  elseif ($experiment_id) {
    $form[$AJAX_wrapper]['experiment_id']['#disabled'] = FALSE;
  }

  // Traits.
  // Retrieve options for the trait selector.
  $traits = array();
  if ($selection_path == 'trait') {
    // Retrieve all traits.
    $traits = chado_query('SELECT p.attr_id as trait_id, cvt.name FROM {phenotype} p
      LEFT JOIN {cvterm} cvt ON p.attr_id=cvt.cvterm_id
      GROUP BY p.attr_id, cvt.name')->fetchAllKeyed();
  }
  elseif (($selection_path == 'experiment') AND ($experiment_id > 0)) {
    // Retrieve only traits in the current project.
    $traits = chado_query('SELECT p.attr_id as trait_id, cvt.name FROM {phenotype} p
      LEFT JOIN {cvterm} cvt ON p.attr_id=cvt.cvterm_id
      WHERE p.project_id=:experiment
      GROUP BY p.attr_id, cvt.name',
      array(':experiment' => $experiment_id))->fetchAllKeyed();
  }
  $form[$AJAX_wrapper]['trait_id'] = array(
    '#type' => 'select',
    '#title' => 'Trait',
    '#required' => TRUE,
    '#options' => $traits,
    '#empty_option' => '- Select -',
    '#default_value' => $trait_id,
    '#description' => 'Show only phenotypic data for the specified trait.',
    '#ajax' => array(
      'callback' => 'ajax_analyzedphenotypes_traitplot_page_nav_callback',
      'wrapper' => 'traitplot-nav',
      'method' => 'replace',
    ),
    '#disabled' => TRUE,
  );
  if (($selection_path == 'experiment') AND ($experiment_id > 0)) {
    $form[$AJAX_wrapper]['trait_id']['#disabled'] = FALSE;
    $form[$AJAX_wrapper]['trait_id']['#prefix'] = '<span class="now-this">';
    $form[$AJAX_wrapper]['trait_id']['#suffix'] = '</span>';
  }
  elseif ($selection_path == 'trait' AND !$trait_id) {
    $form[$AJAX_wrapper]['trait_id']['#disabled'] = FALSE;
    $form[$AJAX_wrapper]['trait_id']['#prefix'] = '<span class="now-this">';
    $form[$AJAX_wrapper]['trait_id']['#suffix'] = '</span>';
  }
  elseif ($trait_id) {
    $form[$AJAX_wrapper]['trait_id']['#disabled'] = FALSE;
  }

  if ($trait_id AND $experiment_id AND ($selection_path != 'none')) {

    unset($form[$AJAX_wrapper]['trait_id']['#prefix'],
      $form[$AJAX_wrapper]['trait_id']['#suffix'],
      $form[$AJAX_wrapper]['experiment_id']['#prefix'],
      $form[$AJAX_wrapper]['experiment_id']['#suffix']);

    $methods = array();
    $experiment_genus = 'Lens';
    $methods = ap_get_trait_methods($experiment_genus, $traits[$trait_id], array(), $trait_id);
    $form[$AJAX_wrapper]['method_id'] = array(
      '#type' => 'radios',
      '#title' => 'Method',
      '#description' => 'Describes how the data was taken. Since it is often not valid to combine data taken using different methods, you need to select the method you are interested in here.',
      '#options' => $methods,
      '#required' => TRUE,
      '#ajax' => array(
        'callback' => 'ajax_analyzedphenotypes_traitplot_page_nav_callback',
        'wrapper' => 'traitplot-nav',
        'method' => 'replace',
      ),
      '#prefix' => '<span class="now-this">',
      '#suffix' => '</span>',
    );

    if ($method_id) {
      unset($form[$AJAX_wrapper]['method_id']['#prefix'],
        $form[$AJAX_wrapper]['method_id']['#suffix']);

      $units = ap_get_method_units($experiment_genus, $methods[$method_id], array(), $method_id);
      $form[$AJAX_wrapper]['unit_id'] = array(
        '#type' => 'radios',
        '#title' => 'Unit',
        '#description' => 'The unit used to measure the data.',
        '#options' => $units,
        '#required' => TRUE,
        '#prefix' => '<span class="now-this">',
        '#suffix' => '</span>',
      );
    }

  }

  // Submit button.
  $form[$AJAX_wrapper]['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Generate Chart',
    '#weight' => 100,
  );

  $form[$AJAX_wrapper]['btn_goto_summarytable'] = array(
    '#markup' => '<div style="float: right">' . l('Go back to Summary Table', '/phenotypes') . '</div>',
    '#weight' => 100,
  );

  return $form;
}

/**
 * TraitPlot Single Page Application: Submit Handler.
 *
 * @param $form
 *   The current form.
 * @param $form_state
 *   The current state of the form.
 */
function analyzedphenotypes_traitplot_app_form_submit($form, &$form_state) {
  // Ensure the form saves its values when auto-submitted by select boxes.
  //$form_state["rebuild"] = TRUE;
  global $base_url;

  // Redirect to URL with experiment/trait in path to allow for bookmarking.
  $experiment_id = $form_state['values']['experiment_id'];
  $trait_id = $form_state['values']['trait_id'];
  $method_id = (isset($form_state['values']['method_id'])) ? $form_state['values']['method_id'] : 0;
  $unit_id = (isset($form_state['values']['unit_id'])) ? $form_state['values']['unit_id'] : 0;

  if ($unit_id > 0) {
    $redirect = $base_url . '/phenotypes/trait-distribution/' . $experiment_id . '/' . $trait_id . '/' . $method_id . '/' . $unit_id;
  }
  else {
    $redirect = $base_url . '/phenotypes/trait-distribution/';
  }

  $form_state['redirect'] = url($redirect);
}

/**
 * AJAX Callback: Trait Distribution plot page navagation.
 *
 * @param $form
 *   The current form.
 * @param $form_state
 *   The current state of the form.
 * @return
 *   The part of the form to render again.
 */
function ajax_analyzedphenotypes_traitplot_page_nav_callback($form, $form_state) {
  return $form['nav'];
}

/**
 * Trait Distribution Form
 *
 * This form MUST be supplied the trait, experiment, method and unit.
 * @param $form
 *   An initialized form.
 * @param $form_state
 *   The current state of the form. Used to determine defaults.
 * @param $trait_id
 *   The cvterm_id for the trait to show.
 * @param $experiment_id
 *   The project_id for the experiment.
 * @param $method_id
 *   The cvterm_id for the method.
 * @param $unit_id
 *   The cvterm_id for the unit.
 */
function analyzedphenotypes_traitplot_form(&$form, &$form_state,
      $trait_id, $experiment_id, $method_id, $unit_id) {

  // Construct chart text/wordings.
  $experiment_name = chado_query('SELECT name FROM {project} WHERE project_id=:id', array(':id' => $experiment_id))->fetchField();
  $trait_name = chado_query('SELECT name FROM {cvterm} WHERE cvterm_id=:id', array(':id' => $trait_id))->fetchField();
  $unit = chado_query('SELECT name FROM {cvterm} WHERE cvterm_id=:id', array(':id' => $unit_id))->fetchField();
  $method = chado_query('SELECT definition FROM {cvterm} WHERE cvterm_id=:id', array(':id' => $method_id))->fetchField();
  // Actually pull it all together.
  $xaxis = 'Site-Year';
  $yaxis = $trait_name.' ('.$unit.')';
  $figtitle = "Comparison of $trait_name observed values between site year for $experiment_name.";
  $figlegend = "$trait_name was measured in $unit. "
   . $method . " Replicates were then averaged per germplasm within a single site-year."
   . " The chart shows the traditional box plot with the kernel density estimation flanking it."
   . " This visualizes both the structure (i.e. median, interquartile range, and 95% confidence interval) and distribution of the data.";

  // Add needed JS for the chart.
  $module_path = drupal_get_path('module','analyzedphenotypes');
  $form['#attached']['js'][] = 'https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.17/d3.min.js';
  $form['#attached']['js'][] = $module_path . '/theme/js/distroChart.js';
  $form['#attached']['js'][] = $module_path . '/theme/js/violin.chart.js';
  $form['#attached']['js'][] = array(
    'type' => 'setting',
    'data' => array('analyzedPhenotypes' => array(
      'experiment_id' => $experiment_id,
      'trait_id' => $trait_id,
      'method_id' => $method_id,
      'unit_id' => $unit_id,
      'xaxis' => $xaxis,
      'yaxis' => $yaxis,
    )),
  );

  // Add CSS for the chart.
  $form['#attached']['css'][] = $module_path . '/theme/css/violinplot.chart.css';

    // Add a canvas for the chart.
    $form['figure']['chart'] = array(
      '#type' => 'item',
      '#markup' => '<div class="chart-wrapper" id="tripal-ap-violin-plot"></div>'
    );

    // Figure Legend.
    $form['figure']['legend'] = array(
      '#type' => 'markup',
      '#prefix' => '<div class="legend">',
      '#suffix' => '</div>',
    );

    $form['figure']['legend']['title'] = array(
      '#type' => 'markup',
      '#prefix' => '<div class="title">',
      '#markup' => 'Figure: '. $figtitle,
      '#suffix' => '</div>',
    );

    $form['figure']['legend']['description'] = array(
      '#type' => 'markup',
      '#prefix' => '<div class="description">',
      '#markup' => $figlegend,
      '#suffix' => '</div>',
    );

  return $form;
}

/**
 * Generates the JSON needed for the Trait Distribution chart.
 *
 * Specifically, this chart expects
 *   - the category for the X-axis. This is the site-year.
 *   - the value measured. This is the average for all replicates per site-year.
 *
 * @param $trait_id
 *   The cvterm_id for the trait to show.
 * @param $experiment_id
 *   The project_id for the experiment.
 * @param $method_id
 *   The cvterm_id for the method.
 * @param $unit_id
 *   The cvterm_id for the unit.
 *
 * @return
 *   The formatted json describing this dataset.
 */
function analyzedphenotypes_traitplot_json($project_id, $trait_id, $method_id, $unit_id) {
  $results = array();

  if ($project_id && $trait_id && $method_id && $unit_id) {
    $resource = chado_query(
      "SELECT location, year, stock_name, mean
        FROM chado.mview_phenotype
        WHERE experiment_id=:project_id
          AND trait_id=:trait_id
          AND method_id= :method_id
          AND unit_id=:unit_id
        ORDER BY year ASC, location ASC",
      array(
        ':project_id' => $project_id,
        ':trait_id' => $trait_id,
        ':method_id' => $method_id,
        ':unit_id' => $unit_id,
    ));
    foreach($resource as $r) {

      $results[] = array(
        'category' => $r->location . ' ' . $r->year,
        'value' => $r->mean,
      );
    }
  }

  // Sort the results by key.
  ksort($results);

  return drupal_json_output($results);
}
