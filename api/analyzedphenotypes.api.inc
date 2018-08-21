<?php
/**
 * @file
 * Contains general API required by this module.
 */


/**
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_stockprop($property, $parameter = null) {
  return ap_match_germplasm($parameter['name'], $parameter['uniquename']);
}

/**
 * FUNCTION:
 * Fetch stock/germplasm property form chado.stock given stock name and stock uniquename.
 *
 * @param $germplasm_name
 *   A string, the name of a stock that corresponds to name field in chado.stock.
 * @param $germplasm_uniquename
 *   A string, the uniquename of a stock that corresponds to uniquename field in chado.stock.
 *
 * @return
 *   An integer, the stock id number of the matching record.
 */
function ap_match_germplasm($germplasm_name, $germplasm_uniquename) {
  if (empty($germplasm_name) || empty($germplasm_uniquename)) {
    return null;
  }
  else {
    $germplasm = $germplasm_name . $germplasm_uniquename;

    $result = chado_query(
      "SELECT stock_id FROM {stock} WHERE CONCAT(name, uniquename) = :germplasm LIMIT 1",
      array(':germplasm' => $germplasm)
    );

    return ($result->rowCount()) ? $result->fetchField() : null;
  }
}

///////

/**
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_traitprop($property, $parameter) {
  $command = array(
    'no_unit' => array(
      'func'  => 'ap_format_traitname',
      'param' => $parameter['trait_name'],
      'opts'  => 'trait_name',
    ),
    'unit' => array(
      'func'  => 'ap_format_traitname',
      'param' => $parameter['trait_name'],
      'opts'  => 'trait_unit',
    ),
  );

  return call_user_func($command[$property]['func'], $command[$property]['param'], $command[$property]['opts']);
}

/**
 * FUNCTION:
 * Format data entry under Trait Name column header.
 *
 * @param $traitname
 *   String, trait name in the following format Trait Name (unit).
 * @param $format
 *   String, trait_name or trait_unit - extract trait name or unit respectively.
 *
 * @return
 *   String, trait name or unit.
 */
function ap_format_traitname($traitname, $format = 'trait_unit') {
  if ($format == 'trait_name') {
    preg_match("/.*\(([^)]*)\)/", $trait_name, $match);
    // Return text if unit cannot be determined.
    $result = (isset($match[1])) ? $match[1] : 'text';
  }
  else {
    // Default to extract unit.
    $result = preg_replace("/\(.+\)/", '', $traitname);
  }

  return trim($result);
}

///////

/**
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_validationresult($property, $parameter) {
  $command = array(
    'data' => array(
      'func'  => 'ap_report_validationresult',
      'param' => array('data' => $parameter),
    ),
    'file' => array(
      'func'  => 'ap_report_validationresult',
      'param' => array('file' => $parameter),
    ),
  );

  return call_user_func($command[$property]['func'], $command[$property]['param']);
}

/**
 * FUCTION:
 * Prepare validation result window for validation performed on file and data level.
 *
 * @param $identifiers
 *   An array with key:
 *   - data : scope is data level and validation report is sent to JS.
 *   - file : scope is file level and validation report is sent to Form API.
 */
function ap_report_validationresult($identifiers) {
  // Scope of validation done, file (stage 1) or data (stage 2).
  $scope = (isset($identifiers['data'])) ? 'data' : 'file';

  if ($scope == 'data' || $scope == 'file') {
    if ($scope == 'file') {
      // When scope is file, it is expecting the completed status message of
      // the validation result.
      $status = $identifiers['file'];
      print theme('analyzedphenotypes_validator_report', array('status' => $status, 'scope' => 'ap-file-scope'));
    }
    else {
      // Default to scope data.
      // When scope is data level, it is expecting the Tripal job id number.
      $tripaljob_id = $identifiers['data'];

      $s = analyzedphenotypes_tripaljobprop('read_progress', array(
        'job_id' => $tripaljob_id,
        'source' => 'validateresult',
      ));

      $status = json_decode($s, TRUE);
      return theme('analyzedphenotypes_validator_report', array('status' => $status, 'scope' => 'ap-data-scope'));
    }
  }
}


///////

/**
 * FUNCTION:
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_AJAX_callback($form, $form_state) {
  return call_user_func('ap_send_ajaxrequest', $form, $form_state);
}

/**
 * FUNCTION:
 * General AJAX function callback to enable AJAX feature to a field.
 */
function ap_send_ajaxrequest($form, $form_state) {
  // Match the ID attribute of the containing element referenced.
  $case = $form_state['triggering_element']['#ajax']['case'];
  // Reference #id attribute.
  $element = 'ap_AJAX_wrapper' . '_' . $case;

  if (isset($form_state['triggering_element']['#ajax']['contain'])) {
    // The element referenced is within another container that is also implementing
    // and AJAX call. Account for the parent container.
    $contain = $form_state['triggering_element']['#ajax']['contain'];
  }

  // AJAX refresh element.
  return (isset($contain)) ? $form[$contain][$element] : $form[$element];
}

