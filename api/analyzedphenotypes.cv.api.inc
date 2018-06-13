<?php
/**
 * @file
 *
 * API: CONTROLLED VOCABULARY
 * Manage property and operation pertaining to data in chado.cvterm and chado.cv tables.
 */


///////


/**
 * CONTROL/SWITCHBOARD.
 * Pass a request to a corresponding function.
 * @TODO: leave out when api reorganize/update is complete and use functions below.
 */
function analyzedphenotypes_cvprop($property = null, $parameter = null) {
  if (is_null($property)) {
    // Default.
    $property = 'default';
  }

  // Process request.
  $command = array(
    'default' => array(
      'func'  => 'ap_get_cv',
      'param' => null,
      'opts'  => null,
    ),
    'match_key' => array(
      'func'  => 'ap_get_cvterm',
      'param' => array('keyword' => $parameter['cvterm_keyword'], 'genus' => $parameter['genus']),
      'opts'  => null,
    ),
    'match_cvid' => array(
      'func'  => 'ap_get_cv',
      'param' => array('cv_id' => $parameter['cv_id']),
      'opts'  => null,
    ),
    'terms_in_cv' => array(
      'func'  => 'ap_get_cvterm',
      'param' => array('cv_id' => $parameter['cv_id']),
      'opts'  => null,
    ),
    'get_cvterm' => array(
      'func'  => 'ap_get_cvterm',
      'param' => array('cvterm_id' => $parameter['cvterm_id'], 'genus' => $parameter['genus']),
      'opts'  => array('resultset' => 'full'),
    ),
    'insert_cvterm' => array(
      'func'  => 'ap_insert_cvterm',
      'param' => array('name' => $parameter['name'], 'definition' => $parameter['definition'], 'genus' => $parameter['genus']),
      'opts'  => array('return_inserted_id' => TRUE),
    ),
    'cvterm_save_photo' => array(
      'func'  => 'ap_upload_cvtermphoto',
      'param' => array('cvterm_id' => $parameter['cvterm_id'], 'photo' => $parameter['photo']),
      'opts'  => array('scaleto' => 400),
    ),
    'cvterm_get_photo' => array(
      'func'  => 'ap_download_cvtermphoto',
      'param' => $parameter,
      'opts'  => null,
    ),
    'suggest_term' => array(
      'func'  => 'ap_autocomplete_cvtermfields',
      'param' => array('keyword' => $parameter['keyword']),
      'opts'  => null,
    ),
    'term_cv' => array(
      'func'  => 'ap_autocomplete_cvtermfields',
      'param' => array('cvtermcv_cvtermid' => $parameter['term_cv']),
      'opts'  => null,
    ),
    'term_id' => array(
      'func'  => 'ap_autocomplete_cvtermfields',
      'param' => array('cvtermid_cvtermcv' => $parameter['term_id']),
      'opts'  => null,
    ),
  );

  // Function name, parameter (project name key or name), additional options.
  return call_user_func($command[$property]['func'], $command[$property]['param'], $command[$property]['opts']);
}


///////


/**
 * FUNCTION:
 * Fetch controlled vocabulary term from chado.cvterm table.
 * Dependencies: ap_get_variablenames() - systemvars.api.
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - keyword : match a cvterm record given a name keyword.
 *   or - cvtermcv : match cvterm record given a pre-formatted [cvterm name (cv name)] combination.
 *   or - cv_id : search term(s) in/given a cv id number.
 *   or - name : match a cvterm record given a cvterm name.
 *   or - cvterm_id : match a cvterm record given a cvterm_id number.
 *   - genus : genus.
 * @param $options
 *   An array to be used as additional options to be performed to
 *   returned result. Keys:
 *   - resultset : when set to full, additional information such as db, cv, ontology and photo info will be added.
 *     A cvtermcv, will return cvterm and cv information in cvterm name (cv name) format.
 *     Default to idname, which will return only the name and cvterm_id number.
 *
 * @return
 *   An array, information about controlled vocabulary term.
 */
