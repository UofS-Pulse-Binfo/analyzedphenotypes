/**
 * @file
 * Contains scripts to manage behaviours used in Drag and Drop.
 */

(function($) {
  Drupal.behaviors.apDragAndDrop = {
    attach: function (context, settings) {
      $(document)
      .ajaxStart(function() {
        // Remove any previous messages or validation result window.
        $('#ap-validation-result-embed').children().remove();
      })
      .ajaxComplete(function() {
        var vr = $('.ap-content-window').clone();
        var em = $('.ap-content-window').next('.messages').clone();

        $('.ap-content-window').next('.messages').remove();
        $('.ap-content-window').remove();

        $(vr).appendTo('#ap-validation-result-embed');
        $(em).appendTo('#ap-validation-result-embed');

        // Mute any calls to drupal_set_message() when autoloading
        // project genus. No clue why autocompletesearch + AJAX reloads
        // previously posted messages.
        $('#ap-AJAX-wrapper').find('.messages').remove();
      });
    }
  };
}(jQuery));
