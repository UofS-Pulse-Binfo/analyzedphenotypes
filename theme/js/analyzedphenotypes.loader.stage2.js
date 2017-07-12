/**
 * @file
 * Contains scripts to manage behaviours used in Drag and Drop.
 */

(function($) {
  Drupal.behaviors.stage02 = {
    attach: function (context, settings) {
      //
      var vrContainerId = 'ap-validation-result-embed';
      var vrId = 'ap-content-window-container';



      $(document)
        //
        .ajaxComplete(function() {
          if ($('#ap-validator-passed').length > 0) {
            $('#ap-dnd-container').hide();
            $('.ap-tsv-file-form-element').show();
          }


          if (Drupal.settings.analyzedphenotypes.job_id) {
            var jobId = Drupal.settings.analyzedphenotypes.job_id;
            var jobIdField = $('#ap-job-id-hidden-field');

            jobIdField.once(function() {
              $(this).val(jobId);
            });
          }
        });


      // Tell user what's next.
      $('#ap-main-form-fieldset').once(function() {
        $('<span class="text-next-step">&#x25B8; Next Step: Stage 3 - Describe Traits</span>')
          .insertAfter('#ap-next-stage-submit-field');
      });
      //
} }; }(jQuery));
