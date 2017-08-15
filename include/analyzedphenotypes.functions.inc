<?php
/**
 * @file
 * Contains helper functions required by this module.
 */


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_projectprop($property = null, $parameter = null) {
  $projectprop = null;

  $result = chado_query(
    "SELECT project_id, TRIM(name) AS name FROM {project} ORDER BY name ASC"
  );

  if ($result->rowCount() <= 0) {
    return $projectprop;
  }

  switch($property) {
    //
    case 'match_key':
      // Return all project matching a keyword, as an
      // associative array where key is project_id and
      // value is the project name.
      $keyword = trim($parameter['project_keyword']);

      // Match project that contains keyword.
      // To ensure list will not grow longer, limit the result to x.
      $projectprop_limitrows = 10;

      foreach($result as $i => $project) {
        if (($i + 1) > $projectprop_limitrows) {
          break;
        }

        $project_name = $project->name;

        if (stripos($project_name, $keyword) !== FALSE) {
          $projectprop[$project->project_id] = $project_name;
        }
      }

      break;

    //
    case 'match_name':
      // Return all project matching project name, as an
      // associative array where key is project_id and
      // value is the project name.
      $name = trim($parameter['project_name']);

      foreach($result as $project) {
        $project_name = $project->name;

        if (strtolower($project_name) == strtolower($name)) {
          $projectprop[$project->project_id] = $project_name;

          break;
        }
      }

      break;

    //
    // Define additional case here.

    //
    default:
      // Return all project as an associative array where the key
      // is the project id and the value as the project name.
      $projectprop = $result->fetchAllKeyed(0, 1);
  }


  return $projectprop;
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_genusprop($property = null, $parameter = null) {
  $genusprop = null;

  // Term: genus
  // CV: taxonimic_rank
  $genusprop_term = array(
    'name' => 'genus',
    'cv_id' => array('name' => 'taxonomic_rank'),
  );

  switch($property) {
    //
    case 'initialize':
      // On install, see if the term genus in cv taxonomic rank is
      // present in the system.
      $genusprop_cv = tripal_get_cvterm($genusprop_term);
      $genusprop = ($genusprop_cv) ? 1 : $genusprop;

      break;

    //
    case 'project_genus':
      // Given a project id, test to see if a genus has been
      // associated in chado.projectprop table.
      $genusprop_cv = tripal_get_cvterm($genusprop_term);

      if ($genusprop_cv) {
        $genusprop_cvid = $genusprop_cv->cvterm_id;
        unset($genusprop_cv);

        $result = chado_query(
          "SELECT value FROM {projectprop} WHERE type_id = :type_id AND project_id = :project_id LIMIT 1",
          array(
            ':type_id' => $genusprop_cvid,
            ':project_id' => $parameter['project_id'],
          )
        );
      }

      $genusprop = ($result) ? $result->fetchField() : $genusprop;

      break;

    //
    // Define additional case here.

    //
    default:
      // Return all genus.
      $result = chado_query(
        "SELECT genus FROM {organism} GROUP BY genus ORDER BY genus ASC"
      );

      $genusprop = ($result) ? $result->fetchCol() : $genusprop;
  }


  return $genusprop;
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_stockprop($property, $parameter = null) {
  $stockprop = null;

  switch($property) {
    //
    case 'match_name':
      // Match a given stock/germplasm name and accession.
      $result = chado_query(
        "SELECT stock_id FROM {stock} WHERE name = :name AND uniquename = :uniquename LIMIT 1",
        array(
          ':name' => $data['name'],
          ':uniquename' => $data['uniquename'],
        )
      );

      $stockprop = ($result) ? $result->fetchField() : $stockprop;

      break;

    //
    // Define additional case here.

    // Default:
  }

  return $stockprop;
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_datafileprop($property = null, $data_file = null) {
  $datafileprop = null;

  switch($property) {
    //
    case 'get_contents':
      // Read the supplied file and return the contents.
      $source = drupal_realpath($data_file->uri);
      $datafile_contents = file_get_contents($source);

      if ($datafile_contents) {
        $datafileprop = $datafile_contents;
      }

      break;

    //
    case 'get_columns':
      // Read the file supplied and return the columns row.
      $source = drupal_realpath($data_file->uri);
      $datafile_contents = file_get_contents($source);

      if ($datafile_contents) {
        $rows = explode("\n", $datafile_contents);
        $columns_row = $rows[0];
        unset($rows);

        if (!empty($columns_row)) {
          $datafileprop = str_getcsv($columns_row, "\t");
        }
      }

      break;

    //
    // Define additional case here.

    //
    default:
      $datafileprop = array(
        'tsv' => 'Tab Separated Values',
        'txt' => 'Text File',
      );
  }


  return $datafileprop;
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_columnsprop($property = null, $parameter = null) {
  $columnsprop = null;

  // Trait name, data type, index number
  $columnsprop_expectedcols = array(
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
    case 'combination':
      $cols = array_keys($columnsprop_expectedcols);

      $columnsprop = array(
        $cols[0],  // Trait Name.
        $cols[1],  // Germplasm Accession.
        $cols[3],  // Year.
        $cols[4],  // Location.
        $cols[5],  // Replicate.
      );

      break;

    //
    // Define additional case here.

    //
    default:
      // Default to all expected columns. An associative array
      // where the key is the column and value is the data type.
      $columnsprop = $columnsprop_expectedcols;
  }


  return $columnsprop;
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_datatypeprop($property, $parameter) {
  $datatypeprop = TRUE;

  $value = $paramenter['value'];

  switch($property) {
    //
    case 'unit_type':
      $datatypeprop = array(
        'g' => array('name' => 'grams', 'type' => 'number'),
        'kg' => array('name' => 'kilograms', 'type' => 'number'),
        'cm'  => array('name' => 'centimeters', 'type' => 'number'),
        'days' => array('name' => 'days', 'type' => 'number_no_zero'),
        'y/n/?' => array('name' => 'Yes (y), No (n) or Unsure (?)', 'type' => 'y/n/?'),
        'count'  => array('name' => 'count', 'type' => 'number'),
        'colour'  => array('name' => 'color', 'type' => 'text'),
        'comments' => array('name' => 'comments', 'type' => 'text'),
        'scale_1-5' => array('name' => 'scale 1-5', 'type' => 'scale 1-5'),
        'scale_1-10' => array('name' => 'scale 1-10', 'type' => 'scale 2-5'),
      );

      break;

    //
    case 'text':
      if (ctype_digit($value)) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'number':
      if (!is_numeric($value)) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'number_no_zero':
      if (is_numeric($value) && $value <= 0) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'scale_1-5':
      $data = (int)$value;
      $min = 1;
      $max = 5;

      if ($value < $min || $value > $max) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'scale_1-10':
      $data = (int)$value;
      $min = 1;
      $max = 10;

      if ($value < $min || $value > $max) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'y/n/?':
      $enum = array('y', 'yes', 'n', 'no', 'unsure', '?');
      if (!in_array($value, $enum)) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'date':
      if (preg_match('/^([0-9]{4})-([0-9]{2})-([0-9]{2})/', $value, $matches)) {
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


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_cvprop($property = null, $parameter = null) {
  $cvprop = null;

  switch($property) {
    //
    case 'match_cvid':
      // Match controlled vocabulary (cv) given a cv_id number.
      $cv = tripal_get_cv(array(
        'cv_id' => $parameter['cv_id'],
      ));

      if ($cv) {
        $cvprop[$cv->cv_id] = array(
          'name' => $cv->name,
          'definition' => $cv->definition,
        );
      }

      break;

    //
    case 'terms_in_cv':
      // Fetch all cvterms in a controlled vocabulary.
      $terms = chado_query(
        "SELECT cvterm_id, name, definition FROM {cvterm} WHERE cv_id = :cv_id ORDER BY name ASC",
        array(':cv_id' => $parameter['cv_id'])
      );

      if ($terms->rowCount() > 0) {
        foreach($terms as $term) {
          $cvprop[$term->cvterm_id] = array(
            'name' => $term->name,
            'definition' => $term->definition,
          );
        }
      }

      break;

    //
    case 'get_cvterm':
      // Get term definition in chado.cvterm and chado.db.
      $vars = analyzedphenotypes_moduleprop('settings');
      $cv_id = variable_get($vars['ap_cv']);

      $term = array();
      $term['cv_id'] = array('cv_id' => $cv_id);
      $index = isset($parameter['cvterm_id']) ? 'cvterm_id' : 'name';
      $term[$index] = $parameter[$index];

      $cvterm = tripal_get_cvterm($term);

      if ($cvterm) {
        $cvprop = array(
          'cvterm_id'  => $cvterm->cvterm_id,
          'dbxref_id'  => $cvterm->dbxref_id,
          // An object.
          'cv_id'      => $cvterm->cv_id->cv_id,
          'cv_name'    => $cvterm->cv_id->name,
          'name'       => $cvterm->name,
          'definition' => $cvterm->definition,
          'photo_1'    => $cvterm->cvterm_id . '_1.jpg',
          'photo_2'    => $cvterm->cvterm_id . '_2.jpg',
        );

        // Databse.
        $result = chado_query(
          "SELECT db_id FROM {dbxref} WHERE dbxref_id = :dbxref_id LIMIT 1",
          array(':dbxref_id' => $cvprop['dbxref_id'])
        );

        if ($result->rowCount() == 1) {
          $db_id = $result->fetchField();

          $cvprop['db_id'] = $db_id;
        }
        else {
          $cvprop['db_id'] = null;
        }

        // Crop ontology.
        $cvprop['crop_ontology'] = null;

        // Scale.
        $cvprop['scale'] = null;
      }

      break;

    //
    case 'insert_cvterm':
      // Insert a cterm record.
      $vars = analyzedphenotypes_moduleprop('settings');

      $db = variable_get($vars['ap_database']);
      $cv = variable_get($vars['ap_cv']);
      $allow_new = variable_get($vars['ap_allow_new']);

      // Find the term in the controlled vocabulary.
      $cvterm = analyzedphenotypes_cvprop('get_cvterm', array(
        'cvterm_name' => $parameter['cvterm_name'],
      ));

      if ($cvterm) {
        if ($cvterm['definition']   == $parameter['definition']
            && $cvterm['db_id'] == $db
            && $cvterm['cv_id']->cv_id == $cv) {

          // Term exists
          $traitprop = $cvterm['id'];
        }
        else {
          // A new trait.
          $newtrait = 1;
        }
      }

      if (!$cvterm || isset($newtrait)) {
        if ($allow_new) {
          $cv_name = analyzedphenotypes_cvprop('get_cvname', $cv);
          $db_name = analyzedphenotypes_dbprop('get_dbname', $db);

          // Save it.
          $cvterm = tripal_insert_cvterm(array(
            'id' => trim($db_name) . ':' . $data['cvterm'],
            'name' => $parameter['cvterm_name'],
            'cv_name'  => $cv_name,
            'definition' => $parameter['definition'],
          ));

          $traitprop = $cvterm->cvterm_id;
        }
      }

    //
    case 'cvterm_save_photo':
      // Save photo for a given trait (cvterm).
      $cvterm_id = $parameter['cvterm_id'];

      foreach($parameter['photo'] as $i => $photo) {
        $tmp_file = file_save_upload($photo, array(), FALSE, FILE_EXISTS_REPLACE);

        if ($tmp_file) {
          $file_obj = file_load($tmp_file->fid);

          $photo_dir = $all_path['photo_dir'];
          file_prepare_directory($photo_dir, FILE_CREATE_DIRECTORY);

          $extension = pathinfo($file_obj->filename, PATHINFO_EXTENSION);
          $new_filename = $cvterm_id . '_' . ($i + 1) . '.' . $extension;

          $scaled = file_move($file_obj, $photo_dir . '/' . $new_filename, FILE_EXISTS_REPLACE);

          // Tried scaling image - this block works but
          // the original image dimension remains.
          /*
          $img = $scaled->uri;
          $img_info = image_get_info($img);

          if ($img_info['width'] > 400 && $img_info['height'] > 400) {
            $img_obj = image_load($img);
            $k = image_scale($img_obj, 400, 400, $upscale = FALSE);
            dpm($k); // return 1 a success.
          }
          */

          $scaled->filename = $new_filename;
          $scaled->status = FILE_STATUS_PERMANENT;
          $m = file_save($scaled);
          file_usage_add($scaled, 'analyzedphenotypes', 'file', 1);
        }
      }

      break;

    //
    // Define additional case here.

    //
    default:
      // Return all cvs in chado.cv table.
      $cvs = chado_query(
        "SELECT cv_id, name, definition FROM {cv} ORDER BY name ASC"
      );

      if ($cvs->rowCount() > 0) {
        foreach($cvs as $cv) {
          $cvprop[$cv->cv_id] = array(
            'name' => $cv->name,
            'definition' => $cv->definition,
          );
        }
      }
  }


  return $cvprop;
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_traitprop($property, $data) {
  $traitprop = '';

  switch($property) {
    //
    case 'right_format':
      // Ensures that each trait provided matches the
      // the following format: Trait Name (unit).
      if (preg_match('/\A[^()]+\s*\({1}[^)(]+\)\z/i', $data) !== 1) {
        $traitprop = $data;
      }

      break;

    //
    case 'no_unit':
      // Exclude the unit from the returned value
      // given a Trait Name (unit) value.
      $traitprop = preg_replace("/\(.+\)/", '', $data);

      break;

    //
    case 'unit':
      // Exclude the trait name from the returned value
      // given a Trait Name (unit) value.
      // Note: When unit could not be established, default to text.
      preg_match("/.*\(([^)]*)\)/", $data, $match);
      $traitprop = (isset($match[1])) ? $match[1] : 'text';

      break;

    //
  }


  return $traitprop;
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_AJAX_callback($form, $form_state) {
  $case = $form_state['triggering_element']['#ajax']['case'];
  $element = 'ap_AJAX_wrapper' . '_' . $case;


  return $form[$element];
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_dbprop($property = null, $parameter = null) {
  $dbprop = '';

  $dbs = chado_query(
    "SELECT db_id, name, description FROM {db} ORDER BY name ASC"
  );

  if ($dbs->rowCount() <= 0) {
    return $dbprop;
  }

  switch($property) {
    //
    case 'match_name':
      foreach($dbs as $db) {
        if ($db->db_id == $parameter['db_name']) {
          $dbprop = $db->name;

          break;
        }
      }

      break;

    //
    // Define additional case here.

    //
    default:
      foreach($dbs as $db) {
        $dbprop[$db->db_id] = array(
          'name' => $db->name,
          'description' => $db->description,
        );
      }
  }

  return $dbprop;
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_validation_result($property, $parameter) {
  switch($case) {
    //
    case 'data':
      $job_id = $parameter['job_id'];

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
    // Define additional case here.

    // default:
  }
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_tripaljobprop($property, $parameter = null) {
  $tripaljobprop = null;

  switch($property) {
    //
    case 'new_job':
      // Create/Register a Tripal Job.
      $job_id = tripal_add_job(
        $dataset['description'], 'analyzedphenotypes', $dataset['callback'],
        array(serialize($parameter['dataset'])),
        $user->uid
      );

      if ($job_id) {
        $tripaljobprop = $job_id;
      }

      break;

    //
    case 'get_job':
      // Return Tripal Job registered to user.
      global $user;

      $job = tripal_get_job($parameter['job_id']);
      $job_status = array('Completed', 'Error', 'Cancelled');

      if ($job && $job->uid == $user->uid && !in_array($job->status, $job_status)) {
        $tripaljobprop = $job->job_id;
      }

      break;

    //
    case 'tripal_fail':
      // Tripal failed message.
      $ap = 'analyzedphenotypes';
      $true_print = array('print' => TRUE);

      tripal_report_error($ap, TRIPAL_CRITICAL,
        "\n" . $dataset['message'] . ' [VALUE: @param]' . "\n",
        array('@param' => $dataset['param']),
        $true_print
      );

      tripal_report_error($ap, TRIPAL_CRITICAL,
        'Failed to load phenotypic data in Tripal Job #!job_id.',
        array('!job_id' => $dataset['job_id']),
        $true_print
      );

      break;

    //
    // Additional case here.

    default:
      // Settings used in creating an instance of Tripal Job.
      $tripaljobprop = array(
        'error_limit'       => 10,
        'report_frequency'  => 5,
        // Filenames used to track progress and validation result.
        // Replace @JOB_ID with job_id.
        'validate_result'   => 'ap_validate_tripaljob_result@JOB_ID.txt',
        'progress_save'     => 'ap_save_tripaljob_progress@JOB_ID.txt',
        'progress_validate' => 'ap_validate_tripaljob_progress@JOB_ID.txt',
      );
  }


  return $tripaljobprop;
}


/**
 * ####
 * ####
 *
 * @param $property
 *   ####
 * @param $parameter
 *   ####
 *
 * @return
 *   #####
 *
 * @see
 *   ####
 */
function analyzedphenotypes_ontologyprop($property = null, $parameter = null) {
  $ontologyprop = '';

  switch($property) {
    //
    case 'suggest':
      // Get module system settings.
      $vars = analyzedphenotypes_moduleprop('settings');
      $cv = variable_get($vars['ap_cv']);

      $all_terms = analyzedphenotypes_cvprop('terms_in_cv', $cv);
      foreach($all_terms as $i => $term) {
        if ($i == $data) {
          $ontologyprop[$i] = $term;
          break;
        }
        else {
          $keywords = explode(' ', $data);

          foreach($keywords as $key) {
            if (strstr($i, trim($key))) {
              $ontologyprop[$i] = $term;
            }
          }
        }
      }

      if (empty($ontologyprop)) {
        $ontologyprop = array($data => array());
      }

      break;

    //
    // Additional case here.

    // default:
  }


  return $ontologyprop;
}










/**
 * Function: Manage module settings and operations.
 * Default to return all module related directories.
 *
 * @param $property
 *   An sting value indicating property requested.
 *   Each $property corresponds to a case in the main switch inmplementation.
 *
 * @return $moduleprop
 *   Information about the module.
 *
 * @see analyzedphenotypes_datatypeprop().
 */
function analyzedphenotypes_moduleprop($property = null) {
  $moduleprop = null;

  switch($property) {
    //
    case 'settings':
      // Define module variable names to define:.
      // 1. Database and Controlled Vocabulary.
      // 2. Controlled Vocabulary Terms for plant property types.
      $basename = 'analyzedphenotypes_modulevar_';

      $moduleprop = array(
        // Database to use in data upload.
        'ap_cv'        => $basename . 'cv',
        // Controlled Vocabulary CV to use in data upload.
        'ap_database'  => $basename . 'db',
        // Allow new trait to be inserted into the CV.
        'ap_allow_new' => $basename . 'allow_new',
        // Plant property cvterms.
        'ap_year'      => $basename . 'year',
        'ap_location'  => $basename . 'location',
        'ap_replicate' => $basename . 'replicate',
        'ap_collector' => $basename . 'collector',
      );

      break;

    //
    // Additional case here.

    //
    default:
      // Return all module relavant directories.
      $tripal_ext = 'admin/tripal/extension';
      $module = 'analyzedphenotypes';

      $moduleprop = array(
        // Base directory of tripal extension.
        'ap'   => $tripal_ext . '/' . $module,
        // Temporary directory.
        'temp'  => file_directory_temp(),
        // Path to the data loader.
        'loader' => base_path() . $tripal_ext . '/' . $module . '/upload',
        // Path to analyzedphenotype module.
        'module'  => drupal_get_path('module', $module),
        // Absolute path to base tripal extension.
        'extension' => base_path() . $tripal_ext,
        // Directory for storing trait photo.
        'photo_dir'  => 'public://ap-photo',
        // Absolute path to analyzedphenotype administrative pages.
        'base_admin'  =>  base_path() . $tripal_ext . '/' . $module,
      );
  }


  return $moduleprop;
}