function ap_get_cvterm($identifiers, $options = array('resultset' => 'idname')) {
  // Inspect what is provided:
  $value = reset(array_keys($identifiers));

  // Require genus cv configuration.
  // Prepare system variables:


  if (in_array($value, array('cvterm_id', 'name', 'keyword'))) {
    $sysvar = ap_get_variablenames(
      array('variablename' => $identifiers['genus']),
      array('set' => 'cvdbon', 'suffix' => 'cv')
    );

    $sysvar_genus_cv = variable_get($sysvar);

    if ($value == 'cvterm_id' && $options['resultset'] == 'full') {
      // When searching by cvterm_id and requiring detailed properties.
      // Get cv term related, to be used to fetch ontology information.
      unset($sysvar);
      $sysvar = ap_get_variablenames(
        array('variablename' => 'related'),
        array('set' => 'terms')
      );

      $sysvar_related = variable_get($sysvar);
    }
  }


  // Process search.

  $cvterm = null;

  // # SEARCH BY NAME KEYWORD.
  // search is limited to the genus cv configuration.
  if ($value == 'keyword') {
    $cvterm = chado_query(
      "SELECT cvterm_id, name FROM {cvterm}
      WHERE cv_id = :cv_id AND name LIKE :value ORDER BY name ASC",
      array(
        ':cv_id' => $sysvar_genus_cv,
        ':value' => '%' . $identifiers['keyword'] . '%'
      )
    );

    return ($cvterm->rowCount()) ? $cvterm->fetchAllKeyed() : null;
  }

  // # SEARCH BY CVTERM NAME (CV NAME) COMBINATION.
  // search is not limited to an controlled vocabulary (cv).
  if ($value == 'cvtermcv') {
    $cvterm = chado_query(
      "SELECT cvterm_id FROM {cvterm} AS t1 INNER JOIN {cv} AS t2 USING(cv_id)
      WHERE CONCAT(t1.name, ' (' , t2.name , ')') = :value LIMIT 1",
      array(':value' => $identifiers['cvtermcv'])
    );

    return ($cvterm->rowCount()) ? $cvterm->fetchField() : null;
  }

  // # SEARCH BY CV ID.
  // search is limited to controlled vocabulary (cv) provided.
  if ($value == 'cv_id') {
    $cvterm = chado_query(
      "SELECT cvterm_id, name, definition FROM {cvterm}
      WHERE cv_id = :value ORDER BY name ASC",
      array(':value' => $identifiers['cv_id'])
    );

    if ($cvterm) {
      $term = array();

      foreach($cvterm as $c) {
        $term[$c->cvterm_id] = array(
          'name' => $c->name,
          'definition' => $c->definition,
        );
      }

      return $term;
    }
  }

  // May request detailed cvterm properties when implied in $options parameter.

  // # SEARCH BY NAME.
  // search is limited to the genus cv configuration.
  if ($value == 'name') {
    $args = array(
      'cv_id' => $sysvar_genus_cv,
      'name' => $identifiers['name']
    );

    $cvterm = (function_exists('chado_get_cvterm')) ? chado_get_cvterm($args) : tripal_get_cvterm($args);
  }


  // # SEARCH BY CVTERM ID NUMBER.
  // search is limited to the genus cv configuration.
  if ($value == 'cvterm_id') {
    $args = array(
      'cv_id' => $sysvar_genus_cv,
      'cvterm_id' => $identifiers['cvterm_id']
    );

    $cvterm = (function_exists('chado_get_cvterm')) ? chado_get_cvterm($args) : tripal_get_cvterm($args);
  }


  // Result options.


  if ($cvterm) {
    $term = array();

    if ($options['resultset'] == 'full') {
      // Full set.

      // Database info.
      $database = chado_generate_var('dbxref',
        array('dbxref_id' => $cvterm->dbxref_id)
      );

      // Ontology info.
      $ontology = chado_query(
        "SELECT name FROM {cvterm} WHERE cvterm_id =
          (SELECT subject_id FROM {cvterm_relationship}
            WHERE type_id = :related AND object_id = :value LIMIT 1)",
        array(
          ':related' => $sysvar_related,
          ':value' => $cvterm->cvterm_id,
        )
      )
      ->fetchField();

      // Construct info array.
      $term = array(
        // cvterm.
        'cvterm_id' => $cvterm->cvterm_id,
        'name' => $cvterm->name,
        'definition' => $cvterm->definition,

        // cv.
        'cv_id' => $cvterm->cv_id->cv_id,
        'cv_name' => $cvterm->cv_id->name,

        // photo.
        'photo_1' => $cvterm->cvterm_id . '_1.jpg',
        'photo_2' => $cvterm->cvterm_id . '_2.jpg',

        // database.
        'dbxref_id' => $cvterm->dbxref_id,
        'db_id' => ($database) ? $database->db_id->db_id : 0,

        // ontology
        'crop_ontology' => ($ontology) ? $ontology : null
      );
    }
    elseif ($options['resultset'] == 'cvtermcv') {
      // cvterm name (cv name) format.
      $term = $cvterm->name . ' (' . $cvterm->cv_id->name . ')';
    }
    else {
      // Id and name - default:
      $term[ $cvterm->cvterm_id ] = $cvterm->name;
    }

    return $term;
  }
  else {
    // null.
    return $cvterm;
  }
}

