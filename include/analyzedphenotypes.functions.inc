<?php
/**
 * @file
 * Contains helper functions required by this module.
 */

/**
 * Helper function: fetch relevant records about a project.
 *
 * @param $project
 *
 */
function analyzedphenotypes_project_properties($property, $key = null) {
  if ($key) {
    $key = strip_tags($key);
  }

  switch($property) {
    //
    case 'project_all':
      $sql = "SELECT project_id, name FROM {project} ORDER BY name ASC";
      $result = chado_query($sql);

      $data = ($result->rowCount() > 0) ? $result->fetchAllKeyed(0, 1) : 0;

      break;

    //
    case 'project_pattern_match_name':
      $sql = "SELECT name FROM {project}
      WHERE LOWER(name) LIKE :project_name ORDER BY name ASC
      LIMIT 10";

      $args = array(':project_name' => '%' . strtolower($key) . '%');
      $result = chado_query($sql, $args);

      $data = ($result->rowCount() > 0) ? $result->fetchAllKeyed(0, 0) : 0;

      break;

    //
    case 'project_match_name':
      $sql = "SELECT project_id FROM {project}
      WHERE LOWER(name) = TRIM(LOWER(:project_name)) ORDER BY name ASC
      LIMIT 10";

      $args = array(':project_name' => $key);
      $result = chado_query($sql, $args);

      $data = ($result->rowCount() > 0) ? $result->fetchField() : 0;

      break;

    //
    case 'project_genus':
      // Get genus cvterm.
      $t = analyzedphenotypes_cv_properties('genus', 'taxonomic_rank');
      $cvid_genus = $t['id'];

      $sql = "SELECT value FROM {projectprop}
      WHERE project_id = :project_id AND type_id = :type_id
      LIMIT 1";

      $args = array(':project_id' => $key, ':type_id' => $cvid_genus);
      $result = chado_query($sql, $args);

      $data = ($result->rowCount() > 0) ? $result->fetchAllKeyed(0, 0) : 0;

      break;
  }

  return $data;
}






function analyzedphenotypes_genus_property() {
  // All genus.
  $sql = "SELECT genus FROM {organism}
  GROUP BY genus ORDER BY genus ASC";

  $result = chado_query($sql);

  return ($result->rowCount() > 0) ? $result->fetchAllKeyed(0, 0) : 0;
}









/**
 * Helper funciton: get cv or cvterm.
 */
function analyzedphenotypes_cv_properties($term, $cv) {
  $term = array(
    'name' => $term,
    'cv_id' => array('name' => $cv)
  );

  $cvterm = tripal_get_cvterm($term);

  if ($cvterm) {
    return array(
      'id' => $cvterm->cvterm_id,
      'name' => $cvterm->name,
    );
  }
  else {
    return 0;
  }
}













/**
 *  Function callback: AJAX function callback.
 */
function analyazedphenotypes_AJAX_callback($form, $form_state) {
  // Main fieldset form element container.
  $main_fieldset = $form['ap_main_fieldset_title']['#value'];

  return $form[$main_fieldset]['ap_AJAX_container'];
}
