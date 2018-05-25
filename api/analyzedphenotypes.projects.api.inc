<?php
/**
 * @file
 *
 * API: PROJECT/EXPERIMENT PROPERTY
 * Manage property and operation pertaining to data in chado.project table.
 */


/**
 * DATA SOURCE - chado.project.
 * Fetch all records in chado.project as an associative array
 * where the key is project id number and the value is project name.
 * Result is sorted by name alphabetically.
 *
 * @return
 *   Array, all project records in chado.project as an associative array.
 *   0 if no records found.
 */
function ap_get_projects() {
  // Fetch all project/experiment records in chado.project.
  $result = chado_query(
    "SELECT project_id, TRIM(name) AS name FROM {project} ORDER BY name ASC"
  );

  return ($result->rowCount()) ? $result->fetchAllKeyed() : 0;
}


///////


/**
 * CONTROL/SWITCHBOARD.
 * Pass a request to a corresponding function.
 * @TODO: leave out when api reorganize/update is complete and use functions below.
 */
function analyzedphenotypes_projectprop($property = null, $parameter = null) {
  if (is_null($property)) {
    // Default.
    $property = 'default';
  }

  // Process request.
  // Default to all projects.
  $command = array(
    'default' => 'ap_get_projects',
    'match_id' => 'ap_match_projectid',
    'match_key' => 'ap_match_projectnamekey',
    'match_name' => 'ap_match_projectname',
  );

  return call_user_func($command[$property], $parameter);
}


///////


/**
 * FUNCTION:
 * Fetch record in chado.project that matches experiment id number.
 * Dependencies: ap_get_projects().
 *
 * @param $identifiers
 *   An array with the following keys:
 *     - project_id : project id number.
 * @param $options
 *   An array to be used as additional options to be performed to
 *   returned result.
 *
 * @return
 *   An array containing the project id number and project name.
 */
function ap_match_projectid($identifiers, $options = array()) {
  $all_projects = ap_get_projects();
  $projectprop = null;

  if (!is_array($all_projects)) {
    return $projectprop;
  }

  foreach($all_projects as $id => $name) {
    if ($id == trim($identifiers['project_id'])) {
      // Match found
      $projectprop = array(
        'project_id' => $id,
        'project_name' => $name,
      );

      break;
    }
  }

  return $projectprop;
}

/**
 * FUNCTION:
 * Fetch all records in chado.project that match a keyword.
 * Dependencies: ap_get_projects().
 *
 * @param $identifiers
 *   An array with the following keys:
 *    - project_keyword : a string, keyword entered.
 * @param $options
 *   An array to be used as additional options to be performed to
 *   returned result.
 *   - limitrows : limit the number of rows returned. Default to 10.
 *
 * @return
 *   An array with project_id number as the key and project name as value.
 */
function ap_match_projectnamekey($identifiers, $options = array('limitrows' => 10)) {
  $all_projects = ap_get_projects();
  $projectprop = null;

  if (!is_array($all_projects)) {
    return $projectprop;
  }

  $i = 0;
  foreach($all_projects as $id => $name) {
    if (($i + 1) > $options['limitrows']) {
      // Limit rows so it does not suggest a lengthy list.
      break;
    }

    if (stripos($name, trim($identifiers['project_keyword'])) !== FALSE) {
      // Match found.
      // Project has the keyword, save it.
      $projectprop[$id] = $name;
    }

    $i++;
  }

  return $projectprop;
}

/**
 * FUNCTION:
 * Fetch record in chado.project that matches experiment name or title.
 * Dependencies: ap_get_projects().
 *
 * @param $identifiers
 *   An array with the following keys:
 *    - project_name : a string, project name or title.
 * @param $options
 *   An array to be used as additional options to be performed to
 *   returned result.
 *
 * @return
 *   An array containing the project id number and project name.
 */
function ap_match_projectname($identifiers, $options = array()) {
  $all_projects = ap_get_projects();
  $projectprop = null;

  if (!is_array($all_projects)) {
    return $projectprop;
  }

  foreach($all_projects as $id => $name) {
    if (strtolower($name) == strtolower(trim($identifiers['project_name']))) {
      // Match found.
      $projectprop = array(
        'project_id' => $id,
        'project_name' => $name,
      );

      break;
    }
  }

  return $projectprop;
}
