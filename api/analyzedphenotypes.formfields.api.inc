<?php
/**
 * @file
 *
 * API: DOWNLOAD FORM FIELDS.
 * Manage property and operation pertaining to download data form fields.
 */


/**
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_fieldnames() {
  return ap_construct_download_fieldnames();
}

/**
 * FUNCTION:
 * Global field names.
 * Create/Register field names for every form field element in the download form,
 * to be used in various hooks processing download data request by using these
 * names as reference to the element.
 *
 * @return
 *   An array of string, field names one for base name, a cache name, a field id and a hidden field name.
 */
function ap_construct_download_fieldnames() {
  // Base field names used in data download form.
  // Field can be a multi-checkbox, a checkbox, select field or a text field.
  $basenames = array(
    'experiment',    // Experiment field.
    'genus',         // Genus field.
    'species',       // Species field.
    'traits',        // Trait names field.
    'year',          // Year field.
    'location',      // Locationfield.
    'germplasmtype', // Germplasm type field.
    'germplasm',     // Germplasm field.
    'missingdata',   // Missing data field.
    'filetype',      // File type field.
    'averagerep',    // Average replicate field.
    'rfriendly',     // R Friendly field.
    'columnheaders', // Column headers field.
    'origin',        // Origin field.
    'value',         // Value field.
    'replicate',     // Replicate field.
    'collector'      // Data collector field.
  );

  // Each field requires a field name for Name attribute, #id attribute, name use for
  // labelling cached value in Drupal Form APi and a name for hidden field.
  $fieldtypes = array(
    'base',          // Base name attribute.
    'cache',         // Name used for variable storing caching field value.
    'field',         // Name attached to Name attribute of field.
    'field-id',      // Name attached to Id attribute of field.
    'field_hidden',  // Name attached to Name attribute of a hidden field.
  );

  // Array to hold constructed fieldnames.
  $fieldnames = array();

  // For each field, create a field name used for #id, cache, #hidden fieldname etc.
  foreach($basenames as $base) {
    foreach($fieldtypes as $type) {
      $separator = ($type == 'field-id') ? '-' : '_';

      // ap-fieldname-field-id.
      $value = ($type == 'base') ? $base : 'ap' . $separator . $base . $separator . $type;

      $fieldnames[$base][$type] = $value;
    }
  }

  return $fieldnames;
}


///////


/**
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_cachevalue($value) {
  return ap_cache_download_fieldvalue($value);
}

/**
 * FUNCTION:
 * Form field values are save (cached) when nanipulating fields. Values are then compared
 * to current selection to predict if new value had been selected or form retained same
 * values in each AJAX call. The form state can then be used wheather to process a request
 * or just reload values/selections.
 *
 * @param $value
 *   Value entered or selection made to field element.
 *
 * @return
 *   An array, to register a value in Drupal Form API.
 */
function ap_cache_download_fieldvalue($value) {
  if (is_array($value)) {
    $value = implode('_', $value);
  }

  return array(
    '#type' => 'value',
    '#value' => $value,
  );
}

///////


/**
 * FUNCTION:
 * @TODO: replace this function with functions below.
 */
function analyzedphenotypes_populatefield($field, $parameter = null) {
  // Use this character to separate entries in a string.
  $field_values = null;

  switch($field) {
    //
    case 'field_project':
      $field_values = ap_populate_fieldproject();

      break;

    //
    case 'field_project_summary':
      $field_values = ap_populate_fieldprojectsummary();

      break;

    //
    case 'field_genus':
      $field_values = ap_populate_fieldgenus(
        array('project_id' => $parameter['project_id'])
      );

      break;

    //
    case 'field_species':
      $field_values = ap_populate_fieldspecies(
        array(
          'project_id' => $parameter['project_id'],
          'genus' => $parameter['genus']
        )
      );

      break;

    //
    case 'field_traits':
      $field_values = ap_populate_fieldtraits(
        array(
          'project_id' => $parameter['project_id'],
          'genus' => $parameter['genus'],
          'species' => $parameter['species']
        )
      );

      break;

    //
    case 'field_year':
      $field_values = ap_populate_fieldyear(
        array(
          'project_id' => $parameter['project_id'],
          'genus' => $parameter['genus'],
          'species' => $parameter['species'],
          'traits' => $parameter['traits']
        )
      );

      break;

    //
    case 'field_location':
      $field_values = ap_populate_fieldlocation(
        array(
          'project_id' => $parameter['project_id'],
          'genus' => $parameter['genus'],
          'species' => $parameter['species'],
          'traits' => $parameter['traits'],
          'year' => $parameter['year']
        )
      );

      break;

    //
    case 'field_germplasmtype':
      $field_values = ap_populate_fieldgermplasmtype(
        array(
          'project_id' => $parameter['project_id'],
          'genus' => $parameter['genus'],
          'species' => $parameter['species'],
          'traits' => $parameter['traits'],
          'year' => $parameter['year'],
          'location' => $parameter['location']
        )
      );

      break;

    //
    case 'field_germplasm':
      $field_values = ap_populate_fieldgermplasmlist(
        array(
          'project_id' => $parameter['project_id'],
          'genus' => $parameter['genus'],
          'species' => $parameter['species'],
          'traits' => $parameter['traits'],
          'year' => $parameter['year'],
          'location' => $parameter['location'],
          'germplasmtype' => $parameter['germplasmtype']
        )
      );

      break;
  }

  return $field_values;
}

