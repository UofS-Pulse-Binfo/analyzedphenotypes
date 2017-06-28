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

  <fieldset id="ap-main-form-fieldset" class="form-wrapper">
    <legend>
      <span class="fieldset-legend">
        <?php print $stage_title; ?>
      </span>
    </legend>

    <div class="fieldset-wrapper">
      <div id="ap-stage-indicators-container">
        <?php
          $i = 1;
          foreach($stages as $stage => $status) {
            $class = ($status == 'todo') ? 'ap-progress-stage-todo' : '';
            print '<div class="' . $class . ' ap-progress-stage" title="' . $stage . '"><span>&nbsp;</span>' . $i . '. ' . $stage . '<span>&nbsp;</span></div>';

            $i++;
          }
        ?>
        <div class="ap-clear-both">&nbsp;</div>
      </div>

      <div id="ap-validation-result-embed"></div>


      <div id="ap-main-form-elements-container">
        <?php
          switch($current_stage) :
            //
            case 'upload':
        ?>
              <div id="ap-project-select-field-container">
                <?php print drupal_render($form['ap_project_select_field']); ?>
              </div>

              <div id="ap-genus-select-field-container">
                <?php print drupal_render($form['ap_AJAX_wrapper']) . drupal_render($form['ap_genus_select_field']); ?>
              </div>

              <div id="ap-dnd-container">
                <?php print drupal_render($form['ap_dnd_field']); ?>
              </div>

        <?php
              break;

            //
            case 'validate':
        ?>

              <div>
                <label>Experiment (Genus)</label>
                <span>AGILE: Application of Genomic Innovation in the Lentil Economy (Lens)</span>
              </div>

              <br />

              <div>
                <label>TSV Data File</label>
                <span>Validating...</span>
              </div>

              <div id="ap-progress-container">
                <div class="ap-progress-wrapper">
                  <div class="progress-pane"></div>
                </div>
              <div>

              <?php print drupal_render($form['tripal_job_id']); ?>

        <?php
            break;
          endswitch;
        ?>
      </div>
    </div>
  </fieldset>

  <?php print drupal_render_children($form); ?>

<?php
endif;
?>

