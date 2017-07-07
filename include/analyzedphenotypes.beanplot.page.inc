<?php
/**
 * @file
 *
 */

/**
 * Beanplot Page
 * Implements hook_form().
 */
function analyzedphenotypes_beanplot_form($form, $form_state) {
  global $base_url;
  
  // Determine experiment/trait defaults.
  $experiment_id = NULL;
  if (isset($form_state['build_info']['args'][0])) {
    $experiment_id = $form_state['build_info']['args'][0];
  }
  if (isset($form_state['values']['experiment'])) {
    $experiment_id = $form_state['values']['experiment'];
  }
  $trait_id = NULL;
  if (isset($form_state['build_info']['args'][1])) {
    $trait_id = $form_state['build_info']['args'][1];
  }
  if (isset($form_state['values']['trait'])) {
    $trait_id = $form_state['values']['trait'];
  }

  // We would like ot show more intuitive form elements for the the first
  // time people are choosing a trait while still cleanly including the form
  // elements once the chart is drawn to allow users to change them quickly.
  // We are going to achomplish this by having two "stages".
  $stage = 1;
  if ($experiment_id AND $trait_id) {
    $stage = 2;
  }

  // Style, Javascript and Libraries.
  $module_path = drupal_get_path('module','analyzedphenotypes');
  $form['#attached']['js'] = array(
    'https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.17/d3.min.js',
    $module_path . '/include/js/beanplot.chart.js',
  );
  $form['#attached']['css'] = array(
    $module_path . '/include/css/beanplot.chart.css',
  );
  
  // Retrieve options for Experiment (Project).
  $experiments = array();
  if (!empty($trait_id)) {
    $experiments = chado_query(
      'SELECT project_id, name FROM {project} 
      WHERE project_id IN (
          SELECT project_id from {phenotype} WHERE attr_id=:trait_id)',
      array(':trait_id' => $trait_id))->fetchAllKeyed();
  }
  else {
    $experiments = chado_query('SELECT project_id, name FROM {project}  WHERE project_id IN (SELECT project_id from {phenotype})')->fetchAllKeyed();
  }

  // Retrieve options for Trait.
  $traits = array();
  if (!empty($experiment_id)) {
    $traits = chado_query(
      'SELECT cvt.cvterm_id as trait_id, cvt.name as trait_name 
       FROM {cvterm} cvt
       WHERE cvt.cvterm_id IN (SELECT attr_id FROM {phenotype} WHERE project_id=:experiment_id)',
       array(':experiment_id' => $experiment_id))->fetchAllKeyed();
  }
  else {
    $traits = chado_query(
      'SELECT cvt.cvterm_id as trait_id, cvt.name as trait_name 
       FROM chado.cvterm cvt
       WHERE cvt.cvterm_id IN (SELECT attr_id FROM chado.phenotype)',
       array(':experiment_id' => $experiment_id))->fetchAllKeyed();
  }

  /////////////////////////////////////
  // STAGE I
  // Intuitive form for choosing experiment & trait.
  if ($stage === 1) {

    // Navigation.
    $form['nav'] = array(
      '#type' => 'fieldset',
      '#title' => 'Select Dataset',
      '#prefix' => '<div class="beanplot-nav expanded">',
      '#suffix' => '</div>',
    );
    
    $form['nav']['intro'] = array(
      '#type' => 'item',
      '#markup' => '<p>This chart is meant to summarize the observed values for a specific trait in a particular phenotyping experiment. Since it is not always statistically correct to combine data across sites and years, this summary keeps the data seperate. This allows you to compare values across site-years.</p>',
    );
  
    // Experiment.
    $form['nav']['experiment'] = array(
      '#type' => 'select',
      '#title' => 'Experiment',
      '#required' => TRUE,
      '#empty_option' => '- Select -',
      '#options' => $experiments,
      '#default_value' => $experiment_id,
      '#description' => 'Show only phenotypic data from the selected project.',
      '#ajax' => array(
        'callback' => 'ajax_analyzedphenotypes_beanplot_page_nav_callback',
        'wrapper' => 'beanplot-nav',
        'method' => 'replace',
        'effect' => 'fade',
      ),
    );
    $experiment_name = (isset($experiments[ $experiment_id ])) ? $experiments[ $experiment_id ] : '';
  
    // Traits.
    $form['nav']['trait'] = array(
      '#type' => 'select',
      '#title' => 'Trait',
      '#required' => TRUE,
      '#empty_option' => '- Select -',
      '#options' => $traits,
      '#default_value' => $trait_id,
      '#description' => 'Show only data for the selected trait.',
      '#ajax' => array(
        'callback' => 'ajax_analyzedphenotypes_beanplot_page_nav_callback',
        'wrapper' => 'beanplot-nav',
        'method' => 'replace',
        'effect' => 'fade',
      ),
    );
    $trait_name = (isset($traits[ $trait_id ])) ? $traits[ $trait_id ] : '';

  }
  
  /////////////////////////////////////
  // STAGE II
  // Experiment & Trait already choosen.
  if ($stage === 2) {
  
    // Navigation.
    $form['nav'] = array(
      '#type' => 'fieldset',
      '#title' => 'Select Dataset',
      '#prefix' => '<div class="beanplot-nav condensed">',
      '#suffix' => '</div>',
    );
  
    // Experiment (project).
    $form['nav']['experiment'] = array(
      '#type' => 'select',
      '#title' => 'Experiment',
      '#empty_option' => 'Select an Experiment',
      '#title_display' => 'invisible',
      '#options' => $experiments,
      '#default_value' => $experiment_id,
      '#attributes' => array('onChange' => 'document.getElementById("analyzedphenotypes-beanplot-form").submit();'),
    );
    $experiment_name = (isset($experiments[ $experiment_id ])) ? $experiments[ $experiment_id ] : '';
  
    // Traits.
    $form['nav']['trait'] = array(
      '#type' => 'select',
      '#title' => 'Trait',
      '#empty_option' => 'Select a Trait',
      '#title_display' => 'invisible',
      '#options' => $traits,
      '#default_value' => $trait_id,
      '#attributes' => array('onChange' => 'document.getElementById("analyzedphenotypes-beanplot-form").submit();'),
    );
    if (empty($experiment_id)) {
      $form['nav']['trait']['#disabled'] = TRUE;
    }
    $trait_name = (isset($traits[ $trait_id ])) ? $traits[ $trait_id ] : '';
  
    // Chart Placeholder.
    // This div will be populated by js.
    $form['figure']['placeholder'] = array(
      '#type' => 'markup',
      '#markup' => '<div id="beanplot-container" style="height: 500px; width: 100%;"></div>',
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
      '#suffix' => '</div>',
    );
  
    $form['figure']['legend']['description'] = array(
      '#type' => 'markup',
      '#prefix' => '<div class="description">',
      '#suffix' => '</div>',
    );
  
    $form['figure']['legend']['title']['#markup'] = 'Figure: The Distribution of mean <em>'.$trait_name.'</em> per site year for <em>'.$experiment_name.'</em>.';
    
    $form['figure']['legend']['description']['#markup'] = 'This chart shows the distribution of means observered (y-axis) per site year (x-axis) with the width of each bar representing the number of germplasm a given mean was observed/measured in. Only the data for the <em>'.$experiment_name.'</em> phenotyping experiment is being shown.';

    $settings = array(
      'trait' => $trait_name,
      'germplasm' => '',
      'jsonUrl' => strtr(
        $base_url . '/json/phenotypes/beanplot/:project_id/:trait_id', 
        array(
            ':project_id' => $experiment_id, 
            ':trait_id' => $trait_id
        )
      ),
    );
    drupal_add_js(array('analyzedPhenotypes' => array('beanplot' => $settings)), 'setting');
  }

  // Submit button.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Generate Chart',
  );
  // Hide the submit button if this is stage 2.
  if ($stage === 2) {
    $form['submit']['#attributes'] = array(
        'class' => array('element-invisible')
    );
  }

  return $form;
}

