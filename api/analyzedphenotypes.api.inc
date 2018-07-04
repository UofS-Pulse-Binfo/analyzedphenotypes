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
 * Dependencies: analyzedphenotypes_columnsprop() in columnheaders.api.
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
  $c = analyzedphenotypes_columnsprop();
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
