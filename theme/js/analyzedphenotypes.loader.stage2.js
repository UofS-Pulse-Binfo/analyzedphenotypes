/**
 * @file
 * Contains scripts to manage behaviours used in Drag and Drop.
 */

(function($) {
  Drupal.behaviors.stage02 = {
    attach: function (context, settings) {
      // Disable elements while validating.

      // Handle Next Step button.
      var submitButton = $('#ap-next-stage-submit-field');
      submitButton.addClass('form-button-disabled');
      submitButton.attr('disabled','disabled');

      // Handle file reupload.
      $('a.ap-option-link-file-reupload').hide();

      setTimeout(APValidatorUpdateStatus, 1000);

       function APValidatorUpdateStatus() {
         var fileStatus = $('.ap-tsv-file-form-element span');
         var pane = $('#ap-progress-container');
         var job_id = Drupal.settings.analyzedphenotypes.vars.job_id;

         $.ajax({
           type: 'GET',
           url: Drupal.settings.analyzedphenotypes.vars.path_JSON + job_id,
           data: '',
           dataType: 'json',
           success: function (progress) {

             // If the job is complete then we want to put our validation
             // result in place of the progress spinner.
             if (progress.percentage == 100) {
               if (progress.message == 'Completed') {
                 APShowValidationResult(pane, fileStatus, job_id);
                 console.log('Validation Complete');
               }
               else {
                 console.log(progress);
                 console.log('ERROR: the job did not complete.');
                 fileStatus.replaceWith('An error was encountered when trying to validate the file...');
               }
             }
             // If the job is not complete then we want to check again in 1s.
             else {
               setTimeout(APValidatorUpdateStatus, 1000);
             }
           },
           error: function (xmlhttp) {
             console.log(xmlhttp);
             console.log('ERROR: the job did not complete.');
           }
         });
       };

       /**
        * Replaces the progress spinner with the result summary from validation.
        */
      function APShowValidationResult(pane, fileStatus, job_id) {

        $.ajax({
          type: 'GET',
          url: Drupal.settings.analyzedphenotypes.vars.path_VR + job_id,
          data: '',
          dataType: 'html',
          success: function (validation_result) {
            pane.replaceWith(validation_result);
            fileStatus.replaceWith('Validation complete...');

            // If there were no errors then re-enable the next button.
            if (validation_result.indexOf('file could not be uploaded') == -1) {
              if ($('.ap-failed').length) {
                // Has class for items that failed - the x mark icon.
                // Keep next disabled but offer option to reupload file.
                $('a.ap-option-link-file-reupload').show();
              }
              else {
                // All passed validation - enable next.
                submitButton.removeClass('form-button-disabled');
                submitButton.removeAttr('disabled');
              }
            }
          },
          error: function (xmlhttp) {
            console.log(xmlhttp);
            console.log('ERROR: the job did not complete.');
            fileStatus.replaceWith('An error was encountered when trying to validate the file...');
          },
        });
      }
      //
} }; }(jQuery));
