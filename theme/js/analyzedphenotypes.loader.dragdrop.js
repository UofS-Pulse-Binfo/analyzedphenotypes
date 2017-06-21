/**
 * @file
 * Contains scripts to manage behaviours used in Drag and Drop.
 */

(function($) {
  Drupal.behaviors.apDragAndDrop = {
    attach: function (context, settings) {
      // Add event listener to project autocomplete search field.
      $('#ap-project-select-field').click(function() {
        $(this).select();
      });

      // Handle events when user drags a file to drop zone.
      // Reference main container of drag and drop.
      var dnd = 'ap-dnd-field-upload';

      // Replace button value from browse to choose your file.
      $('#' + dnd + ' .droppable-browse-button').text('choose your file');

      // Drop area element.
	    var dropZone = document.getElementById('droppable-ap-dnd-field');
	    // Initial/default meassage of the drop area.
	    var dropMessage = $('.droppable-message');

      if (dropZone) {
	      // User drags file into the drop area.
		    dropZone.addEventListener('dragover', function() {
		      dropMessage.css('border','2px dashed #AAAAAA');
	  	  });

      	// User cancels file drop.
		    dropZone.addEventListener('dragleave', function() {
		      dropMessage.css('border','none');
		    });
      }


      // Handle Next Step button.
      var submitButton = $('#ap-next-stage-submit-field');
      // Message to flag form stage that upload was successful.
      var successMsg = $('#rawpheno-upload-successful');

      if (successMsg.length) {
        // Form stage does contain a successful message.
        submitButton.removeClass('form-button-disabled');
        submitButton.removeAttr('disabled');
      }
      else {
        // Not successful.
        submitButton.addClass('form-button-disabled');
        submitButton.attr('disabled','disabled');
      }


      $(document)
      .ajaxComplete(function() {
        // Mute any calls to drupal_set_message() when autoloading
        // project genus. No clue why autocompletesearch + AJAX reloads
        // previously posted messages.
        $('#ap-AJAX-container').find('.messages').remove();

        // Reset the select field to always default to the first option.
        $('#ap-genus-select-field')
          .find('option')
          .each(function() {
            $(this).attr('selected', '');
          });


      });

    }
  };
}(jQuery));
