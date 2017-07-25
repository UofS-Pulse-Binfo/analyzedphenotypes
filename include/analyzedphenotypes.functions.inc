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
    case 'module_vars':
      $default_db = analyzedphenotypes_dbprop('default');
      $default_cv = analyzedphenotypes_cvprop('default');
      $default_allow_new = 1;

      $moduleprop = array(
        'ap_database'  => array(
          'var' => 'analyzedphenotypes_module_db',
          'default' => $default_db
        ),

        'ap_cv' => array(
          'var' => 'analyzedphenotypes_module_cv',
          'default' => $default_cv
        ),

        'ap_allow_new' => array(
          'var' =>  'analyzedphenotypes_module_allow_new',
          'default' => $default_allow_new
        )
      );

      break;


    //
    case 'db':
      $moduleprop = 'analyzedphenotype-tripal';

      break;

    //
    case 'column_type':
      $moduleprop = array(
        'Trait Name'          => 'text',   // #0
        'Germplasm Accession' => 'text',   // #1
        'Germplasm Name'      => 'text',   // #2
        'Year'                => 'number', // #3
        'Location'            => 'text',   // #4
        'Replicate'           => 'number', // #5
        'Value'               => '',       // #6
        'Data Collector'      => 'text',   // #7
      );

      break;

    //
    case 'unit_type':
      $moduleprop = array(
        'cm'         => array('name' => 'centimeters', 'type' => 'number'),
        'days'       => array('name' => 'days',        'type' => 'number'),
        'count'      => array('name' => 'count',       'type' => 'number'),
        'g'          => array('name' => 'grams',       'type' => 'number'),
        'kg'         => array('name' => 'kilograms',   'type' => 'number'),
        'scale 1-5'  => array('name' => 'scale 1-5',   'type' => 'scale 1-5'),
        'scale 1-10' => array('name' => 'scale 1-10',  'type' => 'scale 2-5'),
        'comments'   => array('name' => 'comments',    'type' => 'text'),
        'colour'     => array('name' => 'color',       'type' => 'text'),
        'y/n/?'      => array('name' => 'Yes (y), No (n) or Unsure (?)',  'type' => 'y/n/?'),
      );

      break;

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
      // Every 5%.
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
        'loader'     => base_path() . $tripal_ext . '/' . $module . '/upload',
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
      $terms = array(
        'term' => 'genus',
        'cv' => 'taxonomic_rank'
      );

      $t = analyzedphenotypes_cvprop('get_term', $terms);
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
      $term = array(
        'term' => 'genus',
        'cv'   => 'taxonomic_rank'
      );

      $t = analyzedphenotypes_cvprop('get_term', $term);
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

    case 'match_combination':
      // REPLACE THIS WITH AN SQL QUERY TO FIND THIS STRING
      // IN THE TABLE.
      $combo = 'Plant Height (cm)KP:GERM260282015Saskatchewan, Canada1';
      if ($combo == $data) {
        $columnsprop = TRUE;
      }

      break;

    //
    // Additional case here.

    //
    default:
      $columnsprop = $columns;
  }


  return $columnsprop;
}

