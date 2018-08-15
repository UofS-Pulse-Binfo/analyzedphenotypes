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
        if ($('#ap-reset-traits').val()) {
          $('#ap-traits-field-id input').each(function(){
            $(this).removeAttr('checked');      
          });    
        }      
      });
      
      // YEAR:
      // LOCATION:
      // GERMPLASM TYPE:
      
      // GERMPLASM NAMES:
      
      // Support for germplasm name as additional filter option.
      var suggest = Drupal.settings.analyzedphenotypes.suggest;

      // GERMPLASM NAME FIELD.
      
      // Autocomplete support.
      if (suggest) {
        // All fields with this class name is autocomplete field.
        $( '.ap-textfield-germplasmname' ).autocomplete({
          source: suggest
        });
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
        $('#ap-table-default-headers em, #ap-table-optional-headers em').css('display', r);
      });
      
      
      // COLUMN HEADERS:
      // OPTIONAL HEADERS:
      
      // Adjust button controls.
      apHeadersSetControls();
      apHeadersPreview();

      // Listen for any click events in default headers table for column re-ordering.
      // Second, one row is added to default, it needs to listen for moving it back to optional.
      $('#ap-table-default-headers').once(function() { 
        document.getElementById('ap-table-default-headers')
          .addEventListener('click', function(e) {
            // Read the class from the triggering element.
            var c = e.target.className;
            
            if (c == 'ap-row-up' || c == 'ap-row-down') {
              // Row up or row down, a level at a time.
              var tblRow = $('#' + e.target.id).closest('tr');
            
              // Animate row in reference to highlight it.
              tblRow.fadeOut('fast').fadeIn();
        
              if (c == 'ap-row-up' && tblRow.prev().length) {
                // Move up.
                tblRow.prev()
                  .before(tblRow);      
              }
              else if (c == 'ap-row-down' && tblRow.next().length) {
                // Move down.
                tblRow.next()
                  .after(tblRow);
              }
              
              // Adjust sequence count and button controls.
              apHeadersAdjustSequence();
              apHeadersSetControls();
              apHeadersCache();
              apHeadersPreview();
            }
            else if (e.target.type == 'checkbox' && e.target.value) {
              // Put back option trait that put in to default and now has
              // to go back to optional headers table.
              var tblRow = $('#' + e.target.id).closest('tr');    
              tblRow.find('td').last().removeClass('ap-row-up')
                               .prev().removeClass('ap-row-down');

              var lastRow = $('#ap-table-optional-headers tr').length;
              
              if (lastRow > 0) {
                $('#ap-table-optional-headers tr')
                  .last().after(tblRow)
              }
              else {
                $('#ap-table-optional-headers')
                  .append(tblRow);
              }
              
              // Adjust sequence count and button controls.
              apHeadersAdjustSequence();
              apHeadersSetControls();
              apHeadersCache();
              apHeadersPreview();
            }
          });
      });
      
      // Listen for any click events in optional header for inclusion of other header.
      $('#ap-table-optional-headers').once(function() {
        document.getElementById('ap-table-optional-headers')
          .addEventListener('click', function(e) {
             // Read the element type from the triggering element.
             var c = e.target.type;
             
             if (c == 'checkbox') {
               var tblRow = $('#' + e.target.id).closest('tr');
               tblRow.find('td').last().addClass('ap-row-up')
                                .prev().addClass('ap-row-down');

               // Insert row into the last row in default headers table.
               var lastRow = $('#ap-table-default-headers tr').length;
               $('#ap-table-default-headers tr')
                 .eq(lastRow - 1).after(tblRow);
                 
               // Adjust sequence count and button controls.
               apHeadersAdjustSequence();
               apHeadersSetControls();
               apHeadersCache();
               apHeadersPreview();
             }  
          });      
      });
      
      /**
       * Function manage buttons up and down for each row in headers table.
       * This function ensures that first item allows only to go down and the
       * last item to go up.
       */
      function apHeadersSetControls() {
        var findClass, addClass;
        var rowCount = $('#ap-table-default-headers tr').length;
        
        $('#ap-table-default-headers').find('tr')
          .each(function(i) {
            if (i == 0) {
              findClass = 'ap-row-up';
              addClass  = 'ap-row-up-first';
            }
            else {
              findClass = 'ap-row-up-first';
              addClass  = 'ap-row-up';
            }
            
            $(this).find('.' + findClass)
              .removeClass(findClass)
                 .addClass(addClass);

            if (i == (rowCount - 1)) {
              findClass = 'ap-row-down';
              addClass  = 'ap-row-down-last';
            }
            else {
              findClass = 'ap-row-down-last';
              addClass  = 'ap-row-down';
            }

            $(this).find('.' + findClass)
              .removeClass(findClass)
                 .addClass(addClass);
          });
      }
      
      /**
       * Function to adjust column header counter/sequence order.
       */
      function apHeadersAdjustSequence() {
        $('#ap-table-default-headers td:first-child').each(function(i) {
          $(this).text('#' + (i + 1));
        });
      }
      
      /**
       * Funciton cache options (including order of headers chosen) into a hidden
       * field. This option and order are important in ordering of headers and which
       * header to gather data to in the final exported file.
       */
      function apHeadersCache() {
        var cacheHeaders = [];
        
        $('#ap-table-default-headers td span').each(function() {
          var header = $(this).text();          
          
          if (header.indexOf(',')) {
            // List detected in Trait name. Break this list into pieces.
            var trait = header.split(',');
            for (var j = 0; j < trait.length; j++) {
              if (trait[j] && trait[j] !== 'undefined') {
                cacheHeaders.push(trait[j]);
              }
            }
          }
          else {
            cacheHeaders.push(header);
          }
        });
        
        // Save value.
        $('#ap-columnheaders-field-id').val(cacheHeaders.join('~'));
      }

      // PREVIEW TABLE:
      
      // # Add event listener to preview column headers.
      $('#ap-preview-link').once(function() {
        $(this).click(function(e) { 
        e.preventDefault();
            
        var previewTable = $('#ap-table-preview-headers-container');
        var baseWidth = $('.fieldset-wrapper').width();

        if (previewTable.is(':hidden')) {
          $(this).text('Close preview');
            previewTable.css('width', baseWidth)
              .slideDown();
        }
        else {
          $(this).text('Preview headers');
          previewTable.slideUp();
        }
        });
      });
      
      // Preview headers selected and fill data with random characters.
      function apHeadersPreview() {
        // Access the cached column headers and generate preview table.
        var cacheHeaders = $('#ap-columnheaders-field-id').val().split('~');
        var preview;
        
        var previewTbl = $('#ap-table-preview-headers');
        previewTbl.children().remove();
        
        // Prepare rows. 
        for (var i = 0; i < 4; i ++) {
          previewTbl.append('<tr id="ap-preview-' + (i + 1) + '"></tr>');
        }
        
        // Just five sample rows + a header row.
        for (var i = 0; i < cacheHeaders.length; i++) {
          var content;
          
          for (var j = 0; j < 4; j++) {
            content = (j == 0) 
              ? '<th class="ap-header" title="Column Header #' + (i + 1) + '">' + cacheHeaders[i] + '</th>'
              : '<td class="ap-sample-data">data.' + String.fromCharCode(Math.floor(Math.random() * 26) + 97) + '</td>';
          
            $('#ap-preview-' + (j+1)).append(content);
          }
        }
      }
      
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
          // Enable all form elements after AJAX call.
          formFields.removeAttr('disabled');
          // Exclude the checkboxes in default column headers.
          $('#ap-table-default-headers').find('input').attr('disabled', 'disabled');
          
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