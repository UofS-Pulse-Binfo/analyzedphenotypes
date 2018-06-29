<?php
/**
 * @file
 *
 * API: MATERIALIZED VIEWS.
 * Manage property and operation pertaining to materizalized view.
 */


/**
 * @TODO: Replace this function with the function below.
 */
function analyzedphenotypes_mview($property, $parameter = null) {
  $mviewquery = ap_construct_mviewquery();
  $mviewschema = ap_construct_mviewschema();

  return array(
    'query' => $mviewquery,
    'schema' => $mviewschema,
  );
}

/**
 * @TODO: Replace this function with the function below.
 */
function analyzedphenotypes_updatemview() {
  ap_update_mview();
}


///////


/**
 * Prepare SQL query required by materialized view.
 * Dependencies: ap_get_variablenames() in systemvars.api.
 *
 * @return
 *   Text block, sql query statement.
 */
function ap_construct_mviewquery() {
  $sysvar = ap_get_variablenames(
    array('variablename' => 'varset'),
    array('set' => 'terms')
  );

  // Values will be set during install and on each time these terms
  // get reconfigured with a new value.
  // @see ontology.api and system configuration page.
  $sysvar_location = variable_get($sysvar['ap_location']);
  $sysvar_year = variable_get($sysvar['ap_year']);

  $mviewquery = sprintf("
    SELECT
      o.genus         AS organism_genus,
      trait.cvterm_id AS trait_id,
      trait.name      AS trait_name,
      proj.project_id AS project_id,
      proj.name       AS project_name,
      loc.value       AS location,
      yr.value        AS year,
      s.stock_id      AS germplasm_id,
      s.name          AS germplasm_name,
      avg( CAST(p.value as FLOAT) ) AS mean
    FROM {phenotype} p
      LEFT JOIN {cvterm} trait ON trait.cvterm_id=p.attr_id
      LEFT JOIN {project} proj USING(project_id)
      LEFT JOIN {stock} s USING(stock_id)
      LEFT JOIN {organism} o ON o.organism_id=s.organism_id
      LEFT JOIN {phenotypeprop} loc ON loc.phenotype_id=p.phenotype_id AND loc.type_id = %d
      LEFT JOIN {phenotypeprop} yr ON yr.phenotype_id=p.phenotype_id AND yr.type_id = %d
    GROUP BY
      trait.cvterm_id,
      trait.name,
      proj.project_id,
      proj.name,
      loc.value,
      yr.value,
      s.stock_id,
      s.name,
      o.genus
    ", $term_id_location, $sysvar_year
  );

  return $mviewquery;
}

/**
 * Prepare schema required by materialized view.
 *
 * @return
 *   An array, field definitions.
 */
function ap_construct_mviewschema() {
  $mviewschema = array(
    'description' => 'Caches phenotypic data for easier retrieval of means. Data replicates are combined.',
    'table' => 'mview_phenotype',
    'fields' => array (
      'organism_genus'   => array (
        'type' => 'varchar',
        'length' => '255',
        'not null' => TRUE,
      ),
      'trait_id'         => array (
        'size' => 'big',
        'type'   => 'int',
        'not null' => TRUE,
      ),
      'trait_name'       => array (
        'type' => 'varchar',
        'length' => '255',
        'not null' => TRUE,
      ),
      'experiment_id'    => array (
        'size' => 'big',
        'type'   => 'int',
        'not null' => TRUE,
      ),
      'experiment_name'  => array (
        'type' => 'text',
        'not null' => TRUE,
      ),
      'location'         => array (
        'type' => 'text',
        'not null' => TRUE,
      ),
      'year'             => array (
        'type' => 'text',
        'not null' => TRUE,
      ),
      'stock_id'         => array (
        'size' => 'big',
        'type'   => 'int',
        'not null' => TRUE,
      ),
      'stock_name'       => array (
        'type' => 'varchar',
        'length' => '255',
        'not null' => TRUE,
      ),
      'mean'             => array (
        'type' => 'float',
      ),
    ),

    'indexes' => array(),
  );

  return $mviewschema;
}

/**
 * Update/populate materialized view.
 * Dependencies: ap_construct_mviewquery().
 */
function ap_update_mview() {
  $mview_id = tripal_get_mview_id('mview_phenotype');

  if ($mview_id) {
    $mviewquery = ap_construct_mviewquery();

    // This works but it deletes the schema and brakes the view.
    // tripal_edit_mview($mview_id, $mview_phenotype, 'analyzedphenotypes', '', '', 0, $update_mview['query'], '');
    // Updateing via chado_query().

    chado_query("UPDATE tripal_mviews SET query = :mview_query WHERE mview_id = :mview_id",
      array(
        ':mview_query' => $mviewquery,
        ':mview_id' => $mview_id,
      )
    );
  }
}
