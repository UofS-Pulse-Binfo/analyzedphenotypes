<?php
/**
 * @file
 * PROJECTS/EXPERIMENT API.
 */


/**
 * API: PROJECT/EXPERIMENT PROPERTY
 * Manage property and operation pertaining to data in chado.project table.
 * Default to: return all project as an associative array where key is project_id and value is project name.
 * Dependencies: analyzedphenotypes_projectprop_matchid(), analyzedphenotypes_projectprop_matchkey(), analyzedphenotypes_projectprop_matchname().
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - match_key  : Fetch all project records that match a keyword.
 *   - match_name : Fetch project record that matches a project name.
 *   - match_id   : Fetch project record that matches a project id.
 *   - default    : Fetch all project records.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - project_id      : project id number.
 *   - project_name    : project title or name.
 *   - project_keyword : keywords that form project title or name.
 *
 * @return Array $projectprop
 *   An associative array where the key is project_id and value is project_name;
 *
 * @see analyzedphenotypes_validator_project() in validators.inc.
 * @see analyzedphenotypes_save_tsv_data() in validator.inc.
 * @see analyzedphenotypes_data_json() in .module.
 */
function analyzedphenotypes_projectprop($property = null, $parameter = null) {
  // Update: used in data downloader form.
  $projectprop = null;

  // Fetch all project/experiment records in chado.project.
  $result = chado_query(
    "SELECT project_id, TRIM(name) AS name FROM {project} ORDER BY name ASC"
  );

  // No result.
  if ($result->rowCount() <= 0) {
    // 0 project in the system.
    return $projectprop;
  }

  $all_project = $result->fetchAllKeyed();

  // Default.
  if (is_null($property)) {
    // Return all projects.
    $projectprop = $all_project;
  }
  else {
    // Add result for matching.
    $parameter['all_projects'] = $all_project;

    // Process request.
    $command = array(
      'match_id' => 'ap_projectprop_matchid',
      'match_key' => 'ap_projectprop_matchkey',
      'match_name' => 'ap_projectprop_matchname',
    );

    $projectprop = call_user_func($command[$property], $parameter);
  }


  return $projectprop;
}


///////


/**
 * Fetch record in chado.project that matches experiment id number.
 * Dependencies: analyzedphenotypes_projectprop().
 *
 * $param $parameter
 *   An array with the following keys:
 *     - all_project : all project records in chado.project.
 *     - project_id : project id number.
 *
 * @return
 *   An array containing the project id number and project name.
 */
function ap_projectprop_matchid($parameter) {
  $projectprop = null;

  foreach($parameter['all_projects'] as $id => $name) {
    if ($id == trim($parameter['project_id'])) {
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
 * Fetch all records in chado.project that match a keyword.
 * Dependencies: analyzedphenotypes_projectprop().
 *
 * @param $parameter
 *   An array with the following keys:
 *    - all_project : all project records in chado.project.
 *    - project_keyword : a string, keyword entered.
 *
 * @return
 *   An array with project_id number as the key and project name as value.
 */
function ap_projectprop_matchkey($parameter) {
  $projectprop = null;

  // To ensure list will not grow longer, limit the result to x.
  $projectprop_limitrows = 10;

  $i = 0;
  foreach($parameter['all_projects'] as $id => $name) {
    if (($i + 1) > $projectprop_limitrows) {
      break;
    }

    if (stripos($name, trim($parameter['project_keyword'])) !== FALSE) {
      // Match found.
      // Project has the keyword, save it.
      $projectprop[$id] = $name;
    }

    $i++;
  }

  return $projectprop;
}

/**
 * Fetch record in chado.project that matches experiment name or title.
 * Dependencies: analyzedphenotypes_projectprop().
 *
 * @param $parameter
 *   An array with the following keys:
 *    - project_name : a string, project name or title.
 *
 * @return
 *   An array containing the project id number and project name.
 */
function ap_projectprop_matchname($parameter) {
  $projectprop = null;

  foreach($parameter['all_projects'] as $id => $name) {
    if (strtolower($name) == strtolower(trim($parameter['project_name']))) {
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
