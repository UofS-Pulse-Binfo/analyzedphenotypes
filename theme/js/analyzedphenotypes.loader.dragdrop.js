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

/*
      // Handle Next Step button.
      var submitButton = $('#ap-next-stage-submit-field');

      // Not successful.
      submitButton.addClass('form-button-disabled');
      submitButton.attr('disabled','disabled'); */


      $(document)
      .ajaxStart(function() {
        // Remove any previous messages or validation result window.
        $('#ap-validation-result-embed').children().remove();

        // Disable form fields.
        $('#ap-project-select-field, #ap-genus-select-field').attr('readonly', 'readonly');
      })
      .ajaxComplete(function() {
        var vr = $('.ap-content-window').clone();
        var em = $('.ap-content-window').next('.messages').clone();

        $('.ap-content-window').next('.messages').remove();
        $('.ap-content-window').remove();

        $(vr).appendTo('#ap-validation-result-embed');
        $(em).appendTo('#ap-validation-result-embed');


        // When DOM has upload success message.
        if ($('div.ap-validator-success').length) {
          $('.form-item').hide();

          submitButton.removeClass('form-button-disabled');
          submitButton.removeAttr('disabled');
          nextStage();
        }

        // Enable form fields.
        $('#ap-project-select-field, #ap-genus-select-field').removeAttr('readonly', 'readonly');

        // Mute any calls to drupal_set_message() when autoloading
        // project genus. No clue why autocompletesearch + AJAX reloads
        // previously posted messages.
        $('#ap-AJAX-wrapper').find('.messages').remove();

        // Reset the select field to always default to the first option.
        $('#ap-genus-select-field')
          .find('option')
          .each(function() {
            $(this).attr('selected', '');
          });
      });


      /**
       * Inform user of the next stage before clicking the next step
       * button in every stage.
       */
      function nextStage() {
        // Need to figure out what is the current stage and next stage.
        // Fron the progress indicator, count the remaining stages
        var countStagesLeft = $('div.ap-progress-stage-todo').length;
        var nextStage = '';

        if (countStagesLeft == 1) {
          // Completed stages 1 and 2.
          nextStage = 'Stage 4 - Save Data';
        }
        else if(countStagesLeft == 2) {
          // Completed stage 1 only.
          nextStage = 'Stage 3 - Describe Traits';
        }
        else if(countStagesLeft == 3) {
          // Completed stage 1 only.
          nextStage = 'Stage 2 - Validate Data';
        }


        $('#ap-main-form-fieldset').once(function() {

          $('<span class="text-next-step">&#x25B8; Next Step: ' + nextStage + '</span>')
           .insertAfter('#ap-next-stage-submit-field');
        });
      }

    }
  };
}(jQuery));
