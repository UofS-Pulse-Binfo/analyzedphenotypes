<?php
/**
 * @file
 * Contains API required by this module.
 */


/**
 * API: PROJECT/EXPERIMENT PROPERTY
 * Manage property and operation pertaining to data in chado.project table.
 * Default to: return all project as an associative array where key is project_id and value is project name.
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - match_key  : fetch all project records that matches a keyword.
 *   - match_name : fetch project record that matches a project name.
 *   - match_id   : fetch project record that matches a project id.
 *   - default    : fetch all project records.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - project_keyword
 *   - project_name
 *
 * @return Array $projectprop
 *   An associative array where the key is project_id and value is project_name;
 *
 * @see analyzedphenotypes_validator_project() in validators.inc.
 * @see analyzedphenotypes_save_tsv_data() in validator.inc.
 * @see analyzedphenotypes_data_json() in .module.
 */
function analyzedphenotypes_projectprop($property = null, $parameter = null) {
  $projectprop = null;

  // Fetch all project/experiment records in chado.project.
  $result = chado_query(
    "SELECT project_id, TRIM(name) AS name FROM {project} ORDER BY name ASC"
  );

  if ($result->rowCount() <= 0) {
    return $projectprop;
  }

  switch($property) {
    //
    case 'match_key':
      $keyword = trim($parameter['project_keyword']);
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
      $name = trim($parameter['project_name']);

      foreach($result as $project) {
        $project_name = $project->name;

        if (strtolower($project_name) == strtolower($name)) {
          $projectprop = array(
            'project_id' => $project->project_id,
            'project_name' => $project_name,
          );

          break;
        }
      }

      break;

    //
    case 'match_id':
      $id = trim($parameter['project_id']);

      foreach($result as $project) {
        $project_id = $project->project_id;

        if ($project_id == $id) {
          $projectprop = array(
            'project_id' => $project->project_id,
            'project_name' => $project_name,
          );

          break;
        }
      }

      break;

    //
    // Define additional case here.

    //
    default:
      $projectprop = $result->fetchAllKeyed(0, 1);
  }


  return $projectprop;
}


/**
 * API: GENUS PROPERTY
 * Manage property and operation pertaining to Genus in chado.organism.
 * Default to: return all genus as an array.
 * Dependencies: analyzedphenotypes_systemvars().
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - project_genus  : Fetch the genus a project or experiment is specific to.
 *   - assign_genus   : Assign a genus to a project.
 *   - default        : Fetch all Genus records.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - project_id
 *   - project_name
 *
 * @return $genusprop
 *   Genus as string or list of genus as an array.
 *
 * @see analyzedphenotypes_loader_form_upload() in admin.form.inc.
 * @see analyzedphenotypes_validator_genus() in validator.inc.
 * @see analyzedphenotypes_save_tsv_data() in validator.inc.
 */
