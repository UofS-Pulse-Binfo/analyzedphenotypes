<?php
/**
 * @file
 * Functions providing validation of analyzed phenotypic data.
 * These functions are currently used when new data is being uploaded.
 */

/**
 * VALIDATOR DEFINITIONS
 * List of validators performed to user data file.
 * Default to: Return all scope of validation.
 * Dependencies: analyzedphenotypes_systemvars().
 *
 * @param String $all_scope
 *  A string value indicating a request to return all validator scope values.
 *
 * @return Array
 *   Array of validator definition or validator scope values.
 *
 * @see analyzedphenotypes_validate_tsv_file().
 */
function analyzedphenotypes_ap_validators($all_scope = NULL) {
  $scope = array(
    'PROJECT' => 'project',  // Project scope.
    'GENUS' => 'genus',      // Genus scope.
    'FILE' => 'file',        // Data file scope.
    'VALUE' => 'value',      // Data scope.
  );

  if ($all_scope == 'return_all_scope') {
    return $scope;
  }

  $validators = array();

  // EXPERIMENT/PROJECT.
  $validators['PROJECT_exists'] = array(
    'label' => 'Experiment specified exists',
    'scope' => $scope['PROJECT'],
    'validation callback' => 'analyzedphenotypes_validator_project',
    'case' => 'exists',
    'error message' => 'Please ensure to select experiment suggested by the Experiment text field before uploading your file.',
  );

  // GENUS.
  $validators['GENUS_matched_project'] = array(
    'label' => 'Genus specified exists and/or matched the Experiment',
    'scope' => $scope['GENUS'],
    'validation callback' => 'analyzedphenotypes_validator_genus',
    'case' => 'exists',
    'error message' => 'Project and genus do not match. Please contact the administrator of this website for the correct genus of @replace',
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
    'error message' => 'Please ensure that all required columns are present. The following columns are missing: @replace',
  );

  // Optional, only when allow new trait is set to not allow.
  $vars = analyzedphenotypes_systemvariables('options');
  $allow_new = variable_get($vars['ap_allownew']);

  if (!$allow_new) {
    $validators['FILE_allow_new'] = array(
      'label' => 'Allow new traits to be added',
      'scope' => $scope['FILE'],
      'validation callback' => 'analyzedphenotypes_validator_datafile',
      'case' => 'allow_new',
      'error message' => 'The system has detected new traits in your file, and this module is set to not allow new traits to be added. Please contact the administrator of this website.',
    );
  }



  ///////



  // VALUE.
  $validators['VALUE_value_provided'] = array(
    'label' => 'All columns have value',
    'scope' => $scope['VALUE'],
    'validation callback' => 'analyzedphenotypes_validator_value',
    'case' => 'value_empty',
    'columns required' => '',
    'error message' => 'A column has a blank or 0 value in the following items: @replace',
  );

  $validators['VALUE_column_format'] = array(
    'label' => 'Value matched the column data type',
    'scope' => $scope['VALUE'],
    'validation callback' => 'analyzedphenotypes_validator_value',
    'case' => 'match_type',
    'columns required' => '',
    'error message' => 'A value does not match the data type expected in items: @replace',
  );

  $e = analyzedphenotypes_columnsprop();
  $expected_columns = array_keys($e);
  $trait_name = $expected_columns[6];

  $validators['VALUE_value_type'] = array(
    'label' => 'Data was measured using expected units',
    'scope' => $scope['VALUE'],
    'validation callback' => 'analyzedphenotypes_validator_value',
    'case' => 'unit_data',
    'columns required' => $trait_name,
    'error message' => 'Data does not match the unit of the trait provided in items: @replace',
  );

  $germplasm = array(
    $expected_columns[1],
    $expected_columns[2],
  );

  $validators['VALUE_germplasm_recognized'] = array(
    'label' => 'All germplasm names are recognized by this resource',
    'scope' => $scope['VALUE'],
    'validation callback' => 'analyzedphenotypes_validator_value',
    'case' => 'germplasm_recognized',
    'columns required' => $germplasm,
    'error message' => 'Germplasm is not recognized by this resource in items: @replace Please contact the administrator of this website.',
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


/**
 * DATA FILE LEVEL VALIDATION
 * Perform data-file level, non Tripal Job validation to user data file.
 * Default to:
 * Dependencies: analyzedphenotypes_ap_validators().
 *
 * @param Array $dataset
 *  An associative array containing user provide values:
 *  - project_name  : Project/experiment selected.
 *  - project_genus : Project/experiment genus selected.
 *  - data_file     : tsv or txt data file containing analyzed phenotypic data.
 *
 * @return Array
 *   An associative array, indicating if a validator passed or failed the requirements.
 *
 * @see analyzedphenotypes_file_validate() in admin.form.inc.
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


/**
 * PROJECT VALIDATOR
 * Perform validation to project/experiment.
 * Default to:
 * Dependencies: analyzedphenotypes_projectprop().
 *
 * @param String $case
 *  A value representing a validation requested (each case represents a case in the switch implementation):
 *  - exists : Check to see if project exists in chado.projects table.
 * @param Array $dataset
 *  An associative array containing user provide values:
 *  - project_name  : Project/experiment selected.
 *  - project_genus : Project/experiment genus selected.
 *  - data_file     : tsv or txt data file containing analyzed phenotypic data.
 *
 * @return Array
 *   An associative array, indicating if a validator passed or failed the requirements.
 */
function analyzedphenotypes_validator_project($case, $dataset) {
  $status = array(
    'type' => 'passed',
    'details' => '',
  );

  switch($case) {
    //
    case 'exists':
      if (empty($dataset['project_name'])) {
        $status['type'] = 'failed';
      }
      else {
        $projectprop = analyzedphenotypes_projectprop('match_name', array(
          'project_name' => $dataset['project_name'],
        ));

        if (!$projectprop) {
          $status['type'] = 'failed';
        }
      }

      break;

    //
    // Define case here for additional validation on EXPERIMENT/PROJECT.
  }


  return $status;
}


/**
 * GENUS VALIDATOR
 * Perform validation to project/experiment genus.
 * Default to:
 * Dependencies: analyzedphenotypes_genusprop().
 *
 * @param String $case
 *  A value representing a validation requested (each case represents a case in the switch implementation):
 *  - exists : Check to see if genus exits in the system and see if it matches the Experiment/project.
 * @param Array $dataset
 *  An associative array containing user provide values:
 *  - project_name  : Project/experiment selected.
 *  - project_genus : Project/experiment genus selected.
 *  - data_file     : tsv or txt data file containing analyzed phenotypic data.
 *
 * @return Array
 *   An associative array, indicating if a validator passed or failed the requirements.
 */
function analyzedphenotypes_validator_genus($case, $dataset) {
  $status = array(
    'type' => 'passed',
    'details' => '',
  );

  switch($case) {
    //
    case 'exists':
      if (empty($dataset['project_genus'])) {
        $status['type'] = 'failed';
      }
      else {
        $genusprop = analyzedphenotypes_genusprop('project_genus', array(
          'project_name' => $dataset['project_name'],
        ));

        if ($genusprop && $dataset['project_genus'] != $genusprop) {
          $status['type'] = 'failed';
          $status['details'] = $dataset['project_name'];
        }
      }

      break;

    //
    // Define case here for additional validation on GENUS.
  }


  return $status;
}


/**
 * GENUS VALIDATOR
 * Perform validation to project/experiment genus.
 * Default to:
 * Dependencies: analyzedphenotypes_datafileprop(), analyzedphenotypes_columnsprop(), analyzedphenotypes_cvprop().
 *
 * @param String $case
 *  A value representing a validation requested (each case represents a case in the switch implementation):
 *  - format    : Check file format.
 *  - columns   : Check if all expected column headres are present.
 *  - allow_new : When set to not allow new trait to be added, this check will test if all traits are existing trait.
 * @param Array $dataset
 *  An associative array containing user provide values:
 *  - project_name  : Project/experiment selected.
 *  - project_genus : Project/experiment genus selected.
 *  - data_file     : tsv or txt data file containing analyzed phenotypic data.
 *
 * @return Array
 *   An associative array, indicating if a validator passed or failed the requirements.
 */
function analyzedphenotypes_validator_datafile($case, $dataset) {
  $status = array(
    'type' => 'passed',
    'details' => '',
  );

  switch($case) {
    //
    case 'format':
      $e = analyzedphenotypes_datafileprop();
      $valid_extension = array_keys($e);

      $file_extension = pathinfo($dataset['data_file']->filename, PATHINFO_EXTENSION);
      $file = drupal_realpath($dataset['data_file']->uri);
      $handle = fopen($file, 'r');

      if (!in_array($file_extension, $valid_extension) || FALSE === $handle) {
        $status['type'] = 'failed';
      }
      
      fclose($handle);
      break;

    //
    case 'columns':
      $file = drupal_realpath($datafile->uri);
      $handle = fopen($file, 'r');
      
      if ($handle) {
        while(!feof($handle)) {
          $columns_row = fgets($handle);
      
          if (!empty($column_row)) {
            // Extract the first row which is the column headers row.
            $columns_noformat  = array_map('analyzedphenotypes_noformat', $file_columns);

            $columnsprop = analyzedphenotypes_columnsprop();
            $expected_columns = array_keys($columnsprop);
            $expected_columns_noformat = array_map('analyzedphenotypes_noformat', $expected_columns);

            $missing = array_diff($expected_columns_noformat, $columns_noformat);

            if (count($missing) > 0) {
              $status['type'] = 'failed';

              $missing = array_map('ucwords', $missing);
              $status['details'] = implode(', ', $missing) . ' in line #1';
            }

            break;
          }
        }  
      }
      else {
        $status['type'] = 'failed';
        $status['details'] = 'Column headers not found in line #1';
      }
      
      fclose($handle);
      break;

    //
    case 'allow_new':
      $file = drupal_realpath($dataset['data_file']->uri);
      $handle = fopen($file, 'r');

      if ($handle) {
        // @TODO: replace with new columns api.
        $columnsprop = analyzedphenotypes_columnsprop();
        $expected_columns = array_keys($columnsprop);
        $trait_name = $expected_columns[0];

        $arr_new_trait = array();
        $i = 0;

        while(!feof($handle)) {
          $cur_row = fgets($handle);
          
          if ($i == 0) {
            // Extract column headers row.
            $columns_row = str_getcsv($cur_row, "\t");

            // Index of Trait Name in columns row.
            $trait_name_index = array_search($trait_name, $columns_row);

            $i = 1;
          }
          else {
            // Data row.
            $data = str_getcsv($cur_row, "\t");
            $file_traitname_value = $data[ $trait_name_index ];
            
            if (in_array($file_traitname_value, $arr_new_trait)) {
              continue;
            }
            else {
              // @TODO: Replace with new cv api.
              $exists = analyzedphenotypes_cvprop('get_cvterm', array(
                'name' => $file_traitname_value,
                'genus' => $dataset['project_genus'],
              ));

              if (empty($exists)) {
                // Found new trait, takes only a condition to satisfy that
                // this file could not be process any further because it
                // contains a new trait and settings disallows new traits to
                // be added.
                $status['type'] = 'failed';
                
                fclose($handle);
                break;
              }
              else {
                $arr_new_trait[] = $file_traitname_value;
              }
            }
          }
        }
      }

      break;

    //
    // Define case here for additional validation on DATA FILE.
  }


  return $status;
}




///////




/**
 * DATA FILE VALUE LEVEL VALIDATION
 * Perform data-file value level, a Tripal Job validation to user data file.
 * Default to:
 * Dependencies: analyzedphenotypes_ap_validators(), analyzedphenotypes_datafileprop(), analyzedphenotypes_tripaljobprop().
 *
 * @param Array $dataset
 *  An associative array containing user provide values:
 *  - project_name  : Project/experiment selected.
 *  - project_genus : Project/experiment genus selected.
 *  - data_file     : Standard Drupal File Id Number (FID).
 * @param Integer $job_id
 *  - Registered Tripal Job Id number.
 *
 * @return Array
 *   An associative array, indicating if a validator passed or failed the requirements.
 *
 * @see analyzedphenotypes_loader_form_save() in admin.form.inc.
 */
function analyzedphenotypes_validate_tsv_data($dataset, $job_id) {
  $dataset = unserialize($dataset);

  $file = drupal_realpath($dataset['data_file']->uri);
  $handle = fopen($file, 'r');
  
  if ($handle) {
    $arr_status = array();

    // Tripal Job module settings.
    $tripaljob_settings = analyzedphenotypes_tripaljobprop();
    $error_counter = 0;
    
    print '0% Complete...' . "\n";

    // Scope - just do value/data scope.
    $scope = analyzedphenotypes_ap_validators('return_all_scope');
    $scope = $scope['VALUE'];

    // Validators.
    $validator = module_invoke_all('ap_validators');
    foreach($validator as $k => $v) {
      if ($v['scope'] == $scope) {
        $arr_status[$k] = array(
          'type' => 'todo',
          'details' => '',
        );
      }
      else {
        unset($validator[$k]);
      }
    }
    
    // Column headers.
    // @TODO: replace with new column headers api.
    $e = analyzedphenotypes_columnsprop();
    $expected_columns = array_keys($e);
    
    $percent = 0;
    $line_number = 1;
    
    // Begin validation of values.
    $i = 0;
    while (!feof($handle)) {
      $cur_row = fgets($handle);
      
      if ($i == 0) {
        // Extract column headers.
        $columns_row = str_getcsv($cur_row, "\t");

        // Index of Trait Name in columns row.
        $trait_name_index = array_search($expected_columns[0], $columns_row);
        $i = 1;
      }
      else {
        // Data row.
        if (empty($line)) {
          continue;
        }
        else {
          $line_number++;
        }
        
        // Remove leading and trailing whitespaces.
        $data = array_map('trim', $cur_row);
        
        // For a row, extract the trait.
        $trait_name = $data[ $trait_name_index ];
        
        // Execute applicable validators.
        foreach($validator as $key => $item) {
          $case = $item['case'];
          $callback = $item['validation callback'];
          $columns_required = $item['columns required'];
          
          // Start positive and prove it otherwise.
          if ($arr_status[$key]['type'] != 'failed') {
            $arr_status[$key]['type'] = 'passed';
          }
          
          // Inspect validator apply-to-columns requirements.
          // - When none is supplied, validator should be applied to each column
          // - When an array is supplied, validator should be applied to a column but
          // take into account these column combination.
          // - When a single column is provided, apply to that column only.
          
          if (empty($columns_required)) {
            // Apply to all.  
            foreach($data as $j => $value) {
              $value = trim($value, '"');
              $result = call_user_func($callback, $case, $value, $columns_row[$j], $line_number, $trait_name);

              if ($result['type'] == 'failed') {
                if ($arr_status[$key]['type'] == 'passed') {
                  $arr_status[$key]['type'] = 'failed';
                }

                // Group items that caused error and apply a single error message.
                $arr_status[$key]['details'] .= $result['details'] . ', ';

                $error_counter++;
              }
            }
          }
          else {
            if (is_array($columns_required)) {
              // A combination.
              $value = array();
              foreach($columns_required as $v) {
                $j = array_search($v, $columns_row);
                $value[$v] = trim($data[$j], '"');
              }
            }
            else {
              // A specific column,
              $j = array_search($columns_required, $columns_row);
              $value = trim($data[$j], '"');
            }
          }
          
          // End validator foreach.  
        }
        
        if ($error_counter >= $tripaljob_settings['TRIPALJOB_error_limit']) {
          // Stop validation when number of error limit (set in the config)
          print 'Error limit reached. Job terminated.' . "\n";
          break;
        }
        
        $percent = round((($i + 1) / $rows_count) * 100);
        
        if ($percent % $tripaljob_settings['TRIPALJOB_report_frequency'] == 0) {
          // Report ever x number of times to terminal (set in the config)
          if ($percent != $prev_percent) {
            print $percent . '% Complete...' . "\n";
            $prev_percent = $percent;
          }
        }

        analyzedphenotypes_tripaljobprop('write_progress', array(
          'job_id' => $job_id,
          'source' => 'jobprogress',
          'progress' => $percent,
        ));
        
        // End else data row.  
      }
      // End while.
    }
    
    // Report validation status.
    $status_JSON = json_encode($arr_status);
    analyzedphenotypes_tripaljobprop('write_progress', array(
      'job_id' => $job_id,
      'source' => 'validateresult',
      'progress' => $status_JSON,
    ));

    print '100% Complete...' . "\n";
    
    // End if $handle.  
  }
}


/**
 * DATA FILE VALUE VALIDATOR
 * Perform validation to values.
 * Default to:
 * Dependencies: analyzedphenotypes_columnsprop(), analyzedphenotypes_datatypeprop(), analyzedphenotypes_stockprop().
 *
 * @param String $case
 *  A value representing a validation requested (each case represents a case in the switch implementation):
 *  - value_empty         : Check if value is empty.
 *  - match_type          : Check if value in a column matches the expected data type of the column header.
 *  - unit_data           : Check if value matches the unit in the trait name.
 *  - gemplsam_recognized : Check if germ accession is recognized by this resource.
 *  - unique_combo        : Check if a combination of column header is unique record.
 * @param $value
 *  An array of values when validator requires a set of values or string value.
 * @param String $column
 *  Column header of the current row.
 * @param Integer $row
 *  The current row number.
 * @param String $trait_name
 *  A value representing the trait name in a given row.
 *
 * @return Array
 *   An associative array, indicating if a validator passed or failed the requirements.
 */
function analyzedphenotypes_validator_value($case, $value, $column, $row, $trait_name) {
  $status = array(
    'type' => 'passed', // Passed or Failed. Default to passed.
    'details' => '',    // Include more information.
  );

  switch($case) {
    //
    case 'value_empty':
      // Check the if value is empty.
      if ($value == '') {
        $status['type'] = 'failed';
        $status['details'] = ' in ' . $column . ' line #' . $row;
      }

      break;

    //
    case 'match_type':
      if (!empty($value)) {
        // Check if the data in a column match the data type required.
        $required_columns = analyzedphenotypes_columnsprop();
        $column_data_type = $required_columns[$column];
        $column_names = array_keys($required_columns);

        if (!empty($column_data_type) && $column_names[$column] != $column_names[6]) {
          $coltype_value_check = analyzedphenotypes_datatypeprop($column_data_type, array(
            'value' => $value,
          ));

          if (!$coltype_value_check) {
            $status['type'] = 'failed';
            $status['details'] = ' in ' . $column . ' line #' . $row . ' (' . $value . ' is not a valid ' . $column_data_type . ')';
          }
        }
      }

      break;

    //
    case 'unit_data':
      if (!empty($value)) {
        $unit = analyzedphenotypes_traitprop('unit', array(
          'trait_name' => $trait_name,
        ));

        $unit_type = analyzedphenotypes_datatypeprop('unit_type');
        $trait_unit_datatype = $unit_type[$unit]['type'];

        $trait_value_check = analyzedphenotypes_datatypeprop($trait_unit_datatype, array(
          'value' => $value,
        ));

        if (!$trait_value_check) {
          $status['type'] = 'failed';
          $status['details'] = ' in ' . $column . ' line #' . $row . ' (' . $value . ' is not a valid ' . $unit . '/' . $unit_type[$unit]['name'] . ')';
        }
      }

      break;

    //
    case 'germplasm_recognized':
      if ($value) {
        $e = analyzedphenotypes_columnsprop();
        $expected_columns = array_keys($e);

        $stock_id_check = analyzedphenotypes_stockprop('match_name', array(
          'name' => $value[$expected_columns[2]],
          'uniquename' => $value[$expected_columns[1]],
        ));

        if (!$stock_id_check) {
          $status['type'] = 'failed';
          $status['details'] = ' in ' . implode(', ', $column) . ' line #' . $row . '(' . $value[$expected_columns[1]] . ' - ' . $value[$expected_columns[2]] . ')';
        }
      }

      break;

    //
    case 'unique_combo':
      if ($value) {
        $match = analyzedphenotypes_columnsprop('match_combination', array(
          'combination' => $value,
        ));

        if ($match) {
          $status['type'] = 'failed';
          $status['details'] = 'in line #'. $row;
        }
      }

      break;

    //
    // Define case here for additional validation on VALUE.
  }


  return $status;
}


/**
 * SAVE ANALYZED PHENOTYPES.
 * Save project, genus and analyzed phenotypic data.
 * Default to:
 * Dependencies: analyzedphenotypes_tripaljobpro(), analyzedphenotypes_projectprop(), analyzedphenotypes_systemvariables().
 *
 * @param Array $dataset
 *  An associative array containing user provide values:
 *  - project_name  : Project/experiment selected.
 *  - project_genus : Project/experiment genus selected.
 *  - data_file     : Standard Drupal File Id Number (FID).
 * @param Integer $job_id
 *  - Registered Tripal Job Id number.
 */
function analyzedphenotypes_save_tsv_data($dataset, $job_id) {
  // Dataset - project_name.
  // Dataset - project_genus.
  // Dataset - datafile (FID).
  // Dataset - traits.
  $dataset = unserialize($dataset);
  $trait_cvterm = $dataset['trait_cvterm'];

  // # PROJECT.
  $projectprop = analyzedphenotypes_projectprop('match_name', array(
    'project_name' => $dataset['project_name'],
  ));

  $project_id = ($projectprop) ? $projectprop['project_id'] : 0;

  if (!$project_id) {
    analyzedphenotypes_tripaljobprop('tripal_fail', array(
      'message' => 'CODE:100 Experiment or project is not recognized by this system.',
      'param' => $dataset['project_name'],
      'job_id' => $job_id,
    ));

    exit(1);
  }

  // # GENUS.
  $genus = $dataset['project_genus'];
  $genusprop = analyzedphenotypes_genusprop();

  if (!$genus || array_search($genus, $genusprop) === FALSE) {
    analyzedphenotypes_tripaljobprop('tripal_fail', array(
      'message' => 'CODE:101 Experiment or project genus is not recognized by this system.',
      'param' => $genus,
      'job_id' => $job_id,
    ));

    exit(2);
  }

  // # FILE.
  $data_file = file_load($dataset['data_file']);
  $file = drupal_realpath($datafile->uri);
  
  if (!$data_file || !file_exists($file)) {
    analyzedphenotypes_tripaljobprop('tripal_fail', array(
      'message' => 'CODE:102 Unable to read or open data file.',
      'param' => 'FID#' . $dataset['data_file'],
      'job_id' => $job_id,
    ));

    exit(3);
  }

  $file_contents = file_get_contents($file);

  if (empty($file_contents)) {
    analyzedphenotypes_tripaljobprop('tripal_fail', array(
      'message' => 'CODE:103 File is empty.',
      'param' => $data_file->uri,
      'job_id' => $job_id,
    ));

    exit(4);
  }

  // # DATA
  $tripaljob_settings = analyzedphenotypes_tripaljobprop();

  $rows = explode("\n", $file_contents);
  $columns = str_getcsv($rows[0], "\t");
  unset($file_contents, $rows[0]);
  // Excluding columns row.
  $row_count = count($rows);

  $col_index = array();
  $columnsprop = analyzedphenotypes_columnsprop();
  foreach($columnsprop as $col => $t) {
    $j = array_search($col, $columns);
    $col_index[$col] = $j;
  }
  unset($columnsprop);

  $sysvars = analyzedphenotypes_systemvariables('terms');
  // Assay id : method/taken by eye.
  $sysvar_method = variable_get($sysvars['ap_method']);

  $plantprop = array();
  $plantprop['Year'] = variable_get($sysvars['ap_year']);
  $plantprop['Location'] = variable_get($sysvars['ap_location']);
  $plantprop['Replicate'] = variable_get($sysvars['ap_replicate']);
  $plantprop['Data Collector'] = variable_get($sysvars['ap_collector']);
  unset($sysvars);

  print '0% Complete...' . "\n";

  // # DB TRANSACTION.
  $TRANSACTION = db_transaction();

  try {
    // BEGIN TRANSACTION:
    foreach($rows as $i => $line) {
      if (!empty($line)) {
        $data = str_getcsv($line, "\t");

        // ## PHENOTYPES.
        $file_traitname = $data[$col_index['Trait Name']];
        // cvterm_id used by this trait name from trait to cvterm_id in previous stage.
        $cvterm_id = $trait_cvterm[$file_traitname];

        $trait_property = analyzedphenotypes_cvprop('get_cvterm', array(
          'cvterm_id' => $cvterm_id,
        ));

        if (!$trait_property) {
          analyzedphenotypes_tripaljobprop('tripal_fail', array(
            'message' => 'CODE:104 Trait name is not recognized by this system.',
            'param' => $file_traitname,
            'job_id' => $job_id,
          ));

          exit(31);
        }

        $file_germplasm_accession = $data[$col_index['Germplasm Accession']];
        $file_germplasm_name = $data[$col_index['Germplasm Name']];

        $stock_id = analyzedphenotypes_stockprop('match_name', array(
          'name' => $file_germplasm_name,
          'uniquename' => $file_germplasm_accession,
        ));

        if (!$stock_id) {
          analyzedphenotypes_tripaljobprop('tripal_fail', array(
            'message' => 'CODE:105 Stock/Germplasm name is not recognized by this system.',
            'param' => $file_germplasm_name . ':' . $file_germplasm_accession,
            'job_id' => $job_id,
          ));

          exit(32);
        }

        $trait_id = $trait_property['cvterm_id'];

        $file_year = $data[$col_index['Year']];
        $file_location = $data[$col_index['Location']];
        $file_replicate = $data[$col_index['Replicate']];

        $uniquename = str_replace(array(' ', ','), '', implode('_', array(
          $trait_id, $project_id, $stock_id, $file_location, $file_year, $file_replicate, date('c'), $i
        )));

        $file_value = $data[$col_index['Value']];

        $phenotype_data = array(
          'uniquename'    => $uniquename,
          'observable_id' => null,
          'attr_id'       => $trait_id,
          'value'         => $file_value,
          'cvalue_id'     => null,
          'assay_id'      => $method,
          'project_id'    => $project_id,
          'stock_id'      => $stock_id,
        );

        $phenotype_insert = db_insert('chado.phenotype')
         ->fields($phenotype_data)
         ->execute();

        if ($phenotype_insert) {
          $phenotype_id = $phenotype_insert;

          foreach($plantprop as $col_title => $var_value) {
            $file_property_value = $data[$col_index[$col_title]];

            $property_data = array(
              'phenotype_id' => $phenotype_id,
              'type_id'   => $var_value,
              'value'  => $file_property_value,
              // NOTE: THIS FIELD HAS NOT-NULL CONSTRAINT.
              'cvalue_id' => 1,
              'rank' => 0,
            );

            $property_insert = db_insert('chado.phenotypeprop')
              ->fields($property_data)
              ->execute();

            if (!$property_insert) {
              analyzedphenotypes_tripaljobprop('tripal_fail', array(
                'message' => 'CODE:107 Unable to insert phenotype property record.',
                'param' => print_r($property_data),
                'job_id' => $job_id,
              ));

              exit(34);
            }
          }
        }
        else {
          analyzedphenotypes_tripaljobprop('tripal_fail', array(
            'message' => 'CODE:106 Unable to insert phenotype record.',
            'param' => print_r($phenotype_data),
            'job_id' => $job_id,
          ));

          exit(33);
        }

        // Report.
        $percent = round(($i / $row_count) * 100);

        if ($percent % $tripaljob_settings['report_frequency'] == 0) {
          print $percent . '% Complete...' . "\n";
        }

        analyzedphenotypes_tripaljobprop('write_progress', array(
          'job_id' => $job_id,
          'source' => 'jobprogress',
          'progress' => $percent,
        ));
      }
    }
    // END TRANSACTION:
  }
  catch(Exception $e) {
    $TRANSACTION->rollback();
    watchdog_exception($ap, $e);

    analyzedphenotypes_tripaljobprop('tripal_fail', array(
      'message' => 'CODE:107 Failed to insert records.',
      'param' => 'Tripal Job',
      'job_id' => $job_id,
    ));

    exit(5);
  }

  // Make file permanent.
  if ($data_file) {
    analyzedphenotypes_datafileprop('make_permanent', array(
      'data_file' => $data_file,
    ));
  }

  // Update materialized views.
  foreach(array('mview_phenotype', 'mview_phenotype_summary') as $mview) {
    $mview_id = tripal_get_mview_id($mview);
    if ($mview_id) tripal_populate_mview($mview_id);
  }
}


/**
 * Function callback.
 * Remove text fomatting, leading and trailing spaces and convert to lowercase.
 */
function analyzedphenotypes_noformat($string) {
  return strtolower(trim($string));
}
