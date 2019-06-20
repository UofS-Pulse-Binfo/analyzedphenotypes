<?php
/**
 * @file
 * Contains general API required by this module.
 */



/**
 * FUNCTION:
 * Match a stock property form chado.stock given stock name and stock uniquename combination.
 *
 * @param $germplasm_name
 *   A string, the name of a stock that corresponds to name field in chado.stock.
 * @param $germplasm_uniquename
 *   A string, the uniquename of a stock that corresponds to uniquename field in chado.stock.
 * @param $genus
 *   A string, genus to be use to limit the search of germplasm to only those that match.
 *
 * @return
 *   An integer, the stock id number of the matching record.
 */
function ap_match_germplasm($germplasm_name, $germplasm_uniquename, $genus) {
  if (empty($germplasm_name) || empty($germplasm_uniquename)) {
    return null;
  }
  else {
    $germplasm = trim($germplasm_name) . trim($germplasm_uniquename);

    // Query stock_id by combining name and uniquename columns as the filter combination.
    $result = chado_query(
      "SELECT stock_id FROM {stock} WHERE CONCAT(TRIM(name), TRIM(uniquename)) = :germplasm
      AND organism_id IN (SELECT organism_id FROM {organism} WHERE LOWER(genus) = LOWER(:genus))",
      array(':germplasm' => $germplasm, ':genus' => $genus)
    );

    $result_count = $result->rowCount();

    if ($result_count == 1) {
      return $result->fetchField();
    }
    else {
      // Somethings not right, stock seems to have a duplicate record or
      // name combination did not match a record at all.
      return null;
    }
  }
}


/**
 * FUNCTION:
 * Extract name or unit from trait name following Trait Name (unit) format.
 *
 * @param $traitname
 *   String, trait name in the indicated format.
 * @param $get
 *   String, trait_name or trait_unit - extract trait name or unit respectively.
 *
 * @return
 *   String, trait name or unit.
 *
function ap_format_traitname($traitname, $get = 'trait_unit') {
  if ($get == 'trait_unit') {
    // Extract trait name.
    preg_match("/.*\(([^)]*)\)/", $traitname, $match);
    // Return text if unit cannot be determined.
    $result = (isset($match[1])) ? $match[1] : 'text';
  }
  else {
    // Default to extract unit (unit), excluding parenthesis.
    $result = preg_replace("/\(.+\)/", '', $traitname);
  }


  return trim($result);
}
*/


/**
 * FUNCTION:
 * Prepare validation result window for validation performed on file and data level.
 *
 * File validation.
 *  - A Tripal Job is created, then in file validate, the file gets validated
 *  on file level and validation status will be sent to this function
 *  to be rendered and reported to user.
 *
 *  @see hook_validate() for uploader.
 *
 * Data validation.
 *  - A Tripal Job is created, then performs a data level validation.
 *  The validation result is written in a temporary file using the job id
 *  as the part of the filename. To report the validation result status
 *  that temporary file is retrieved using the job id and contents which is
 *  the validation error status is then rendered.
 *
 * @see hook_validate for uploader
 * @see hook_menu() (validation result)
 *
 * @param $scope
 *   String either File or Data.
 * @param $resource
 *   - data level : scope is data level and validation report is sent to JS.
 *   - file level : scope is file level and validation report is sent to Form API.
 */
