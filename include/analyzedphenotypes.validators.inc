<?php
/**
 * @file
 * Functions providing validation of analyzed phenotypic data.
 * These functions are currently used when new data is being uploaded.
 */



/**
 *
 */
function analyzedphenotypes_validate_tsv_file($project_name, $genus, $file) {
  // Array to hold validation status.
  $arr_status = array();
  // Array to hold all registered validators.
  $validator = module_invoke_all('ap_validators');
  // Array to hold all scope of validators.
  $scope = analyzedphenotypes_ap_validators('return_all_scope');
  // Variable to indicate that a validator has returned a failed status.
  $error_flag = 'no_error';

  // Read each scope. This will ensure that validation is in order of scope.
  foreach($scope as $scope_key => $scope_val) {

    // VALIDATORS.
    foreach($validator as $key => $item) {
      if ($item['scope'] == $scope_val
          && isset($item['validation callback'])
          && function_exists($item['validation callback'])) {

        $callback  = $item['validation callback'];
        $parameter = $item['arguments'];

        if ($error_flag == 'no_error') {
          // Perform the validator.
          $arr_status[$key] = call_user_func($callback, $parameter, $project_name);
        }
        else {
          // Error returned, set all other validators to todo.
          $arr_status[$key] = 'todo';
        }

        // When validation failed in any of the validators, stop other validation
        // and tag all the rest to 'todo' - corresponding css rules will indicate
        // that such validation process was not performed.
        if ($arr_status[$key] === FALSE) {
          $error_flag = 1;
        }

      }
    }
    //

  }



  return $arr_status;
}







/**
 *
 */
function analyzedphenotypes_ap_validators($all_scope = NULL) {
  // Array to hold validator scopes.
  // This is the order of validation flow.
  $scope = array(
    'PROJECT' => 'project',
    'GENUS' => 'genus',
    'FILE' => 'file',
  );

  if ($all_scope == 'return_all_scope') {
    // When scope list is required.
    return $scope;
  }

  // Array to hold validations.
  $validators = array();

  // Project validators.
  $validators['PROJECT_is_specified'] = array(
    'label' => 'Experiment is specified',
    'scope' => $scope['PROJECT'],
    'message callback' => NULL,
    'validation callback' => 'analyzedphenotypes_validator_project',
    'arguments' => 'specified'
  );

  $validators['PROJECT_exists'] = array(
    'label' => 'Experiment exists',
    'scope' => $scope['PROJECT'],
    'message callback' => NULL,
    'validation callback' => 'analyzedphenotypes_validator_project',
    'arguments' => 'exists'
  );






/*

  // Genus Validators.
  $validators['GENUS_matched_project'] = array(
    'label' => 'Genus matched project genus',
    'scope' => $scope['GENUS'],
    'message callback' => NULL,
    'validation callback' => 'analyzedphenotypes_validator_genus',
    'arguments' => 'match'
  );


  // File validators.
  $validators['FILE_has_right_extension'] = array();
  $validators['FILE_is_readable'] = array();
  $validators['FILE_has_content_and_tab_separted'] = array();


*/

  return $validators;
}






/**
 *
 */
function analyzedphenotypes_validator_project($validator, $project_name) {
  switch($validator) {
    case 'specified':
      if (empty(trim($project_name))) {
        return FALSE;
      }

      break;

    case 'exists':
      // See if the name given matched any records in the project table.
      $exists = analyzedphenotypes_project_properties('project_match_name', $project_name);

      if (!$exists) {
        return FALSE;
      }

      break;
  }

  return TRUE;
}






