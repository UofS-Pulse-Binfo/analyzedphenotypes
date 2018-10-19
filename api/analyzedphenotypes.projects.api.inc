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
    // No project records.
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
 * Fetch all records in chado.project that match a keyword (pattern search) or project name (full search).
 * Dependencies: ap_get_projects().
 *
 * @param $identifiers
 *   An array with the following keys:
 *    - key  : a string, keyword or project name entered.
 *    - name : project name/title.
 * @param $options
 *   An array to be used as additional options to be performed to
 *   returned result.
 *   - fullmatch : flag if search is a pattern or full search. Defaul to TRUE - a fullmatch.
 *   - limitrows : limit the number of rows returned. Default to 1.
 *
 * @return
 *   An array with project_id number as the key and project name as value.
 */
function ap_match_projectname($identifiers, $options = array('fullmatch' => TRUE, 'limitrows' => 1)) {
  $all_projects = ap_get_projects();
  $projectprop = null;

  if (!is_array($all_projects)) {
    // No project records.
    return $projectprop;
  }

  $i = 0;

  foreach($all_projects as $id => $name) {
    if ($options['fullmatch']) {
      // Fullmatch search.
      if (strtolower($name) == strtolower(trim($identifiers['name']))) {
        // Match found.
        // Return record.
        $projectprop = array(
          'project_id' => $id,
          'project_name' => $name,
        );

        $i++;
      }
    }
    else {
      // Pattern match search.
      if (stripos($name, trim($identifiers['key'])) !== FALSE) {
        // Match found.
        // Project has the keyword, save it.
        $projectprop[$id] = $name;

        $i++;
      }
    }

    // Compare limit rows if search had to stop.
    if ($i >= $options['limitrows']) {
      break;
    }
  }


  return $projectprop;
}