<?php

/**
 * @file
 * Contains configuration interface of this module.
 */



// Ontologies API.
module_load_include('inc', 'analyzedphenotypes', 'api/analyzedphenotypes.ontology.api');


/**
 * Implements hook_form().
 * Construct module configuration page.
 */
function analyzedphenotypes_configuration_form($form, &$form_state) {
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
      '<br />Chado requires a "database" container to be associated with all controlled vocabularies. Please select the "database" container you would like to be associated with your trait vocabulary. If needed, create one '. l('here', 'admin/tripal/loaders/chado_db/add', array('attributes' => array('target'=>'_blank'))).'.',
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
  $dbs = ap_get_database();
  $db_options = array_map(function($d) {
    return $d['name'];
  }, $dbs);


  // Fetch all traits in a cv that have been used in phenotype table
  // to see if a cv setting has been used or has data already attached to it.
  // A single row would be enought to satisfy this condition.
  $sql = "SELECT cvterm_id FROM {cvterm} INNER JOIN {phenotype} ON cvterm_id = attr_id
    WHERE cv_id = :cv_id LIMIT 1";

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

        if ($j == 1) {
          // Inspect if this genus has data attached to it and disable it.
          $has_data = chado_query($sql, array(':cv_id' => $field_default))
            ->fetchAll();

          $is_disabled = ($has_data) ? TRUE : FALSE;
        }

        $field_prop = array(
          '#type' => 'select',
          '#title' => '',
          '#options' => array(0 => $var_token[$j][1]) + $field_options,
          '#attributes' => array('class' => array('ap-table-select-field')),
          '#default_value' => $field_default,
          '#disabled' => $is_disabled,
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
    '#markup' => '<div class="messages  warning">
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



  // To ensure any data uses same terms throughout by disabling this fieldset
  // when phenotype table has data saved in it. 1 row is sufficient to meet this condition.
  $has_data = chado_query("SELECT phenotype_id FROM {phenotype} LIMIT 1")
    ->fetchAll();

  $is_disabled = ($has_data) ? TRUE : FALSE;

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
        '#disabled' => $is_disabled,
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
    '#markup' => '<div class="messages  warning">
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
 * Implements hook_validate().
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
 * Implements hook_submit().
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
