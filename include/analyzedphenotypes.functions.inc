<?php
/**
 * @file
 * Contains helper functions required by this module.
 */

/**
 * Helper function: fetch relevant records based on data type request.
 */
function analyzedphenotypes_fetch_data($data_type, $key = null) {
  switch($data_type) {
    case 'project':
      $sql = "
        SELECT project_id, name
        FROM {project}
        WHERE LOWER(name) LIKE :query_string
        ORDER BY name ASC
        LIMIT 10
      ";

      $args = array(':query_string' =>  strtolower($key) . '%');
      $result = chado_query($sql, $args);

      if ($result->rowCount() > 0) {
        return $result->fetchAllKeyed();
      }

      break;

  case 'genus':
    $sql = "SELECT genus, UPPER(genus) FROM {organism} GROUP BY genus ORDER BY genus";
    $result = chado_query($sql);

    if ($result->rowCount() > 0) {
      return $result->fetchAllKeyed();
    }

    break;
  }
}
