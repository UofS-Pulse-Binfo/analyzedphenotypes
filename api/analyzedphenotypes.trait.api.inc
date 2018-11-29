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
 *
 * @return
 *   An array with the following keys where each value is the new cvterm:
 *   trait, method, unit.
 */
function ap_insert_trait($data) {

  // @debug dpm($data, 'inserting trait via new api function');

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

  // Insert the trait cvterm
  $trait_cvterm = array(
    'cv_name' => $cv_name,
    'id' => trim($db_name) . ':' . $data['name'],
    'name' => $data['name'],
    'definition' => $data['description'],
  );
  $trait_cvterm = tripal_insert_cvterm($trait_cvterm);

  // Insert the method cvterm
  $method_cvterm = array(
    'cv_name' => $cv_name, // @todo we shouldnt use the same cv... @config-needed
    'id' => trim($db_name) . ':' . $data['method_title'],
    'name' => $data['method_title'],
    'definition' => $data['method'],
  );
  $method_cvterm = tripal_insert_cvterm($method_cvterm);

  // Insert the unit cvterm
  $unit_cvterm = array(
    'cv_name' => $cv_name, // @todo we shouldnt use the same cv... @config-needed
    'id' => trim($db_name) . ':' . $data['unit'],
    'name' => $data['unit'],
    'definition' => '',
  );
  $unit_cvterm = tripal_insert_cvterm($unit_cvterm);

  // Relate Trait => Method
  chado_insert_record('cvterm_relationship', array(
    'subject_id' => $trait_cvterm->cvterm_id,
    'type_id' => array(
      'cv_id' => array('name' => 'NCIT'), // @todo @config-needed
      'name' => 'method',
    ),
    'object_id' => $method_cvterm->cvterm_id,
  ));

  // Relate Method => Unit
  chado_insert_record('cvterm_relationship', array(
    'subject_id' => $method_cvterm->cvterm_id,
    'type_id' => array(
      'cv_id' => array('name' => 'uo'), // @todo @config-needed
      'name' => 'unit',
    ),
    'object_id' => $unit_cvterm->cvterm_id,
  ));

  return array(
    'trait' => $trait_cvterm,
    'method' => $method_cvterm,
    'unit' => $unit_cvterm,
  );
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
  $results = chado_select_record('cvterm_relationship', array('object_id'),
    array(
      'subject_id' => $trait_id,
      'type_id' => array(
        'cv_id' => array('name' => 'NCIT'), // @todo @config-needed
        'name' => 'method',
      ),
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

    // Retrieve the CV the traits are stored in.
    // Currently we are using the same cv as the trait...
    // @todo @config-needed
    $sysvar = ap_get_variablenames(
      array('variablename' => $project_genus),
      array('set' => 'cvdbon', 'suffix' => 'cv')
    );
    $cv_id = variable_get($sysvar);

    // Search for an exact match of the method cvterm.definition in the expected cv.
    $results = chado_select_record('cvterm', array('cvterm_id'),
      array('cv_id' => $cv_id, 'definition' => $method));
    if (isset($results[0]->cvterm_id)) {
      $method_id = $results[0]->cvterm_id;
    }
    else {
      return FALSE;
    }
  }

  // Now look up all method cvterm_ids for that trait using the relationship.
  $results = chado_select_record('cvterm_relationship', array('object_id'),
    array(
      'subject_id' => $method_id,
      'type_id' => array(
        'cv_id' => array('name' => 'uo'), // @todo @config-needed
        'name' => 'unit',
      ),
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