/**
 * Beanplot Page: Submit Handler.
 */
function analyzedphenotypes_beanplot_form_submit($form, &$form_state) {

  // Ensure the form saves its values when auto-submitted by select boxes.
  //$form_state["rebuild"] = TRUE;

  // Redirect to URL with experiment/trait in path to allow for bookmarking.
  $experiment_id = $form_state['values']['experiment'];
  $trait_id = $form_state['values']['trait'];
  $form_state['redirect'] = url('phenotypes/trait-distribution/' . $experiment_id . '/' . $trait_id);
}

/**
 * AJAX Callback: Beanplot page navagation.
 */
function ajax_analyzedphenotypes_beanplot_page_nav_callback($form, $form_state) {
  return $form['nav'];
}

/**
 * Generates the data for the beanplot as JSON.
 * 
 * @param $project_id
 *   Retrieve phenotypic data for the specified project.
 * @param $trait_id
 *   Retrieve phenotypic data for the specified trait.
 * @return
 *   JSON array summarizing the phenotypic data for display in a beanplot chart.
 */
function analyzedphenotypes_beanplot_json($project_id, $trait_id) {
  $results = array();

  $resource = chado_query(
    "SELECT loc.value as location, yr.value as year, s.name as germplasm_name, p.value as mean
      FROM chado.phenotype p 
      LEFT JOIN chado.cvterm trait ON trait.cvterm_id=p.attr_id 
      LEFT JOIN chado.project proj USING(project_id) 
      LEFT JOIN chado.stock s USING(stock_id)
      LEFT JOIN chado.phenotypeprop loc ON loc.phenotype_id=p.phenotype_id AND loc.type_id IN (SELECT cvterm_id FROM chado.cvterm WHERE name='Location')
      LEFT JOIN chado.phenotypeprop yr ON yr.phenotype_id=p.phenotype_id AND yr.type_id IN (SELECT cvterm_id FROM chado.cvterm WHERE name='Year')
      WHERE p.project_id=:project_id AND p.attr_id=:trait_id",
    array(':project_id' => $project_id, 'trait_id' => $trait_id));
  foreach($resource as $r) {

    $index = $r->location . '-' . $r->year . '-' . $r->mean;

    // If index is found, increment the number of times it appeard in the dataset.
    if (isset($results[$index])) {
      $results[$index]['no']++;
    }
    // Otherwise initialize the data point.
    else {

      $mean = strstr($r->mean, '.') ? (float)$r->mean : (int)$r->mean;

      $results[$index] = array(
        'location' => $r->location,
        'year' => $r->year,
        'germ' => 0,
        'mean' => $mean,
        'no' => 1,
      );
    }
  }
  
  // Sort the results by key.
  ksort($results);
  
  // Switch from associative array to sequenctial indexed array.
  $results = array_values($results);
  
  return drupal_json_output($results);
}