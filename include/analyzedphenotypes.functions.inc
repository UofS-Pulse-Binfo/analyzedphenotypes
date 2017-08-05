<?php
/**
 * @file
 * Contains helper functions required by this module.
 */


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
    case 'column_type':
      // Define the unit/data type for each column in a
      // standard tsv file. The key value is the column name
      // whereas the value represents the data type.
      // For each data type a corresponding validator in
      // FUNCTION: analyzedphenotypes_datatypeprop()
      // used to validate values provided.
      $moduleprop = array(
        'Trait Name' => 'text',          // #0
        'Germplasm Accession' => 'text', // #1
        'Germplasm Name' => 'text',      // #2
        'Year' => 'number',              // #3
        'Location' => 'text',            // #4
        'Replicate' => 'number',         // #5
        'Value' => '',                   // #6
        'Data Collector' => 'text',      // #7
      );

      break;

    //
    case 'unit_type':
      // Define trait units and data type. Similar to column_type case
      // each data type corresponds to a validator in function specified.
      // Each unit (key value) contains an associative array representing
      // the readable/full name of the unit and data type, repectively.
      $moduleprop = array(
        'g' => array('name' => 'grams', 'type' => 'number'),
        'kg' => array('name' => 'kilograms', 'type' => 'number'),
        'cm'  => array('name' => 'centimeters', 'type' => 'number'),
        'days' => array('name' => 'days', 'type' => 'number_no_zero'),
        'y/n/?' => array('name' => 'Yes (y), No (n) or Unsure (?)', 'type' => 'y/n/?'),
        'count'  => array('name' => 'count', 'type' => 'number'),
        'colour'  => array('name' => 'color', 'type' => 'text'),
        'comments' => array('name' => 'comments', 'type' => 'text'),
        'scale 1-5' => array('name' => 'scale 1-5', 'type' => 'scale 1-5'),
        'scale 1-10' => array('name' => 'scale 1-10', 'type' => 'scale 2-5'),
      );

      break;

    //
    case 'stages':
      // Define the stages in data upload. In this order.
      $moduleprop = array(
        'Upload',   // Stage 1 - Select project, genus and upload data file.
        'Validate', // Stage 2 - Validate data.
        'Describe', // Stage 3 - Fully describe all traits.
        'Save',     // Stage 4 - Save data and file.
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


/**
 * Function: Manage project property and operations.
 * Default to return all projects in chado.project table.
 *
 * @param $property
 *   An sting value indicating property requested.
 *   Each $property corresponds to a case in the main switch inmplementation.
 *   Default to null.
 * @param $data
 *   A value to limit the result set to specific value and as a
 *   parameter to each property (case).
 *   Default to null.
 *
 * @return $projectprop
 *   Information and or properties about project.
 */
function analyzedphenotypes_projectprop($property = null, $data = null) {
  $projectprop = null;

  $sql = "SELECT project_id, name FROM {project} ORDER BY name ASC";
  $result = chado_query($sql);

  if ($result->rowCount() <= 0) {
    return $projectprop;
  }

  switch($property) {
    //
    case 'settings':
      // Number of rows to return.
      $projectprop = 10;

      break;

    //
    case 'match_key':
      // $data : Keyword (string Project/Experiment name or title).

      // Match project that contains keyword.
      // To ensure list will not grow longer, limit the result to x.
      $limit = analyzedphenotypes_projectprop('settings');

      foreach($result as $i => $project) {
        if (($i + 1) > $limit) {
          break;
        }

        $title = $project->name;

        if (stripos($title, trim($data)) !== FALSE) {
          $projectprop[$title] = $title;
        }
      }

      break;

    //
    case 'match_name':
      // $data : String value, A Project/Experiment name or title.

      // Match project that is equal to the project title.
      // Return the project id number when a match is first found.
      foreach($result as $project) {
        $title = $project->name;

        if (strtolower($title) == strtolower(trim($data))) {
          $projectprop = $project->project_id;
          break;
        }
      }

      break;

    //
    // Additional case here.

    //
    default:
      // $data : null

      // Return all project as an associative array where the key
      // is the project id and the value as the project title.
      $projectprop = $result->fetchAllKeyed(0, 1);
  }


  return $projectprop;
}


/**
 * Function: Manage genus property and operations.
 * Default to return all projects in chado.organism table.
 *
 * @param $property
 *   An sting value indicating property requested.
 *   Each $property corresponds to a case in the main switch inmplementation.
 *   Default to null.
 * @param $data
 *   A value to limit the result set to specific value and as a
 *   parameter to each property (case).
 *   Default to null.
 *
 * @return $projectprop
 *   Information and or properties about genus.
 *
 * @see
 *   analyzedphenotypes_cvprop().
 */
function analyzedphenotypes_genusprop($property = null, $data = null) {
  $genusprop = null;

  switch($property) {
    //
    case 'settings':
      // Genus cv.
      $genusprop = array(
        'term' => 'genus',
        'cv' => 'taxonomic_rank',
      );

      break;

    //
    case 'match_genus':
      // $data : String value, A genus (eg. Lens).

      // Match a given genus and return Genus organism_id.
      $sql = "SELECT organism_id FROM {organism} 'WHERE LOWER(genus) = LOWER(:genus) LIMIT 1";
      $result = chado_query($sql, array(
        ':genus' => $data,
      ));

      $genusprop = ($result) ? $result->fetchField() : $genusprop;

      break;

    //
    case 'project_genus':
      // $data : Project Id number.

      // Get the project genus when available and return the genus name.
      $term = analyzedphenotypes_genusprop('settings');

      $traitprop = analyzedphenotypes_cvprop('get_term', $term);
      $cvid_genus = $traitprop['id'];
      unset($traitprop);

      $sql = "SELECT value FROM {projectprop} WHERE project_id = :project_id AND type_id = :type_id LIMIT 1";
      $result = chado_query($sql, array(
        ':type_id' => $cvid_genus,
        ':project_id' => $data,
      ));

      $genusprop = ($result) ? $result->fetchField() : $genusprop;

      break;

    //
    // Additional case here.

    //
    default:
      // $data : null.

      // Return all the genus.
      $sql = "SELECT genus FROM {organism} GROUP BY genus ORDER BY genus ASC";
      $result = chado_query($sql);

      $genusprop = ($result) ? $result->fetchAllKeyed(0, 0) : $genusprop;
  }


  return $genusprop;
}


/**
 *
 */
function analyzedphenotypes_stockprop($property, $data = null) {
  $stockprop = null;

  switch($property) {
    //
    case 'match_name':
      $sql = "SELECT stock_id FROM {stock} WHERE name = :name AND uniquename = :uniquename LIMIT 1";
      $result = chado_query($sql, array(
        ':name' => $data['name'],
        ':uniquename' => $data['uniquename']
      ));

      $stockprop = ($result) ? $result->fetchField() : $stockprop;

      break;

    //
  }

  return $stockprop;
}



/**
 * Function: Manage file property and operations.
 * Default to return a set of file extensions allowed.
 *
 * @param $property
 *   An sting value indicating property requested.
 *   Each $property corresponds to a case in the main switch inmplementation.
 * @param $file
 *   Drupal standard file object.
 *
 * @return $projectprop
 *   Information about file.
 */
function analyzedphenotypes_fileprop($property = null, $file = null) {
  $fileprop = null;

  switch($property) {
    //
    case 'get_contents':
      // Read the supplied file and return the contents.
      $source = drupal_realpath($file->uri);
      $text_data = file_get_contents($source);

      $fileprop = $text_data;

      break;

    //
    case 'readable':
      // Test if the file supplied is readable.
      $source = drupal_realpath($file->uri);
      $text_data = file_get_contents($source);

      $fileprop = ($text_data) ? TRUE : FALSE;

      break;

    //
    case 'columns':
      // Read the file supplied and return the columns row.
      $source = drupal_realpath($file->uri);
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
      // Return the filename used when creating a temporary
      // file used when in updating the progress bar in Tripal Job
      // implementations.

      // Replace @JOB_ID with registered Tripal Job Id number.
      $fileprop = array(
        'validate progress' => 'ap_validate_tripaljob_progress@JOB_ID.txt',
        'validate result'   => 'ap_validate_tripaljob_progress@JOB_ID.txt',
        'save progress'     => 'ap_save_tripaljob_progress@JOB_ID.txt',
      );

      break;

    //
    case 'save_photo':
      // Save trait images provided in Describe form.
      // NOT COMPLETE.
      $all_path = analyzedphenotypes_moduleprop();

      foreach($file['photo'] as $i => $photo) {
        // Temporary file.
        $tmp_file = file_save_upload($photo, array(), FALSE, FILE_EXISTS_REPLACE);

        if ($tmp_file) {
          $file_obj = file_load($tmp_file->fid);

          $photo_dir = $all_path['photo_dir'];
          file_prepare_directory($photo_dir, FILE_CREATE_DIRECTORY);

          $extension = pathinfo($file_obj->filename, PATHINFO_EXTENSION);
          $new_filename = $file['cvterm_id'] . '_' . ($i + 1) . '.' . $extension;

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


/**
 * Function: Manage columns property and operations.
 * Default to return a set of expected columns and data type.
 *
 * @param $property
 *   An sting value indicating property requested.
 *   Each $property corresponds to a case in the main switch inmplementation.
 *
 * @return $projectprop
 *   Information about file.
 *
 * @see
 *   analyzedphenotypes_moduleprop().
 */
function analyzedphenotypes_columnsprop($property = null, $data = null) {
  $columnsprop = null;
  $columns = analyzedphenotypes_moduleprop('column_type');

  switch($property) {
    //
    case 'expected':
      // Return an array of expected columns.
      $columnsprop = array_keys($columns);

      break;

    //
    case 'combination':
      // Construct a combination of columns to be used when
      // validating lines for unique column combination.
      // Refer to analyzephenotypes_module to add or remove
      // columns to/from this list.
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
    case 'match_combination':
      $mod_vars = analyzedphenotypes_moduleprop('settings');
      unset($mod_vars['ap_database'], $mod_vars['ap_cv'], $mod_vars['ap_allow_new'], $mod_vars['ap_collector']);

      $arr_plantprop = array();

      foreach($mod_vars as $k => $var) {
        $property = trim(str_replace('ap_', '', $k));

        $system_var = variable_get($var);
        $arr_plantprop[$property] = $system_var;
      }

      $sql = "SELECT phenotype.phenotype_id
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
          AND CONCAT(trait_name.name, germplasm_accession.uniquename, year.value, location.value, replicate.value) = :value_combination";

      $result = chado_query($sql, array(
        ':year_cvterm'    => $arr_plantprop['year'],
        ':location_cvterm' => $arr_plantprop['location'],
        ':replicate_cvterm' => $arr_plantprop['replicate'],
        ':value_combination' => $data
      ));

      $columnsprop = ($result) ? $result->fetchField() : $columnsprop;

      break;

    //
    // Additional case here.

    //
    default:
      // Default to all expected columns. An associative array
      // where the key is the column and value is the data type.
      $columnsprop = $columns;
  }


  return $columnsprop;
}


/**
 * Function: Manage datatype property and operations.
 * No default set.
 *
 * @param $property
 *   An sting value indicating property requested.
 *   Each $property corresponds to a case in the main switch inmplementation.
 * @param $data
 *   A string value matching a data type of a column or a trait.
 *
 * @return $datatypeprop
 *   Boolean value indicating if value passed the data type requirement.
 *
 * @see
 */
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
      if (!is_numeric($data)) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'number_no_zero':
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


/**
 * Function: Manage cv/cvterm property and operations.
 * Default to return all Controlled Vocabulary (CV).
 *
 * @param $property
 *   An sting value indicating property requested.
 *   Each $property corresponds to a case in the main switch inmplementation.
 * @param $data
 *   A string value matching a data type of a column or a trait.
 *
 * @return $datatypeprop
 *   Boolean value indicating if value passed the data type requirement.
 *
 * @see
 */
function analyzedphenotypes_cvprop($property = null, $data = null) {
  $cvprop = null;

  switch($property) {
    //
    case 'get_cvname':
      // Return the cv name of a given cv_id.
      $cv = tripal_get_cv(array('cv_id' => $data));
      if ($cv) {
        $cvprop = $cv->name;
      }

      break;

    //
    case 'get_term':
      // Get cvterm properties: cvterm_id, name, cv_id, definition
      // db_id and dbxref_id.
      // Note: cv_id in the return value is an object contining
      // information about the cv.
      $term = array();

      if (isset($data['cv'])) {
        $term['cv_id'] = array('name' => $data['cv']);
      }

      if (isset($data['cvterm_id'])) {
        $term['cvterm_id'] = $data['cvterm_id'];
      }
      else {
        $term['name'] = $data['term'];
      }

      $cvterm = tripal_get_cvterm($term);

      if ($cvterm) {
        // Database information.
        $sql = "SELECT db_id FROM {dbxref} WHERE dbxref_id = :dbxref_id LIMIT 1";

        $args = array(':dbxref_id' => $cvterm->dbxref_id);
        $db_id = chado_query($sql, $args)
          ->fetchField();

        $cvprop = array(
          'id' => $cvterm->cvterm_id,
          'name' => $cvterm->name,
          // This value is an object.
          'cv_id' => $cvterm->cv_id,
          'db_id'  => $db_id,
          'dbxref_id' => $cvterm->dbxref_id,
          'definition' => $cvterm->definition,
        );
      }

      break;

    //
    case 'terms_in_cv':
      // Given a Controlled Vocabulary, get all terms defined.
      // Return an array where key is the cvterm name and value
      // is an associative array containing the cvterm_id and definition.
      $sql = "SELECT cvterm_id, name, definition FROM {cvterm} WHERE cv_id = :cv_id ORDER BY name ASC";

      $args = array(':cv_id' => $data);
      $terms = chado_query($sql, $args);

      if ($terms->rowCount() > 0) {
        foreach($terms as $c) {
          $cvprop[$c->name] = array(
            'id' => $c->cvterm_id,
            'definition' => $c->definition,
          );
        }
      }

      break;

    //
    default:
      // Return all Controlled Vocabulary.
      // Return an array where key is the cvterm id and value
      // is an associative array containing the name and definition.
      $sql = "SELECT cv_id, name, definition FROM {cv} ORDER BY name ASC";
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


/**
 * Function: Manage cv/cvterm property and operations.
 * Default to return all Controlled Vocabulary (CV).
 *
 * @param $property
 *   An sting value indicating property requested.
 *   Each $property corresponds to a case in the main switch inmplementation.
 * @param $data
 *   A string value matching a data type of a column or a trait.
 *
 * @return $datatypeprop
 *   Boolean value indicating if value passed the data type requirement.
 *
 * @see
 */function analyzedphenotypes_traitprop($property, $data) {
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
    case 'sanitize':
      // Clear any new line, formatting etc.
      $traitprop = analyzedphenotypes_noformat($data);

      break;

    //
    case 'recordset':
      // Return all relevant information given a trait name.
      // Info includes: cvterm_id, name, definition, photos, unit, scale and otology.
      // A flag $found indicates if terms is present in the database or is a new trait.
      $vars = analyzedphenotypes_moduleprop('settings');

      $cv_id = variable_get($vars['ap_cv']);
      $cv_name = analyzedphenotypes_cvprop('get_cvname', $cv_id);

      $cvterm = analyzedphenotypes_cvprop('get_term', array(
        'term' => trim($data),
        'cv' => $cv_name,
      ));

      if ($cvterm) {
        $found = TRUE;
        $term = $cvterm['name'];
        $name = $term;
        $definition = $cvterm['definition'];
        $photo_1 = $cvterm['id'] . '_1.jpg';
        $photo_2 = $cvterm['id'] . '_2.jpg';
      }
      else {
        $found = FALSE;
        $term = $data;
        $name = $term;
        $definition = '';
        $photo_1 = '';
        $photo_2 = '';
      }

      $unit = analyzedphenotypes_traitprop('unit', $name);
      $name = analyzedphenotypes_traitprop('no_unit', $name);

      $traitprop = array(
        'found'      => $found,
        'term'       => $term,
        'name'       => $name,
        'unit'       => $unit,
        'definition' => $definition,
        'photo_1'    => $photo_1,
        'photo_2'    => $photo_2,
      );

      // TODO:
      /*
        'unit'           => 'Test Unit',
        'scale'          => 'Test Scale',
        'crop_ontology'  => 'Test Crop Ontology',
        'plant_ontology' => 'Test Plant Ontology',
      */

      break;

    //
    case 'insert_trait':
      // Operation insert a new trait.
      // This is perfomed if module is on allow new trait settings set to on.
      // Note: Photo part of this operation is perfomed by fileprop function.
      $vars = analyzedphenotypes_moduleprop('settings');

      $db = variable_get($vars['ap_database']);
      $cv = variable_get($vars['ap_cv']);
      $allow_new = variable_get($vars['ap_allow_new']);

      // Find the term in the controlled vocabulary.
      $cvterm = analyzedphenotypes_cvprop('get_term', array(
        'term' => $data['cvterm'],
      ));

      if ($cvterm) {
        if ($cvterm['definition']   == $data['description']
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

      // Save.
      if (!$cvterm || isset($newtrait)) {
        if ($allow_new) {
          $cv_name = analyzedphenotypes_cvprop('get_cvname', $cv);
          $db_name = analyzedphenotypes_dbprop('get_dbname', $db);

          // Save it.
          $cvterm = tripal_insert_cvterm(array(
            'id' => trim($db_name) . ':' . $data['cvterm'],
            'name' => $data['cvterm'],
            'cv_name'  => $cv_name,
            'definition' => $data['description'],
          ));

          $traitprop = $cvterm->cvterm_id;
        }
      }

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
    case 'get_dbname':
      if ($db->rowCount() > 0) {
        foreach($db as $d) {
          if ($d->db_id == $data) {
            $dbprop = $d->name;
            break;
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
function analyzedphenotypes_tripaljobprop($property, $dataset = null) {
  $tripaljobprop = null;

  global $user;

  switch($property) {
    //
    case 'settings':
      $tripaljobprop = array(
        // Report every 5%.
        'report_frequency' => 5,
        // Stop validation for the first 10 errors.
        'error_limit' => 10,
      );

      break;

    //
    case 'new_job':
      $job_id = tripal_add_job($dataset['description'], 'analyzedphenotypes', $dataset['callback'], array(serialize($dataset['data'])), $user->uid);

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
    case 'tripal_fail':
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
  }


  return $tripaljobprop;
}



/**
 *
 */

function analyzedphenotypes_ontologyprop($property = null, $data = null) {
  $ontologyprop = '';

  switch($property) {
    //
    // Additional case here.

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
  }


  return $ontologyprop;
}
