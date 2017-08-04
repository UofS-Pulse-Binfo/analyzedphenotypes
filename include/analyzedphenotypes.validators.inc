<?php
/**
 * @file
 * Functions providing validation of analyzed phenotypic data.
 * These functions are currently used when new data is being uploaded.
 */


/**
 *
 */
function analyzedphenotypes_save_tsv_data($dataset, $job_id) {
  // Dataset - project_name
  // Dataset - project_genus
  // Dataset - datafile (FID)
  $dataset = unserialize($dataset);

  // # PROJECT.
  $project_id = analyzedphenotypes_projectprop('match_name', $dataset['project_name']);

  if (!$project_id) {
    analyzedphenotypes_tripaljobprop('tripal_fail', array(
      'message' => 'CODE:100 Experiment or project is not recognized by this system.',
      'param' => $dataset['project_name'],
      'job_id' => $job_id
    ));

    exit(1);
  }

  // # GENUS.
  $genus_id = analyzedphenotypes_genusprop('match_name', $dataset['project_genus']);

  if (!$genus_id) {
    analyzedphenotypes_tripaljobprop('tripal_fail', array(
      'message' => 'CODE:101 Experiment or project genus is not recognized by this system.',
      'param' => $dataset['project_genus'],
      'job_id' => $job_id
    ));

    exit(2);
  }

  // # FILE.
  $data_file = file_load($dataset['data_file']);

  if (!$data_file) {
    analyzedphenotypes_tripaljobprop('tripal_fail', array(
      'message' => 'CODE:102 Unable to read or open data file.',
      'param' => 'FID#' . $dataset['data_file'],
      'job_id' => $job_id
    ));

    exit(3);
  }

  $file_content = analyzedphenotypes_fileprop('get_contents', $data_file);

  if (empty($file_content)) {
    analyzedphenotypes_tripaljobprop('tripal_fail', array(
      'message' => 'CODE:103 File is empty.',
      'param' => $data_file->uri,
      'job_id' => $job_id
    ));

    exit(4);
  }

  // # DATA
  $all_path = analyzedphenotypes_moduleprop();
  $tripaljob_settings = analyzedphenotypes_tripaljobprop('settings');

  $error_limit = $tripaljob_settings['error_limit'];
  $report_frequency = $tripaljob_settings['report_frequency'];

  $all_filename = analyzedphenotypes_fileprop('filename');
  $file_progress = $all_path['temp'] . '/' . str_replace('@JOB_ID', $job_id, $all_filename['validate progress']);
  unset($all_path, $all_filename);

  print '0% Complete...' . "\n";

  $rows = explode("\n", $file_content);
  $columns = str_getcsv($rows[0], "\t");
  unset($rows[0]);

  $rows_count = count($rows);

  $all_columns = analyzedphenotypes_columnsprop();
  $index = array();

  foreach($all_columns as $i => $c) {
    $j = array_search($c, $columns);
    $index[$c] = $j;
  }

  $mod_vars = analyzedphenotypes_moduleprop('module_vars');
  $system_cvid = variable_get($mod_vars['ap_cv']);
  $system_cvname = analyzedphenotypes_cvprop('get_cvname', $system_cvid);


  // # DB TRANSACTION.
  $TRANSACTION = db_transaction();

  try {
    // BEGIN TRANSACTION:
    foreach($rows as $i => $line) :
      $data = str_getcsv($line, "\t");

      // ## PHENOTYPES.
      $file_traitname = $data[$index[$all_columns[0]]];
      $trait_property = analyzedphenotypes_traitprop('get_term', array(
        'name' => $file_traitname,
        'cv' => $system_cvname
      ));

      if (!$trait_property) {
        analyzedphenotypes_tripaljobprop('tripal_fail', array(
          'message' => 'CODE:104 Trait name is not recognized by this system.',
          'param' => $file_traitname,
          'job_id' => $job_id
        ));

        exit(31);
      }

      $trait_id = $trait_property['id'];
      $file_year = $data[$index[$all_columns[3]]];
      $file_value = $data[$index[$all_columns[6]]];
      $file_location = $data[$index[$all_columns[4]]];
      $file_replicate = $data[$index[$all_columns[5]]];

      $phenotype_data = array(
        'uniquename' => implode('_', array(
          $trait_id, $file_year, $project_id, $genus_id, $file_location, $file_year, $file_replicate, date('Ymd')
        )),
        'value' => $file_value,
        'attr_id' => $trait_id,
        'stock_id' => $genus_id,
        'project_id' => $project_id,
      );

      $phenotype_insert = chado_insert_record('phenotype', $phenotype_data);

      if ($phenotype_insert) {
        unset($mod_vars['ap_database'], $mod_vars['ap_cv'], $mod_vars['ap_allow_new']);
        $phenotype_id = $phenotype_insert['phenotype_id'];

        foreach($mod_vars as $i => $var) {
          $system_var = variable_get($var);
          $property = trim(str_replace('ap_', '', $i));

          foreach($columns as $i => $c) {
            if (stripos($c, $property)) {
              $property_value = $data[$i];
              break;
            }
          }

          $property_data = array(
            'phenotype_id' => $phenotype_id,
            'type_id'   => $system_var,
            'value'  => $property_value,
            'rank' => 0,
          );

          $property_insert = chado_insert_record('propertyprop', $property_data);

          if (!$property_insert) {
            analyzedphenotypes_tripaljobprop('tripal_fail', array(
              'message' => 'CODE:106 Unable to insert phenotype property record.',
              'param' => $dataset['project_genus'],
              'job_id' => $job_id
            ));

            exit(33);
          }
        }
      }
      else {
        analyzedphenotypes_tripaljobprop('tripal_fail', array(
          'message' => 'CODE:105 Unable to insert phenotype record.',
          'param' => print_r($phenotype_data),
          'job_id' => $job_id
        ));

        exit(32);
      }

      // Report.
      $percent_completed = round(($i / $row_count) * 100);

      if ($percent_complete % $report_frequency == 0) {
        print $percent_complete . '% Complete...' . "\n";
        file_unmanaged_save_data($percent_complete, $file_progress, FILE_EXISTS_REPLACE);
      }
    endforeach;
    // END TRANSACTION:
  }
  catch(Exception $e) {
    $TRANSACTION->rollback();
    watchdog_exception($ap, $e);

    analyzedphenotypes_tripaljobprop('tripal_fail', array(
      'message' => 'CODE:107 Failed to insert records.',
      'param' => 'Tripal Job',
      'job_id' => $job_id
    ));

    exit(5);
  }
}

