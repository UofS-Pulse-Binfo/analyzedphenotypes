<?php
/**
 * @file
 * Master template file of analyzed phenotypes module.
 *
 * Available variables:
 */

$form_id = $form['#form_id'];
$ap_admin = 'analyzedphenotypes_admin_';

// # DATA LOADER/UPLOAD.
if ($form_id == $ap_admin . 'data_loader') {
  if ($system_set == 'not set') {
    // System variables (default cv, db etc.) not set.
    print '<div class="messages warning">System settings not set. Please contact the administrator of this site.</div>';
    print tripal_set_message('Administrators, check to ensure that system settings are set. To review system configuration please click ' . l('Analyzed Phenotypes Settings', 'admin/tripal/extension/analyzedphenotypes/settings') , TRIPAL_INFO, array('return_html' => TRUE));
  }
  else {
  //////////////////////////////
?>
    <div class="messages warning ap-messages">
      Phenotypic data should be <strong>filtered for outliers and mis-entries</strong> before being uploaded here.
      Do not upload data that should not be used in the final analysis for a scientific article.
      Furthermore, data should <strong>NOT be averaged across replicates or site-year.</strong>
    </div>

    <?php
      if ($allow_new == 0) {
        // New traits not allowed.
        print '<div class="messages warning">This module is set to NOT allow new traits to be added into the system. Please contact the administrator of this website before loading your analyzed phenotypes.</div>';
      }
    ?>

    <fieldset id="ap-main-form-fieldset" class="form-wrapper">
      <legend>
        <span class="fieldset-legend">
          <?php print $stage_title; ?>
        </span>
      </legend>

      <div class="fieldset-wrapper">
        <div id="ap-stage-indicators-container">
          <?php
            // Stage indicators.
            $i = 1;

            foreach($stages as $stage => $status) {
              $class = ($status == 'todo') ? 'ap-progress-stage-todo' : '';
              print '<div class="' . $class . ' ap-progress-stage" title="' . $stage . '">
                       <span>&nbsp;</span>' . $i . '. ' . $stage . '<span>&nbsp;</span>
                    </div>';

              $i++;
            }
          ?>
          <div class="ap-clear-both">&nbsp;</div>
        </div>

        <div id="ap-validation-result-embed"></div>

        <div id="ap-main-form-elements-container">
          <?php
            switch($current_stage) {
              //
              case 'validate':
                print drupal_render($form['ap_val_res']);
                ?>

                <div><label>Experiment (Genus)</label><span>AGILE: Application of Genomic Innovation in the Lentil Economy (Lens)</span></div>
                <br />
                <div class="ap-tsv-file-form-element"><label>TSV Data File</label><span>Validating...</span></div>

                <div id="ap-progress-container" class="ap-tsv-file-form-element">
                  <div class="ap-progress-wrapper">
                    <div class="progress-pane"></div>
                  </div>
                </div>

                <div id="ap-dnd-container"><?php print drupal_render($form['ap_dnd_field_reupload']); ?></div>

                <?php
                break;

              //
              case 'describe':
                ?>

                <div><label>Experiment (Genus)</label><span>AGILE: Application of Genomic Innovation in the Lentil Economy (Lens)</span></div>
                <div class="messages warning">
                  Please fully describe the following traits before clicking the next step button.
                </div>
                <div id="ap-describe-main-form-container">
                  <div id="accordion">
                    <?php
                      $main_fieldset = $form['#ap_main_fieldset'];
                      $traits = $form['#ap_describe_trait'];
                      $trait_count = count($traits);

                      $i = 0;

                      foreach($traits as $key => $existing) {
                        if ($existing) {
                          $warn = '<div class="messages warning">We have detected same trait autofilled the form lorem ipsum dolor sit amet.</div>';
                          $form[$main_fieldset . $i]['ap_warning' . $i]['#markup'] = $warn;
                        }

                        print '<h3>#' . ($i + 1) . '. ' . $form['#ap_cvterm_hidden_field' . $i] . '</h3>';
                        print '<div id="ap-accordion-container' . $i . '" class="ap-form-describe-trait">' . drupal_render($form[$main_fieldset . $i]) . '</div>';

                        $i++;
                      }
                    ?>
                  </div>
                </div>

                <?php
                $inform = '<div class="messages warning"><strong>You have fully described <span id="ap-form-filled-count">0</span> of ' . $trait_count . ' Traits</strong>. Please ensure to describe all traits before clicking the Next Step button</div>';

                break;

              //
              case 'save':
                ?>

                <div class="messages status">Lorem ipsum dolor sit amet. File succesfully uploaded to server.</div>
                <div class="messages warning">Lorem ipsum dolor sit amet. Wait for 100% before data becomes available.</div>

                <div id="ap-progress-container">
                  <div class="ap-progress-wrapper">
                    <div class="progress-pane"></div>
                  </div>
                <div>

                <?php
                break;

              //
              // Default to upload.
              default:
              ?>

                <div id="ap-project-select-field-container">
                  <?php print drupal_render($form['ap_project_select_field']); ?>
                </div>
                <div id="ap-genus-select-field-container">
                  <?php print drupal_render($form['ap_AJAX_wrapper_autofillgenus']) . drupal_render($form['ap_genus_select_field']); ?>
                </div>
                <div id="ap-dnd-container">
                  <?php print drupal_render($form['ap_dnd_field']); ?>
                </div>

              <?php
            }
          ?>
        </div>
      </div>
    </fieldset>
<?php

  // In describe, notify user of # of described traits.
  if (isset($inform)) {
    print $inform;
  }

  print drupal_render_children($form);

  //////////////////////////////
  }
}
// # SETTINGS.
elseif ($form_id == $ap_admin . 'settings') {
  // Settings form are rendered by system_settings_form()
  // this theme is not recognized for some reason.

  // Displays a blank screen on /settings.
}
// # AP DIRECTORY (DEFAULT PAGE).
else {
  print '<ul>';

  foreach($directory as $key => $options) {
    $url = l(ucwords($key), $path_extension . '/' . $options['page_id']);
    print '<li>' . $url . '<br />' . $options['info'] . '</li>';
  }

  print '</ul>';
}
?>




