function analyzedphenotypes_genusprop($property = null, $parameter = null) {
  $genusprop = null;

  switch($property) {
    //
    case 'project_genus':
      $vars = analyzedphenotypes_systemvars('db_cv');
      $var_genus = $vars['ap_genus'];
      $var_genus_value = variable_get($var_genus);

      if (isset($parameter['project_id'])) {
        $field = 't1.project_id';
        $value = 'project_id';
      }
      else {
        $field = 't1.name';
        $value = 'project_name';
      }

      // Fetch projectprop.value of type (type_id) Genus for a project id or project name.
      $sql = sprintf("SELECT t2.value FROM {project} AS t1 INNER JOIN {projectprop} AS t2 USING (project_id)
      WHERE t2.type_id = :type_id AND %s = :value LIMIT 1", $field);

      $result = chado_query($sql,
        array(
          ':type_id' => $var_genus_value,
          ':value' => $parameter[$value],
        )
      );

      $genusprop = ($result->rowCount() == 1) ? $result->fetchField() : $genusprop;

      break;

    case 'assign_genus':
      $vars = analyzedphenotypes_systemvars('db_cv');
      $var_genus = $vars['ap_genus'];
      $var_genus_value = variable_get($var_genus);

      $values = array(
        'type_id' => $var_genus_value,
        'project_id' => $parameter['project_id'],
        'value' => $parameter['project_genus'],
      );

      chado_insert_record('projectprop', $values);

      break;

    //
    // Define additional case here.

    //
    default:
      // Fetch all unique genus in chado.organism table.
      $result = chado_query(
        "SELECT genus FROM {organism} GROUP BY genus ORDER BY genus ASC"
      );

      $genusprop = ($result->rowCount() > 0) ? $result->fetchCol() : $genusprop;
  }


  return $genusprop;
}


/**
 * API: STOCKS PROPERTY
 * Manage property and operation pertaining to Stocks in chado.stock.
 * Default to:
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   - match_name : Fetch the stock given stock name and uniquename.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - uniquename
 *   - name
 *
 * @return String $stockprop
 *   Stock id number.
 *
 * @see analyzedphenotypes_validator_value() in validators.inc.
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
 * API: USER DATA FILE
 * Manage property and operation pertaining to user data file (tsv or txt).
 * Default to: Return valid file extensions.
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - get_contents : Get the tab-delimeted contents of file.
 *   - get_columns  : Return row # 0 or the columns row of file.
 *   - default      : Valid file extensions.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - data_file
 *
 * @return $datafileprop
 *   Contents fo file as Text stream or Array of file extensions/columns.
 *
 * @see analyzedphenotypes_dragdrop() in admin.form.inc.
 * @see analyzedphenotypes_loader_form_describe() in admin.form.inc.
 * @see analyzedphenotypes_validator_datafile() in validators.inc.
 * @see analyzedphenotypes_validator_tsv_data() in validators.inc.
 */
function analyzedphenotypes_datafileprop($property = null, $parameter = null) {
  $datafileprop = null;

  switch($property) {
    //
    case 'get_contents':
      $data_file = $parameter['data_file'];

      $source = drupal_realpath($data_file->uri);
      $datafile_contents = file_get_contents($source);

      if ($datafile_contents) {
        $datafileprop = $datafile_contents;
      }

      break;

    //
    case 'get_columns':
      $data_file = $parameter['data_file'];

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

    case 'make_permanent':
      $data_file = $parameter['data_file'];

      if ($data_file) {
        $data_file->status = FILE_STATUS_PERMANENT;
        file_save($data_file);

        // Also, point out that we are using it ;-)
        // Note, the file_usage_add() function expects a numerical unique id which we don't have.
        // We have gotten around this by using the uid concatenated with the timestamp using
        // the assumption that a single user cannot upload more than one phenotype file within a second.
        file_usage_add($data_file, 'analyzedphenotypes', 'analyzedphenotypes-file', $data_file->uid . $data_file->timestamp);
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
 * API: COLUMN HEADERS
 * Manage property and operation pertaining to column headers.
 * Default to: Return expected column headers.
 * Dependencies: analyzedphenotypes_systemvars().
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - combination       : Return a subset of expected column headers.
 *   - match_combination : Check if a given subset/combination matches a record in the database.
 *   - default           : Return all expected column headers and data type.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - data_file
 *
 * @return $datafileprop
 *   Contents fo file as Text stream or Array of file extensions/columns.
 *
 * @see analyzedphenotypes_dragdrop() in admin.form.inc.
 * @see analyzedphenotypes_loader_form_describe() in admin.form.inc.
 * @see analyzedphenotypes_validator_datafile() in validators.inc.
 * @see analyzedphenotypes_validator_tsv_data() in validators.inc.
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
      $cols = array_keys($columnsprop_expectedcols);

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

           AND trait_name.name = :value_traitname
           AND germplasm_accession.uniquename = :value_accession
           AND year.value = :value_year
           AND location.value = :value_location
           AND replicate.value = :value_replicate"
        ,
        array(
          ':year_cvterm'    => $arr_plantprop['year'],
          ':location_cvterm' => $arr_plantprop['location'],
          ':replicate_cvterm' => $arr_plantprop['replicate'],

          ':value_traitname' => $parameter[$cols[0]],
          ':value_accession' => $parameter[$cols[1]],
          ':value_year'      => $parameter[$cols[3]],
          ':value_location'  => $parameter[$cols[4]],
          ':value_replicate' => $parameter[$cols[5]],
        )
      );

      $columnsprop = ($result) ? $result->fetchField() : $columnsprop;

      break;

    //
    // Define additional case here.

    //
    default:
      $columnsprop = $columnsprop_expectedcols;
  }


  return $columnsprop;
}


/**
 * API: DATA TYPE
 * Manage property and operation pertaining to data types.
 * Default to:
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   - unit_type       : Define a set of standard units and expected data type of the value.
 *   - alphanumeric    : Letters, number combination.
 *   - trait           : String value strickly in Trait name (unit) format.
 *   - text            : Letters only.
 *   - number          : Numbers + 0.
 *   - number_no_zero  : Numbers > 0.
 *   - scale_1-5       : 1,2,3,4 and 5.
 *   - scale_1-10      : 1 - 10.
 *   - y/n/?           : y, yes, n, no, unsure, ?.
 *   - date            : YYYY-MM-DD.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - value
 *
 * @return Boolean $datatypeprop
 *   Default to TRUE as value passed all conditions and FALSE when proven otherwise.
 *
 * @see analyzedphenotypes_validator_value() in validators.inc.
 */
function analyzedphenotypes_datatypeprop($property, $parameter = null) {
  $datatypeprop = TRUE;

  $value = $parameter['value'];

  switch($property) {
    //
    case 'unit_type':
      // unit, description, data type.
      // Create a case for custom data type that will be used to validate value of such type.
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
      // Letters, numbers and any 0 or more characters.
      if (preg_match('/[a-z0-9.*]/i', $value) !== 1) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'trait':
      // Alphanumeric chars followed by ( letters and ) - Trait name (unit).
      if (preg_match('/\A[^()]+\s*\({1}[^)(]+\)\z/i', $value) !== 1) {
        $datatypeprop = FALSE;
      }

      break;

    case 'text':
      // Letters.
      if (preg_match('/[a-z]/i', $value) !== 1) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'number':
      // Numbers including 0.
      if (preg_match('/[0-9]/', $value) !== 1) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'number_no_zero':
      // Numbers, no 0.
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
      // YYYY-MM-DD format.
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
    // Define additional case here.

    // default:
  }


  return $datatypeprop;
}


/**
 * API: CONTROLLED VOCABULARIES
 * Manage property and operation pertaining to controlled vocabularies in chado.cv and chado.cvterm.
 * Default to: Return all controlled vocabularies in chado.cv.
 * Dependencies: analyzedphenotypes_systemvars(), analyzedphenotypes_scale_photo(), analyzedphenotypes_dbprop().
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   - match_key         : Match a record in chado.cvterm given a keyword.
 *   - match_cvid        : Match a record in chado.cv given a cv_id.
 *   - terms_in_cv       : Fetch all records in chado.cvterm given a cv_id.
 *   - get_cvterm        : Fetch records in chado.cvterm, chado.dbxref and chado.cvterm_relationship (term properties) given a trait.
 *   - insert_cvterm     : Insert a record into chado.cvterm.
 *   - cvterm_save_photo : Handle saving of photo to a trait/cvterm.
 *   - cvterm_get_photo  : Get photo information.
 *   - default           : All records in chado.cv.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - cvterm_keyword
 *   - cv_id
 *   - cvterm_id
 *   - name
 *   - photo
 *
 * @return $cvprop
 *   Array of term properties.
 *
 * @see analyzedphenotypes_validator_value() in validators.inc.
 */
function analyzedphenotypes_cvprop($property = null, $parameter = null) {
  $cvprop = null;

  switch($property) {
    //
    case 'match_key':
      $vars = analyzedphenotypes_systemvars('dv_cv');
      $cv_id = variable_get($vars['ap_cv']);
      unset($vars);

      // Find a cvterm that matches a keyword.
      $result = chado_query(
        "SELECT cvterm_id, name FROM {cvterm} WHERE cv_id = :cv_id AND LOWER(name) LIKE :cvterm_key ORDER BY name ASC",
        array(
          'cv_id' => $cv_id,
          'cvterm_key' => '%' . strtolower(trim($parameter['cvterm_keyword'])) . '%',
        )
      );

      if ($result->rowCount() > 0) {
        $cvprop = $result->fetchAllKeyed();
      }

      break;

    //
    case 'match_cvid':
      $cv = tripal_get_cv(array(
        'cv_id' => $parameter['cv_id'],
      ));

      if ($cv) {
        $cvprop = array(
          'cv_id' => $cv->cv_id,
          'name' => $cv->name,
          'definition' => $cv->definition,
        );
      }

      break;

    //
    case 'terms_in_cv':
      // Fetch all cvterms in a cv.
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
      $term = array();
      $vars = analyzedphenotypes_systemvars('db_cv');

      if (isset($parameter['cvterm_id'])) {
        // Global search.
        $index = 'cvterm_id';
      }
      else {
        // Term in cv.
        if (isset($parameter['cv_id'])) {
          $term['cv_id'] = array('cv_id' => $parameter['cv_id']);
        }
        else {
          $cv_id = variable_get($vars['ap_cv']);
          $term['cv_id'] = array('cv_id' => $cv_id);
        }

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

        if ($result->rowCount() == 1) {
          $db_id = $result->fetchField();

          $cvprop['db_id'] = $db_id;
        }
        else {
          $cvprop['db_id'] = null;
        }

        // Crop ontology.
        unset($result);
        $related_cvterm = variable_get($vars['ap_related']);

        // Fetch crop ontology of trait in chado.cvterm_relationship.
        $result = chado_query(
          "SELECT t1.name AS name, t2.subject_id AS subject_id FROM {cvterm} AS t1 INNER JOIN {cvterm_relationship} AS t2 ON t1.cvterm_id = t2.object_id
           WHERE t2.type_id = :related AND t2.object_id = :cvterm_id LIMIT 1",
          array(
            ':related' => $related_cvterm,
            ':cvterm_id' => $cvprop['cvterm_id'],
          )
        );

        if ($result->rowCount() == 1) {
          $co = $result->fetchObject();

          $cvprop['crop_ontology'] = array(
            'cvterm_id' => $co->subject_id,
            'ontology_name' => $co->name,
          );
        }
        else {
          $cvprop['crop_ontology'] = array(
            'cvterm_id' => null,
            'ontology_name' => null,
          );
        }

        // Scale.
        $cvprop['scale'] = null;
      }

      break;

    //
    case 'insert_cvterm':
      $vars = analyzedphenotypes_systemvars('db_cv');

      $var_db_value = variable_get($vars['ap_database']);
      $dbprop = analyzedphenotypes_dbprop('match_db', array(
        'db_id' => $var_db_value,
      ));
      $db_name = $dbprop['name'];

      $var_cv_value = variable_get($vars['ap_cv']);
      $cvprop = analyzedphenotypes_cvprop('match_cvid', array(
        'cv_id' => $var_cv_value,
      ));
      $cv_name = $cvprop['name'];

      $cvterm = tripal_insert_cvterm(array(
        'id' => trim($db_name) . ':' . $parameter['name'],
        'name' => $parameter['name'],
        'cv_name'  => $cv_name,
        'definition' => $parameter['definition'],
      ));

      if ($cvterm) {
        $cvprop = $cvterm->cvterm_id;
      }

      break;

    //
    case 'cvterm_save_photo':
      $cvterm_id = $parameter['cvterm_id'];

      foreach($parameter['photo'] as $i => $photo) {
        $tmp_file = file_save_upload($photo, array(), FALSE, FILE_EXISTS_REPLACE);

        if ($tmp_file) {
          $file_obj = file_load($tmp_file->fid);

          $source_file = drupal_realpath($file_obj->uri);
          $info = image_get_info($source_file);

          $scale = analyzedphenotypes_scale_photo(array(
            'height' => $info['height'],
            'width' => $info['width'],
            'scale_to' => 400,
          ));

          $photo_dir = 'public://ap-photo'; 
          file_prepare_directory($photo_dir, FILE_CREATE_DIRECTORY);

          $new_image = imagecreatetruecolor($scale['width'], $scale['height']);
          $img_file  = imagecreatefromjpeg($source_file);
          imagecopyresampled($new_image, $img_file, 0, 0, 0, 0, $scale['width'], $scale['height'], $info['width'], $info['height']);

          $new_filename = $cvterm_id . '_' . ($i + 1) . '.jpg';
          imagejpeg($new_image, drupal_realpath('public://ap-photo') . '/' . $new_filename, 100);

          imagedestroy($img_file);
          imagedestroy($new_image);

          $tmp_file->filename = $new_filename;
          $tmp_file->uri = 'public://ap-photo/' . $new_filename;
          $tmp_file->status = FILE_STATUS_PERMANENT;
          file_save($tmp_file);
          file_usage_add($tmp_file, 'analyzedphenotypes', 'file', 1);
        }
      }

      break;

    //
    case 'cvterm_get_photo':
      $photo = array('ap_photo_1', 'ap_photo_2');

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
 * API: TRAIT NAME
 * Manage property and operation pertaining to trait name.
 * Default to:
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   - no_unit : Exclude unit from a trait name.
 *   - unit    : Extract the unit part from a trait name.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Array keys:
 *   - trait_name
 *
 * @return String $traitprop
 *   Unit or trait name.
 *
 * @see analyzedphenotypes_loader_form_describe() in admin.form.inc.
 * @see analyzedphenotypes_validator_value() in validators.inc.
 */
function analyzedphenotypes_traitprop($property, $parameter) {
  $traitprop = null;

  switch($property) {
    //
    case 'no_unit':
      $traitprop = preg_replace("/\(.+\)/", '', $parameter['trait_name']);

      break;

    //
    case 'unit':
      preg_match("/.*\(([^)]*)\)/", $parameter['trait_name'], $match);
      $traitprop = (isset($match[1])) ? $match[1] : 'text';

      break;

    //
  }


  return $traitprop;
}


/**
 * API: DATABASE PROPERTY
 * Manage property and operation pertaining to database in chado.db table.
 * Default to: Return all databases in chado.db table.
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - match_db  : Match a record in chado.db given a database id.
 *   - default   : All records in chado.db table.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - db_id
 *
 * @return Array $dbprop
 *   Array of database properties.
 *
 * @see analyzedphenotypes_admin_settings() in admin.form.inc.
 */
function analyzedphenotypes_dbprop($property = null, $parameter = null) {
  $dbprop = null;

  // Fetch all databases in chado.db table.
  $dbs = chado_query(
    "SELECT db_id, name, description FROM {db} ORDER BY name ASC"
  );

  if ($dbs->rowCount() <= 0) {
    return $dbprop;
  }

  switch($property) {
    //
    case 'match_db':
      foreach($dbs as $db) {
        if ($db->db_id == $parameter['db_id']) {
          $dbprop = array(
            'db_id' => $db->db_id,
            'name' => $db->name,
            'description' => $db->description,
          );

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
 * FUNCTION CALLBACK: VALIDATION RESULT
 * Manage validation result window.
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
 * API: TRIPAL JOB
 * Manage property and operation pertaining to Tripal Job requests.
 * Default to: Return error limit and report frequency settings.
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - new_job         : Register a new Tripal Job.
 *   - get_job         : Get job details.
 *   - update_progress : Update tripal job progres in tripal_jobs table.
 *   - write_progress  : Write percentage complete value/validation result to a temporary file used by progress bar/validation result window.
 *   - read_progress   : Read values in temporary file by write_progress instance.
 *   - tripal_fail     : Standard tripal error terminal reporting/messaging.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - description
 *   - callback
 *   - dataset
 *   - job_id
 *   - progress
 *   - source (filenames used to write and read: source=jobprogress for standard progress bar and source=validateresult for validation result)
 *   - message
 *   - param
 * @return $tripaljobprop
 *
 * @see analyzedphenotypes_validate_tsv_file() in validators.inc.
 * @see analyzedphenotypes_save_tsv_data() in validators.inc.
 * @see analyzedphenotypes_data_json() in .module.
 * @see analyzedphenotypes_file_insert() in admin.form.inc.
 */
function analyzedphenotypes_tripaljobprop($property = null, $parameter = null) {
  $tripaljobprop = null;

  switch($property) {
    //
    case 'new_job':
      global $user;

      $job_id = tripal_add_job(
        $parameter['description'], 'analyzedphenotypes', $parameter['callback'],
        array(serialize($parameter['dataset'])),
        $user->uid
      );

      if ($job_id > 0) {
        $tripaljobprop = $job_id;
      }

      break;

    //
    case 'get_job':
      global $user;

      $job = tripal_get_job($parameter['job_id']);

      if ($job > 0 && $job->uid == $user->uid && $job->modulename == 'analyzedphenotypes') {
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
      $ap = 'analyzedphenotypes';
      $true_print = array('print' => TRUE);

      tripal_report_error($ap, TRIPAL_CRITICAL,
        "\n" . $parameter['message'] . ' [VALUE: @param]' . "\n",
        array('@param' => $parameter['param']),
        $true_print
      );

      tripal_report_error($ap, TRIPAL_CRITICAL,
        'Failed to load phenotypic data in Tripal Job #!job_id.',
        array('!job_id' => $parameter['job_id']),
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
 * API: ONTOLOGY PROPERTY
 * Manage property and operation pertaining to ontology.
 * Default to:
 * Dependencies: analyzedphenotypes_systemvars().
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - map_ontology_trait : Map the ontology to trait in describe trait form (stage 3).
 *   - suggest            : Suggest ontology in ontology field in describe trait form (stage 3) based on project genus and keywords in the trait.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - ontology_cvterm_id
 *   - trait_cvterm_id
 *   - project_genus
 *   - trait_name
 *
 * @return Array $ontologyprop
 *   Array of ontology properties.
 *
 * @see analyzedphenotypes_loader_form_describe() in admin.form.inc.
 */
function analyzedphenotypes_ontologyprop($property = null, $parameter = null) {
  $ontologyprop = null;

  switch($property) {
    //
    case 'map_ontology_trait':
      $vars = analyzedphenotypes_systemvars('db_cv');
      $related_cv_id = variable_get($vars['ap_related']);

      $values = array(
        'type_id' => $related_cv_id,
        'subject_id' => $parameter['ontology_cvterm_id'],
        'object_id'  => $parameter['trait_cvterm_id'],
      );

      chado_insert_record('cvterm_relationship', $values);

      break;

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

      foreach($ontology_terms as $i => $term) {
        $name = strtolower(trim($term['name']));
        if ($name == $trait_name) {
          $ontologyprop[$i] = $term;

          break;
        }
        else {
          $j = 0;
          $keywords = explode(' ', $name);

          foreach($keywords as $key) {
            if (stristr($trait_name, $key)) {
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
 * API: SYSTEM VARIABLE
 * Manage property and operation pertaining to system variable.
 * Default to: Return variables.
 * Dependencies: analyzedphenotypes_genusprop().
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - db_cv          : Variables to define database and cv to use.
 *   - plant_property : Variables to hold cvterm for plant properties: Location, Replicate, Year and Data Collector.
 *   - genus_ontology : Variables to hold ontology for each genus.
 *   - default        : All variables defined.
 *
 * @return Array $systemvars
 *   Array of variables names.
 *
 * @see analyzedphenotypes_admin_settings() in admin.form.inc.
 * @see analyzedphenotypes_loader_form_save() in admin.form.inc.
 * @see analyzedphenotypes_ap_validators() in validators.inc.
 * @see analyzedphenotypes_save_tsv_data() in admin.form.inc.
 * @see analyzedphenotypes_cvprop() in api.inc.
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

  // Genus to ontology terms.
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


/**
 * FUNCTION CALLBACK: AJAX CALLBACK
 * Manage AJAX callback.
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
 * FUNCTION: COMPUTE STANDARD DEVIATION.
 * Credits to: https://www.mathsisfun.com/data/standard-deviation.html.
 * This user-land implementation follows the implementation quite strictly;
 * it does not attempt to improve the code or algorithm in any way. It will
 * raise a warning if you have fewer than 2 values in your array, just like
 * the extension does (although as an E_USER_WARNING, not E_WARNING).
 *
 * @param array $a
 * @param bool $sample [optional] Defaults to false
 * @return float|bool The standard deviation or false on error.
 */
function analyzedphenotypes_stats_standard_deviation(array $a, $sample = false) {
  $n = count($a);

  if ($n === 0) {
      return false;
  }

  if ($sample && $n === 1) {
      return false;
  }

  $mean = array_sum($a) / $n;
  $carry = 0.0;

  foreach ($a as $val) {
    $d = ((double) $val) - $mean;
    $carry += $d * $d;
  };

  if ($sample) {
     --$n;
  }

  return sqrt($carry / $n);
}


/**
 * FUNCTION CONSTRUCT SUMMARY TABLE.
 */
function analyzedphenotypes_table_values($rows, $column_row, $trait) {
  $c = analyzedphenotypes_columnsprop();
  $columns = array_keys($c);

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

      if ($data[$index_trait] == $trait) {
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


/**
 * FUNCTION SCALE PHOTO.
 */
function analyzedphenotypes_scale_photo($parameter) {
  $h = $parameter['height'];
  $w = $parameter['width'];
  $scale_to = $parameter['scale_to'];

  $ratio = ($w > $h) ? $scale_to / $w : $scale_to / $h;

  $new_w = round($w * $ratio);
  $new_h = round($h * $ratio);

  return array('height' => $new_h, 'width' => $new_w);
}
