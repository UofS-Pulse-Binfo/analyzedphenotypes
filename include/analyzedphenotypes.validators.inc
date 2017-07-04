<?php
/**
 * @file
 * Functions providing validation of analyzed phenotypic data.
 * These functions are currently used when new data is being uploaded.
 */


/**
 * FUNCTION CALLBACK: Validate dataset containing experiment/project,
 * genus and data file. A set of validator will be performed on each.
 *
 * @param $dataset
 *   An array containing:
 *   1. Experiment/project name entered in the Experiment field.
 *   2. Genus selected or auto filled in Genus option select field.
 *   3. Data file in .tsv or .txt.
 *
 * @return
 *   An array indicating the status of validators plus any additional
 *   information about the error (eg line no., columns).
 *
 * @see
 *   analyzedphenotypes_ap_validators() function below.
 */
function analyzedphenotypes_validate_tsv_file($dataset) {
  // Array to hold validation status.
  $arr_status = array();
  // Array to hold all registered validators.
  $validator = module_invoke_all('ap_validators');
  // Array to hold all scope of validators.
  $scope = analyzedphenotypes_ap_validators('return_all_scope');
  unset($scope['VALUE']);
  // Variable to indicate that a validator has returned a failed status.
  $error_flag = 'no_error';

  // Read each scope. This will ensure that validation is in order of scope.
  // SCOPE.
  foreach($scope as $scope_key => $scope_val) {
    // VALIDATORS.
    foreach($validator as $key => $item) {
      if ($item['scope'] == $scope_val
          && isset($item['validation callback'])
          && function_exists($item['validation callback'])) {

        $callback  = $item['validation callback'];
        $case      = $item['case'];

        if ($error_flag == 'no_error') {
          // Perform the validator.
          $arr_status[$key] = call_user_func($callback, $case, $dataset);
        }
        else {
          // Error returned, set all other validators to todo.
          $arr_status[$key]['type'] = 'todo';
        }

        // When validation failed in any of the validators, stop other validation
        // and tag them as to 'todo' - corresponding css rules will indicate
        // that such validation process was not performed and is pending.
        if ($arr_status[$key]['type'] == 'failed') {
          $error_flag = 'error_found';
        }

        unset($validator[$key]);
      }
    }
    //
  }


  return $arr_status;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/**
 *
 */
function analyzedphenotypes_validate_tsv_data($project_name, $project_genus, $data_file, $job_id) {
  // Frequency for Tripal to print to terminal.
  $report_frequency = 10;
  // Number or 'all' to validate all values and report the error.
  $rows_check = 20;

  // Temporary files
  $tmp_dir = file_directory_temp();
  $file_progress = $tmp_dir . '/ap-validate-job-progress' . $job_id . '.txt';
  $file_status   = $tmp_dir . '/ap-validate-job-status'   . $job_id . '.txt';

  // Validators.
  $arr_status = array();
  $validator  = module_invoke_all('ap_validators');
  $scope = analyzedphenotypes_ap_validators('return_all_scope');
  $scope = $scope['VALUE'];

  // Data file
  $file = file_load($data_file);
  $source = drupal_realpath($file->uri);
  $data_stream = file_get_contents($source);
  $data = explode("\n", $data_stream);
  $data_count = count($data) - 1;
  $columns = str_getcsv($data[0], "\t");

  $error_flag = 'no_error';
  $error_ctr = 0;

  // VALIDATOR - VALUE/DATA SCOPE.
  foreach($validator as $key => $item) {
    if ($error_ctr == $rows_check) {
      $arr_status[$key]['type']  = 'todo';
      continue;
    }

    $callback = (isset($item['validation callback']) && function_exists($item['validation callback']))
      ? $item['validation callback'] : FALSE;

    if ($callback && $item['scope'] == $scope) {
      $col_required = $item['columns required'];
      $case = $item['case'];

      $arr_status[$key]['type']  = 'passed';
      $arr_status[$key]['details'] = '';

      // DATA ROW.
      foreach($data as $i => $row) {
        if ($erro_ctr == $rows_check) {
          break;
        }


        if ($i > 0 && !empty($row)) {

          $cols = str_getcsv($row, "\t");

          ///////
          if (is_array($col_required)) {
            // Apply to all.
            $combine_cols = implode(', ', $col_required);

            foreach($col_required as $c) {
              $j = array_search($c, $columns);

              $value .= $columns[$j];
            }

            // Do validator.
            $result = call_user_func($callback, $case, $value, $combine_cols, $i);

            // Result.
            if ($result['status'] == 'failed') {
              if ($arr_status[$key]['type'] == 'passed') {
                $arr_status[$key]['status'] = 'failed';
              }

              // Group details.
              $arr_status[$key]['details'] .= $result['details'];
              $error_ctr++;
            }
          }
          elseif ($col_required != '') {
            // Apply to specific column.
            $j = array_search($col_required, $columns);
            $value = $cols[$j];

            // Do validator.
            $result = call_user_func($callback, $case, $value, $columns[$j], $i);

            // Result.
            if ($result['status'] == 'failed') {
              if ($arr_status[$key]['type'] == 'passed') {
                $arr_status[$key]['type'] = 'failed';
              }

              // Group details.
              $arr_status[$key]['details'] .= $result['details'];
              $error_ctr++;
            }
          }
          else {
            // Apply to all columns.
            foreach($cols as $j => $value) {
              // Do validator.
              $result = call_user_func($callback, $case, $value, $columns[$j], $i);

              // Result.
              if ($result['status'] == 'failed') {
                if ($arr_status[$key]['type'] == 'passed') {
                  $arr_status[$key]['type'] = 'failed';
                }

                // Group details.
                $arr_status[$key]['details'] .= $result['details'];
                $error_ctr++;
              }
            }
          }
          ///////




        }

      }
      //
    }
  }

  $status_JSON = json_encode($arr_status);
  file_unmanaged_save_data($status_JSON, $file_status, FILE_EXISTS_REPLACE);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * FUNCTION CALLBACK: Register a set of validator to be carried out to individual
 * piece of data collected in the form interface.
 *
 * @param $all_scope
 *  A string, default to null, indicating what the function should return.
 *
 * @return
 *   An array containing either validation scope or validation definitions.
 */
function analyzedphenotypes_ap_validators($all_scope = NULL) {
  // Array to hold validator scopes.
  // This is the order of validation flow.
  $scope = array(
    'PROJECT' => 'project',  // Project scope.
    'GENUS' => 'genus',      // Genus scope.
    'FILE' => 'file',        // Data file scope.
    'VALUE' => 'value',     // Data scope.
  );

  if ($all_scope == 'return_all_scope') {
    // When scope list is required.
    return $scope;
  }

  // Array to hold validations.
  $validators = array();

  // EXPERIMENT/PROJECT.
  $validators['PROJECT_exists'] = array(
    'label' => 'Experiment specified exists',
    'scope' => $scope['PROJECT'],
    'validation callback' => 'analyzedphenotypes_validator_project',
    'case' => 'exists',
    'error message' => 'Please ensure to select a project suggested by the Experiment text field before uploading your file.',
  );

  // GENUS.
  $validators['GENUS_matched_project'] = array(
    'label' => 'Genus specified exists and matched the Experiment',
    'scope' => $scope['GENUS'],
    'validation callback' => 'analyzedphenotypes_validator_genus',
    'case' => 'exists',
    'error message' => '',
  );

  // FILE.
  $validators['FILE_format'] = array(
    'label' => 'Data file is a valid .tsv or .txt file',
    'scope' => $scope['FILE'],
    'validation callback' => 'analyzedphenotypes_validator_datafile',
    'case' => 'format',
    'error message' => 'Please ensure that the file you are uploading has .tsv or .txt file extension and is readable.',
  );

  $validators['FILE_columns'] = array(
    'label' => 'File has expected column headers',
    'scope' => $scope['FILE'],
    'validation callback' => 'analyzedphenotypes_validator_datafile',
    'case' => 'columns',
    'error message' => 'Please ensure that all required columns are present. The following columns are missing: @replace.',
  );

  // VALUE.
  $validators['VALUE_value_provided'] = array(
    'label' => 'All column has value',
    'scope' => $scope['VALUE'],
    'validation callback' => 'analyzedphenotypes_validator_value',
    'case' => 'value_empty',
    'columns required' => '',
    'error message' => 'A column has a blank or 0 value in the following items: @replace',
  );

  $all_columns = analyzedphenotypes_columnsprop('all');
  $trait_name = array_values($all_columns)[0];
  $validators['VALUE_trait_format'] = array(
    'label' => 'Trait name has the correct format',
    'scope' => $scope['VALUE'],
    'validation callback' => 'analyzedphenotypes_validator_value',
    'case' => 'trait_format',
    'columns required' => $trait_name,
    'error message' => 'A value in Trait Name column has incorrect format in items: @replace',
  );

  $validators['VALUE_column_format'] = array(
    'label' => 'Value matched the column data type',
    'scope' => $scope['VALUE'],
    'validation callback' => 'analyzedphenotypes_validator_value',
    'case' => 'match_type',
    'columns required' => '',
    'error message' => 'A value does not match the data type expected in items: @replace',
  );

  $validators['VALUE_value_type'] = array(
    'label' => 'Data was measured using expected units',
    'scope' => $scope['VALUE'],
    'validation callback' => 'analyzedphenotypes_validator_value',
    'case' => 'unit_data',
    'columns required' => '',
    'error message' => 'Data does not match the unit of the trait provided in items: @replace',
  );

  $arr_combine_cols = analyzedphenotypes_columnsprop('combination');
  $validators['VALUE_unique_combo'] = array(
    'label' => 'Column combination is unique',
    'scope' => $scope['VALUE'],
    'validation callback' => 'analyzedphenotypes_validator_value',
    'case' => 'unique_combo',
    'columns required' => $arr_combine_cols,
    'error message' => 'Column combination is not unique in items: @replace',
  );


  return $validators;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/**
 * FUNCTION CALLBACK: Main function to validate experiment/project field.
 * A set of validation process perfomed on EXPERIMENT/PROJECT field.
 *
 * @param $case
 *   A string matching the definition of case in Validator array in validator function.
 *   Each case will correspond to a case block which will perform the requsted of validation process.
 *   @see analyzedphenotypes_ap_validators().
 *
 * @param $dataset
 *   An array containing user supplied values of experiment, genus and data file.
 *
 * @return string
 *   An string passed or failed.
 *
 * @see
 *   analyzedphenotypes_project_properties() in functions.inc.
 */
function analyzedphenotypes_validator_project($case, $dataset) {
  $project_name = $dataset['project_name'];
  $project_name = trim($project_name);

  $status = array(
    'type' => 'passed', // Passed or Failed.
    'details' => '',    // Include more information.
  );

  switch($case) {
    //
    case 'exists':
      // Project in chado.project?
      if (empty($project_name)) {
        $status['type'] = 'failed';
      }
      else {
        // See if the name given matched any records in the project table.
        $exists = analyzedphenotypes_projectprop('project_match_name', $project_name);

        if (!$exists) {
          $status['type'] = 'failed';
        }
      }

      break;


    // Define case here for additional validation on EXPERIMENT/PROJECT.
  }


  return $status;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * FUNCTION CALLBACK: main function to validate genus field.
 * A set of validation process performed on GENUS field.
 *
 * @param $case
 *   A string matching the definition of case in Validator array in validator function.
 *   Each case will correspond to a case block which will perform the requsted of validation process.
 *   @see analyzedphenotypes_ap_validators().
 *
 * @param $dataset
 *   An array containing user supplied values of experiment, genus and data file.
 *
 * @return string
 *   An string passed or failed.
 *
 * @see
 *   analyzedphenotypes_projectprop() in functions.inc.
 */
function analyzedphenotypes_validator_genus($case, $dataset) {
  $project_genus = $dataset['project_genus'];

  $project_name = $dataset['project_name'];
  $project_name = trim($project_name);

  $status = array(
    'type' => 'passed', // Passed or Failed. Default to passed.
    'details' => '',    // Include more information.
  );

  switch($case) {
    //
    case 'exists':
      // Genus specified and match the project.
      if ($project_genus == '0') {
        $status['type'] = 'failed';
      }
      else {
        // Genus matched project genus.
        $project_id = analyzedphenotypes_projectprop('project_match_name', $project_name);

        if ($project_id) {
          $exists = analyzedphenotypes_projectprop('project_genus', $project_id);

          if ($exists) {
            $genus = array_values($exists)[0];

            if ($project_genus != $genus) {
              $status['type'] = 'failed';
            }
          }
        }
      }

      break;


    // Define case here for additional validation on GENUS.
  }


  return $status;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * FUNCTION CALLBACK: main function to validate data file.
 * A set of validation process performed on DATA FILE field.
 *
 * @param $case
 *   A string matching the definition of case in Validator array in validator function.
 *   Each case will correspond to a case block which will perform the requsted of validation process.
 *   @see analyzedphenotypes_ap_validators().
 *
 * @param $dataset
 *   An array containing user supplied values of experiment, genus and data file.
 *
 * @return string
 *   An string passed or failed.
 *
 * @see
 *   analyzedphenotypes_datafile_read() in functions.inc
 */
function analyzedphenotypes_validator_datafile($case, $dataset) {
  $data_file = $dataset['data_file'];

  $status = array(
    'type' => 'passed', // Passed or Failed. Default to passed.
    'details' => '',    // Include more information.
  );

  switch($case) {
    //
    case 'format':
      // TSV or TXT?
      $arr_valid_extension = array('tsv', 'txt');
      $file_extension = pathinfo($data_file->filename, PATHINFO_EXTENSION);

      if (!in_array($file_extension, $arr_valid_extension)) {
        $status['type'] = 'failed';
      }
      else {
        // Readable?
        $readable = analyzedphenotypes_fileprop('readable', $data_file);

        if (!$readable) {
          $status['type'] = 'failed';
        }
      }

      break;

    //
    case 'columns':
      // All columns in?
      $columns = analyzedphenotypes_fileprop('columns', $data_file);
      $expected = analyzedphenotypes_columnsprop('expected');

      $expected_noformat = array_map('analyzedphenotypes_noformat', $expected);
      $columns_noformat = array_map('analyzedphenotypes_noformat', $columns);

      $missing = array_diff($expected_noformat, $columns_noformat);

      if (count($missing) > 0) {
        $status['type'] = 'failed';

        $missing = array_map('ucwords', $missing);
        $status['details'] = implode(', ', $missing) . ' in line #1';
      }

      break;


    // Define case here for additional validation on EXPERIMENT/PROJECT.
  }


  return $status;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * FUNCTION CALLBACK: main function to validate data file.
 * A set of validation process performed on DATA FILE field.
 *
 * @param $case
 *   A string matching the definition of case in Validator array in validator function.
 *   Each case will correspond to a case block which will perform the requsted of validation process.
 *   @see analyzedphenotypes_ap_validators().
 *
 * @param $dataset
 *   An array containing user supplied values of experiment, genus and data file.
 *
 * @return string
 *   An string passed or failed.
 *
 * @see
 *   analyzedphenotypes_datafile_read() in functions.inc
 */
function analyzedphenotypes_validator_value($case, $value, $column, $row) {
  $value = trim($value);
  $collumn = trim($column);

  $status = array(
    'type' => 'passed', // Passed or Failed. Default to passed.
    'details' => '',    // Include more information.
  );

  switch($case) {
    //
    case 'value_empty':
      // Check the if value is empty.
      if (empty($value)) {
        $status['type'] = 'failed';
        $status['details'] = ' in ' . $column . ' line #' . $row;
      }

      break;

    //
    case 'match_type':
      // Check if the data in a column match the data type required.
      if (!empty($value)) {
        $data_type = analyzedphenotypes_columnsprop('data_type');
        $column_data_type = $data_type[$column];

        unset($data_type);

        switch($column_data_type) {
          //
          case 'text':
            if (ctype_digit($value)) {
              $status['type'] = 'failed';
              $status['details'] = ' in ' . $column . ' line #' . $row . ' (' . $value . ' not a text value)';
            }

            break;

          //
          case 'number':
            if (!ctype_digit($value) || (int)$value < 0) {
              $status['type'] = 'failed';
              $status['details'] = ' in ' . $column . ' line #' . $row . ' (' . $value . ' not a number or a negative value)';
            }

            break;
        }
      }

      break;

      //
      case 'trait_format':
        // Check if trait name follows expected trait name format.
        if (!empty($value)) {
          if (preg_match('/\A[^()]+\s*\({1}[^)(]+\)\z/i', $value) !== 1) {
            $status['type'] = 'failed';
            $status['details'] = ' in ' . $column . ' line #' . $row . ' (' . $value . ' invalid trait format)';
          }
        }

        break;

      //
      case 'unit_data':
        // Check if trait unit match the data.


        break;

      //
      case 'unique_combo':

        break;

      //
  }


  return $status;
}
