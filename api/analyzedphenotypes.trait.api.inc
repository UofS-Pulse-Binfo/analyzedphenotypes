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
  $trait_cvterm = [
    'cv_name' => $cv_name,
    'id' => trim($db_name) . ':' . $data['name'],
    'name' => $data['name'],
    'definition' => $data['description'],
  ];
  $trait_cvterm = tripal_insert_cvterm($trait_cvterm);

  // Insert the method cvterm
  $method_cvterm = [
    'cv_name' => $cv_name, // @todo we shouldnt use the same cv...
    'id' => trim($db_name) . ':' . $data['method_title'],
    'name' => $data['method_title'],
    'definition' => $data['method'],
  ];
  $method_cvterm = tripal_insert_cvterm($method_cvterm);

  // Insert the unit cvterm
  $unit_cvterm = [
    'cv_name' => $cv_name, // @todo we shouldnt use the same cv...
    'id' => trim($db_name) . ':' . $data['unit'],
    'name' => $data['unit'],
    'definition' => '',
  ];
  $unit_cvterm = tripal_insert_cvterm($unit_cvterm);

  // Relate Trait => Method
  chado_insert_record('cvterm_relationship', [
    'subject_id' => $trait_cvterm->cvterm_id,
    'type_id' => [
      'cv_id' => ['name' => 'NCIT'],
      'name' => 'method',
    ],
    'object_id' => $method_cvterm->cvterm_id,
  ]);

  // Relate Method => Unit
  chado_insert_record('cvterm_relationship', [
    'subject_id' => $method_cvterm->cvterm_id,
    'type_id' => [
      'cv_id' => ['name' => 'uo'],
      'name' => 'unit',
    ],
    'object_id' => $unit_cvterm->cvterm_id,
  ]);

  return [
    'trait' => $trait_cvterm,
    'method' => $method_cvterm,
    'unit' => $unit_cvterm,
  ];
}
