/**
 * @file
 * Contains scripts to manage behaviours used in data downloader.
 */

(function($) {
  Drupal.behaviors.apDataDownloaderBehaviors = {
    attach: function (context, settings) {
      /////
      // Main AJAX wrapper.
      var mainAJAXWrapper = $('#ap-AJAX-wrapper-main');

      // Flag if submit should be enabled or disabled.
      var submitOk = Drupal.settings.analyzedphenotypes.btn;
      var submitBtn = $('#ap-download-submit-field');

      // All fields.
      var formFields = $('#ap-AJAX-wrapper-main input, #ap-AJAX-wrapper-main select');

      //
      // Call to AJAX.
      $(document)
        .ajaxStart(function() {
          // Disable options.
          formFields.attr('disabled', 'disabled');

          if (!submitBtn.hasClass('form-button-disabled')) {
            submitBtn
              .addClass('form-button-disabled')
              .attr('disabled', 'disabled');
          }
        })
        .ajaxComplete(function() {
          // Enable options.
          formFields.removeAttr('disabled', 'disabled');
          $('#ap-table-default-headers').find('input').attr('disabled', 'disabled');

          // Manage submit button.
          if (submitOk) {
            // Minimum requirements met - a project and trait selected.
            submitBtn
              .removeClass('form-button-disabled')
              .removeAttr('disabled');
          }
          else {
            // Otherwise, keep the button disabled.
            submitBtn
              .addClass('form-button-disabled')
              .attr('disabled', 'disabled');
          }

          $('#ap-ajax-wait').remove();
          apAutofieldControls();
        });


        apColumnPickControls();
        apPreviewCols();

        //
        // The following will ensure that Drupal will not execute
        // AJAX request more than once.
        mainAJAXWrapper.once(function() {
          // # Add event listener to checkboxes.
          $(this).find('input[type=checkbox]').click(function() {
            // Tell user to wait while AJAX is updating options.
            // but skip those fields that do not require this.
            if (!$(this).hasClass('ap-skip-loading')) {
              $(this).closest('.fieldset-wrapper')
                .prepend('<div id="ap-ajax-wait">Loading options... Please wait.</div>');
            }
          });

          // # Add event listener to selectboxes.
          $(this).find('select').change(function(e) {
            // Tell user to wait while AJAX is updating options.
            // but skip fields that dow not require this.
            if (e.target.id == 'ap-filetype-field-id') {
              var warn = $('#ap-filetype-warning div');

              if ($(this).val() != 'tsv') {
                warn.slideDown('fast');
              }
              else {
                warn.slideUp('fast');
              }
            }

            // Additionally, when user selects xlsx as the file type,
            // notify user that some values get altered by Excel.
            if (!$(this).hasClass('ap-skip-loading')) {
              $(this).closest('.fieldset-wrapper')
                .prepend('<div id="ap-ajax-wait">Loading options... Please wait.</div>');
            }
          });

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

          // # Maximize and minimize list of germplasm found.
          $('#ap-filter-germplasm-reveal div').click(function() {
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
          }),

          // # Add event listener to missing data field.
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

          // # Add event listener to R Friendly option.
          $('#ap-rfriendly-field-id').click(function(e) {
            // Update column header table to show R version of headers.
            apColumnPickControls();
          });

          // # Add event listener to preview column headers.
          $('#ap-preview-link').click(function(e) {
            e.preventDefault();
            var previewTable = $('#ap-table-preview-headers-container');

            if (previewTable.is(':hidden')) {
              $(this).text('Close preview');
              previewTable.slideDown();
            }
            else {
              $(this).text('Preview headers');
              previewTable.slideUp();
            }
          });

          // # Add event listener to button in column header picker.
          document.getElementById('ap-AJAX-wrapper-main')
            .addEventListener('click', function(e) {
              var c = e.target.className;

              if (c == 'ap-row-up' || c == 'ap-row-down' || c == 'ap-column' || c.indexOf('ap-optional-traits') >= 0) {
                var parentTR = $('#' + e.target.id).closest('tr');

                if (c.indexOf('ap-optional-traits') >= 0) {
                  // Adding or removing optional trait.
                  var add_rem = ($('#' + e.target.id).attr('checked')) ? 'add' : 'rem';
                  apColumnPickCol(parentTR, add_rem);
                }
                else {
                  // Move row one level up or down.
                  var up_down = (c == 'ap-row-down') ? 'down' : 'up';
                  apColumnPickColMove(parentTR, up_down);
                }

                // Manage control buttons.
                apColumnPickControls();
                // Prepare preview and cache values.
                apPreviewCols();
              }
            });
        });

        //
        // Auto complete field - add additional field as a filter to germplasm.
        var germplasm = $('#ap-germplasm-found-cache');

        if (germplasm.val()) {
          var autofield = 'ap-autofield-field';
          var matched;

          var apAJAX = {
            on : 'ap-autofield-AJAX-on',
            off: 'ap-autofield-AJAX-off'
          };

          // When list found.
          document.addEventListener('keyup', function(e) {
            // Suggest germplasm.
            if (e.target) {
              var fldClass = e.target.classList;

              if (fldClass.contains(autofield)) {
                // User is typing in the filter germplasm field.
                var germplasmField   = {};
                  germplasmField.id  = e.target.id;
                  germplasmField.obj = $('#' + germplasmField.id);
                  germplasmField.val = germplasmField.obj.val();
                  germplasmField.div = germplasmField.obj.parent();

                // Clear previous suggestions.
                germplasmField.div.find('ul').parent().remove();

                // Begin autosuggest.
                if (germplasmField.val == '') {
                  // Nothing to search.
                  germplasmField.div
                    .removeClass(apAJAX.on)
                    .addClass(apAJAX.off);
                }
                else {
                  // Something to crunch.
                  germplasmField.div
                    .removeClass(apAJAX.off)
                    .addClass(apAJAX.on);

                  if (germplasmField.val.length > 1) {
                    var g = germplasm.val().split('~');
                    matched = [];

                    // Values from other field.
                    var fldOther = $('.' + autofield).map(function() {
                      return $(this).val().trim();
                    }).get();

                    g.forEach(function(v) {
                      // Auto suggest but do not suggest what has been entered already.
                      if (v.toLowerCase().indexOf(germplasmField.val.toLowerCase()) > -1) {
                        if (fldOther.length < 2 || $.inArray(v, fldOther) == -1) {
                          matched.push('<li>' + v + '</li>');
                        }
                      }
                    });

                    var suggest = (matched.length) ? matched.join('') : '<li>Not found</li>';

                    germplasmField.div
                      .append('<div><ul>' + suggest + '</ul></div>')
                      // AJAX Off.
                      .removeClass(apAJAX.on)
                         .addClass(apAJAX.off)
                      // # Add event listener to suggested items.
                      .find('li')
                        .click(function() {
                          var v = $(this).text();

                          germplasmField.obj.val(function() {
                            return (v == 'Not found') ? 'Type Germplasm/Stock Name' : v.trim();
                          }).select();

                          apAutofieldControls();
                          apAutofieldCache();

                          $(this).closest('div').fadeOut(300, function() {
                            $(this).remove();
                          });
                        });
                  }
                }
              }
            }
          });


          // Select the content of field when field gets selected.
         $('.' + autofield)
            // Field selected.
            .focusin(function() {
              $(this).select();
            })
            // Abandoned field.
            .focusout(function() {
              var fld = $(this);

              if (fld.val() == '') {
                fld.val('Type Germplasm/Stock Name');
              }
            });
        }

        // Select newly added field.
        apAutofieldSelect();
        apAutofieldControls();

        //
        // Functions:

        // Autofield:
        /*
         * Select text/value of newly added field.
         * Ref: .ap-autofield - main container for autofield fields.
         */
        function apAutofieldSelect() {
          var x = 0;
          $('#ap-autofield span a:last-child').click(function() {
            x++;
            $(document).ajaxStop(function() {
              if (x > 0) {
                $('.' + autofield).eq(0).select();
                x = 0;
              }
            });
          });
        }

        /*
         * Enable and disable delete or add button of a newly added field.
         * Ref: .ap-autofield-field - class for each autofield field.
         *      germplasm - hidden field of all germplasm found given a filter set.
         */
        function apAutofieldControls() {
          var fld = $('.ap-autofield-field');
          var btn = { rem : 0, add: 1 };

          fld.each(function(i, element) {

            // Add and delete buttons set for this field.
            var btnSet = $(this).next('span').find('a');

            // Reset the the buttons.
            btnSet.show();

            if(germplasm.val() == null) {
              btnSet.hide();
            }
            else if (i == 0 && fld.length < 2) {
              btnSet.eq(btn.rem).hide();
            }
            else if (i > 0) {
              btnSet.eq(btn.add).hide();
            }

            // No more add button.
            if (fld.length >= germplasm.val().split('~').length) {
              btnSet.eq(btn.add).hide();
            }
          });
        }

        /*
         * Save field values.
         * Ref: #ap-autofield-cahce - field hidden used to save values entered.
         *      .ap-autofield-field - class for each autofield field.
         */
        function apAutofieldCache() {
          var fld = $('#ap-autofield-cache');

          fld.val(function() {
            return $('.ap-autofield-field').map(function() {
              var v = $(this).val().trim();

              if (v != 'Type Germplasm/Stock Name' && v != '') {
                return v;
              }
            }).get().join('~');
          });
        }


        // Column header picker.

        /*
         * Return count of table rows in default headers table.
         */
        function apColumnPickColCount(set) {
          var t = (set == 'default') ? '#ap-table-default-headers' : '#ap-table-optional-headers';
          return $(t).find('tr').length;
        }

        /*
         * Add or remove optional column headers to default headers table.
         * @param thisRow - current row to add or remove.
         * @param cmd - command, operation to perform (add, remove) to row;
         *
         * Ref: #ap-table-default-headers and #ap-table-optional-headers - default and option headers table.
         */
        function apColumnPickCol(thisRow, cmd) {
          var rowData = thisRow.find('td');
          var tdClass = ['ap-seq-no', 'ap-column', 'ap-row-up', 'ap-row-down'];

          // Add class.
          rowData.each(function(i, element) {
            if (cmd == 'add') {
              rowData.eq(i).addClass(tdClass[i]);
            }
            else {
              rowData.eq(i).removeClass(tdClass[i]);
              if (i == 0) {
                rowData.eq(i).text('+');
              }
            }
          });

          // Remove or add:
          if (cmd == 'add') {
            // Add at the end of the list.
            var lastRow = apColumnPickColCount('default');

            $('#ap-table-default-headers')
              .find('tr').eq(lastRow - 1).after(thisRow);
          }
          else {
            var lastRow = apColumnPickColCount('optional');

            if (lastRow > 0) {
              $('#ap-table-optional-headers tr')
                .last().after(thisRow);
            }
            else {
              $('#ap-table-optional-headers')
                .append(thisRow);
            }
          }
        }

        /*
         * Move row one level up or down.
         * @param thisRow - current row to move.
         * @param cmd - command, operation to perform (move up or down) to row;
         */
        function apColumnPickColMove(thisRow, cmd) {
          thisRow.fadeOut('fast').fadeIn();

          if (cmd == 'up') {
            if (thisRow.prev().length) {
              thisRow.prev().before(thisRow);
            }
          }
          else {
            if (thisRow.next().length) {
              thisRow.next().after(thisRow);
            }
          }
        }

        /*
         * Add control buttons, rfriendly text and row count.
         */
        function apColumnPickControls() {
          var rowCount = apColumnPickColCount('default');
          var findClass, addClass;

          // Show R Friendly headers.
          var r = ($('#ap-rfriendly-field-id').attr('checked')) ? 'block' : 'none';
          $('#ap-table-optional-headers em').css('display', 'none');

          $('#ap-table-default-headers').find('tr')
            .each(function(i) {
              // R Friendly.
              $(this).find('em').css('display', r);
              // Sequence Number.
              $(this).find('.ap-seq-no').text('#' + (i +1));
              // Manage control buttons.
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

          // Finallly, mark optional header table when no option available.
          $('#ap-table-optional-headers').append(function() {
            if ($(this).find('tr').length <= 0) {
              return '<tr class="ap-no-options"><td>0 Optional Headers</td></tr>';
            }
            else {
              if ($('tr.ap-no-options')) {
                $('tr.ap-no-options').remove();
              }
            }
          });
        }


        // Preview column header.


        /*
         * Construct table to preview column headers selected.
         */
        function apPreviewCols() {
          var previewTbl = $('#ap-table-preview-headers');
          previewTbl.children().remove();
          var id = 'ap-preview';
          var header = new Array();

          for (var i = 0; i < 4; i ++) {
            previewTbl.append('<tr id="' + id + '-' + (i + 1) + '"></tr>');
          }

          $('#ap-table-default-headers').find('.ap-column')
            .each(function(i) {
              var txt = $(this).html().split('&nbsp;-');
              var row = txt[1].replace(/\<.*/, '').trim();

              if (row.indexOf(',') >= 0) {
                // List detected.
                var k = row.split(',');

                for (var j = 0; j < k.length; j++) {
                  if (k[j] !== 'undefined') {
                    header.push(k[j]);
                  }
                }
              }
              else {
                header.push(row);
              }
            });

            // Output headers horizontally in a table header.
            for (var l = 0; l < header.length; l++) {
              var content;

              for (var m = 0; m < 4; m++) {
                content = (m == 0)
                  ?  content = '<th class="ap-header">' + header[l] + '</th>'
                  :  content = '<td class="ap-sample-data">data' + String.fromCharCode(Math.floor(Math.random() * 26) + 97) + '</td>';

                $('#' + id + '-' + (m + 1)).append(content);
              }
            }

            // Cache values.
            $('#ap-columnheaders-field-id').val(header.join('~'));
          }
} }; }(jQuery));
