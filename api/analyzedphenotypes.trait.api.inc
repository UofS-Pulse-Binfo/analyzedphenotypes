<?php
/**
 * @file
 *
 * API: TRAIT MANAGEMENT
 * Handles creation of traits and associated cvterms (i.e. method and unit)
 */

/**
 * Add a trait to Chado.
 *
 * @param $data
 *   - genus: the organism genus the trait is for (e.g. Lens).
 *   - name: the name of the trait (e.g. Plant Height).
 *   - description: the description of the trait (e.g. the height of the plant from the
 *      ground to it's tallest point without stretching the plant.)
 *   - method_title: a short name for the method (e.g. Average of 5 plants)
 *   - method: a description of the method used to collect the phenotypic data
 *      (e.g. measured 5 plants from the plot and then averaged them.)
 *   - unit: the full word describing the unit used in the method (e.g. centimeters)
 *   - type: one of "qual" or "quant" for qualitative and quantitative respectively.
 *
 * @return
 *   An array with the following keys where each value is the new cvterm:
 *   trait, method, unit.
 */
function ap_insert_trait($data) {

  // @debug dpm($data, 'inserting trait via new api function');

  // Ensure that parameters are available.
  if (!isset($data['genus'])) {
    tripal_report_error('ap_trait', TRIPAL_ERROR, '"genus" is required to insert a trait.');
  }
  if (!isset($data['name']) AND !isset($data['trait_id'])) {
    tripal_report_error('ap_trait', TRIPAL_ERROR, '"name" is required to insert a trait.');
  }
  if (!isset($data['description']) AND !isset($data['trait_id'])) {
    tripal_report_error('ap_trait', TRIPAL_ERROR, '"description" is required to insert a trait.');
  }
  if (!isset($data['method_title'])) {
    tripal_report_error('ap_trait', TRIPAL_ERROR, '"method_title" is required to insert a trait.');
  }
  if (!isset($data['method'])) {
    tripal_report_error('ap_trait', TRIPAL_ERROR, '"method" is required to insert a trait.');
  }
  if (!isset($data['unit'])) {
    tripal_report_error('ap_trait', TRIPAL_ERROR, '"unit" is required to insert a trait.');
  }

  // Defaults.
  if (!isset($data['type'])) {
    $data['type'] = 'qual';
  }

  // Grab necessary system config.
  //  - Get system configuration of this genus.
  $sysvar = ap_get_variablenames(
    array('variablename' => $data['genus']),
    array('set' => 'cvdbon', 'suffix' => 'allsuffix')
  );

  //  - Database (db). Used in the ID in cvterm property array below.
  $sysvar_genus_db = variable_get($sysvar['db']);
  $dbprop = ap_match_database($sysvar_genus_db);
  $db_name = $dbprop['name'];

  //  - Controlled Vocabulary (cv). Used in the cv_name in cvterm property array below.
  $sysvar_genus_cv = variable_get($sysvar['cv']);
  $cvprop = ap_get_cv(array(
    'cv_id' => $sysvar_genus_cv
  ));
  $cv_name = $cvprop['name'];

  //  - Method Controlled Vocabulary (cv). Used in the cv_name in cvterm property array below.
  $sysvar_genus_method = variable_get($sysvar['method']);
  $cvprop = ap_get_cv(array(
    'cv_id' => $sysvar_genus_method
  ));
  $method_cvname = $cvprop['name'];

  //  - Unit Controlled Vocabulary (cv). Used in the cv_name in cvterm property array below.
  $sysvar_genus_unit = variable_get($sysvar['unit']);
  $cvprop = ap_get_cv(array(
    'cv_id' => $sysvar_genus_unit
  ));
  $unit_cvname = $cvprop['name'];

  // Insert the trait cvterm
  if (!isset($data['trait_id'])) {
    $trait_cvterm = array(
      'cv_name' => $cv_name,
      'id' => trim($db_name) . ':' . $data['name'],
      'name' => $data['name'],
      'definition' => $data['description'],
    );
    $trait_cvterm = tripal_insert_cvterm($trait_cvterm);
    if (!$trait_cvterm) {
      tripal_report_error('ap_trait', TRIPAL_ERROR, 'Unable to create trait when the following is supplied: '.print_r($data,TRUE));
      return FALSE;
    }
    $trait_id = $trait_cvterm->cvterm_id;
  }
  else {
    $trait_cvterm = tripal_get_cvterm(array('cvterm_id' => $data['trait_id']));
  }

  // Check to see if the method exists.
  // This is needed because the method uses cvterm.definition rather then name.
  $exists = chado_query('SELECT cvterm_id FROM {cvterm} WHERE definition=:method AND cv_id IN (SELECT cv_id FROM {cv} WHERE name=:cv_name)',
    array(':method' => $data['method'], ':cv_name' => $cv_name))->fetchField();
  if (!$exists) {
    // Insert the method cvterm
    $method_cvterm = array(
      'cv_name' => $method_cvname,
      'id' => trim($db_name) . ':' . $data['method_title'],
      'name' => $data['method_title'],
      'definition' => $data['method'],
    );
    $method_cvterm = tripal_insert_cvterm($method_cvterm);
    if (!$method_cvterm) {
      tripal_report_error('ap_trait', TRIPAL_ERROR, 'Unable to create method when the following is supplied: '.print_r($data,TRUE));
    }
  }
  else {
    $method_cvterm = tripal_get_cvterm(array('cvterm_id' => $exists));
  }

  // Insert the unit cvterm
  $unit_cvterm = array(
    'cv_name' => $unit_cvname,
    'id' => trim($db_name) . ':' . $data['unit'],
    'name' => $data['unit'],
    'definition' => '',
  );
  $unit_cvterm = tripal_insert_cvterm($unit_cvterm);
  if (!$unit_cvterm) {
    tripal_report_error('ap_trait', TRIPAL_ERROR, 'Unable to create unit when the following is supplied: '.print_r($data,TRUE));
  }

  // Add type property to the unit.
  $unit_type = array(
    'cvterm_id' => $unit_cvterm->cvterm_id,
    'type_id' => array('name' => 'additionalType', 'cv_id' => array('name' => 'schema')),
  );
  $exists = chado_select_record('cvtermprop', array('cvtermprop_id'), $unit_type);
  if (empty($exists)) {
    if ($data['type'] == 'quant') {
      $unit_type['value'] = 'quantitative';
    }
    else {
      $unit_type['value'] = 'qualitative';
    }
    chado_insert_record('cvtermprop', $unit_type);
  }

  // Relate Trait => Method
  $sysvar = ap_get_variablenames(
    array('variablename' => 'method'),
    array('set' => 'terms')
  );
  $type_id = variable_get($sysvar);
  $values = array(
    'subject_id' => $trait_cvterm->cvterm_id,
    'type_id' => $type_id,
    'object_id' => $method_cvterm->cvterm_id,
  );
  $exists = chado_select_record('cvterm_relationship', array('cvterm_relationship_id'), $values);
  if (empty($exists)) {
    chado_insert_record('cvterm_relationship', $values);
  }

  // Relate Method => Unit
  $sysvar = ap_get_variablenames(
    array('variablename' => 'unit'),
    array('set' => 'terms')
  );
  $type_id = variable_get($sysvar);
  $values = array(
    'subject_id' => $method_cvterm->cvterm_id,
    'type_id' => $type_id,
    'object_id' => $unit_cvterm->cvterm_id,
  );
  $exists = chado_select_record('cvterm_relationship', array('cvterm_relationship_id'), $values);
  if (empty($exists)) {
    chado_insert_record('cvterm_relationship', $values);
  }

  return array(
    'trait' => $trait_cvterm,
    'method' => $method_cvterm,
    'unit' => $unit_cvterm,
  );
}

