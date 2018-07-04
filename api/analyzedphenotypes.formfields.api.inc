<?php
/**
 * @file
 *
 * API: SYSTEM VARIABLES.
 * Manage property and operation pertaining to system variables.
 */


/**
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_fieldnames() {
  return ap_construct_downloadablefieldnames();
}

/**
 * FUNCTION:
 * Global field names.
 * Create/Register field names for every form element in the download form and use
 * in various hooks involved in processing download request.
 *
 * @param
 *
 * @return
 *   An array of string, field names grouped by base name, cache name, field id and hidden field.
 */
function ap_construct_downloadable_fieldnames() {
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
    'base',        // Base name attribute.
    'cache',       // Name used for variable storing caching field value.
    'field',       // Name attached to Name attribute of field.
    'field-id',    // Name attached to Id attribute of field.
    'field_hidden',// Name attached to Name attribute of a hidden field.
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
 * CONTROL/SWITCHBOARD.
 * Pass a request to a corresponding function.
 * @TODO: leave out when api reorganize/update is complete and use functions below.
 */
function analyzedphenotypes_systemvariables2($property = null, $parameter = null) {
  $systemvars = null;

  if (is_null($property)) {
    // Default.
    $property = 'default';
  }

  // Process request.
  // Default to all system variables.
  $command = array(
    'default' => array(
      'func'  => 'ap_construct_variablenames',
      'param' => null,
      'opts'  => null,
    ),
    'options' => array(
      'func'  => 'ap_get_variablenames',
      'param' => array('variablename' => 'varset'),
      'opts'  => array('set' => 'options'),
    ),
    'cvdbon' => array(
      'func'  => 'ap_get_variablenames',
      'param' => array('variablename' => 'varset'),
      'opts'  => array('set' => 'cvdbon'),
    ),
    'terms' => array(
      'func'  => 'ap_get_variablenames',
      'param' => array('variablename' => 'varset'),
      'opts'  => array('set' => 'terms'),
    ),
    'genus_vars' => array(
      'func'  => 'ap_get_variablenames',
      'param' => array('variablename' => $parameter['genus']),
      'opts'  => array('set' => 'cvdbon', 'suffix' => 'allsuffix'),
    ),
  );

  // Function name, parameter (project name key or name), additional options.
  return call_user_func($command[$property]['func'], $command[$property]['param'], $command[$property]['opts']);
}


///////

/**
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_cachevalue($value) {
  return ap_cache_downloadable_fieldvalue($value);
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
function ap_cache_downloadable_fieldvalue($value) {
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
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_autofield_callback($command, $element, $default_value = null) {
  return ap_insert_autocomplete_formfield($command, $element, $default_value);
}

/**
 * FUNCTION CALLBACK:
 * AJAX add and remove form field.
 * Dependencies: analyzedphenotypes_autofieldprop().
 *
 * $param $command
 *   String, add (add) or remove (rem) to add and remove autocomplete from field element.
 * $param $element
 *   String, id attribute of element where to insert field in case of command add and
 *   id attribute of field to delete in case of command rem.
 * @param $default_value
 *   String, default value of autocomplete field. base64 encoding used.
 *
 * @return
 *   Rendered autocomplete form element.
 *
 * @see
 *   hook_menu().
 *   autocomplete form field API definition.
 */
function ap_insert_autocomplete_formfield($command, $element, $default_value = null) {
  // Add or remove autofield command.
  $cmd = trim($command);
  $elm = trim($element);

  if (($cmd == 'add' || $cmd == 'rem') && !empty($elm)) {
    // Add or remove autofield command.
    $element_id = '#' . $elm;

    if ($cmd == 'add') {
      // Create a from field api and append to element wrapper.
      $default_value = base64_decode($default_value);
      $field_api = analyzedphenotypes_autofieldprop($default_value);
      $field_htm = drupal_render($field_api);

      $AJAX_cmd = ajax_command_prepend($element_id, $field_htm);
    }
    else {
      // Given a reference id to an element, remove if from DOM.
      $AJAX_cmd = ajax_command_remove($element_id);
    }

    return array(
      '#type' => 'ajax',
      '#commands' => array($AJAX_cmd),
    );
  }
  else {
    // Not valid command. No element. Do nothing.
    return FALSE;
  }
}

/**
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_autofieldprop($default_value) {
  return ap_construct_autocomplete_formfield($default_value);
}

/**
 * FUNCTION:
 * Define a autocomplete form field. Button set add and remove rendered ajacent to each element.
 * Dependencies:
 *
 * @param $default_value
 *   Default to: Type Germplasm/Stock Name.
 *   Initial value of autocomplete form field (#value key of the form field API).
 *
 * @return array
 *   Array, form autocomplete form field API array.
 */
function ap_construct_autocomplete_formfield($default_value) {
  // Menu item that will handle AJAX request.
  $autofield_menu = 'download/autofield/';
  // Id each field added. This id will be use to reference an element.
  $autofield_id = uniqid();
  // Add and remove command - argument to menu item.
  $autofield_rem  = 'rem';
  $autofield_add  = 'add';
  // Make an AJAX link.
  $autofield_attr = array(
    'attributes' => array(
      'class' => array('use-ajax', 'ap-autofield-germplasm-control'),
      'title' => 'Click to Add (+) or Remove (x) Germplasm',
    )
  );

  $emptyfield_default_value = 'Type Germplasm/Stock Name';

  // l(text, url + element, attributes);
  // Use id #ap-autofield in the main form where field will be inserted.
  // Remove does not require a default value - &.
  $autofield_btns = l('Rem', $autofield_menu . $autofield_rem . '/ap-autofield-wrapper-' . $autofield_id . '/&', $autofield_attr)
                  . l('Add', $autofield_menu . $autofield_add . '/ap-autofield/' . base64_encode($emptyfield_default_value), $autofield_attr);

  // Define field API.
  $default_value = ($default_value) ? $default_value : $emptyfield_default_value;
  $autofield_fld['ap_autofield_' . $autofield_id] = array(
    '#type' => 'textfield',
    '#value' => $default_value,
    '#theme_wrappers' => array(),
    // Wrap this field.
    '#prefix' => '<div id="ap-autofield-wrapper-' . $autofield_id . '" class="ap-autofield-wrapper ap-autofield-AJAX-off">',
    // Buttons.
    '#suffix' => '<span>' . $autofield_btns . '</span></div>',
    //
    '#attributes' => array('class' => array('ap-autofield-field')),
    '#id' => 'ap-autofield-' . $autofield_id,
  );


  return $autofield_fld;
}

/**
 * TODO: Replace this function with the function below.
 */
function analyzedphenotypes_populatefield($field, $parameter = null) {
  return ap_populate_downloadable_formfield($field, $parameter);
}

/**
 * FUNCTION:
 * Populate form fields.
 * AJAX load values to each form field in the form.
 * Dependencies: analyzedphenotypes_fieldnames(), analyzedphenotypes_systemvariables().
 *
 * @param $field
 *   String, the target field element.
 *   - file : Fill/create a file with datapoints based on filters. File provided by Tripal Job.
 *   - field_project : Fill select project/experiment field (select field).
 *   - field_project_summary : Fill project/experiment summary count information/tip window (on mouse over to activate).
 *   - field_genus : Fill select genus field (select field).
 *   - field_species : Fill select species field (select field).
 *   - field_traits : Fill select traits field (checkbox group).
 *   - field_year : Fill select year field (checkbox group).
 *   - field_location : Fill select locaiton field (checkbox group).
 *   - field_germplasmtype : Fill select germplasm type field (checkbox group).
 *   - field_germpasm : Fill germplasm field/list (collapsible window).
 * @param $parameter
 *   An array containing form field values preceeding selected field.
 *   - experiment : experiment field.
 *   - genus : genuse field.
 *   - species : species field.
 *   - traits : traits field.
 *   - year : year field.
 *   - location : location field.
 *   - germplasmtype : germplasmtype field.
 *
 * @return
 *   An array of values based on field request.
 *
 * @see analyzedphenotypes_data_download in admin.form.inc.
 */
function ap_populate_downloadable_formfield($field, $parameter = null) {
  // Use this character to separate entries in a string.
  $delimiter = '~';
  $field_values = null;

  switch($field) {
    //
    case 'field_project':
      // Fill field project.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter : null.

      $options = chado_query(
        "SELECT project_id, t1.name
         FROM {project} AS t1 INNER JOIN {phenotype} USING(project_id)
         GROUP BY project_id ORDER BY t1.name ASC"
      );

      if (count($options) > 0) {
        $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed() : $field_values;
      }

      break;

    //
    case 'field_project_summary':
      // Fill field project summary count information.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter : null.

      $options = chado_query(
        "SELECT project_id, CONCAT('*' || COUNT(DISTINCT attr_id) || ' Traits, ' || COUNT(phenotype_id) || ' Data Points ')
         FROM {phenotype}
         GROUP BY project_id
         ORDER BY project_id ASC"
      );

      if ($options) {
        $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed() : $field_values;
      }

      break;

    //
    case 'field_genus':
      // Fill field genus.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.

      $options = analyzedphenotypes_genusprop('project_genus', array(
        'project_id' => $parameter['project_id'],
      ));

      if ($options) {
        $field_values = array($options => $options);
      }

      break;

    //
    case 'field_species':
      // Fill field species.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.
      //   $parameter['genus'] : genus selected.

      $options = chado_query(
        "SELECT species
         FROM {organism}
         WHERE genus = :genus
           AND organism_id
             IN (SELECT organism_id FROM {stock} INNER JOIN {phenotype} USING (stock_id) WHERE project_id = :project_id)
         GROUP BY species ORDER BY species ASC",
        array(
          ':genus' => $parameter['genus'],
          ':project_id' => $parameter['project_id'],
        )
      );

      $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed(0, 0) : $field_values;

      break;

    //
    case 'field_traits':
      // Fill field traits.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.
      //   $parameter['genus'] : genus selected.
      //   $parameter['species'] : species selected.

      if (count($parameter['project_id']) > 1) {
        $options = chado_query(
          "SELECT t1.cvterm_id, t1.name
           FROM {cvterm} AS t1 INNER JOIN {phenotype} ON t1.cvterm_id = attr_id
           WHERE project_id IN (:project_id)
           GROUP BY t1.cvterm_id, t1.name
           ORDER BY t1.name ASC",
          array(
            ':project_id' => $parameter['project_id'],
          )
        );

        $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed() : $field_values;
      }
      else {
        $options = chado_query(
          "SELECT cvterm_id, name FROM {cvterm} WHERE cvterm_id IN (SELECT attr_id FROM {phenotype}
           WHERE project_id = :project_id
             AND stock_id IN
               (SELECT stock_id FROM {stock} WHERE organism_id IN (SELECT organism_id from {organism}
                WHERE LOWER(genus) = LOWER(:genus) AND LOWER(species) = LOWER(:species))))",
          array(
            ':project_id' => $parameter['project_id'],
            ':genus' => $parameter['genus'],
            ':species' => $parameter['species'],
          )
        );

        $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed() : $field_values;
      }

      break;

    //
    case 'field_year':
      // Fill field year.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.
      //   $parameter['genus'] : genus selected.
      //   $parameter['species'] : species selected.
      //   $parameter['traits'] : traits selected.

      $sysvars = analyzedphenotypes_systemvariables('terms');

      $year_property = variable_get($sysvars['ap_year']);

      $options = chado_query(
        "SELECT value FROM {phenotypeprop} WHERE type_id = :year_property
         AND phenotype_id IN (SELECT phenotype_id FROM {phenotype}
           WHERE
             project_id IN (:project_id)
             AND attr_id IN (:traits)
             AND stock_id IN (SELECT stock_id FROM {stock} WHERE organism_id IN
               (SELECT organism_id from {organism} WHERE LOWER(genus) = LOWER(:genus) AND LOWER(species) = LOWER(:species)))
             )
         GROUP BY value ORDER BY value DESC",
        array(
          ':year_property' => $year_property,
          ':project_id' => $parameter['project_id'],
          ':traits' => $parameter['traits'],
          ':genus' => $parameter['genus'],
          ':species' => $parameter['species'],
        )
      );

      $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed(0, 0) : $field_values;

      break;

    //
    case 'field_location':
      // Fill field location.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.
      //   $parameter['genus'] : genus selected.
      //   $parameter['species'] : species selected.
      //   $parameter['traits'] : traits selected.
      //   $parameter['year'] : years selected.

      $sysvars = analyzedphenotypes_systemvariables('terms');

      $year_property = variable_get($sysvars['ap_year']);
      $location_property = variable_get($sysvars['ap_location']);

      if (isset($parameter['genus']) && isset($parameter['species'])) {
        $condition_species = 'AND stock_id IN (SELECT stock_id FROM {stock} WHERE organism_id IN (
          SELECT organism_id FROM {organism} WHERE LOWER(genus) = LOWER(:genus) AND LOWER(species) = LOWER(:species)
        ))';

        $args_species = array(
          ':genus' => $parameter['genus'],
          ':species' => $parameter['species'],
        );
      }
      else {
        $condition_species = '';
        $args_species = array();
      }


      $sql = sprintf(
        "SELECT t1.value
         FROM
           {phenotype}
           INNER JOIN {phenotypeprop} AS t1 USING (phenotype_id)
           INNER JOIN {phenotypeprop} AS t2 USING (phenotype_id)
         WHERE
           project_id IN (:project_id)
           AND attr_id IN (:traits)
           AND t1.type_id = :location_property
           AND t2.type_id = :year_property
           AND t2.value IN (:year)
           %s ORDER BY t1.value ASC", $condition_species);

      $options = chado_query($sql,
        array(
          ':project_id' => $parameter['project_id'],
          ':traits'  => $parameter['traits'],
          ':year' => $parameter['year'],
          ':location_property' => $location_property,
          ':year_property' => $year_property,
        )
        + $args_species
      );

      $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed(0, 0) : $field_values;

      break;

    //
    case 'field_germplasmtype':
      // Fill field germplasm type.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.
      //   $parameter['genus'] : genus selected.
      //   $parameter['species'] : species selected.
      //   $parameter['traits'] : traits selected.
      //   $parameter['year'] : years selected.
      //   $parameter['location'] : locations selected.

      $sysvars = analyzedphenotypes_systemvariables('terms');

      $year_property = variable_get($sysvars['ap_year']);
      $location_property = variable_get($sysvars['ap_location']);

      if (isset($parameter['genus']) && isset($parameter['species'])) {
        $condition_species = 'AND stock_id IN (SELECT stock_id FROM {stock} WHERE organism_id IN (
          SELECT organism_id FROM {organism} WHERE LOWER(genus) = LOWER(:genus) AND LOWER(species) = LOWER(:species)
          ))';

        $args_species = array(
          ':genus' => $parameter['genus'],
          ':species' => $parameter['species'],
        );
      }
      else {
        $condition_species = '';
        $args_species = array();
      }

      $sql = sprintf(
        "SELECT cvterm_id, t0.name FROM {cvterm} AS t0 INNER JOIN {stock} ON cvterm_id = type_id
         WHERE stock_id IN (
           SELECT stock_id
             FROM
               {phenotype}
               INNER JOIN {phenotypeprop} AS t1 USING (phenotype_id)
               INNER JOIN {phenotypeprop} AS t2 USING (phenotype_id)
             WHERE
               project_id IN (:project_id)
               AND attr_id IN (:traits)
               AND t1.type_id = :location_property
               AND t1.value IN (:location)
               AND t2.type_id = :year_property
               AND t2.value IN (:year)
               %s)", $condition_species);

      $options = chado_query($sql,
        array(
          ':project_id' => $parameter['project_id'],
          ':traits' => $parameter['traits'],
          ':year_property' => $year_property,
          ':year' => $parameter['year'],
          ':location_property' => $location_property,
          ':location' => $parameter['location'],
        )
        + $args_species
      );

      $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed() : $field_values;

      break;

    //
    case 'field_germplasm':
      // List germplasm found base on the filters selected.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.
      //   $parameter['genus'] : genus selected.
      //   $parameter['species'] : species selected.
      //   $parameter['traits'] : traits selected.
      //   $parameter['year'] : years selected.
      //   $parameter['location'] : locations selected.

      if (isset($parameter['genus'])) {
        $sysvars = analyzedphenotypes_systemvariables('terms');

        $var_value = array(
          ':year' => variable_get($sysvars['ap_year']),
          ':location' => variable_get($sysvars['ap_location']),
        );

        $fields = array(
          'plantprop' => array(
            ':project'  => array('operator' => 'IN', 'field' => 'project_id', 'value' => $parameter['project_id'], 'type' => ''),
            ':traits'   => array('operator' => 'IN', 'field' => 'attr_id',    'value' => $parameter['traits'],     'type' => ''),
            ':year'     => array('operator' => 'IN', 'field' => 't1.value',   'value' => $parameter['year'],       'type' => 't1.type_id'),
            ':location' => array('operator' => 'IN', 'field' => 't2.value',   'value' => $parameter['location'],   'type' => 't2.type_id'),
          ),
          'organism' => array(
            ':genus'    => array('operator' => '=',  'field' => 'genus',      'value' => $parameter['genus'],       'type' => ''),
            ':species'  => array('operator' => '=',  'field' => 'species',    'value' => $parameter['species'],     'type' => ''),
          ),
        );

        $condition = array();
        $where_clause = array();
        $args = array();

        foreach($fields as $field => $prop) {
          foreach($prop as $col => $value) {
            if (isset($value['value']) && !empty($value['value'])) {
              if ($value['type']) {
                $p = $col . '_property';
                $condition[$field][] = $value['type'] . ' = ' . $p;
                $args[$p] = $var_value[$col];
              }

              $v = ($value['operator'] == 'IN') ? ' (' . $col . ')' : $col;
              $condition[$field][] = $value['field'] . ' ' . $value['operator']  .  $v;

              $args[$col] = $value['value'];
            }
          }

          $where_clause[$field] = implode(' AND ', $condition[$field]);
        }

        $sql1 = "SELECT stock_id, CONCAT(name, ' (', uniquename, ')') AS germplasm FROM {stock} WHERE ";

        $sql2 = sprintf("
          stock_id IN (
            SELECT stock_id FROM {phenotype}
              INNER JOIN {phenotypeprop} AS t1 USING (phenotype_id)
              INNER JOIN {phenotypeprop} AS t2 USING (phenotype_id)
            WHERE %s
        ) ", $where_clause['plantprop']);

        $sql3 = sprintf(" AND organism_id IN (SELECT organism_id FROM {organism} WHERE %s) ", $where_clause['organism']);
        $sql4 = " AND type_id IN (:germplasmtype) ";
        $sql5 = " GROUP BY stock_id, name, uniquename ORDER BY name ASC";

        $sql = $sql1 . $sql2;

        if (!empty($where_clause['organism'])) {
          $sql .= $sql3;
        }

        if (!empty($parameter['germplasmtype'])) {
          $sql .= $sql4;
          $args[':germplasmtype'] = $parameter['germplasmtype'];
        }

        $sql .= $sql5;
        $options = chado_query($sql, $args);
      }
      else {
        $options = chado_query(
          "SELECT t1.stock_id, CONCAT(t1.name, ' (', t1.uniquename, ')') AS germplasm
          FROM {stock} AS t1 INNER JOIN {phenotype} USING(stock_id) WHERE project_id IN (:project_id)
          GROUP BY t1.stock_id, t1.name, t1.uniquename ORDER BY t1.name ASC",
          array(
            ':project_id' => $parameter['project_id'],
          )
        );
      }

      $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed() : $field_values;

      break;
  }


  return $field_values;
}

