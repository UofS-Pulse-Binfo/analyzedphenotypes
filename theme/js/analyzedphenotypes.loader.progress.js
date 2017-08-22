/**
 * @file
 * Contains scripts to manage behaviours used in Tripal Download Progress Bar.
 */

(function($) {
  Drupal.behaviors.apProgress = {
    attach: function (context, settings) {
      //
      var tmp = Drupal.settings.analyzedphenotypes.vars;

      if (tmp) {
        var pbVar = [];
        pbVar.jobId    = tmp['job_id'];
        pbVar.pathJSON = tmp['path_JSON'];
        pbVar.pathVR   = tmp['path_VR'];
        pbVar.stage    = tmp['stage'];

        var vrContainerId = '#ap-validation-result-embed';

        var DrupalProgressBar = new Drupal.progressBar('ap-tripal-download-progress-bar', progressBarResult);
        $('.progress-pane').once().append(DrupalProgressBar.element);

        DrupalProgressBar.setProgress(0, '...');
        DrupalProgressBar.startMonitoring(pbVar.pathJSON + pbVar.jobId, 500);
      }

      /**
       * Function:
       */
      function progressBarResult(percentage, message) {
        if ((percentage == '100' || message == 'Error') && pbVar.jobId > 0) {
          if (pbVar.stage == 'validate') {
            $(vrContainerId).once().load(pbVar.pathVR + pbVar.jobId, progressBarValidationResult);
          }

          DrupalProgressBar.stopMonitoring();
        }
      }

      /**
       *
       */
      function progressBarValidationResult() {
        if ($('#ap-validator-failed').length > 0) {
          // With error.
          $('.ap-tsv-file-form-element').hide();
          $('#ap-dnd-container').show();
        }

        if ($('#ap-validator-passed').length > 0) {
          $('#ap-main-form-elements-container').hide();
        }
      }
      //
} }; }(jQuery));