/**
 * Retrieve the method details based on the name.
 *
 * @param $project_genus
 *   The organism genus the trait is measured in.
 * @param $method_name
 *   The full name of the trait.
 * @param $options
 *   An array with the following possible keys:
 *    [NONE YET SUPPORTED]
 */
function ap_get_method($project_genus, $method_name, $options = []) {

  // @todo restrict this by the project genus.

  // Now lookup the details for the specified method.
  $method = chado_select_record('cvterm', array('cvterm_id', 'name', 'definition'),
      array('name' => $method_name));

  return $method[0];
}

/**
 * Retrieve the collection methods based on the name of the trait.
 *
 * @param $project_genus
 *   The organism genus the trait is measured in.
 * @param $trait_name
 *   The full name of the trait.
 * @param $options
 *   An array with the following possible keys:
 *    [NONE YET SUPPORTED]
 * @param $trait_id
 *   (OPTIONAL) The cvterm_id of the trait.
 */
function ap_get_trait_methods($project_genus, $trait_name, $options = [], $trait_id = NULL) {

  // If we don't have the trait cvterm_id then look it up.
  if (!$trait_id) {

    // Retrieve the CV the traits are stored in.
    $sysvar = ap_get_variablenames(
      array('variablename' => $project_genus),
      array('set' => 'cvdbon', 'suffix' => 'cv')
    );
    $cv_id = variable_get($sysvar);

    // Search for an exact match of the trait name in the expected cv.
    $results = chado_select_record('cvterm', array('cvterm_id'),
      array('cv_id' => $cv_id, 'name' => $trait_name));
    if (isset($results[0]->cvterm_id)) {
      $trait_id = $results[0]->cvterm_id;
    }
    else {
      return FALSE;
    }
  }

  // Now look up all method cvterm_ids for that trait using the relationship.
  $sysvar = ap_get_variablenames(
    array('variablename' => 'method'),
    array('set' => 'terms')
  );
  $type_id = variable_get($sysvar);
  $results = chado_select_record('cvterm_relationship', array('object_id'),
    array(
      'subject_id' => $trait_id,
      'type_id' => $type_id,
    )
  );

  // Now lookup the description for each method found above.
  $methods = array();
  foreach ($results as $r) {
    $method_r = chado_select_record('cvterm', array('cvterm_id', 'definition'),
      array('cvterm_id' => $r->object_id));
    $methods[ $method_r[0]->cvterm_id ] = $method_r[0]->definition;
  }

  return $methods;
}