/**
 * FUNCTION:
 * Populate select project field.
 * Fetch all project with associated phenotypic data in phenotype table.
 *
 * @return
 *   Associative array, where the key is project id number and value is project name.
 */
function ap_populate_fieldproject() {
  $sql = "
    SELECT project_id, t1.name
    FROM
      {project} AS t1 INNER JOIN {phenotype} USING(project_id)
    GROUP BY project_id
    ORDER BY t1.name ASC
  ";

  $query = chado_query($sql);
  return ($query->rowCount()) ? $query->fetchAllKeyed() : null;
}

/**
 * FUNCTION:
 * Populate project summary info tool tip text.
 * Fetch data count of each project in phenotype table.
 *
 * @return
 *   Associative array, where the key is project id number and value is project summary info
 *   showing trait count and number of datapoints.
 */
function ap_populate_fieldprojectsummary() {
  $sql = "
    SELECT project_id, CONCAT('*' || COUNT(DISTINCT attr_id) || ' Traits, ' || COUNT(phenotype_id) || ' Data Points ')
    FROM {phenotype} 
    GROUP BY project_id 
    ORDER BY project_id ASC";

  $query = chado_query($sql);
  return ($query->rowCount()) ? $query->fetchAllKeyed() : null;
}

/**
 * FUNCTION:
 * Populate select genus field.
 * Fetch project genus in projectprop table where entry type is genus.
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - project_id : project or experiment id number corresponding to project_id in chado.project.
 *
 * @return
 *   Associative array, where key and value are genus.
 */
function ap_populate_fieldgenus($identifiers) {
  $values = null;

  if (isset($identifiers['project_id']) && $identifiers['project_id'] > 0) {
    $sql = "
      SELECT value, value FROM {projectprop}
      WHERE type_id = :term_genus AND project_id = :project_id
      LIMIT 1";

    $sysvar = ap_get_variablenames(
      array('variablename' => 'genus'),
      array('set' => 'term')
    );

    $sysvar_genus = variable_get($sysvar);

    $args = array(
      ':term_genus' => $sysvar_genus,
      ':project_id' => $identifiers['project_id']
    );

    $query = chado_query($sql, $args);
    $values = ($query->rowCount()) ? $query->fetchAllKeyed() : $values;
  }

  return $values;
}

/**
 * FUNCTION:
 * Populate select species field.
 * Select all unique species in organism table, filtered by genus value and
 * the species of stock set of an experiment.
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - project_id : project or experiment id number corresponding to project_id in chado.project.
 *   - genus : eg. Lens, string value, a genus.
 *
 * @return
 *   Associative array, where key and value are species.
 */
function ap_populate_fieldspecies($identifiers) {
  $values = null;

  if (isset($identifiers['project_id']) && $identifiers['project_id'] > 0 && isset($identifiers['genus'])) {
    $sql = "
      SELECT species, species FROM {organism} WHERE genus = :genus
        AND organism_id IN (SELECT organism_id FROM {stock} INNER JOIN {phenotype} USING (stock_id) WHERE project_id = :project_id)
      GROUP BY species
      ORDER BY species ASC";

    $args = array(
      ':genus' => $identifiers['genus'],
      ':project_id' => $identifiers['project_id']
    );

    $query = chado_query($sql, $args);
    $values = ($query->rowCount()) ? $query->fetchAllKeyed() : $values;
  }

  return $values;
}

