<?php
/**
 * @file
 *
 * API: GENUS.
 * Manage property and operation pertaining to data (Genus) in chado.organism table.
 */


/**
 * DATA SOURCE - chado.organism.
 * Fetch all genus in chado.organism.
 * Result is sorted by genus alphabetically.
 *
 * @return
 *   Array, all unique genus.
 *   0 if no records found.
 */
function ap_get_genus() {
  // All genus.
  $result = chado_query(
    "SELECT genus FROM {organism} GROUP BY genus ORDER BY genus ASC"
  );

  return ($result->rowCount()) ? $result->fetchCol() : 0;
}


///////


/**
 * CONTROL/SWITCHBOARD.
 * Pass a request to a corresponding function.
 * @TODO: leave out when api reorganize/update is complete and use functions below.
 */
function analyzedphenotypes_genusprop($property = null, $parameter = null) {
  if (is_null($property)) {
    // Default.
    $property = 'default';
  }

  // Process request.
  // Default to all projects.
  $command = array(
    'default' => array(
      'func'  => 'ap_get_genus',
      'param' => null,
      'opts'  => null,
    ),
    'project_genus' => array(
      'func'  => 'ap_get_projectgenus',
      'param' => $parameter,
      'opts'  => null,
    ),
    'assign_genus' => array(
      'func'  => 'ap_set_projectgenus',
      'param' => array('project_id' => $parameter['project_id'], 'genus' => $parameter['genus']),
      'opts'  => null,
    ),
    'sysvar_genus_set' => array(
      'func'  => 'ap_get_activegenus',
      'param' => null,
      'opts'  => null,
    ),
  );

  // Function name, parameter (project name key or name), additional options.
  return call_user_func($command[$property]['func'], $command[$property]['param'], $command[$property]['opts']);
}


///////


/**
 * FUNCTION:
 * Assign a genus to an experiment/project in chado.projectprop table.
 * Dependencies: ap_get_projectgenus(), ap_get_variablenames() - systemvars.api.
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - project_id : project id number, to which the genus will be linked to.
 *   - genus : genus.
 * @param $options
 *   An array to be used as additional options to be performed to
 *   returned result. Keys:
 *   - replace : Flag request to replace/overwrite existing project genus. Default to FALSE.
 */
function ap_set_projectgenus($identifiers, $options = array('replace' => FALSE)) {
  // Fetch the configuration for the term Genus from terms system variable set.
  $sysvar = ap_get_variablenames(
    array('variablename' => 'genus'),
    array('set' => 'terms')
  );

  $sysvar_genus = variable_get($sysvar);

  // Before relating genus to a project, test for any previous correlations.
  $projectgenus = ap_get_projectgenus(
    array('project_id' => $identifiers['project_id'])
  );

  $values = array(
    'project_id' => $identifiers['project_id'],
    'type_id' => $sysvar_genus,
    'value' => $identifiers['genus']
  );

  if ($projectgenus && $options['replace']) {
    // Has genus, replace current project genus when implied in the options.
    // No value key.
    $umatch = array_slice($values, 0, 2);
    // Just value key.
    $uvalue = array_slice($values, 2, 1);
    chado_update_record('projectprop', $umatch, $uvalue);
  }
  else {
    // When project genus matches, do nothing. Only when 0 record found we create a relationship.
    // Regardless of replace option.

    if (!$projectgenus) {
      // No past relations made. Set genus to project.
      chado_insert_record('projectprop', $values);
    }
  }
}

/**
 * FUNCTION:
 * Fetch genus assigned to an experiment/project in chado.projectprop table.
 * Dependencies: ap_get_variablenames() - systemvars.api.
 *
 * @param $identifiers
 *   An array, with either one of the following keys:
 *     - project_id : project id number
 *     - project name : project name/title.
 *
 *  @return
 *    A string, genus of an experiment/project.
 */
function ap_get_projectgenus($identifiers) {
  // Fetch the configuration for the term Genus from terms system variable set.
  $sysvar = ap_get_variablenames(
    array('variablename' => 'genus'),
    array('set' => 'terms')
  );

  $sysvar_genus = variable_get($sysvar);

  // What is provided? project id or project name?
  if (isset($identifiers['project_id'])) {
    $sql = "SELECT value FROM {projectprop} WHERE type_id = :type_id AND project_id = :value LIMIT 1";
    $field = 'project_id';
  }
  else {
    $sql = "SELECT projectprop.value FROM {projectprop} INNER JOIN {project} USING (project_id)
      WHERE projectprop.type_id = :type_id AND project.name = :value LIMIT 1";
    $field = 'project_name';
  }

  $result = chado_query($sql,
    array(
      ':type_id' => $sysvar_genus,
      ':value' => $identifiers[$field],
    )
  );

  return ($result->rowCount() == 1) ? $result->fetchField() : 0;
}

/**
 * FUNCTION:
 * Fetch all genus with controlled vocabulary (cv) and database (db) configuration set.
 * This function will limit genus suggested in select genus field to only those genus with
 * properly configured values.
 * Dependencies: ap_get_genus(), ap_get_variablenames() - systemvars.api.
 *
 * @return
 *   An array, list of configured genus.
 */
function ap_get_activegenus() {
  // All genus.
  $genus = ap_get_genus();
  // Array to hold actuve system variables.
  $sysvar_genus_set = array();

  // Inspect each genus and see if cv and db were set.
  foreach($genus as $g) {
    $sysvar_genus = ap_get_variablenames(
      array('variablename' => $g),
      array('set' => 'cvdbon', 'suffix' => 'allsuffix')
    );

    $sysvar_genus_cv = variable_get($sysvar_genus['cv']);
    $sysvar_genus_db = variable_get($sysvar_genus['db']);

    if ($sysvar_genus_cv > 0 && $sysvar_genus_db > 0) {
      // Save those with cv and db conf.
      $sysvar_genus_set[] = ucfirst($g);
    }
  }

  return (count($sysvar_genus_set)) ? $sysvar_genus_set : 0;
}
