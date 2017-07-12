/**
 * @file
 * Contains scripts to manage behaviours used in Drag and Drop.
 */

(function($) {
  Drupal.behaviors.apDragAndDrop = {
    attach: function (context, settings) {
      // Main container.
      var dndId = '#ap-dnd-container';

      // Active dropzone.
      var settings = Drupal.settings.analyzedphenotypes.vars;
      var dndField = 'droppable-' + settings['source'];

      // DND validation Result window.
      var vrEmbedId = '#ap-validation-result-embed';

      // Customize:
      // Replace browse button to link.
      $(dndId).find('a').text('choose a file')
        .click(function() {
          $(vrEmbedId).children().remove();
        });

      // Event listeners:
      // When user drags or drops a file.
      var dropZone = document.getElementById(dndField);
      var dropMessage = $('.droppable-message');

      if (dropZone) {
        dropZone.addEventListener('dragover', function() {
          dropMessage.css('border', '2px dashed #AAAAAA');
        });

        dropZone.addEventListener('dragleave', function() {
          dropMessage.css('border', 'none');
        });

        dropZone.addEventListener('drop', function() {
          $(vrEmbedId).children().remove();
          dropMessage.css('border', 'none');
        });
      }

      // Reposition the validation result.
      if ($(dndId).has('.ap-result-panel').length > 0) {
        var vrResult = $('.ap-result-panel');
        var vrResultClone = vrResult.clone();

        vrResult.prev('.messages').remove();
        vrResult.next('.messages').remove();

        vrResult.remove();

        $(vrResultClone).appendTo(vrEmbedId);
      }

      // Suppress any alert.
      alerts = function() {
        // no alert.
      }

} }; }(jQuery));