/**
 * FUNCTION:
 * Populate select trait names field.
 * Fetch all traits in phenotype table when 2 or more projects have been selected, and
 * load trait names taking into account genus and species selected when only one project
 * has been selected.
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - project_id : project or experiment id number corresponding to project_id in chado.project.
 *   - genus : eg. Lens, string value, a genus.
 *   - species : string value, a species.
 *
 * @return
 *   Associative array, where key is cvterm id number and value is cvterm name.
 */
function ap_populate_fieldtraits($identifiers) {
  $values = null;

  if (isset($identifiers['project_id']) && $identifiers['project_id'] > 0) {
    // 2 or more experiments - fields genus and species field value become all
    // indicating all species and all genus.
    $sql = "
      SELECT t1.cvterm_id, t1.name
      FROM {cvterm} AS t1 INNER JOIN {phenotype} ON t1.cvterm_id = attr_id
      WHERE project_id IN (:project_id)
      GROUP BY t1.cvterm_id, t1.name
      ORDER BY t1.name ASC";

    $args = array(':project_id' => $identifiers['project_id']);

    $query = chado_query($sql, $args);
    $values = ($query->rowCount()) ? $query->fetchAllKeyed() : $values;
  }
  else {
    // 1 and only experiment.
    if (isset($identifiers['project_id']) && $identifiers['project_id'] > 0
      && !empty($identifiers['genus']) && !empty($identifiers['species'])) {

      $sql = "
        SELECT cvterm_id, name FROM {cvterm}
        WHERE cvterm_id IN (SELECT attr_id FROM {phenotype}
          WHERE
            project_id = :project_id
            AND stock_id IN (SELECT stock_id FROM {stock}
              WHERE organism_id IN (SELECT organism_id FROM {organism}
                WHERE LOWER(genus) = LOWER(:genus) AND LOWER(species) = LOWER(:species))))
        ORDER BY name ASC";

      $args = array(
        ':project_id' => $identifiers['project_id'],

        ':genus' => $identifiers['genus'],
        ':species' => $identifiers['species'],
      );

      $query = chado_query($sql, $args);
      $values = ($query->rowCount()) ? $query->fetchAllKeyed() : $values;
    }
  }

  return $values;
}

/**
 * FUNCTION:
 * Populate select year field.
 * Fetch all unique year in phenotypeprop filtered by project id, traits and genus + species combination.
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - project_id : project or experiment id number corresponding to project_id in chado.project.
 *   - genus : eg. Lens, string value, a genus.
 *   - species : string value, a species.
 *   - traits : integers, corresponding trait id (cvterm_id) of a trait name (cvterm name) selected.
 *
 * @return
 *   Associative array, where key and value are year in YYYY format.
 */
function ap_populate_fieldyear($identifiers) {
  $values = null;

  if (isset($identifiers['project_id']) && $identifiers['project_id'] > 0
    && !empty($identifiers['genus']) && !empty($identifiers['species'])
    && isset($identifiers['traits'])) {

    $sysvar = ap_get_systemvariables(
      array('variablename' => 'year'),
      array('set' => 'terms')
    );

    $sysvar_year = variable_get($sysvar);

    $sql = "
      SELECT value, value FROM {phenotypeprop} WHERE type_id = :year_property
      AND phenotype_id IN (SELECT phenotype_id FROM {phenotype}
        WHERE project_id IN (:project_id) AND attr_id IN (:traits)
          AND stock_id IN (SELECT stock_id FROM {stock}
            WHERE organism_id IN (SELECT organism_id FROM {organism}
              WHERE LOWER(genus) = LOWER(:genus) AND LOWER(species) = LOWER(:species))))
      GROUP BY value
      ORDER BY value DESC";

    $args = array(
      ':year_property' => $sysvar_year,
      ':project_id' => $identifiers['project_id'],
      ':traits' => $identifiers['traits'],

      ':genus' => $identifiers['genus'],
      ':species' => $identifiers['species'],
    );

    $query = chado_query($sql, $args);
    $values = ($query->rowCount()) ? $query->fetchAllKeyed() : $values;
  }

  return $values;
}

