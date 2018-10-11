<?php
/**
 * @file
 *
 * API: TRIPAL JOB
 * Manage property and operation pertaining to Tripal Job.
 */


/**
 * FUNCTION:
 * Define configuration used in Tripal job process.
 *
 * @return
 *   An array, Tripal job configuration.
 */
function ap_configure_tripaljob() {
  // Configurations:
  // file_path : temp directory where file is generated. File is used to write and read progress.
  // job_tabl  : chado jobs table.
  // error_type : TRIPAL_CRITICAL.
  // error_limit : number of errors encountered to stop validation process and report errors found.
  // write_option : write option used by file unmanaged implementation.
  // source_module : calling module, in this case AP.
  // file_extension : job uses txt file.
  // report_frequencey : report every x number of error to terminal.

  return array(
    'TRIPALJOB_file_path' => file_directory_temp(),
    'TRIPALJOB_job_table'  => 'tripal_jobs',
    'TRIPALJOB_error_type'  => TRIPAL_CRITICAL,
    'TRIPALJOB_error_limit'  => 5,
    'TRIPALJOB_write_option'  => 'FILE_EXISTS_REPLACE',
    'TRIPALJOB_source_module'  => 'analyzedphenotypes',
    'TRIPALJOB_file_extension'  => 'txt',
    'TRIPALJOB_report_frequency' => 5
  );
}


///////
/**
 * CONTROL/SWITCHBOARD.
 * Pass a request to a corresponding function.
 * @TODO: leave out when api reorganize/update is complete and use functions below.
 */
function analyzedphenotypes_triapjob($property = null, $parameter = null) {
  if (is_null($property)) {
    // Default.
    $property = 'default';
  }
  // Process request.
  $command = array(
    'default' => array(
      'func'  => 'ap_configure_tripaljob',
      'param' => null,
      'opts'  => null,
      'val'   => null,
    ),
    'new_job' => array(
      'func'  => 'ap_create_tripaljob',
      'param' => array('description' => $parameter['description'], 'dataset' => $parameter['dataset'], 'callback' => $parameter['callback']),
      'opts'  => null,
      'val'   => null,
    ),
    'get_job' => array(
      'func'  => 'ap_get_tripaljob',
      'param' => $parameter['job_id'],
      'opts'  => null,
      'val'   => null,
    ),
    'update_progress' => array(
      'func'  => 'ap_update_tripaljob_progress',
      'param' => $parameter['job_id'],
      'opts'  => $parameter['progress'],
      'val'   => null,
    ),
    'write_progress' => array(
      'func'  => 'ap_readwrite_tripaljob_progress',
      'param' => $parameter['job_id'],
      'opts'  => $parameter['source'],
      'val'   => $parameter['progress'],
    ),
    'read_progress' => array(
      'func'  => 'ap_readwrite_tripaljob_progress',
      'param' => $parameter['job_id'],
      'opts'  => $parameter['source'],
      'val'   => null,
    ),
    'tripal_fail' => array(
      'func'  => 'ap_echo_tripaljob_error',
      'param' => $parameter['tripal_job'],
      'opts'  => $parameter['param'],
      'val'   => $parameter['message'],
    ),
  );
  return call_user_func($command[$property]['func'], $command[$property]['param'], $command[$property]['opts'], $command[$property]['val']);
}


/**
 * FUNCTION:
 * Register a Tripal Job.
 * Dependencies: ap_configure_tripaljob().
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - description - a text description of job.
 *   - dataset - an array contaning data to be processed.
 *   - callback - string, the name of the function (callback) used to process data in $dataset array.
 * @param $options
 *   An array to be used as additional options to be performed to
 *   returned result. Keys:
 *   - return_job - boolean true or false, return Tripal job object when set to true. Default to true.
 *
 * @return
 *   An integer, Tripal job object.
 */
function ap_create_tripaljob($identifiers, $options = array('return_jobid' => TRUE)) {
  $tripal_config = ap_configure_tripaljob();
  global $user;

  $job_id = tripal_add_job(
    $identifiers['description'],
    $tripal_config['TRIPALJOB_source_module'],
    $identifiers['callback'],
    serialize($identifiers['dataset']),
    $user->uid
  );

  if ($job_id > 0 && $options['return_jobid']) {
    return $job_id;
  }
}