/**
 * FUNCTION:
 * AJAX callback for managing germplasm fields as filter.
 * 
 * NOTE: checking and uncheking in done in hook_form does not seem to work.
 *       move this operation to this callback.
 */
function ap_AJAX_germplasmfield_callback($form, $form_state) {
  // Trigerring elements value and command option.
  $triggering_element = array(
    'command' => $form_state['triggering_element']['#ajax']['command'],
    'value' => $form_state['triggering_element']['#value']
  );
  
  // Reference standard filed element id element.
  $fldname = analyzedphenotypes_fieldnames();
  $fld_germplasm = $fldname['germplasm']['base'];

  // Checking and unchecking.
  if ($triggering_element['command'] == 'germplasm_remove_element') {
    $germplasm_name = $form_state['values']['ap_field_hidden_values'][ $triggering_element['value'] ];
    $form['ap_AJAX_wrapper_main']['ap_additional_filter_fieldset']['ap_AJAX_germplasmname_field_wrapper'][ $fldname[$fld_germplasm]['field'] ][$germplasm_name]['#checked'] = FALSE;
  }
  elseif ($triggering_element['command'] == 'germplasm_update_element') {
    // User interacting with field - using delete or backspace to remove 
    // germplasm/stock name.
    $field_attr_id =  $form_state['triggering_element']['#id'];
    $field_value = $form_state['input']['ap_AJAX_germplasmname_field_wrapper']['ap_field_textfield_' . $field_attr_id];
    
    if (in_array($field_value, $form['ap_AJAX_wrapper_main']['ap_additional_filter_fieldset']['ap_AJAX_germplasmname_field_wrapper'][ $fldname[$fld_germplasm]['field'] ]['#options'])) {
      // Field has value.
      $form['ap_AJAX_wrapper_main']['ap_additional_filter_fieldset']['ap_AJAX_germplasmname_field_wrapper'][ $fldname[$fld_germplasm]['field'] ][ $triggering_element['value'] ]['#checked'] = TRUE;  
    }
    else {
      // Read cached value of field before it got erased.
      $field_value = $form_state['values']['ap_cache_germplasmname'];
      $form['ap_AJAX_wrapper_main']['ap_additional_filter_fieldset']['ap_AJAX_germplasmname_field_wrapper'][ $fldname[$fld_germplasm]['field'] ][$field_value]['#checked'] = FALSE;  
      
      // Now we can unset the field in values array and empty the field.
      $form_state['values']['ap_cache_germplasmname'] = '';
    }
  }
  
  // Update AJAX field wrapper.
  return $form['ap_AJAX_wrapper_main']['ap_additional_filter_fieldset']['ap_AJAX_germplasmname_field_wrapper'];
}


/////

/**
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_stats_standard_deviation(array $a, $sample = false) {
  return ap_calculate_standard_deviation($a, $sample);
}

/**
 * FUNCTION:
 * Calculate standard deveiation.
 * Credits to: https://www.mathsisfun.com/data/standard-deviation.html.
 *
 * @param $values
 *   Array of numerical values recorded in Value column header.
 * @param $sample
 *   A boolean true or false to indicate if the values were selection from a larger dataset or population.
 *   Default to FALSE.
 *
 * @return
 *   Standard deviation.
 */
function ap_calculate_standard_deviation(array $values, $sample = FALSE) {
  $n = count($values);

  if ($n === 0) {
    return false;
  }

  if ($sample && $n === 1) {
    return false;
  }

  $mean = array_sum($a) / $n;
  $carry = 0.0;

  foreach ($values as $val) {
    $d = ((double) $val) - $mean;
    $carry += $d * $d;
  };

  if ($sample) {
    --$n;
  }

  return sqrt($carry / $n);
}

//////

/**
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_table_values($rows, $column_row, $trait) {
  return ap_create_summarytable($rows, $column_row, $trait);
}

/**
 * FUNCTION:
 * Construct values for summary table of trait values. Table is displayed in Stage 3 - Describe Trait.
 * Dependencies: ap_create_columnheaders() in columnheaders.api.
 *               analyzedphenotypes_datatypeprop() in datatype.api.
 *
 * @param $rows
 *   All rows/datapoints collected for a trait.
 * @param $column_row
 *   Column header row (row 0) in the file.
 * @param $trait
 *   Trait name.
 *
 * @return
 *   An array of values, summarized by site and year.
 */
