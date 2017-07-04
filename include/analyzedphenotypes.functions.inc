<?php
/**
 * @file
 * Contains helper functions required by this module.
 */

/**
 * HELPER FUNCTION: Fetch relevant records about a project.
 */
function analyzedphenotypes_projectprop($property, $key = null) {
  if ($key) {
    $key = strip_tags($key);
  }

  switch($property) {
    //
    case 'project_all':
      $sql = "SELECT project_id, name FROM {project} ORDER BY name ASC";
      $result = chado_query($sql);

      $data = ($result->rowCount() > 0) ? $result->fetchAllKeyed(0, 1) : 0;

      break;

    //
    case 'project_pattern_match_name':
      $sql = "SELECT name FROM {project}
      WHERE LOWER(name) LIKE :project_name ORDER BY name ASC
      LIMIT 10";

      $args = array(':project_name' => '%' . strtolower($key) . '%');
      $result = chado_query($sql, $args);

      $data = ($result->rowCount() > 0) ? $result->fetchAllKeyed(0, 0) : 0;

      break;

    //
    case 'project_match_name':
      $sql = "SELECT project_id FROM {project}
      WHERE LOWER(name) = TRIM(LOWER(:project_name)) ORDER BY name ASC
      LIMIT 10";

      $args = array(':project_name' => $key);
      $result = chado_query($sql, $args);

      $data = ($result->rowCount() > 0) ? $result->fetchField() : 0;

      break;

    //
    case 'project_genus':
      // Get genus cvterm.
      $t = analyzedphenotypes_cvprop('genus', 'taxonomic_rank');
      $cvid_genus = $t['id'];

      $sql = "SELECT value FROM {projectprop}
      WHERE project_id = :project_id AND type_id = :type_id
      LIMIT 1";

      $args = array(':project_id' => $key, ':type_id' => $cvid_genus);
      $result = chado_query($sql, $args);

      $data = ($result->rowCount() > 0) ? $result->fetchAllKeyed(0, 0) : 0;

      break;
  }


  return $data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




/**
 * Helper function: fetch relevant records about a project.
 */
function analyzedphenotypes_genusprop() {
  // All genus.
  $sql = "SELECT genus FROM {organism}
  GROUP BY genus ORDER BY genus ASC";

  $result = chado_query($sql);

  return ($result->rowCount() > 0) ? $result->fetchAllKeyed(0, 0) : 0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Helper function: fetch relevant information about data file.
 */
function analyzedphenotypes_fileprop($property, $data_file) {
  $source = drupal_realpath($data_file->uri);
  $text_data = file_get_contents($source);

  switch($property) {
    //
    case 'readable':
      $readable = ($text_data) ? TRUE : FALSE;

      return $readable;

      break;

    //
    case 'columns':
      if ($text_data) {
        $rows = explode("\n", $text_data);

        foreach($rows as $i => $line) {
          if ($i == 0) {
            $arr_columns = str_getcsv($line, "\t");
            return $arr_columns;
          }
        }
      }

      break;

    //
    case 'rows':

      break;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Helper funciton: get cv or cvterm.
 */
function analyzedphenotypes_columnsprop($property) {
  $columns_prop = array(
    'Trait Name'          => 'text',   // #0
    'Germplasm Accession' => 'text',   // #1
    'Germplasm Name'      => 'text',   // #2
    'Year'                => 'number', // #3
    'Location'            => 'text',   // #4
    'Replicate'           => 'number', // #5
    'Value'               => '',       // #6
    'Data Collector'      => 'text',   // #7
  );


  switch($property) {
    //
    case 'all':
      $arr_columns = array_keys($columns_prop);

      break;

    //
    case 'expected':
      $arr_columns = array_keys($columns_prop);

      break;

    //
    case 'combination':
      $cols = array_keys($columns_prop);
      $arr_columns = array(
        $cols[0],
        $cols[2],
        $cols[3],
        $cols[4],
        $cols[5],
      );

      break;

    //
    case 'data_type':
      $arr_columns = $columns_prop;

      break;

    //
  }


  return $arr_columns;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Helper funciton: get cv or cvterm.
 */
function analyzedphenotypes_cvprop($term, $cv) {
  $term = array(
    'name' => $term,
    'cv_id' => array('name' => $cv)
  );

  $cvterm = tripal_get_cvterm($term);

  if ($cvterm) {
    return array(
      'id' => $cvterm->cvterm_id,
      'name' => $cvterm->name,
    );
  }
  else {
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *
 */
function analyzedphenotypes_traitprop($case, $trait) {
  switch($case) {
    //
    case 'right_format':
      break;

    //
    case 'no_unit':
      break;

    //
    case 'unit':
      break;

    //
    case 'sanitize':
      break;

    //
    case 'record_set':
      break;

    //
    case 'data_type':
      break;


    //
  }
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *  Function callback: AJAX function callback.
 */
function analyazedphenotypes_AJAX_callback($form, $form_state) {
  $case = $form_state['triggering_element']['#ajax']['case'];

  switch($case) {
    //
    case 'autocomplete_search_project':
      break;

    //
  }

  return $form['ap_AJAX_wrapper'];
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Helper function: Remove leading and trailing spaces. Return string in lowercase.
 */
function analyzedphenotypes_noformat($string) {
  return strtolower(trim($string));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *
 */
function analyzedphenotypes_validation_result($case, $data) {
  switch($case) {
    //
    case 'data':
      $job_id = $data;
      $tmp_dir = file_directory_temp();
      $filename_error = $tmp_dir . '/ap-validate-job-status' . $job_id . '.txt';
      $e = file_get_contents($filename_error);
      $status = json_decode($e, TRUE);

      print theme('analyzedphenotypes_validator_report', array('status' => $status));

      break;

    //
    case 'file':
      // Data is validation result array.
      return theme('analyzedphenotypes_validator_report', array('status' => $data));

      break;

    //
  }
}