/**
 *
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
    'label' => 'All columns have value',
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


/**
 *
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
 *
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
      if (empty($project_name)) {
        $status['type'] = 'failed';
      }
      else {
        $exists = analyzedphenotypes_projectprop('match_name', $project_name);

        if (!$exists) {
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
 *
 */
function analyzedphenotypes_validator_genus($case, $dataset) {
  $project_genus = $dataset['project_genus'];
  $project_name  = $dataset['project_name'];
  $project_name  = trim($project_name);

  $status = array(
    'type' => 'passed', // Passed or Failed. Default to passed.
    'details' => '',    // Include more information.
  );

  switch($case) {
    //
    case 'exists':
      if (empty($project_genus)) {
        $status['type'] = 'failed';
      }
      else {
        $project_id = analyzedphenotypes_projectprop('match_name', $project_name);

        if ($project_id) {
          $genus = analyzedphenotypes_genusprop('project_genus', $project_id);

          if ($genus) {
            if ($project_genus != $genus) {
              $status['type'] = 'failed';
            }
          }
        }
      }

      break;

    //
    // Define case here for additional validation on GENUS.
  }


  return $status;
}


/**
 *
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
      $e = analyzedphenotypes_fileprop();
      $valid_extension = array_keys($e);
      $file_extension = pathinfo($data_file->filename, PATHINFO_EXTENSION);
      $readable = analyzedphenotypes_fileprop('readable', $data_file);

      if (!in_array($file_extension, $valid_extension) || !$readable) {
        $status['type'] = 'failed';
      }

      break;

    //
    case 'columns':
      $columns = analyzedphenotypes_fileprop('columns', $data_file);
      $expected = analyzedphenotypes_columnsprop('expected');

      $expected_noformat = array_map('analyzedphenotypes_noformat', $expected);
      $columns_noformat  = array_map('analyzedphenotypes_noformat', $columns);

      $missing = array_diff($expected_noformat, $columns_noformat);

      if (count($missing) > 0) {
        $status['type'] = 'failed';

        $missing = array_map('ucwords', $missing);
        $status['details'] = implode(', ', $missing) . ' in line #1';
      }

      break;


    //
    // Define case here for additional validation on DATA FILE.
  }


  return $status;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/**
 *
 */
