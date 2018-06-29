<?php
/**
 * @file
 *
 * API: DATA FILE
 * Manage property and operation pertaining to tsv or txt data file.
 */


/**
 * DATA SOURCE
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


///////


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
    'get_contents' => array(
      'func'  => 'ap_get_datafile_contents',
      'param' => $parameter['data_file'],
    ),
    'get_columns' => array(
      'func'  => 'ap_get_datafile_columns',
      'param' => $parameter['data_file'],
    ),
    'make_permanent' => array(
      'func'  => 'ap_make_datafile_permanent',
      'param' => $parameter['data_file'],
    ),
  );

  return call_user_func($command[$property]['func'], $command[$property]['param']);
}


///////


/**
 * FUNCTION:
 * Read data file and return file data contents.
 *
 * @param $datafile
 *   Drupal file object.
 *
 * @return
 *   File contents.
 */
function ap_get_datafile_contents($datafile) {
  $file = drupal_realpath($datafile->uri);
  if (file_exists($file)) {
    $datafile_contents = file_get_contents($source);

    if ($datafile_contents) {
      $contents = $datafile_contents;
    }
  }

  return ($contents) ? $contents : null;
}

/**
 * FUNCTION:
 * Read data file and return column headers which is the first row of file contents.
 * Dependencies: ap_get_datafile_columns().
 *
 * @param $datafile
 *   Drupal file object.
 *
 * @return
 *   An array, containing column headers.
 */
function ap_get_datafile_columns($datafile) {
  $contents = ap_get_datafile_contents();

  if ($contents) {
    $rows = explode("\n", $contents);
    $column_row = $rows[0];
    unset($rows);
  }

  return (!empty($column_row)) ? str_getcsv($column_row, "\t") : null;
}

/**
 * FUNCTION:
 * Set the status of file to permanent and inform Drupal File manager that
 * a file is being used by AP module.
 *
 * @param datafile
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
