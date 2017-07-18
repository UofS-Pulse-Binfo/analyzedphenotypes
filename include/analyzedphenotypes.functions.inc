<?php
/**
 * @file
 * Contains helper functions required by this module.
 */


/**
 *
 * Default to return path/directory.
 */
function analyzedphenotypes_moduleprop($property = null) {
  $moduleprop = '';

  switch($property) {
    //
    case 'stages':
      $moduleprop = array(
        'Upload',   // Stage 1 - Select project, genus and upload data file.
        'Validate', // Stage 2 - Validate data.
        'Describe', // Stage 3 - Fully describe all traits.
        'Save',     // Stage 4 - Save data and file.
      );

      break;

    //
    case 'report_frequency':
      // Every 10%.
      $moduleprop = 5;

      break;

    case 'error_limit':
      // Stop when error is x number.
      $moduleprop = 10;

      break;

    //
    // Additional case here.

    //
    default:
      // Return module path.
      $module = 'analyzedphenotypes';
      $tripal_ext = 'admin/tripal/extension';

      $moduleprop = array(
        'module'     => drupal_get_path('module', $module),
        'base_admin' => base_path() . $tripal_ext . '/' . $module,
        'extension'  => base_path() . $tripal_ext,
        'loader'     => base_path() . $tripal_ext . '/' . $module . '/data_loader',
        'temp'       => file_directory_temp(),
        'ap'         => $tripal_ext . '/' . $module,
      );

      break;
  }


  return $moduleprop;
}


/**
 *
 */
