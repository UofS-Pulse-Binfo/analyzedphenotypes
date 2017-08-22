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
          ':name' => $parameter['name'],
          ':uniquename' => $parameter['uniquename'],
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
function analyzedphenotypes_datafileprop($property = null, $parameter = null) {
  $datafileprop = null;

  switch($property) {
    //
    case 'get_contents':
      $data_file = $parameter['data_file'];

      // Read the supplied file and return the contents.
      $source = drupal_realpath($data_file->uri);
      $datafile_contents = file_get_contents($source);

      if ($datafile_contents) {
        $datafileprop = $datafile_contents;
      }

      break;

    //
    case 'get_columns':
      $data_file = $parameter['data_file'];

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
    'Trait Name'          => 'trait',        // #0
    'Germplasm Accession' => 'alphanumeric', // #1
    'Germplasm Name'      => 'alphanumeric', // #2
    'Year'                => 'number',       // #3
    'Location'            => 'text',         // #4
    'Replicate'           => 'number',       // #5
    'Value'               => '',             // #6
    'Data Collector'      => 'text',         // #7
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
    case 'match_combination':
      $vars = analyzedphenotypes_systemvars('plant_property');
      $arr_plantprop = array();

      foreach($vars as $i => $var) {
        $property = trim(str_replace('ap_', '', $i));
        $var_value = variable_get($var);
        $arr_plantprop[$property] = $var_value;
      }

      $result = chado_query(
        "SELECT phenotype.phenotype_id
         FROM
           {phenotype} AS phenotype
           LEFT JOIN {cvterm} AS trait_name ON phenotype.attr_id = trait_name.cvterm_id
           LEFT JOIN {stock} AS germplasm_accession USING (stock_id)
           LEFT JOIN {phenotypeprop} AS year USING (phenotype_id)
           LEFT JOIN {phenotypeprop} AS location USING (phenotype_id)
           LEFT JOIN {phenotypeprop} AS replicate USING (phenotype_id)
         WHERE
           year.type_id = :year_cvterm
           AND location.type_id = :location_cvterm
           AND replicate.type_id = :replicate_cvterm
           AND CONCAT(trait_name.name, germplasm_accession.uniquename, year.value, location.value, replicate.value) = :value_combination"
        ,
        array(
          ':year_cvterm'    => $arr_plantprop['year'],
          ':location_cvterm' => $arr_plantprop['location'],
          ':replicate_cvterm' => $arr_plantprop['replicate'],
          ':value_combination' => $parameter['combination'],
        )
      );

      $columnsprop = ($result) ? $result->fetchField() : $columnsprop;

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
function analyzedphenotypes_datatypeprop($property, $parameter = null) {
  $datatypeprop = TRUE;

  $value = $parameter['value'];

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
    case 'alphanumeric':
      if (preg_match('/[a-z0-9.*]/i', $value) !== 1) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'trait':
      if (preg_match('/\A[^()]+\s*\({1}[^)(]+\)\z/i', $value) !== 1) {
        $datatypeprop = FALSE;
      }

      break;

    case 'text':
      if (preg_match('/[a-z]/i', $value) !== 1) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'number':
      if (preg_match('/[0-9]/', $value) !== 1) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'number_no_zero':
      if (preg_match('/[1-9]/', $value) !== 1) {
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
    case 'match_key':
      $vars = analyzedphenotypes_systemvars('dv_cv');
      $cv_id = variable_get($vars['ap_cv']);
      unset($vars);

      $result = chado_query(
        "SELECT cvterm_id, name FROM {cvterm} WHERE cv_id = :cv_id AND LOWER(name) LIKE :cvterm_key ORDER BY name ASC",
        array(
          'cv_id' => $cv_id,
          'cvterm_key' => '%' . strtolower(trim($parameter['cvterm_keyword'])) . '%',
        )
      );

      if ($result) {
        $cvprop = $result->fetchAllKeyed();
      }

      break;

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
      $term = array();
      $vars = analyzedphenotypes_systemvars('db_cv');
      $cv_id = variable_get($vars['ap_cv']);

      if (isset($parameter['cvterm_id'])) {
        // Global search.
        $index = 'cvterm_id';
      }
      else {
        // Term in cv.
        $term['cv_id'] = array('cv_id' => $cv_id);
        $index = 'name';
      }

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

        if ($result) {
          $db_id = $result->fetchField();

          $cvprop['db_id'] = $db_id;
        }
        else {
          $cvprop['db_id'] = null;
        }

        // Crop ontology.
        unset($result);
        $related_cvterm = variable_get($vars['ap_related']);
        $result = chado_query(
          "SELECT t1.name, t2.subject_id FROM {cvterm} AS t1 INNER JOIN {cvterm_relationship} AS t2 ON t1.cvterm_id = t2.subject_id
           WHERE type_id = :related AND object_id = :cvterm_id LIMIT 1",
          array(
            ':related' => $related_cvterm,
            ':cvterm_id' => $cvprop['cvterm_id'],
          )
        );

        if ($result) {
          $cvprop['crop_ontology_cvterm_id'] = $result->subject_id;
          $cvprop['crop_ontology_name'] = $result->name;
        }
        else {
          $cvprop['crop_ontology'] = null;
        }

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
    case 'cvterm_get_photo':
      $photo = array('photo_1', 'photo_2');

      foreach($photo as $i => $v) {
        $filename = $parameter['cvterm_id'] . '_' . ($i + 1) . '.jpg';
        $filedir = 'public://ap-photo';

        $fileuri = drupal_realpath($filedir) . '/' . $filename;

        if (file_exists($fileuri)) {
          $info = image_get_info($fileuri);

          $cvprop[$v] = array(
            'filename' => $filename,
            'uri' => $filedir . '/' . $filename,
            'height' => $info['height'],
            'width' => $info['width'],
          );
        }
        else {
          $cvprop[$v] = null;
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
function analyzedphenotypes_traitprop($property, $parameter) {
  $traitprop = null;

  switch($property) {
    //
    case 'no_unit':
      // Exclude the unit from the returned value
      // given a Trait Name (unit) value.
      $traitprop = preg_replace("/\(.+\)/", '', $parameter['trait_name']);

      break;

    //
    case 'unit':
      // Exclude the trait name from the returned value
      // given a Trait Name (unit) value.
      // Note: When unit could not be established, default to text.
      preg_match("/.*\(([^)]*)\)/", $parameter['trait_name'], $match);
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

  if ($form_state['triggering_element']['#ajax']['contain']) {
    $contain = $form_state['triggering_element']['#ajax']['contain'];

    return $form[$contain][$element];
  }
  else {
    return $form[$element];
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
function analyzedphenotypes_validationresult($property, $parameter) {
  switch($property) {
    //
    case 'data':
      $job_id = $parameter;

      $result = analyzedphenotypes_tripaljobprop('read_progress', array(
        'job_id' => $job_id,
        'source' => 'validateresult',
      ));

      $status = json_decode($result, TRUE);
      print theme('analyzedphenotypes_validator_report', array('status' => $status, 'scope' => 'ap-data-scope'));

      break;

    //
    case 'file':
      $status = $parameter;

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
function analyzedphenotypes_tripaljobprop($property = null, $parameter = null) {
  // Filenames:
  // validateprogress - validation progress.
  // validateresult   - validation result.
  // savefile         - save file progress.

  $tripaljobprop = null;

  switch($property) {
    //
    case 'new_job':
      // Create/Register a Tripal Job.
      global $user;

      $job_id = tripal_add_job(
        $parameter['description'], 'analyzedphenotypes', $parameter['callback'],
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

      if ($job && $job->uid == $user->uid && $job->modulename == 'analyzedphenotypes') {
        $tripaljobprop = array(
          'job_id' => $job->job_id,
          'status' => $job->status,
          'uid' => $job->uid,
          'progress' => $job->progress,
        );
      }

      break;

    //
    case 'update_progress':
      $tripaljobprop = dp_update('tripal_jobs')
        ->fields(array('progress' => $parameter['progress']))
        ->condition('job_id', $parameter['job_id'], '=')
        ->execution();

      break;

    //
    case 'write_progress':
      $file = file_directory_temp() . '/ap_' . $parameter['source'] . '_tripaljob_' . $parameter['job_id'] . '.txt';
      $tripaljobprop = file_unmanaged_save_data($parameter['progress'], $file, FILE_EXISTS_REPLACE);

      break;

    //
    case 'read_progress':
      $file = file_directory_temp() . '/ap_' . $parameter['source'] . '_tripaljob_' . $parameter['job_id'] . '.txt';
      $tripaljobprop = file_get_contents($file);

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
        'error_limit'       => 5,
        'report_frequency'  => 5,
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
  $ontologyprop = null;

  switch($property) {
    //
    case 'suggest':
      $suggest_limit = 10;

      $vars = analyzedphenotypes_systemvars('genus_ontology');
      $var_name = 'ap_' . strtolower(str_replace(' ', '_', $parameter['project_genus']));
      $cv_id = variable_get($vars[$var_name]);
      unset($vars);

      $ontology_terms = analyzedphenotypes_cvprop('terms_in_cv', array(
        'cv_id' => $cv_id,
      ));

      // No unit.
      $trait_name = strtolower(trim($parameter['trait_name']));
      $keywords = explode(' ', $trait_name);

      foreach($ontology_terms as $i => $term) {
        $name = strtolower(trim($term['name']));

        if ($name == $trait_name) {
          $ontologyprop[$i] = $term;

          break;
        }
        else {
          $j = 0;

          foreach($keywords as $key) {
            if (strstr($name, $key)) {
              $ontologyprop[$i] = $term;

              if ($j >= $suggest_limit) {
                break;
              }

              $j++;
            }
          }
        }
      }

      break;

    //
    // Additional case here.

    // default:
  }


  return $ontologyprop;
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
function analyzedphenotypes_systemvars($property = null) {
  $systemvars = null;

  $basename = 'analyzedphenotypes_systemvar_';

  $vars_database = array(
    'ap_cv'        => $basename . 'cv',
    'ap_database'  => $basename . 'db',
    'ap_allow_new' => $basename . 'allow_new',
    // cvterm genus.
    'ap_genus'     => $basename . 'genus',
    // cvterm related used in ontology.
    'ap_related'   => $basename . 'related',
  );

  $vars_plantprop = array(
    // Plant property cvterms.
    'ap_year'      => $basename . 'year',
    'ap_location'  => $basename . 'location',
    'ap_replicate' => $basename . 'replicate',
    'ap_collector' => $basename . 'collector',
  );

  $genus = analyzedphenotypes_genusprop();
  $vars_ontology = array();

  foreach($genus as $g) {
    $g = strtolower(str_replace(' ', '_', $g));
    $vars_ontology['ap_' .  $g] = $basename . $g;
  }

  switch($property) {
    //
    case 'db_cv':
      $systemvars = $vars_database;

      break;

    case 'plant_property':
      $systemvars = $vars_plantprop;

      break;

    case 'genus_ontology':
      $systemvars = $vars_ontology;

      break;

    //
    // Additional case here.

    default:
      $systemvars = $vars_database + $vars_plantprop + $vars_ontology;
  }


  return $systemvars;
}
