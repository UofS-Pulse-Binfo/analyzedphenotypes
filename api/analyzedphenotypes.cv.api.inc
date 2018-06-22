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
      'opts'  => array('dataset' => 'fullset'),
      'result' => 1,
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
  $cvterm = call_user_func($command[$property]['func'], $command[$property]['param'], $command[$property]['opts']);

  if ($command[$property]['result'] == 1) {
    $cvterm = reset($cvterm);
  }

  return $cvterm;
}


///////


/**
 * FUNCTION:
 * Fetch controlled vocabulary term from chado.cvterm table.
 * Dependencies: ap_get_variablenames() - systemvars.api.
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - keyword : fetch terms that match a name keyword.
 *   or - cvtermcv : fetch terms that match a pre-formatted name and cv name combination.
 *   or - cv_id : fetch terms that match a controlled vocabulary (cv) id number.
 *   or - name : fetch terms that match name.
 *   or - cvterm_id : fetch term that matches a cvterm id number.
 *   - genus : genus, optional, when provided will restrict/limit search to cv configuration of a genus.
 * @param $options
 *   An array to be used as additional options to be performed to
 *   returned result. Keys:
 *   - dataset : Option to format and limit fields in returned value. Default to cvtermid.
 *     cvtermid - return cvterm id numbers.
 *    or cvtermidname - return cvterm id and cvterm name.
 *    or namecvname - return cvterm name and cv name.
 *    or namecvname_format - return cvterm name and cv name in name (cv name) format.
 *    or fullset - include additional cvterm properties cv, ontology and database.
 *
 * @return
 *   An array, information or properties about controlled vocabulary term.
 */