/////////
function analyzedphenotypes_datatypeprop($property, $data) {
  $datatypeprop = TRUE;

  switch($property) {
    //
    case 'text':
      if (ctype_digit($data)) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'number':
      if (is_numeric($data) && $data <= 0) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'scale 1-5':
      $data = (int)$data;
      $min = 1;
      $max = 5;
      if ($data < $min || $data > $max) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'scale 1-10':
      $data = (int)$data;
      $min = 1;
      $max = 10;
      if ($data < $min || $data > $max) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'y/n/?':
      $enum = array('y', 'yes', 'n', 'no', 'unsure', '?');
      if (!in_array($data, $enum)) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'date':
      if (preg_match('/^([0-9]{4})-([0-9]{2})-([0-9]{2})/', $data, $matches)) {
        $year  = $matches[1];
        $month = $matches[2];
        $day   = $matches[3];

        if ($month >= 1 AND $month <= 12 AND $day >= 1 AND $day <= 31 AND $year > 1900) {
          $today = new DateTime();
          $date = DateTime::createFromFormat('Y-m-d', $value);

          if ($date <= $today) {
            $datatypeprop = FALSE;
          }
        }
      }

      break;

     //
     // Additional case here.
  }

  return $datatypeprop;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Helper funciton: get cv or cvterm.
 */
function analyzedphenotypes_cvprop($property = null, $data = null) {
  $cvprop = '';

  switch($property) {
    //
    case 'default':
      $default_cv = 'tripal';

      $c = tripal_get_cv(array('name' => $default_cv));
      $cvprop = $c->cv_id;

      break;

    //
    case 'get_term':
      $term = array(
        'name' => $data['term'],
        'cv_id' => array('name' => $data['cv'])
      );

      $cvterm = tripal_get_cvterm($term);
      $cvprop = array(
        'id' => $cvterm->cvterm_id,
        'name' => $cvterm->name,
      );

      break;

    //
    default:
      $sql = "SELECT cv_id, name, definition
      FROM {cv}
      ORDER BY name ASC";

      $cv = chado_query($sql);

      if ($cv->rowCount() > 0) {
        foreach($cv as $c) {
          $cvprop[$c->cv_id] = array(
            'name' => $c->name,
            'definition' => $c->definition,
          );
        }
      }
  }

  return $cvprop;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *
 */
function analyzedphenotypes_traitprop($case, $trait) {
  $traitprop = '';

  switch($case) {
    //
    case 'right_format':
      if (preg_match('/\A[^()]+\s*\({1}[^)(]+\)\z/i', $trait) !== 1) {
        $traitprop = $trait;
      }

      break;

    //
    case 'no_unit':
      $traitprop = preg_replace("/.*\(([^)]*)\)/", '', $trait);

      break;

    //
    case 'unit':
      preg_match("/.*\(([^)]*)\)/", $trait, $match);
      $traitprop = (isset($match[1])) ? $match[1] : 'text';

      break;

    //
    case 'sanitize':
      $traitprop = analyzedphenotypes_noformat($trait);

      break;

    //
    case 'recordset':
      //$cvterm = tripal_get_cvterm($trait);

     $traitprop = array(
        'name'           => 'Test Name',
        'definition'     => 'Test Definition',
        'unit'           => 'Test Unit',
        'scale'          => 'Test Scale',
        'crop_ontology'  => 'Test Crop Ontology',
        'plant_ontology' => 'Test Plant Ontology',
        'photo_1'        => 'Test Photo 1',
        'photo_2'        => 'Test Photo 2',
      );

      break;

    //
    case 'data_type':

      break;

    //
    case 'insert_trait':






      break;

    //
    case 'property_insert':

      break;

    //
  }


  return $traitprop;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *  Function callback: AJAX function callback.
 */
function analyzedphenotypes_AJAX_callback($form, $form_state) {
  $case = $form_state['triggering_element']['#ajax']['case'];
  $element = 'ap_AJAX_wrapper' . '_' . $case;


  return $form[$element];
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

function analyzedphenotypes_dbprop($property = null, $data = null) {
  $dbprop = '';

  $sql = "SELECT db_id, name, description
  FROM {db}
  ORDER BY name ASC";

  $db = chado_query($sql);

  switch($property) {
    //
    case 'default':
      $default_db = 'tripal';

      if ($db->rowCount() > 0) {
        foreach($db as $d) {
          if ($d->name == $default_db) {
            $dbprop = $d->db_id;
          }
        }
      }

      break;

    //
    // Additional case here.

    //
    default:
      if ($db->rowCount() > 0) {
        foreach($db as $d) {
          $dbprop[$d->db_id] = array(
            'name' => $d->name,
            'description' => $d->description,
          );
        }
      }
  }

  return $dbprop;
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
    case 'save':
      print '';

      break;

    //
    // Additional case here.

    //
    default:
  }


  return $tripaljobprop;
}
