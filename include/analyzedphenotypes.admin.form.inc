<?php
/**
 * @file
 * Contains administrative interfaces of this module.
 */

/**
 * Function callback: Construct main administrative page directory of this module.
 */
function analyzedphenotypes_admin_page_directory($form, &$form_state) {
  // Create a directory of links in the module.
  // Array to hold administrative page directory in this module.
  // Key - page id and page info.
  $arr_dir = array(
    'data loader' => array('page_id' => 'data_loader', 'info' => 'Lorem ipsum dolor sit amet'),
    'page xyz' => array('page_id' => 'abc_xyz', 'info' => 'Another page - Lorem ipsum dolor sit amet'),
    'yet another page' => array('page_id' => 'cde_fgh', 'info' => 'Yet Another page - Lorem ipsum dolor sit amet'),
  );

  // Construct markup for each link.
  $links = '<ul>';
  foreach($arr_dir as $key => $page_prop) {
    $url = l(ucwords($key), 'admin/tripal/extension/analyzedphenotypes/' . $page_prop['page_id']);

    $links .= '
      <li>
        <p>' . $url . '<br />' . $page_prop['info'] . '</p>
      </li>';
  }
  $links .= '</ul>';

  $form['admin_directory'] = array(
    '#markup' => '
      <h2>Analyzed Phenotypes Module</h2>' . $links
  );

  return $form;
}


/**
 * Function callback: Construct analyzed phenotypes data loader.
 */
function analyzedphenotypes_admin_data_loader($form, &$form_state) {
  $form['admin_data_loader'] = array(
    '#markup' => '
      <h2>Loader here</h2>'
  );

  return $form;
}