/**
 * FUNCTION:
 * Fetch registered Tripal job process.
 * Dependencies: ap_configure_tripaljob().
 *
 * @param $tripaljob_id
 *   An integer, Tripal job id number.
 *
 * @return
 *   An array, Tripal job process properties/information.
 */
function ap_get_tripaljob($tripaljob_id) {
  $tripal_config = ap_configure_tripaljob();
  global $user;

  $job = tripal_get_job($tripaljob_id);
  if ($job->job_id > 0 && $job->uid && $job->modulename == $tripal_config['TRIPALJOB_source_module']) {
    return array(
      'job_id'   => $job->job_id,
      'status' => $job->status,
      'uid' => $job->uid,
      'progress' => $job->progress
    );
  }
}

/**
 * FUNCTION:
 * Write to file Tripal job computed progress (0 - 100%).
 * Dependencies: ap_configure_tripaljob().
 *
 * @param $tripaljob_id
 *   An integer, Tripal job id number.
 * @param $jobtype
 *   A string, resquesting section of the module calling the tripal job.
 *   jobprogress for standard progress bar and validateresult for validation process.
 * @param $progress
 *   Integer value, progress of Tripal job.
 *
 * @return
 *   Drupal file managed object.
 */
function ap_update_tripaljob_progress($tripaljob_id, $progress) {
  $tripal_config = ap_configure_tripaljob();
  
  $update = db_update($tripal_config['TRIPALJOB_job_table'])
    ->fields(array('progress' => $progress))
    ->condition('job_id', $tripaljob_id, '=')
    ->execution();

  return ($update) ? $update : null;
}

/**
 * FUNCTION:
 * Read and write Tripal job progres to a temporary file.
 * Dependencies: ap_configure_tripaljob().
 *
 * @param $tripaljob_id
 *   An integer, Tripal job id number.
 * @param $source
 *   Source of Tripal job request:
 *   - jobprogress : for standard progress bar in upload data.
 *   - validateresult : perform validation.
 * @param $progress
 *   An integer, computed progress of a job from 0 to 100, begin to completed respectively.
 *
 * @return
 *   Drupal file mangaed object.
 */
function ap_readwrite_tripaljob_progress($tripaljob_id, $source, $progress = null) {
  $tripal_config = ap_configure_tripaljob();

  // Construct file name of Tripal job progress file.
  // Filename consist of tripal job id number and the source of Tripal job calls.
  $file = $tripal_config['TRIPALJOB_file_path'] . '/ap_' . $source . '_tripaljob_' . $tripaljob_id . '.' . $tripal_config['TRIPALJOB_file_extension'];

  if (file_exists($file)) {
    if ($progress >= 0) {
      // When progress 0-100 is provided, it wants to write progress to file.
      $tripal_update = file_unmanaged_save_data($progress, $file, $tripal_config['TRIPALJOB_write_option']);
    }
    else {
      // Null progress, read progress form file.
      $tripal_update = file_get_contents($file);
    }
  }

  return ($tripal_update) ? $tripal_update : null;
}

/**
 * FUNCTION:
 * Print Tripal job progress error message to terminal.
 * Dependencies: ap_configure_tripaljob().
 *
 * @param $tripaljob_id
 *   An integer, Tripal job id number.
 * @param $item
 *   The value in the TSV file that caused Tripal job process to fail.
 * @param $message
 *   Text message shown relating to the failure.
 */
function ap_echo_tripaljob_error($tripaljob_id, $item, $message) {
  $tripal_config = ap_configure_tripaljob();
  $true_print = array('print' => TRUE);

  tripal_report_error($ap, $tripal_config['TRIPALJOB_error_type'], "\n" . $message . ' [VALUE: @param]' . "\n",
    array('@param' => $item),
    $true_print
  );

  tripal_report_error($ap, $tripal_config['TRIPALJOB_error_type'], 'Failed to load phenotypic data in Tripal Job #!job_id.',
    array('!job_id' => $tripaljob_id),
    $true_print
  );
}