function analyzedphenotypes_validate_tsv_data($dataset, $job_id) {
  $dataset = unserialize($dataset);
  //
  $data_file = file_load($dataset['data_file']);
  $file_content = analyzedphenotypes_fileprop('get_contents', $data_file);

  if ($file_content) {
  ///

    $arr_status = array();

    // Get module settings.
    $all_path = analyzedphenotypes_moduleprop();
    //
    $error_limit = analyzedphenotypes_moduleprop('error_limit');
    $error_counter = 0;
    //
    $report_frequency = analyzedphenotypes_moduleprop('report_frequency');

    // Temporary files.
    $all_filename = analyzedphenotypes_fileprop('filename');
    $file_progress = $all_path['temp'] . '/' . str_replace('@JOB_ID', $job_id, $all_filename['validate progress']);
    $file_result   = $all_path['temp'] . '/' . str_replace('@JOB_ID', $job_id, $all_filename['validate result']);
    unset($all_path, $all_filename);

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

    // Rows.
    $rows = explode("\n", $file_content);
    unset($file_content);
    $rows_count = count($rows);

    // Columns.
    $columns_row = $rows[0];
    unset($rows[0]);

    $c = str_getcsv($columns_row, "\t");
    $columns = array_map('trim', $c);

    $prev_percent = 0;

    // BEGIN TRIPAL VALIDATION.
    foreach($rows as $i => $line) {
      if (empty($line)) {
        continue;
      }

      $d = str_getcsv($line, "\t");
      $data = array_map('trim', $d);
      $trait_name = $data[0];

      foreach($validator as $key => $item) {
        $case = $item['case'];
        $callback = $item['validation callback'];
        $columns_required = $item['columns required'];

        if ($arr_status[$key]['type'] != 'failed') {
          $arr_status[$key]['type'] = 'passed';
        }

        if (empty($columns_required)) {
          // Apply to all.
          foreach($data as $j => $value) {
            $value = trim($value, '"');
            $result = call_user_func($callback, $case, $value, $columns[$j], ($i + 1), $trait_name);

            if ($result['type'] == 'failed') {
              if ($arr_status[$key]['type'] == 'passed') {
                $arr_status[$key]['type'] = 'failed';
              }

              // Group details.
              $arr_status[$key]['details'] .= $result['details'] . ', ';

              $error_counter++;
            }
          }
        }
        else {
          if (is_array($columns_required)) {
            // Column combination.
            $value = '';
            foreach($columns_required as $v) {
              $j = array_search($v, $columns);
              $value .= trim($data[$j], '"');
            }
          }
          else {
            // Specific column,
            $j = array_search($columns_required, $columns);
            $value = trim($data[$j], '"');
          }

          $result = call_user_func($callback, $case, $value, $columns_required, ($i + 1), $trait_name);

          if ($result['type'] == 'failed') {
            if ($arr_status[$key]['type'] == 'passed') {
              $arr_status[$key]['type'] = 'failed';
            }

            // Group details.
            $arr_status[$key]['details'] .= $result['details'] . ', ';

            $error_counter++;
          }
        }
      }


      if ($error_counter >= $error_limit) {
        print 'Error limit reached. Job terminated.' . "\n";
        break;
      }

      //
      $percent = round((($i + 1) / $rows_count) * 100);

      if ($percent % $report_frequency == 0) {
        if ($percent != $prev_percent) {
          print $percent . '% Complete...' . "\n";
          $prev_percent = $percent;
        }
      }

      file_unmanaged_save_data($percent, $file_progress, FILE_EXISTS_REPLACE);
    }


    $status_JSON = json_encode($arr_status);
    file_unmanaged_save_data($status_JSON, $file_result, FILE_EXISTS_REPLACE);

    print '100% Complete...' . "\n";

  ///
  }
}


/**
 *
 */
function analyzedphenotypes_validator_value($case, $value, $column, $row, $tait_name = null) {
  $value = trim($value);

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
      $data_type = analyzedphenotypes_columnsprop();
      $column_data_type = $data_type[$column];

      if (empty($column_data_type)) {
        $unit = analyzedphenotypes_traitprop('unit', $trait_name);
        $unit_type = analyzedphenotypes_moduleprop('unit_type');
        $column_data_type = $unit_type[$unit]['type'];
      }

      unset($data_type);
      //unset($unit_type);

      $is_unit = analyzedphenotypes_datatypeprop($column_data_type, $value);

      if (!$is_unit) {
        $status['type'] = 'failed';
        $status['details'] = ' in ' . $column . ' line #' . $row . ' (' . $value . ' is not a valid ' . $column_data_type . ')';
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
      case 'unique_combination':
        $match = analyzedphenotypes_columnsprop('match_combination', $value);
        if ($match) {
          $status['type'] = 'failed';
          $status['details'] = 'in line #'. $row;
        }

        break;

      //
  }


  return $status;
}