/**
 * FUNCTION:
 * Populate select location field.
 * Fetch all unique location in phenotypeprop filtered by project id, traits,
 * years and genus + species combination.
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - project_id : project or experiment id number corresponding to project_id in chado.project.
 *   - genus : eg. Lens, string value, a genus.
 *   - species : string value, a species.
 *   - traits : integers, corresponding trait id (cvterm_id) of a trait name (cvterm name) selected.
 *   - year : year values in YYYY.
 *
 * @return
 *   Associative array, where key and value are location values.
 */
function ap_populate_fieldlocation($identifiers) {
  $values = null;

  $sysvar = ap_get_systemvariables(
    array('variablename' => 'varset'),
    array('set' => 'terms')
  );

  $sysvar_year = variable_get($sysvar['ap_year']);
  $sysvar_location = variable_get($sysvar['ap_location']);

  $sql = "
    SELECT t1.value, t1.value FROM {phenotype}
      INNER JOIN {phenotypeprop} AS t1 USING (phenotype_id)
      INNER JOIN {phenotypeprop} AS t2 USING (phenotype_id)
    WHERE
      project_id IN (:project_id)
      AND attr_id IN (:traits)
      AND t2.value IN (:year)
      AND t2.type_id = :year_property
      AND t1.type_id = :location_property";

  $args = array(
    ':project_id' => $field_values['project_id'],
    ':traits' => $field_values['traits'],
    ':year' => $field_values['year'],

    ':year_property' => $sysvar_year,
    ':location_property' => $sysvar_location,
  );

  if (isset($identifiers['project_id']) && $identifiers['project_id'] > 0
    && !empty($identifiers['genus']) && !empty($identifiers['species'])) {

    $sql = $sql . " AND stock_id IN (SELECT stock_id FROM {stock}
      WHERE organism_id IN (SELECT organism_id FROM {organism}
        WHERE LOWER(genus) = LOWER(:genus) AND LOWER(species) = LOWER(:species)))";

    $args[':genus'] = $identifiers['genus'];
    $args[':species'] = $identifiers['species'];
  }

  $sql = $sql . ' ORDER BY t1.value ASC';

  $query = chado_query($sql, $args);
  $values = ($query->rowCount()) ? $query->fetchAllKeyed() : $values;

  return $values;
}

/**
 * FUNCTION:
 * Populate select germplasm type field.
 * Fetch all unique germplasm type base on stocks present in phenotypeprop table. Types are based on
 * type_id of a stock that relates to a cvterm record as a germplasm type value.
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - project_id : project or experiment id number corresponding to project_id in chado.project.
 *   - genus : eg. Lens, string value, a genus.
 *   - species : string value, a species.
 *   - traits : integers, corresponding trait id (cvterm_id) of a trait name (cvterm name) selected.
 *   - year : year values in YYYY.
 *   - location : string value, locations.
 *
 * @return
 *   Associative array, where key is cvterm id and value is name (germplasm type).
 */
function ap_populate_fieldgermplasmtype($identifiers) {
  $values = null;

  $sysvar = ap_get_systemvariables(
    array('variablename' => 'varset'),
    array('set' => 'terms')
  );

  $sysvar_year = variable_get($sysvar['ap_year']);
  $sysvar_location = variable_get($sysvar['ap_location']);

  $sql = "
    SELECT cvterm_id, t0.name FROM {cvterm} AS t0 INNER JOIN {stock} ON cvterm_id = type_id
    WHERE stock_id IN (SELECT stock_id FROM {phenotype}
      INNER JOIN {phenotypeprop} AS t1 USING (phenotype_id)
      INNER JOIN {phenotypeprop} AS t2 USING (phenotype_id)
      WHERE
        project_id IN (:project_id)
        AND attr_id IN (:traits)
        AND t2.value IN (:year)
        AND t1.value IN (:location)
        AND t2.type_id = :year_property
        AND t1.type_id = :location_property";

  $args = array(
    ':project_id' => $parameter['project_id'],
    ':traits' => $parameter['traits'],
    ':year' => $parameter['year'],
    ':location' => $parameter['location'],

    ':year_property' => $year_property,
    ':location_property' => $location_property,
  );

  if (isset($identifiers['project_id']) && $identifiers['project_id'] > 0
    && !empty($identifiers['genus']) && !empty($identifiers['species'])) {

    $sql = $sql . " AND stock_id IN (SELECT stock_id FROM {stock}
      WHERE organism_id IN (SELECT organism_id FROM {organism}
        WHERE LOWER(genus) = LOWER(:genus) AND LOWER(species) = LOWER(:species)))";

    $args[':genus'] = $identifiers['genus'];
    $args[':species'] = $identifiers['species'];
  }

  $sql = $sql . ' ORDER BY t0.name ASC';

  $query = chado_query($sql, $args);
  $values = ($query->rowCount()) ? $query->fetchAllKeyed() : $values;

  return $values;
}