function ap_report_validationresult($scope, $resource) {
  if ($scope == 'file') {
    // When scope is file, it is expecting the completed status message of
    // the validation result.
    $status = $resource;
    return theme('analyzedphenotypes_validator_report', array('status' => $status, 'scope' => 'ap-file-scope'));
  }
  elseif ($scope == 'data') {
    // Default to scope data.
    // When scope is data level, it is expecting the Tripal job id number.
    $s = ap_read_tripaljob_progress($resource, 'validateresult');
    $status = json_decode($s, TRUE);

    print theme('analyzedphenotypes_validator_report', array('status' => $status, 'scope' => 'ap-data-scope' . $tripaljobprop));
  }
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
 * NOTE: checking and uncheking in done in hook_form does not seem to work,
 *       thus move this operation to this callback.
 *
 * @see analyzedphenotypes.downloaddata.page.inc.
 */
function ap_AJAX_germplasmfield_callback($form, $form_state) {
  // Trigerring elements value and command option.
  $triggering_element = array(
    'command' => $form_state['triggering_element']['#ajax']['command'],
    'value' => $form_state['triggering_element']['#value']
  );

  // Reference standard field element id element.
  $fldname = ap_construct_download_fieldnames();
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

  $mean = array_sum($values) / $n;
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


/**
 * FUNCTION:
 * Construct values for summary table of trait values. Table is displayed in Stage 3 - Describe Trait.
 * Dependencies: ap_create_columnheaders() in columnheaders.api.
 *               ap_validate_unitvalue() in datatype.api.
 *
 * @param $file
 *   Drupal file object.
 * @param $column_row
 *   Column header row (row 0) in the file.
 * @param $trait
 *   Trait name.
 *
 * @return
 *   An array of values, summarized by site and year.
 */
function ap_create_summarytable($file, $column_row, $trait_key) {
  // Fetch expected column headers and data type for each.
  $c = ap_create_columnheaders('columns_datatype');
  $columns = array_keys($c);

  // Reference index number of headers in the column rows.
  $index_trait  = array_search($columns[0], $column_row);
  $index_method = array_search($columns[1], $column_row);
  $index_unit   = array_search($columns[2], $column_row);
  $index_value  = array_search($columns[8], $column_row);
  $index_year   = array_search($columns[5], $column_row);
  $index_site   = array_search($columns[6], $column_row);

  $arr_siteyear = array();
  $arr_values = array();


  $handle = fopen($file, 'r');

  if ($handle) {
    $i = 0;
    while(!feof($handle)) {
      $row = fgets($handle);
      if ($i > 0 && !empty($row)) {
        // Known Bug: str_getcsv() is removing internal quotes.
        $data = str_getcsv($row, "\t");

        $data = array_map('trim', $data);
        $data = array_map(function($element) {
            $tmp = str_replace('"',"",$element);
            return trim($tmp, "'");
          }, $data);

        $file_unique_key = implode('; ', array($data[$index_trait], $data[$index_method], $data[$index_unit]));
        if ($file_unique_key == $trait_key) {
          // Each row in trait name.
          $value = $data[$index_value];
          $is_number = ap_validate_unitvalue('number', $value);

          if ($is_number) {
            $site_year =  $data[$index_site] . ' ' . $data[$index_year];

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

      $i++;
    }

    fclose($handle);
  }


  sort($arr_siteyear);
  return array('siteyear' => $arr_siteyear, 'values' => $arr_values);
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
  $ratio = ($width > $height) ? $scale_to / $width : $scale_to / $height;

  $new_w = round($width * $ratio);
  $new_h = round($height * $ratio);


  return array('height' => $new_h, 'width' => $new_w);
}


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

  // Construct relative/absolute path eg. public://tripal/analzedphenotypes/spreadsheet
  // - for data file
  $file_dir[$ap_root_dir] = $drupal_public . $ap_root_dir;
  foreach($directory as $dir) {
    $file_dir[$dir] = $drupal_public . $ap_root_dir . $dir;
  }


  return $file_dir;
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


/**
 * FUNCTION:
 * Convert text value to lowercase and remove leading and trailing spaces.
 *
 * @param $string
 *   String, text value.
 */
function ap_noformat($string) {
  return strtolower(trim($string));
}


/**
 * FUNCTION:
 * Shortens a number in K/M/B.
 *
 * @param $value
 *   An integer containing the number to be formatted.
 *
 * @return string
 *   Shortened result.
 */
function ap_shortval($value) {
  if ($value < 1000) {
    // Hundred and less.
    $value = $value;
  }
  else if ($value < 1000000) {
    // Thousand.
    $value = round($value / 1000, 0) . 'K';
  }
  else if ($value < 1000000000) {
    // Million.
    $value = round($value / 1000000, 0) . 'M';
  }
  else {
    // Billion.
    $value = round($value / 1000000000, 0) . 'B';
  }

  return $value;
}
