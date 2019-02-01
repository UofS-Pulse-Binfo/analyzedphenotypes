<?php
/**
 * @file
 *
 * API: SYSTEM VARIABLES.
 * Manage property and operation pertaining to system variables.
 */



/**
 * DATA SOURCE
 * Define system variable names.
 *
 * @return
 *   An array of strings, constructed variable names grouped by variable type (set).
 */
function ap_construct_variablenames() {
  // Group variable names (types/set).
  $var_set = array(
    // Allow new header to be added during upload.
    'options' => 'options',
    // Controlled vocabulary (cv), database (db) etc.
    'cvdbon' => 'cvdbon',
    // Controlled vocabulary terms etc.
    'terms' => 'terms',

    // Define new set of variable names here and define items below.
    //
  );

  // Use this to prefix all system variables to prevent any unexpected
  // collision with other module names.
  $var_prefix = 'analyzedphenotypes_systemvar_';

  // System variable names.
  // NOTE: each item must be in lowercase and spaces should be replaced with underscore.
  $var_suffix = array(
    $var_set['options'] => array('allownew'),
    $var_set['cvdbon'] => array('cv', 'db', 'ontology', 'method', 'unit'),
    $var_set['terms'] => array('genus', 'year', 'method', 'related', 'location', 'replicate', 'collector')

    // To register system variables in a set, add here.
    //
  );

  // #ANATOMY OF VARIABLE:
  // var_prefix  + var_suffix [var_suffix for genus is genus + var_suffix]
  // eg. analyzedphenotypes_systemvar_method
  //     analyzedphenotypes_systemvar_lens_cv or analyzedphenotypes_systemvar_lens_db

  // Array to hold all system variable names.
  $systemvars = array();

  // Construct name.
  foreach($var_suffix as $set => $suffix) {
    if ($set == $var_set['cvdbon']) {
      // This set is linked to each genus such that
      // a genus would have a cv, db and on (ontology) variable name.
      // eg. LENS: cv - analyzedphenotypes_systemvar_lens_cv.
      //           method - analyzedphenotypes_systemvar_lens_method.
      //           unit - analyzedphenotypes_systemvar_lens_unit.
      //           db - analyzedphenotypes_systemvar_lens_db.
      //           on - analyzedphenotypes_systemvar_lens_ontology.
      $result = chado_query(
        "SELECT genus FROM {organism} GROUP BY genus ORDER BY genus ASC"
      )
      ->fetchCol();

      foreach($result as $genus) {
        $genus = strtolower(preg_replace('/\s+/', '_', $genus));

        foreach($suffix as $term) {
          // cv, db and on.
          // Calling 'ap_ + genus' = ap_lens returns Genus lens cv, db and ontology variables.
          $systemvars[$set]['ap_' . $genus][$term] = $var_prefix . $genus . '_' . $term;
        }
      }
    }
    else {
      // Other terms.
      // eg. For term METHOD: analyzedphenotypes_systemvar_method.
      foreach($suffix as $term) {
        $systemvars[$set]['ap_' . $term] = $var_prefix . $term;
      }
    }
  }


  return $systemvars;
}



/**
 * FUNCTION:
 * Fetch system variable name.
 * Dependencies: ap_define_variablenames().
 *
 * @param $identifiers
 *   An array with the following keys:
 *     - variablename : term, option or genus keyword unique to a system variable.
 * @param $options
 *   An array to be used as additional options to be performed to returned result.
 *     - set : the set a term belongs to. Default to CV, DB and ON (ontology) set.
 *       varset - group of variables in a set.
 *       terms - controlled vocabulary terms method, year etc.
 *       cvdbon - for controlled vocabulary, database and ontology configuration.
 *       options - module options such as allow new traits to be added.
 *     - suffix : when requesting variables for a genus, this key holds value for cv, database or ontology settings.
 *       allsuffix - request cv, db and ontology variable set for a genus.
 *       cv - controlled vocabulary configuration only of a genus.
 *       db - database configuration only  of a genus.
 *       on - ontology configuration only of a genus.
 *
 * @return
 *   A String, variable name or an array a set of variable names.
 */
function ap_get_variablenames($identifiers, $options = array('set' => 'cvdbon', 'suffix' => 'cv')) {
  $systemvars = null;

  // Sanitize variable name requested.
  // This can be a genus (eg. lens, cicer), a term (eg. method) or an option (eg. allownew).
  $variablename = strtolower(preg_replace('/\s+/', '_', $identifiers['variablename']));

  // All defined system variable names.
  $all_vars = ap_construct_variablenames();

  if ($variablename == 'varset') {
    // All system variables in a set requested.
    $systemvars = $all_vars[ $options['set'] ];
  }
  else {
    // Other type of request.

    // Limit to the set requested.
    $var_set = $all_vars[ $options['set'] ];
    // No need for other sets.
    unset($all_vars);

    if ($options['set'] == 'cvdbon') {
      // Genus based system variables.
      // All suffix, return cv, db and otology of a genus otherwise,
      // return specific suffix requested.

      $systemvars = ($options['suffix'] == 'allsuffix')
        ? $var_set[ 'ap_' . $variablename ]
        : $var_set[ 'ap_' . $variablename ][ $options['suffix'] ];
    }
    else {
      // Other system variables.
      $systemvars = $var_set[ 'ap_' . $variablename ];
    }
  }


  return $systemvars;
}
