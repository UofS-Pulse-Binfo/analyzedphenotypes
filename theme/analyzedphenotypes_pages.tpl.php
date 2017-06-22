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
  $title = drupal_render($form['admin_directory']);
  $tmp = $form['admin_directory']['#item'];
  // Use string separator to access each elements.
  $links = explode('@', $tmp);
?>

  <h2><?php print $title; ?></h2>
  <ul>
    <?php print implode('</li><li>', str_replace('#', '<br />', $links)); ?>
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

