<?php
/**
 * @file
 * Master template file of analyzed phenotypes module.
 *
 * Available variables:
 */

$form_id = $form['#form_id'];
?>


<?php
/**
 * Analyzed Phenotypes administrative pages directory.
 */

if ($form_id == 'analyzedphenotypes_admin_page_directory') :
  $links = '';
  foreach($directory as $key => $options) {
    $url = l(ucwords($key), 'admin/tripal/extension/analyzedphenotypes/' . $options['page_id']);
    $links .= '<li>' . $url . '<br />' . $options['info'] . '</li>';
  }
?>

  <ul>
    <?php print $links; ?>
  </ul>


<?php
/**
 * Analyzed Phenotypes Data Loader.
 */

elseif($form_id == 'analyzedphenotypes_admin_data_loader') :
?>

  <div class="messages warning ap-messages">
    Phenotypic data should be <strong>filtered for outliers and mis-entries</strong> before being uploaded here.
    Do not upload data that should not be used in the final analysis for a scientific article.
    Furthermore, data should <strong>NOT be averaged across replicates or site-year.</strong>
  </div>

  <?php print drupal_render_children($form); ?>

<?php
endif;
?>