/**
 * FUNCTION:
 * Insert an entry into chado.cvterm table.
 * Dependencies: ap_get_variablenames() - systemvars.api.
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - name : controlled vocabylary name/trait name/column header.
 *   - definition : human-readable definition of a term.
 *   - genus : genus.
 *
 *   An array to be used as additional options to be performed to
 *   returned result. Keys:
 *   - return_inserted_id : option to return cvterm term id number inserted.
 *
 * @return
 *   An array, term properties.
 */
function ap_insert_cvterm($identifiers, $options = array('return_inserted_id' => TRUE)) {
  $sysvar = ap_get_variablenames(
    array('variablename' => $identifiers['genus']),
    array('set' => 'cvdbon', 'suffix' => 'allsuffix')
  );

  // Database.
  $sysvar_genus_db = variable_get($sysvar['db']);
  //
  // TODO: Replace when database API is ready.
  // Requires the db name.
  $dbprop = analyzedphenotypes_dbprop('match_db', array(
    'db_id' => $sysvar_genus_db,
  ));
  $db_name = $dbprop['name'];

  // Controlled Vocabulary.
  $sysvar_genus_cv = variable_get($sysvar['cv']);
  $cvprop = analyzedphenotypes_cvprop('match_cvid', array(
    'cv_id' => $sysvar_genus_cv,
  ));
  $cv_name = $cvprop['name'];

  //Insert cvterm.
  $cvterm = tripal_insert_cvterm(array(
    'id'  => trim($db_name) . ':' . $identifiers['name'],
    'name'  => $identifiers['name'],
    'cv_name' => $cv_name,
    'definition' => $identifiers['definition']
  ));

  // Return inserted id when implied in options.
  if ($cvterm && $options['return_inserted_id']) {
    return $cvterm->cvterm_id;
  }
}

/**
 * FUNCTION:
 * Upload/save image/photo of a cvterm.
 * Dependencies:
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - cvterm_id : cvterm id number of term/trait.
 *   - photo : File field API value name ap_photo_1 and ap_photo_2.
 *
 * @param $options
 *   An array to be used as additional options to be performed to
 *   returned result. Keys:
 *   - scaleto : scale to or reduce image size to this value. Default to 400X400 px.
 */
function ap_upload_cvtermphoto($identifiers, $options = array('scaleto' => 400)) {
  foreach($identifiers['photo'] as $i => $photo) {
    // Save upload file.
    $tmp_file = file_save_upload($photo, array(), FALSE, FILE_EXISTS_REPLACE);

    // Access file saved and modify dimension plus filename plus filename.
    if ($tmp_file) {
      $file_obj = file_load($tmp_file->fid);

      $source_file = drupal_realpath($file_obj->uri);
      $info = image_get_info($source_file);

      // Scale but maintain aspect ratio.
      $scale = analyzedphenotypes_scale_photo(array(
        'height' => $info['height'],
        'width' => $info['width'],
        'scale_to' => $options['scaleto'],
       ));

      $photo_dir = 'public://ap-photo';
      file_prepare_directory($photo_dir, FILE_CREATE_DIRECTORY);

      // Create a blank image using the computed height and width, then resample uploaded
      // image into this blank image which will become the traits photo.
      $new_image = imagecreatetruecolor($scale['width'], $scale['height']);
      $img_file  = imagecreatefromjpeg($source_file);
      imagecopyresampled($new_image, $img_file, 0, 0, 0, 0, $scale['width'], $scale['height'], $info['width'], $info['height']);

      $new_filename = $identifiers['cvterm_id'] . '_' . ($i + 1) . '.jpg';
      imagejpeg($new_image, drupal_realpath($photo_dir) . '/' . $new_filename, 100);

      imagedestroy($img_file);
      imagedestroy($new_image);

      // Inform Drupal file system about this alterations.
      // When file is present in file_managed, skip this block.
      $check_file = entity_load('file', FALSE, array('filename' => $new_filename));
      if (empty($check_file)) {
        $tmp_file->filename = $new_filename;
        $tmp_file->uri = $photo_dir . '/' . $new_filename;
        $tmp_file->status = FILE_STATUS_PERMANENT;
        file_save($tmp_file);
        file_usage_add($tmp_file, 'analyzedphenotypes', 'file', 1);
      }
    }
  }
}