function ap_create_summarytable($rows, $column_rows, $traitname) {
  // Fetch expected column headers and data type for each.
  $c = ap_create_columnheaders('columns_datatype');
  $columns = array_keys($c);

  // Reference index number of headers in the column rows.
  $index_trait = array_search($columns[0], $column_row);
  $index_value = array_search($columns[6], $column_row);
  $index_year  = array_search($columns[3], $column_row);
  $index_site  = array_search($columns[4], $column_row);

  $arr_siteyear = array();
  $arr_values = array();

  foreach($rows as $i => $line) {
    if ($i > 0 && !empty($line)) {
      $data = str_getcsv($line, "\t");
      $data = array_map('trim', $data);

      if ($data[$index_trait] == $traitname) {
        $value = $data[$index_value];
        $is_number = analyzedphenotypes_datatypeprop('number', array(
          'value' => $value
        ));

        if ($is_number) {
          $site_year = $data[$index_site] . ' ' . $data[$index_year];

          if (in_array($site_year, $arr_siteyear)) {
            $arr_values[$site_year][] = $value;
          }
          else {
            $arr_siteyear[] = $site_year;
            $arr_values[$site_year][] = $value;
          }
        }
        else {
          return 0;
        }
      }
    }
  }

  sort($arr_siteyear);
  return array('siteyear' => $arr_siteyear, 'values' => $arr_values);
}

//////

/**
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_scale_photos($parameter) {
  return call_user_func('ap_scale_photo', $parameter['height'], $parameter['width'], $parameter['scale_to']);
}

/**
 * FUNCTION:
 * Scale photo/image maintaining the aspect ratio.
 *
 * @param $height
 *   Height of image.
 * @param $width
 *   Width of image.
 * @param $scale_to
 *   Target size of image.
 *
 * @return
 *   An array with reduced height and width as array elements.
 */
function ap_scale_photo($height, $width, $scale_to) {
  $h = $parameter['height'];
  $w = $parameter['width'];
  $scale_to = $parameter['scale_to'];

  $ratio = ($w > $h) ? $scale_to / $w : $scale_to / $h;

  $new_w = round($w * $ratio);
  $new_h = round($h * $ratio);

  return array('height' => $new_h, 'width' => $new_w);
}


//////

/**
 * FUNCTION:
 * Define file extension this module supports.
 *
 * @param $set
 *   A string value requesting a subset of file extensions required in file upload
 *   and file extension used in data export. Default to null which will return
 *   all or both subsets.
 *
 * @return
 *   An array, containing file extensions.
 */
function ap_define_datafile_extensions($set = null) {
  $extensions = array(
    'file_upload'   => array(
      'tsv'  => 'Tab Separated Values',
      'txt'  => 'Text File',
    ),
    'file_download' => array(
      'tsv'  => '.TSV - Tab Separated Values',
      'csv'  => '.CSV - Comman Separated Values',
      'xlsx' => '.XLSX - Microsoft Excel Spreadsheet File',
    )
  );

  return ($set) ? $extensions[$set] : $extensions;
}

/**
 * FUNCTION:
 * Define file destination path for file upload and trait image.
 * 
 * @return 
 *   Associative array where the key is directory type and value as directory path.
 */
function ap_construct_datafile_destinationpath() {
  $drupal_public = 'public://';
  $ap_root_dir = 'tripal/analyzedphenotypes/';
  
  // Add directory here. 
  $directory = array(
    'spreadsheet',
    'traitpictures'
  );
  
  // Constuct directories.
  $file_dir = array();
  
  foreach($directory as $dir) {
    $file_dir[$dir] = $drupal_public . $ap_root_dir . $dir; 
  }
  
  return $file_dir;
}

/**
 * CONTROL/SWITCHBOARD.
 * Pass a request to a corresponding function.
 * @TODO: leave out when api reorganize/update is complete and use functions below.
 */
function analyzedphenotypes_datafileprop($property = null, $parameter = null) {
  if (is_null($property)) {
    // Default.
    $property = 'default';
  }

  // Process request.
  // Default to all projects.
  $command = array(
    'default' => array(
      'func'  => 'ap_define_datafile_extensions',
      'param' => 'file_upload',
    ),
    'download_type' => array(
      'func'  => 'ap_define_datafile_extensions',
      'param' => 'file_download',
    ),
    'make_permanent' => array(
      'func'  => 'ap_make_datafile_permanent',
      'param' => $parameter['data_file'],
    ),
  );

  return call_user_func($command[$property]['func'], $command[$property]['param']);
}

/**
 * FUNCTION:
 * Set the status of file to permanent and inform Drupal File manager that
 * a file is being used by AP module.
 *
 * @param $datafile
 *   Drupal file object.
 */
function ap_make_datafile_permanent($datafile) {
  if ($datafile) {
    $datafile->status = FILE_STATUS_PERMANENT;
    file_save($datafile);

    // Also, point out that we are using it ;-)
    // Note, the file_usage_add() function expects a numerical unique id which we don't have.
    // We have gotten around this by using the uid concatenated with the timestamp using
    // the assumption that a single user cannot upload more than one phenotype file within a second.
    file_usage_add($datafile, 'analyzedphenotypes', 'analyzedphenotypes-file', $datafile->uid . $datafile->timestamp);
  }
}