/**
 * Retrieve the potential units for a given method.
 *
 * @param $project_genus
 *   The organism genus the trait is measured in.
 * @param $method
 *   The cvterm.definition of the method you want potential units for.
 * @param $options
 *   An array with the following possible keys:
 *    - name_only (boolean): returns only the unit cvterm.name.
 * @param $method_id
 *   (OPTIONAL) The cvterm_id of the method.
 */
function ap_get_method_units($project_genus, $method, $options = [], $method_id = NULL) {

  // If we don't have the trait cvterm_id then look it up.
  if (!$method_id) {
    $method_id = ap_get_method_id($project_genus, $method);
  }

  // Now look up all method cvterm_ids for that trait using the relationship.
  $sysvar = ap_get_variablenames(
    array('variablename' => 'unit'),
    array('set' => 'terms')
  );
  $type_id = variable_get($sysvar);
  $results = chado_select_record('cvterm_relationship', array('object_id'),
    array(
      'subject_id' => $method_id,
      'type_id' => $type_id,
    )
  );

  // Now lookup the description for each method found above.
  $units = array();
  foreach ($results as $r) {
    $unit_r = chado_select_record('cvterm', array('cvterm_id', 'name'),
      array('cvterm_id' => $r->object_id));
    $units[ $unit_r[0]->cvterm_id ] = $unit_r[0]->name;
  }

  return $units;

}

/**
 * Retrieve the cvterm_id of the specified method in the correct cv.
 *
 * @param $project_genus
 *   The genus of the project. Used to deteremine the correct cv.
 * @param $method
 *   The full text of the collection method. This is the cvterm.definition.
 * @return
 *   The cvterm_id of the method, if found; false otherwise.
 */
function ap_get_method_id($project_genus, $method) {

    // Retrieve the CV the traits are stored in.
    // Currently we are using the same cv as the trait...
    $sysvar = ap_get_variablenames(
      array('variablename' => $project_genus),
      array('set' => 'cvdbon', 'suffix' => 'method')
    );
    $cv_id = variable_get($sysvar);

    // Search for an exact match of the method cvterm.definition in the expected cv.
    $results = chado_select_record('cvterm', array('cvterm_id'),
      array('cv_id' => $cv_id, 'definition' => $method));
    if (isset($results[0]->cvterm_id)) {
      return $results[0]->cvterm_id;
    }
    else {
      return FALSE;
    }
}

/**
 * Retrieve the cvterm_id of the specified unit in the correct cv.
 *
 * @param $project_genus
 *   The genus of the project. Used to deteremine the correct cv.
 * @param $unit
 *   The full text of the collection unit. This is the cvterm.name.
 * @return
 *   The cvterm_id of the unit, if found; false otherwise.
 */
function ap_get_unit_id($project_genus, $unit) {

    // Retrieve the CV the traits are stored in.
    // Currently we are using the same cv as the trait...
    $sysvar = ap_get_variablenames(
      array('variablename' => $project_genus),
      array('set' => 'cvdbon', 'suffix' => 'unit')
    );
    $cv_id = variable_get($sysvar);

    // Search for an exact match of the unit cvterm.name in the expected cv.
    $results = chado_select_record('cvterm', array('cvterm_id'),
      array('cv_id' => $cv_id, 'name' => trim($unit)));
    if (isset($results[0]->cvterm_id)) {
      return $results[0]->cvterm_id;
    }
    else {
      return FALSE;
    }
}