function analyzedphenotypes_projectprop($property = null, $data = null) {
  $projectprop = '';

  if ($data) {
    $data = strip_tags($data);
  }

  switch($property) {
    //
    case 'pattern_match_name':
      $sql = "SELECT name FROM {project}
      WHERE LOWER(name) LIKE :project_name ORDER BY name ASC
      LIMIT 10";

      $args = array(':project_name' => '%' . strtolower($data) . '%');
      $result = chado_query($sql, $args);

      $projectprop = ($result->rowCount() > 0) ? $result->fetchAllKeyed(0, 0) : 0;

      break;

    //
    case 'match_name':
      $sql = "SELECT project_id FROM {project}
      WHERE LOWER(name) = TRIM(LOWER(:project_name)) ORDER BY name ASC
      LIMIT 10";

      $args = array(':project_name' => $data);
      $result = chado_query($sql, $args);

      $projectprop = ($result->rowCount() > 0) ? $result->fetchField() : 0;

      break;

    //
    case 'project_genus':
      // Get genus cvterm.
      $t = analyzedphenotypes_cvprop('genus', 'taxonomic_rank');
      $cvid_genus = $t['id'];

      $sql = "SELECT value FROM {projectprop}
      WHERE project_id = :project_id AND type_id = :type_id
      LIMIT 1";

      $args = array(':project_id' => $data, ':type_id' => $cvid_genus);
      $result = chado_query($sql, $args);

      $projectprop = ($result->rowCount() > 0) ? $result->fetchField() : 0;

      break;

     //
     // Additional case here.

    //
    default:
      $sql = "SELECT project_id, name FROM {project} ORDER BY name ASC";
      $result = chado_query($sql);

      $projectprop = ($result->rowCount() > 0) ? $result->fetchAllKeyed(0, 1) : 0;

      break;
  }


  return $projectprop;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




/**
 * Helper function: fetch relevant records about a project.
 */
function analyzedphenotypes_genusprop($property = null, $data = null) {
  $genusprop = '';

  switch($property) {
    //
    case 'project_genus':
      // Get genus cvterm.
      $t = analyzedphenotypes_cvprop('genus', 'taxonomic_rank');
      $cvid_genus = $t['id'];

      $sql = "SELECT value FROM {projectprop}
      WHERE project_id = :project_id AND type_id = :type_id
      LIMIT 1";

      $args = array(':project_id' => $data, ':type_id' => $cvid_genus);
      $result = chado_query($sql, $args);

      $genusprop = ($result->rowCount() > 0) ? $result->fetchField() : 0;

      break;

    //
    // Additional case here.

    //
    default:
      $sql = "SELECT genus FROM {organism}
      GROUP BY genus ORDER BY genus ASC";

      $result = chado_query($sql);
      $genusprop = ($result->rowCount() > 0) ? $result->fetchAllKeyed(0, 0) : 0;
  }


  return $genusprop;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Helper function: fetch relevant information about data file.
 */
function analyzedphenotypes_fileprop($property = null, $data_file = null) {
  $fileprop = '';

  switch($property) {
    //
    case 'get_contents':
      $source = drupal_realpath($data_file->uri);
      $text_data = file_get_contents($source);

      $fileprop = $text_data;

      break;

    //
    case 'readable':
      $source = drupal_realpath($data_file->uri);
      $text_data = file_get_contents($source);

      $fileprop = ($text_data) ? TRUE : FALSE;

      break;

    //
    case 'columns':
      $source = drupal_realpath($data_file->uri);
      $text_data = file_get_contents($source);

      if ($text_data) {
        $rows = explode("\n", $text_data);
        $columns_row = $rows[0];
        unset($rows);

        if (!empty($columns_row)) {
          $fileprop = str_getcsv($columns_row, "\t");
        }
      }

      break;

    //
    case 'filename':
      // Replace @JOB_ID with registered Tripal job id.
      $fileprop = array(
        'validate progress' => 'ap_validate_tripaljob_progress@JOB_ID.txt',
        'validate result'   => 'ap_validate_tripaljob_progress@JOB_ID.txt',
        'save progress'     => 'ap_save_tripaljob_progress@JOB_ID.txt',
      );

      break;

    //
    // Additional case here.

    //
    default:
      $fileprop = array(
        'tsv' => 'Tab Separated Values',
        'txt' => 'Text File',
      );
  }


  return $fileprop;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Helper funciton: get cv or cvterm.
 */
function analyzedphenotypes_columnsprop($property = null) {
  $columnsprop = '';

  $columns = array(
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
    case 'expected':
      $columnsprop = array_keys($columns);

      break;

    //
    case 'combination':
      $cols = array_keys($columns);
      $columnsprop = array(
        $cols[0],  // Trait Name.
        $cols[1],  // Germplasm Accession.
        $cols[3],  // Year.
        $cols[4],  // Location.
        $cols[5],  // Replicate.
      );

      break;

    //
    // Additional case here.

    //
    default:
      $columnsprop = $columns;
  }


  return $columnsprop;
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
      if (preg_match('/\A[^()]+\s*\({1}[^)(]+\)\z/i', $trait) !== 1) {
        $value = $trait;
      }
      break;

    //
    case 'no_unit':
      $value = preg_replace("/.*\(([^)]*)\)/", '', $trait);
      break;

    //
    case 'unit':
      preg_match("/.*\(([^)]*)\)/", $trait, $match);
      $value = (isset($match[1])) ? $match[1] : 'text';
      break;

    //
    case 'sanitize':
      $value = analyzedphenotypes_noformat($trait);
      break;

    //
    case 'record_set':
      // Query cvterm table.
      break;

    //
    case 'data_type':

      break;

    //
    case 'insert':
      /*
      $cv_description = tripal_get_cv(array('name' => 'phenotype_collection_method'));
      $cv_unit = tripal_get_cv(array('name' => 'phenotype_measurement_units'));

      $name           = $data['name'];
      $description    = $data['description'];
      $unit           = $data['unit'];
      $scale          = $data['scale'];
      $crop_ontology  = $data['crop_ontology'];
      $plant_ontology = $data['plant_ontology'];
      $photo_1        = $data['photo_1'];
      $photo_2        = $data['photo_2'];

      $cvterm_unit = tripal_get_cvterm(array('name' => $unit, 'cv_id' => $cv_unit->cv_id));

      $cvterm = tripal_insert_cvterm(array(
        'id' => 'analyzedphenotype_tripal' . $name,
        'name' => $name,
        'cv_name' => 'phenotypes_measurement_types',
        'definition' => $description,
      ));

      chado_insert_record('cvtermprop', array(
        'cvterm_id' => $cvterm->cvterm_id,
        'type_id' => $cv_description->cv_id,
        'value' => $description,
        'rank' => 0,
      ));

      chado_insert_record('cvterm_relationship', array(
        'type_id' => $cv_unit->cv_id,
        'object_id' => $cvterm->cvterm_id,
        'subject_id' => $cvterm_unit->cvterm_id,
      ));

      $traitprop = $cvterm->cvterm_id; */

      break;

    //
  }


  return $value;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *  Function callback: AJAX function callback.
 */
function analyazedphenotypes_AJAX_callback($form, $form_state) {
  $case = $form_state['triggering_element']['#ajax']['case'];

  switch($case) {
    //
    case 'autofill_genus':
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

function analyzedphenotypes_dataprop($property, $data = null) {
  switch($property) {
    //
    case 'sanitize':

      break;

    //
    case 'save':

      break;

    //
    case 'exists':

      break;
  }
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
      $all_path = analyzedphenotypes_moduleprop();
      $all_filename =  analyzedphenotypes_fileprop('filename');
      $file_result   = $all_path['temp'] . '/' . str_replace('@JOB_ID', $job_id, $all_filename['validate result']);
      $result = file_get_contents($file_result);

      $status = json_decode($result, TRUE);
      print theme('analyzedphenotypes_validator_report', array('status' => $status, 'scope' => 'ap-data-scope'));

      break;

    //
    case 'file':
      // Data is validation result array.
      $status = $data;
      return theme('analyzedphenotypes_validator_report', array('status' => $status, 'scope' => 'ap-file-scope'));

      break;

    //
    case 'save':
      print '';

      break;

    //
  }
}


/**
 *
 */
function analyzedphenotypes_tripaljobprop($property, $dataset) {
  $tripaljobprop = '';
  global $user;

  switch($property) {
    //
    case 'new_job':

      $job_id = tripal_add_job(
        $dataset['description'],
        'analyzedphenotypes',
        $dataset['callback'],
        array(
          serialize($dataset['data'])
        ),
        $user->uid
      );

      if ($job_id) {
        $tripaljobprop = $job_id;
      }

      break;

    //
    case 'get_job':
      $job_id = $dataset;
      $job = tripal_get_job($job_id);
      $job_status = array('Completed', 'Error', 'Cancelled');

      if ($job && $job->uid == $user->uid && !in_array($job->status, $job_status)) {
        $tripaljobprop = $job->job_id;
      }

      break;

    //
    // Additional case here.

    //
    default:
  }


  return $tripaljobprop;
}
