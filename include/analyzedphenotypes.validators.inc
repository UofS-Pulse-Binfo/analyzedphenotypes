<?php
/**
 * @file
 * Functions providing validation of analyzed phenotypic data.
 * These functions are currently used when new data is being uploaded.
 */



/**
 *
 */
function analyzedphenotypes_validate_tsv_file($data) {
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
        $case      = $item['case'];
        $parameter = $item['validation arguments'];
        $arguments = $data[$parameter];

        if ($error_flag == 'no_error') {
          // Perform the validator.
          $arr_status[$key] = call_user_func($callback, $case, $arguments);
        }
        else {
          // Error returned, set all other validators to todo.
          $arr_status[$key] = 'todo';
        }

        // When validation failed in any of the validators, stop other validation
        // and tag them as to 'todo' - corresponding css rules will indicate
        // that such validation process was not performed.
        if ($arr_status[$key] == 'failed') {
          $error_flag = 'error_found';
        }

        unset($validator[$key]);

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
    'validation callback' => 'analyzedphenotypes_validator_project',
    'case' => 'specified',
    'validation arguments' => 'project_name',
    'error message' => 'Please ensure to select a project suggested by the Experiment text field before uploading your file.',
  );

  $validators['PROJECT_exists'] = array(
    'label' => 'Experiment exists',
    'scope' => $scope['PROJECT'],
    'validation callback' => 'analyzedphenotypes_validator_project',
    'case' => 'exists',
    'validation arguments' => 'project_name',
    'error message' => '',
  );


  // Genus Validators.
  $validators['GENUS_matched_project'] = array(
    'label' => 'Genus is specified',
    'scope' => $scope['GENUS'],
    'validation callback' => 'analyzedphenotypes_validator_genus',
    'case' => 'specified',
    'validation arguments' => 'project_genus',
    'error message' => '',
  );


  // File validators.
  $validators['FILE_has_right_file_extension'] = array(
    'label' => 'Data file is .tsv or .txt',
    'scope' => $scope['FILE'],
    'validation callback' => 'analyzedphenotypes_validator_datafile',
    'case' => 'extension',
    'validation arguments' => 'data_file',
    'error message' => 'Please ensure that the file you are uploading has .tsv or .txt file extension.',
  );



/*
  $validators['FILE_is_readable'] = array();
  $validators['FILE_has_content_and_tab_separted'] = array();


*/

  return $validators;
}






/**
 *
 */
function analyzedphenotypes_validator_project($case, $project_name) {
  switch($case) {
    case 'specified':
      // See if project name is provided. Extra trim if blank space is provided.
      if (empty(trim($project_name))) {
        return 'failed';
      }

      break;

    case 'exists':
      // See if the name given matched any records in the project table.
      $exists = analyzedphenotypes_project_properties('project_match_name', $project_name);

      if (!$exists) {
        return 'failed';
      }

      break;
  }


  return 'passed';
}



function analyzedphenotypes_validator_genus($case, $project_genus) {
  switch($case) {
    case 'specified':
      // See if project name is provided. Extra trim if blank space is provided.
      if (empty(trim($project_genus))) {
        return 'failed';
      }

      break;
  }


  return 'passed';
}




function analyzedphenotypes_validator_datafile($case, $data_file) {
  switch($case) {
    case 'extension':
      // Array of valid file extension.
      $arr_valid_extension = array('tsv', 'txt');
      // File check - extension should be tsv or txt only.
      $file_extension = pathinfo($data_file->filename, PATHINFO_EXTENSION);

      if (!in_array($file_extension, $arr_valid_extension)) {
        return 'failed';
      }

      break;
  }


  return 'passed';
}








/**
 *
 */
function analyzedphenotypes_validator_generate_message($validator, $error_info) {
  // Array to hold error messages.
  $arr_messages = array();

  if ($validator == 'PROJECT_is_specified') {
    $arr_message = '';
  }

}



