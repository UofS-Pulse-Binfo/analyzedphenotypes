/**
 * @file
 * Contains scripts to manage behaviours used in data downloader.
 */

(function($) {
  Drupal.behaviors.apDataDownloaderBehaviors = {
    attach: function (context, settings) {
      // PROJECT/EXPERIMENT:

      // # Add event listener to experiment - show a short info about data.
      $('#ap-experiment-field-id div')
        .mouseover(function() {
          var fldVal = $(this).find('input').attr('value');
          var pos = $(this).find('label').position();

          $('#ap-info-experiment' + fldVal).show().css({
            'top': pos.top - 40,
            'left' : pos.left
          });
        })
        .mouseleave(function() {
          $('.ap-tooltip').hide();
        });


      // GENUS:
      // SPECIES:

      // TRAITS:

      // Reset traits - when checked, should be unchecked
      // when species field has been altered.
      $(document).ajaxComplete(function() {
        if ($('#ap-reset-species').val()) {
          var rt = $('#ap-reset-species').val();

          if (rt == 1) {
            $('#ap-species-field-id input').each(function(){
              $(this).removeAttr('checked');
            });
          }
        }


        if ($('#ap-reset-traits').val()) {
          var rt = $('#ap-reset-traits').val();

          if (rt == 1) {
            $('#ap-traits-field-id input').each(function(){
              $(this).removeAttr('checked');
            });
          }
        }
      });

      // YEAR:
      // LOCATION:
      // GERMPLASM TYPE:

      // GERMPLASM NAMES:

      // Support for germplasm name as additional filter option.
      if ($('#ap_field_suggest_germplasmnames')) {
        // GERMPLASM NAME FIELD.
        var names = $('#ap_field_suggest_germplasmnames').val();

        // Autocomplete support.
        if (names) {
          // All fields with this class name is autocomplete field.
          $('.ap-textfield-germplasmname').once(function() {
            var suggest = names.split(',');

            $(this).autocomplete({
              source: suggest
            });
          });
        }
      }

      // Expand germplasm name options/checkboxes.
      $('#ap-filter-germplasm-reveal div').click(function(e) {
        var h, classRem, classAdd;
        var germFound = $(this).parent().prev('div');
        // Default height of the container.
        var minHeight = 40;

        if (germFound.height() == minHeight) {
          h = germFound[0].scrollHeight;
          classRem = 'on';
          classAdd = 'off';
        }
        else {
          h = minHeight + 'px';
          classRem = 'off';
          classAdd = 'on';
        }

        germFound.css('max-height', h);

        $(this)
          .removeClass('ap-filter-germplasm-reveal-' + classRem)
          .addClass('ap-filter-germplasm-reveal-' + classAdd);
      });

      // MAXIMUM ALLOWED MISSING DATA:

      /* Enable field on the front end.
      // Insert % symbol for user and auto select value previously entered
      // to ease updating of value.
      $('#ap-missingdata-field-id').change(function() {
        $(this).val(function(i, v) {
          if (v.trim().length > 0 && v.match(/[0-9]/)) {
            return v.replace(/[^0-9]/g, '') + '%';
          }
        });
      })
      .focusin(function() {
        $(this).select();
      });
      */

      // FILE TYPE:

      // Notify user that data might be converted to a data in MS Excel.
      $('#ap-filetype-field-id').change(function(e) {
        if (e.target.id == 'ap-filetype-field-id') {
          var warn = $('#ap-filetype-warning div');

          if ($(this).val() != 'tsv') {
            warn.slideDown('fast');
          }
          else {
            warn.slideUp('fast');
          }
        }
      });

      // AVERAGE REPLICATES PER SITE YEAR:

      // R COLUMN HEADERS:

      // Show beneath each header an R Friendly version.

      $('#ap-rfriendly-field-id').click(function(e) {
        var r = ($(this).attr('checked')) ? 'block' : 'none';
        $('#ap-table-default-headers em').css('display', r);
      });


      // AJAX REQUEST:
      // Reference submit button.
      var btnSubmit = $('#ap-download-submit-field');
      // Reference input and select elements.
      var formFields = $('#ap-AJAX-wrapper-main input, #ap-AJAX-wrapper-main select');

      // Add listerner to all checkboxes to trigger a message to please wait while AJAX.
      $('input[type=checkbox]').once(function() {
        $(this).click(function() {
          if (!$(this).hasClass('ap-skip-loading')) {
            $(this).closest('.fieldset-wrapper')
              .prepend('<div id="ap-ajax-wait">Loading options... Please wait.</div>');
          }
        });
      });

      $(document)
        .ajaxStart(function() {
          // Disable all form elements while AJAX is busy.
          formFields.attr('disabled', 'disabled');

          if (!btnSubmit.hasClass('form-button-disabled')) {
            btnSubmit.addClass('form-button-disabled')
              .attr('disabled', 'disabled');
          }
        })
        .ajaxComplete(function() {
          // Show R friendly version when option is true.
          if ($('#ap-rfriendly-field-id').attr('checked')) {
            $('#ap-table-default-headers em').css('display', 'block');
          }

          // Enable all form elements after AJAX call.
          formFields.removeAttr('disabled');

          var submitOk = Drupal.settings.analyzedphenotypes.btn;

          if (submitOk) {
            // Minimum requirements met - a rpoject and trait selected, user can start
            // generate file.
            btnSubmit
              .removeClass('form-button-disabled')
              .removeAttr('disabled');
          }
          else {
            // Otherwise - keep submit button disabled.
            btnSubmit
              .addClass('form-button-disabled')
              .attr('disabled', 'disabled');
          }

          $('#ap-ajax-wait').remove();
        });

} }; }(jQuery));
