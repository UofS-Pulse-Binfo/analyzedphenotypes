/**
 * @file
 * Contains scripts to manage behaviours used in Drag and Drop.
 */

(function($) {
  Drupal.behaviors.stage02 = {
    attach: function (context, settings) {
      // Handle Next Step button.
      var submitButton = $('#ap-next-stage-submit-field');
      submitButton.addClass('form-button-disabled');
      submitButton.attr('disabled','disabled');

      var redirect = (Drupal.settings.analyzedphenotypes.redirect)
        ? Drupal.settings.analyzedphenotypes.redirect
        : null;

      if (redirect) {
        $('#ap-main-form-elements-container').children().hide();

        var dMessage = $('#ap-validator-passed');
        dMessage.text('Your file uploaded successfully.');

        var m = 0;
        var t = setInterval(function() {
          m++;

          if (m == 2) {
            dMessage.text('Validating data... Please wait.');
          }

          if (m == 3) {
            location.assign(redirect);
            clearInterval(t);
            return;
          }
        }, 1000);
      }

      $(document)
      //
      .ajaxComplete(function() {
        // When DOM has upload success message.
        if ($('#ap-validator-passed').length && $('#ap-validator-passed').hasClass('ap-data-scope')) {
          // Exclude form fields.
          $('.form-item').hide();

          submitButton.removeClass('form-button-disabled');
          submitButton.removeAttr('disabled');

          // Tell user what's next.
          $('#ap-main-form-fieldset').once(function() {
            $('<span class="text-next-step">&#x25B8; Next Step: Stage 3 - Describe Traits</span>')
              .insertAfter('#ap-next-stage-submit-field');
          });
        }
      });
      //
} }; }(jQuery));
