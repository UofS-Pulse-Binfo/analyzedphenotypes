<?php
/**
 * @file
 * Contains functions related to rendering the violin plots.
 */

/**
 * Violin Plot Page.
 *
 * This page allows users to select an experiment + trait-method-unit
 * combination and visualize the data in violin plot form.
 */
function analyzedphenotypes_traitplot_form($form, $form_state) {
  global $base_url;

  // Stage - Indicate when page is at Trait distribution summary or chart.
  // We would like ot show more intuitive form elements for the the first
  // time people are choosing a trait while still cleanly including the form
  // elements once the chart is drawn to allow users to change them quickly.
  // We are going to achomplish this by having two "stages".
  $experiment_id = null;
  $trait_id = null;
  if (isset($form_state['build_info']['args'][0])) {
    $experiment_id = $form_state['build_info']['args'][0];

    if (!is_numeric($experiment_id)) {
      // Not a valid id - force to stage 1.
      $experiment_id = null;
    }
  }

  if (isset($form_state['build_info']['args'][1])) {
    $trait_id = $form_state['build_info']['args'][1];

    if (!is_numeric($trait_id)) {
      // Not valid id - force to stage 1.
      $trait_id = null;
    }
  }

  $stage = ($experiment_id && $trait_id) ? 2 : 1;

  // When in select dataset form, load all traits, otherwise (in chart page) load traits per project.
  if ($stage == 2) {
    // Retrieve options for Experiment (Project).
    // Fetch experiment selected.
    $experiments = chado_query("
      SELECT t1.project_id, t1.name FROM {project} AS t1 INNER JOIN {phenotype} USING(project_id)
      WHERE t1.project_id = :experiment_id
      GROUP BY t1.project_id, t1.name ORDER BY t1.name ASC
    ", array(
      ':experiment_id' => $experiment_id
    ))
    ->fetchAllKeyed();

    // User can access the url containing query filters.
    // In case none is found. send user to stage 1.
    if (!$experiments) {
      $stage = 1;
    }

    // Fetch only trait in a project.
    $traits = chado_query("
      SELECT cvt.cvterm_id, cvt.name, pnt.project_id FROM {cvterm} AS cvt INNER JOIN {phenotype} AS pnt
        ON cvt.cvterm_id = pnt.attr_id
      WHERE pnt.project_id = :experiment_id
      GROUP BY cvt.cvterm_id, cvt.name, pnt.project_id ORDER BY cvt.name ASC
    ", array(
      ':experiment_id' => $experiment_id
    ))
    ->fetchAll();

    // Project ha no traits.
    if (count($traits) < 0) {
      $stage = 1;
    }
    else {
      $all_id = array_map(function($e) {
        return $e->cvterm_id;
      }, $traits);

      if (!in_array($trait_id, $all_id)) {
        $stage = 1;
      }
    }
  }

  if ($stage == 1) {
    // Retrieve options for Experiment (Project).
    // Fetch all experiments available in chado.phenotype.
    $experiments = chado_query("
      SELECT t1.project_id, t1.name FROM {project} AS t1 INNER JOIN {phenotype} USING(project_id)
      GROUP BY t1.project_id, t1.name ORDER BY t1.name ASC
    ")
    ->fetchAllKeyed();

    // Fetch all traits available in chado.phenotype.
    $traits = chado_query("
      SELECT cvt.cvterm_id, cvt.name, pnt.project_id FROM {cvterm} AS cvt INNER JOIN {phenotype} AS pnt
      ON cvt.cvterm_id = pnt.attr_id GROUP BY cvt.cvterm_id, cvt.name, pnt.project_id ORDER BY cvt.name ASC
    ")
    ->fetchAll();
  }

  // Construct an associative array of traits and pass it to select field trait below.
  $trait_options = array();
  foreach($traits as $t) {
    $trait_options[$t->cvterm_id] = $t->name;
  }

  // Array to hold traits not specific and relevant to an experiment, repectively.
  $not_project_trait = array();
  $project_trait = array();


  // TEST Traits field:
  if (isset($form_state['values']['trait']) && $form_state['values']['trait'] > 0) {
    // Value set.
    // From the traits query result, find the corresponding project id.
    // Use the project id and default experiment field to this project.
    $trait_id = $form_state['values']['trait'];
    $fld_experiment = $form_state['values']['experiment'];

    if ($fld_experiment == $form_state['values']['cache_experiment']) {
      // Check if user changed the proejct by comparing experiment in cache field.
      foreach($traits as $t) {
        if ($t->cvterm_id == $trait_id) {
          $experiment_id = $t->project_id;

          if ($fld_experiment == 0) {
            $experiment_id = $t->project_id;

            break;
          }
          else {
            if ($t->project_id == $fld_experiment) {
              $experiment_id = $fld_experiment;

              break;
            }
            else {
              continue;
            }
          }
        }
      }
    }
  }


  // TEST Experiment field:
  if ((isset($form_state['values']['experiment']) &&
    $form_state['values']['experiment'] > 0)  || $experiment_id > 0) {

    // Value set.
    // Experiment field changed. Find the trait that is specifi to the project
    // and trait no relevant. When complete, join the arrays - this way all traits
    // relevant to the project are on top of non-project traits.
    $experiment_id = ($experiment_id > 0) ? $experiment_id : $form_state['values']['experiment'];

    foreach ($traits as $t) {
      if ($t->project_id == $experiment_id) {
        $project_trait[$t->cvterm_id] = $t->name;
      }
      else {
        $not_project_trait[$t->cvterm_id] = $t->name;
      }
    }

    // Clear the traits array generated above and use this.
    unset($trait_options);
    $trait_options = $project_trait + $not_project_trait;
  }

  // FORM: MAIN AJAX WRAPPER.
  $AJAX_wrapper = 'nav';

  if ($stage == 1) {
    /////////////////////////////////////
    // STAGE I
    // Intuitive form for choosing experiment & trait.

    $module_path = drupal_get_path('module','analyzedphenotypes');
    drupal_add_js($module_path . '/theme/js/beanplot.chart.selectfield.js', array('scope' => 'footer'));

    // AJAX Wrapper.
    $form[$AJAX_wrapper] = array(
      '#type' => 'fieldset',
      '#title' => 'Select Dataset',
      '#prefix' => '<div id="beanplot-nav" class="beanplot-nav expanded">',
      '#suffix' => '</div>',
    );

    // Introduction.
    $form[$AJAX_wrapper]['intro'] = array(
      '#type' => 'item',
      '#markup' => '<p>This chart is meant to summarize the observed values for a specific trait in a particular phenotyping experiment.
        Since it is not always statistically correct to combine data across sites and years, this summary keeps the data seperate.
        This allows you to compare values across site-years.</p>',
    );

    // Experiments.
    $form[$AJAX_wrapper]['experiment'] = array(
      '#type' => 'select',
      '#title' => 'Experiment',
      '#required' => TRUE,
      '#options' => $experiments,
      '#empty_option' => '- Select -',
      // This is not working. Passing the default experiment to JS.
      //'#default_value' => $experiment_id,
      '#description' => 'Show only phenotypic data from the selected project.',
      '#ajax' => array(
        'callback' => 'ajax_analyzedphenotypes_beanplot_page_nav_callback',
        'wrapper' => 'beanplot-nav',
        'method' => 'replace',
      ),
      '#id' => 'ap-trait-distribution-experiment-field',
    );

    // Pass the value to JavaScript.
    $form[$AJAX_wrapper]['cache_default_experiment'] = array(
      '#type' => 'hidden',
      '#value' => $experiment_id,
      '#attributes' => array('id' => 'cache_project_id'),
    );

    // Traits.
    $form[$AJAX_wrapper]['trait'] = array(
      '#type' => 'select',
      '#title' => 'Trait',
      '#required' => TRUE,
      '#empty_option' => '- Select -',
      '#options' => $trait_options,
      '#description' => 'Show only data for the selected trait.',
      '#ajax' => array(
        'callback' => 'ajax_analyzedphenotypes_beanplot_page_nav_callback',
        'wrapper' => 'beanplot-nav',
        'method' => 'replace',
      ),
      '#id' => 'ap-trait-distribution-trait-field',
    );

    // Hidden field. Cache.
    // This value will be altered by JS to give user a clue of which
    // set of traits belong to an experiment by highlighting text.
    $form[$AJAX_wrapper]['cache_project_trait'] = array(
      '#type' => 'hidden',
      '#value' => implode('#', array_keys($project_trait)),
      '#attributes' => array('id' => 'cache-project_traits'),
    );

    $form['values']['cache_experiment'] = array(
      '#type' => 'value',
      '#value' => (isset($experiment_id)) ? $experiment_id : '',
    );
  }
  else {
    /////////////////////////////////////
    // STAGE II
    // Experiment & Trait already choosen.

    // Disable field when user alters the url.
    if (empty($experiment_id)) {
      $form[$AJAX_wrapper]['trait']['#disabled'] = TRUE;
    }

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
      )),
    );
    // Add CSS for the chart.
    $form['#attached']['css'][] = $module_path . '/theme/css/violinplot.chart.css';

    $form[$AJAX_wrapper] = array(
      '#type' => 'fieldset',
      '#title' => 'Select Dataset',
      '#prefix' => '<div id="beanplot-nav" class="beanplot-nav condensed">',
      '#suffix' => '</div>',
    );

    // Experiment (project).
    $form[$AJAX_wrapper]['experiment'] = array(
      '#type' => 'select',
      '#title' => 'Experiment',
      '#empty_option' => 'Select an Experiment',
      '#title_display' => 'invisible',
      '#options' => $experiments,
      '#default_value' => $experiment_id,
    );

    // Traits.
    $form[$AJAX_wrapper]['trait'] = array(
      '#type' => 'select',
      '#title' => 'Trait',
      '#title_display' => 'invisible',
      '#options' => $trait_options,
      '#default_value' => $trait_id,
    );

    // Submit form when either select field got changed.
    drupal_add_js('jQuery(document).ready(function() {
      jQuery("#beanplot-nav select").change(function() {
        this.form.submit();
      });
    })', 'inline');

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
      '#suffix' => '</div>',
    );

    $form['figure']['legend']['description'] = array(
      '#type' => 'markup',
      '#prefix' => '<div class="description">',
      '#suffix' => '</div>',
    );

    // Construct chart text/wordings.
    $experiment_name = (isset($experiments[ $experiment_id ])) ? $experiments[ $experiment_id ] : '';
    $trait_name      = (isset($trait_options[ $trait_id ]))    ? $trait_options[ $trait_id ]    : '';

    $form['figure']['legend']['title']['#markup'] = 'Figure: The Distribution of mean <em>'.$trait_name.'</em> per site year for <em>'.$experiment_name.'</em>.';
    $form['figure']['legend']['description']['#markup'] = 'This chart shows the distribution of means observered (y-axis) per site year (x-axis) with the width at a given mean representing the number of germplasm that mean was observed/measured in.
      Only the data for the <em>'.$experiment_name.'</em> phenotyping experiment is being shown.';

    // Chart variables.
    $settings = array(
      'trait' => $trait_name,
      'germplasm' => '',
      'jsonUrl' => strtr(
        $base_url . '/json/phenotypes/beanplot/:project_id/:trait_id',
        array(
            'project_id' => $experiment_id,
            'trait_id' => $trait_id
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

  $form['btn_goto_summarytable'] = array(
    '#markup' => '<div style="float: right">' . l('Go back to Summary Table', '/phenotypes') . '</div>',
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
function analyzedphenotypes_traitplot_form_submit($form, &$form_state) {
  // Ensure the form saves its values when auto-submitted by select boxes.
  //$form_state["rebuild"] = TRUE;
  global $base_url;

  // Redirect to URL with experiment/trait in path to allow for bookmarking.
  $experiment_id = $form_state['values']['experiment'];
  $trait_id = $form_state['values']['trait'];

  if ($experiment_id > 0) {
    $redirect = $base_url . '/phenotypes/trait-distribution/' . $experiment_id . '/' . $trait_id;
  }
  else {
    $redirect = $base_url . '/phenotypes/trait-distribution/';
  }

  $form_state['redirect'] = url($redirect);
}

/**
 * Generates the CSV needed for the chart.
 *
 * Specifically, this chart expects
 *   - the category for the X-axis (siteyear) and
 *   - the value measured.
 */
function analyzedphenotypes_traitplot_json($project_id, $trait_id, $method_id, $unit_id) {
  $results = array();

  $resource = chado_query(
    "SELECT location, year, stock_name, mean
      FROM chado.mview_phenotype
      WHERE experiment_id=:project_id AND trait_id=:trait_id
      ORDER BY year ASC, location ASC",
    array(':project_id' => $project_id, 'trait_id' => $trait_id));
  foreach($resource as $r) {

    $results[] = array(
      'category' => $r->location . ' ' . $r->year,
      'value' => $r->mean,
    );
  }

  // Sort the results by key.
  ksort($results);

  return drupal_json_output($results);
}