function ap_get_cvterm($identifiers, $options = array('dataset' => 'cvtermid')) {
  // Inspect what is provided:
  $value = reset(array_keys($identifiers));

  // Build Query: Fill in the blanks.
  // SELECT __ FROM __ WHERE __ AND __ ORDER BY __
  // EXECUTE...
  // RESULT ...

  // SELECT.
  $sql = 'SELECT ';
  $sql_fld = 'cvterm.cvterm_id, cvterm.name, cvterm.definition, cvterm.cv_id, cvterm.dbxref_id';
  if ($options['dataset'] != 'cvtermid' || $options['dataset'] != 'cvtermidname') {
    $sql_fld .= ', cv.name AS cv_name';
  }

  $sql .= $sql_fld . ' ';

  // FROM.
  $sql .= 'FROM';
  $sql_tbl = '{cvterm}';
  if ($options['dataset'] != 'cvtermid' || $options['dataset'] != 'cvtermidname') {
    $sql_tbl .= ' INNER JOIN {cv} USING(cv_id)';
  }

  $sql .= ' ' . $sql_tbl . ' ';

  // WHERE.
  $sql .= 'WHERE';
  $sql_where = array('field', 'match' => '=', 'value', 'args' => ':value');

  if ($value == 'cvterm_id') {
    // BY CVTERM ID NUMBER.
    $sql_where['field'] = 'cvterm.cvterm_id';
    $sql_where['value'] = $identifiers[$value];
  }
  elseif ($value == 'name') {
    // BY CVTERM NAME.
    $sql_where['field'] = 'cvterm.name';
    $sql_where['value'] = $identifiers[$value];
  }
  elseif ($value == 'keyword') {
    // BY CVTERM NAME KEYWORD.
    $sql_where['field'] = 'LOWER(cvterm.name)';
    $sql_where['match'] = 'LIKE';
    $sql_where['value'] = '%' . strtolower($identifiers[$value]) . '%';
  }
  elseif ($value == 'cvtermcv') {
    // BY CVTERM NAME AND CV NAME COMBINATION.
    $sql_where['field'] = 'CONCAT(cvterm.name, \' (\', cv.name, \')\')';
    $sql_where['value'] = $identifiers[$value];
  }
  elseif ($value == 'cv_id') {
    // BY CV ID NUMBER.
    $sql_where['field'] = 'cvterm.cv_id';
    $sql_where['value'] = $identifiers[$value];
  }

  // field = :value
  $sql .= ' ' . $sql_where['field'] . ' ' . $sql_where['match'] . ' ' . $sql_where['args'] . ' ';
  $args = array($sql_where['args'] => $sql_where['value']);

  // WHERE AND.
  if (isset($identifiers['genus']) && $value != 'cv_id') {
    // Restrict dataset to the cv configuration for a genus.
    // If both cv_id (search by cv_id) and genus were provided,
    // ignore this block and prioritize cv_id over genus.
    $sysvar = ap_get_variablenames(
      array('variablename' => $identifiers['genus']),
      array('set' => 'cvdbon', 'suffix' => 'cv')
    );

    $sysvar_genus_cv = variable_get($sysvar);
    $sql .= 'AND cvterm.cv_id = :config_cvid ';

    $args[':config_cvid'] = $sysvar_genus_cv;
  }

  // ORDER.
  $sql .= 'ORDER BY cvterm.name ASC';

  //
  // EXECUTE.
  $result = chado_query($sql, $args);

  // RESULT.
  $cvterm = null;

  if ($result->rowCount()) {
    if ($options['dataset'] == 'cvtermid' || $options['dataset'] == 'cvtermidname') {
      // Return cvterm_id number.
      // Return cvterm_id and name.
      foreach($result as $term) {
        $d = ($options['dataset'] == 'cvtermid')
          ? array('cvterm_id' => $term->cvterm_id)
          : array('cvterm_id' => $term->cvterm_id, 'name' => $term->name);

        $cvterm[] = $d;
      }
    }
    elseif ($options['dataset'] == 'namecvname' || $options['dataset'] == 'namecvname_format') {
      // Return name and cv name - no formatting.
      // Return name and cv name - with formatting.
      foreach($result as $term) {
        $d = ($options['dataset'] == 'namecvname')
          ? array('name' => $term->name, 'cv_name' => $term->cv_name)
          : array('namecvname' => $term->name . ' (' . $term->cv_name . ')');

        $cvterm[] = $d;
      }
    }
    elseif ($options['dataset'] = 'fullset') {
      // Include other term properties (cv, db, ontology etc.).
      foreach($result as $term) {
        // DB info.
        $db = chado_generate_var('dbxref',
          array('dbxref_id' => $term->dbxref_id)
        );

        // ON ontology info.
        $sysvar = ap_get_variablenames(
          array('variablename' => 'related'),
          array('set' => 'terms')
        );

        $sysvar_related = variable_get($sysvar);
        $on = chado_generate_var('cvterm_relationship',
          array('object_id' => $term->cvterm_id, 'type_id' => $sysvar_related)
        );

        // Construct full, additional properties of term.
        $cvterm[] = array(
          // cvterm.
          'name'    => $term->name,
          'cvterm_id' => $term->cvterm_id,
          'definition' => $term->definition,

          // cv.
          'cv_id' => $term->cv_id,
          'cv_name' => $term->name,

          // db.
          'dbxref_id' => $term->dbxref_id,
          'db_id'   => ($db) ? $db->db_id->db_id : null,

          // on.
          'crop_ontology' => ($on) ? $on->subject_id->name : null
        );
      }
    }
  }

  return $cvterm;
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
  $cvterm = null;

  if ($value == 'keyword') {
    // Keyword entered in a field.
    $term = ap_get_cvterm(
      array('keyword' => $identifiers['keyword']),
      array('dataset' => 'namecvname')
    );

    if ($term) {
      foreach($term as $i => $t) {
        $cvterm[ $t['name'] ] = $t['cv_name'];

        if ($i >= 20) {
          break;
        }
      }
    }
  }
  elseif ($value == 'cvtermcv_cvtermid') {
    // Cvterm name (cv name) entered in a field and expect cvterm_id number.
    $term = ap_get_cvterm(
      array('cvtermcv' => $identifiers['cvtermcv_cvtermid']),
      array('dataset' => 'cvtermid')
    );

    if ($term) {
      $term = reset($term);
      $cvterm = $term['cvterm_id'];
    }
  }
  elseif ($value == 'cvtermid_cvtermcv') {
    // Cvterm id number entered in a field and expect cvterm name (cv name).
    $term = ap_get_cvterm(
      array('cvterm_id' => $identifiers['cvtermid_cvtermcv']),
      array('dataset' => 'namecvname_format')
    );

    if ($term) {
      $term = reset($term);
      $cvterm = $term['namecvname'];
    }
  }

  return $cvterm;
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
