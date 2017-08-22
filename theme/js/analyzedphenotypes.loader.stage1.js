/**
 * @file
 * Contains scripts to manage STAGE 01 - Upload File.
 */

(function($) {
  Drupal.behaviors.stage01 = {
    attach: function (context, settings) {
      // Add event listener to project autocomplete search field.
      $('#ap-project-select-field').click(function() {
        $(this).select();
        $('#ap-validation-result-embed').children().remove();
      });

      // Handle Next Step button.
      var submitButton = $('#ap-next-stage-submit-field');
      submitButton.addClass('form-button-disabled');
      submitButton.attr('disabled','disabled');

      $(document)
      //
      .ajaxStart(function() {
         // Has project/experiment form field.
        $('#ap-main-form-elements-container')
          .has('#ap-project-select-field')
          .attr('readonly', 'readonly');

        // Has select genus.
        $('#ap-main-form-elements-container')
          .has('#ap-genus-select-field')
          .attr('readonly', 'readonly');
      })
      //
      .ajaxComplete(function() {
        // Mute unnecessary drupal_set_message().
        $('#ap-AJAX-wrapper-autofillgenus').find('.messages').remove();

        // Reset the select field to always default to the first option.
        /*
        $('#ap-genus-select-field')
          .find('option')
          .each(function() {
            $(this).attr('selected', '');
        });*/

        // When DOM has upload success message.
        if ($('#ap-validator-passed').length) {
          // Exclude form fields.
          $('.form-item').hide();

          submitButton.removeClass('form-button-disabled');
          submitButton.removeAttr('disabled');

          // Tell user what's next.
          $('#ap-main-form-fieldset').once(function() {
            $('<span class="text-next-step">&#x25B8; Next Step: Stage 2 - Validate Data</span>')
              .insertAfter('#ap-next-stage-submit-field');
          });
        }
      });
      //
} }; }(jQuery));