/**
 * FUNCTION:
 * Populate list of germplasm found.
 * Fetch all germplasm in stock_id field of phenotype table.
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - project_id : project or experiment id number corresponding to project_id in chado.project.
 *   - genus : eg. Lens, string value, a genus.
 *   - species : string value, a species.
 *   - traits : integers, corresponding trait id (cvterm_id) of a trait name (cvterm name) selected.
 *   - year : year values in YYYY.
 *   - location : string value, locations.
 *
 * @return
 *   Associative array, where key is stock id and value is concatenated germplasm name and accession.
 */
function ap_populate_fieldgermplasmlist($identifiers) {
  $values = null;

  if (!isset($identifiers['genus']) && empty($identifiers['genus'])) {
    // There is just project id then update related germplasm with only project id
    // as the filter.
    $sql = "
      SELECT t1.stock_id, CONCAT(t1.name, ' (', t1.uniquename, ')') AS germplasm
      FROM {stock} AS t1 INNER JOIN {phenotype} USING(stock_id)
      WHERE project_id IN (:project_id)
      GROUP BY t1.stock_id, t1.name, t1.uniquename ORDER BY t1.name ASC";

    $args = array(':project_id' => $identifiers['project_id']);
  }
  else {
    // Multiple combinations of filters.
    // Convert field values into a condition that will form part of the
    // where clause of the query.
    $where_clause = array();
    $args = array();

    $sysvar = ap_get_variablenames(
      array('variablename' => 'varset'),
      array('set' => 'terms')
    );

    if (isset($identifiers['project_id']) && $identifiers['project_id'] > 0) {
      // Experiment.
      $where_clause[] = 'project_id IN (:project_id)';
      $args[':project'] = $identifiers['project_id'];
    }

    if (isset($identifiers['traits']) && !empty($identifiers['traits'])) {
      // Traits.
      $where_clause[] = 'attr_id IN (:traits)';
      $args[':traits'] = $identifiers['traits'];
    }

    if (isset($identifiers['year']) && !empty($identifiers['year'])) {
      // Year.
      $where_clause[] = 't1.value IN (:year)';
      $where_clause[] = 't1.type_id = :year_property';

      $args[':year'] = $identifiers['year'];
      $sysvar_year = variable_get($sysvar['ap_year']);
      $args[':year_property'] = $sysvar_year;
    }

    if (isset($identifiers['location']) && !empty($identifiers['location'])) {
      // Location.
      $where_clause[] = 't2.value IN (:location)';
      $where_clause[] = 't2.type_id = :location_property';

      $args[':location'] = $identifiers['location'];
      $sysvar_location = variable_get($sysvar['ap_location']);
      $args[':location_property'] = $sysvar_location;
    }

    $sql = "
      SELECT stock_id, CONCAT(name, ' (', uniquename, ')') AS germplasm
      FROM {stock} WHERE stock_id IN (SELECT stock_id FROM {phenotype}
        INNER JOIN {phenotypeprop} AS t1 USING (phenotype_id)
        INNER JOIN {phenotypeprop} AS t2 USING (phenotype_id)
        WHERE %s)";

    $sql = sprintf($sql, implode(' AND ', $where_clause));

    // Account for genus and species.
    if (!empty($identifiers['genus']) && !empty($identifiers['species'])) {
      $sql = $sql . " AND organism_id IN (SELECT organism_id FROM {organism} WHERE genus = :genus AND species = :species)";

      $where_args[':genus'] = $identifiers['genus'];
      $args[':species'] = $identifiers['species'];
    }

    // Account for germplasm type.
    if (isset($identifiers['germplasmtype']) && !empty($identifiers['germplasmtype'])) {
      $sql = $sql . " AND type_id IN (:germplasmtype)";

      $args[':germplasmtype'] = $identifiers['germplasmtype'];
    }
  
    $sql = $sql . " ORDER BY germplasm ASC";
  }

  $query = chado_query($sql, $args);
  $values = ($query->rowCount()) ? $query->fetchAllKeyed() : $values;

  return $values;
}