/**
 * FUNCTION:
 * Dowload/get photo file information of a cvterm.
 * Dependencies:
 *
 * @param $cvterm_id
 *   An integer, controlled vocabulary id number.
 *
 * @param
 *   An array, file meta data.
 */
function ap_download_cvtermphoto($cvterm_id) {
  $cvtermphoto = null;
  $photo = array('ap_photo_1', 'ap_photo_2');

  foreach($photo as $i => $v) {
    $filename = $cvterm_id . '_' . ($i + 1) . '.jpg';
    $filedir = 'public://ap-photo';

    $fileuri = drupal_realpath($filedir) . '/' . $filename;

    if (file_exists($fileuri)) {
      $info = image_get_info($fileuri);

      $cvtermphoto[$v] = array(
        'filename' => $filename,
        'uri' => $filedir . '/' . $filename,
        'height' => $info['height'],
        'width' => $info['width'],
      );
    }
    else {
      // No photo.
      $cvtermphoto[$v] = null;
    }
  }

  return $cvtermphoto;
}

/**
 * FUNCTION:
 * Handle autocomplete fields that takes in cvterm name (cv name) or cvterm_id.
 * Return value may either be cvterm_id or cvterm name (cv name).
 * Format: cvterm name (cv name).
 *
 * @param $identifiers
 *   An array with the following key:
 *     - keyword : request terms given a keyword used in populating field suggestions/options.
 *     or - cvtermcv_cvtermid : request cvterm id number given a cvterm name (cv name).
 *     or - cvtermid_cvtermcv : request cvterm name (cv name) given a cvterm id number.
 *
 * @return
 *   cvterm name (cv name) format or cvterm id number.
 */
function ap_autocomplete_cvtermfields($identifiers) {
  $value = reset(array_keys($identifiers));

  // Generate suggestions.
  if ($value == 'keyword') {
    $cvterm = chado_query(
      "SELECT TRIM(t2.name), TRIM(t1.name) AS cv_name FROM {cv} AS t1 INNER JOIN {cvterm} AS t2 USING (cv_id)
      WHERE LOWER(t2.name) LIKE LOWER(:value) ORDER BY t1.name LIMIT 20",
      array(':value' => '%' . $identifiers['keyword'] . '%')
    );

    return ($cvterm->rowCount()) ? $cvterm->fetchAllKeyed() : null;
  }

  // Entered value cvterm name (cv name) return cvterm id.
  if ($value == 'cvtermcv_cvtermid') {
    $cvterm = ap_get_cvterm(
      array('cvtermcv' => $identifiers['cvtermcv_cvtermid'])
    );

    return ($cvterm) ? $cvterm : null;
  }

  // Entered value cvterm id return cvterm name (cv name).
  if ($value == 'cvtermid_cvtermcv') {
    $args = array(
      'cvterm_id' => $identifiers['cvtermid_cvtermcv']
    );

    $cvterm = (function_exists('chado_get_cvterm')) ? chado_get_cvterm($args) : tripal_get_cvterm($args);

    return ($cvterm) ? $cvterm->name . ' (' . $cvterm->cv_id->name . ')' : null;
  }
}

/**
 * FUNCTION:
 * Fetch controlled vocabulary (cv) properties in chado.cv table.
 * Dependencies:
 *
 * @param $identifiers
 *   An array with the following keys: Default to null, which will return all cv name in chado.cv.
 *   - cv_id : controlled vocabulary (cv) id.
 *
 * @return
 *   An array, contolled vocabulary (cv) properties.
 */
function ap_get_cv($identifiers = null) {
  // #SEARCH BY CV ID.
  if (isset($identifiers['cv_id'])) {
    $cv = tripal_get_cv(array(
      'cv_id' => $identifiers['cv_id'],
    ));

    if ($cv) {
      return array(
        'cv_id' => $cv->cv_id,
        'name'    => $cv->name,
        'definition' => $cv->definition,
      );
    }
  }
  else {
    // No id provided, return all cvs in chado.cv table.
    $cv = chado_query(
      "SELECT cv_id, name, definition FROM {cv} ORDER BY name ASC"
    );

    if ($cv->rowCount() > 0) {
      $cvs = array();

      foreach($cv as $c) {
        $cvs[$c->cv_id] = array(
          'name' => $c->name,
          'definition' => $c->definition,
        );
      }

      return $cvs;
    }
  }
}

/**
 * FUNCTION SCALE PHOTO.
 * @TODO: move to file management api.
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
