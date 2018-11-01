/**
 * @file
 * Contains scripts to manage behaviours used in stage 3.
 */

(function($) {
  Drupal.behaviors.stage03 = {
    attach: function (context, settings) {
      // Start JQuery Accordion.
      $('#ap-describe-main-form-container').find('#accordion')
      .accordion({
        autoHeight: false,
        collapsible: true,
      });

      // Listen for when unit field is altered and
      // provide user to revert value back.
      $('.ap-allow-revert').focusout(function() {
        var id = $(this).attr('id');
        var origUnitId = id.replace('ap-unit-text-field', '#ap-orig-unit-field');
        var origUnit = $(origUnitId).val().trim();

        if (origUnit != $(this).val().trim()) {
          // Offer to revert.
          var x = origUnitId.replace('#ap-orig-unit-field', '#ap-revert');
          $(x).show();
        }
      })
      .focusin(function() {
        $(this).select();
      });

      // Listen to revert link.
      $('.ap-field-inline a').click(function(e) {
        e.preventDefault();
        var id = $(this).parent().attr('id');
        var origUnitId = id.replace('ap-revert', '#ap-orig-unit-field');
        var origUnit = $(origUnitId).val().trim();

        var unitTxtField = origUnitId.replace('#ap-orig-unit-field', '#ap-unit-text-field');
        $(unitTxtField).val(origUnit);

        $(this).parent().hide();
      });

      // Reference ul.
      var listItem = $('.ap-item-list a');

      listItem.click(function(e) {
        e.preventDefault();
        var ontology = $(this).text();

        $(this).closest('div').prev('div')
        .find('.ap-crop-ontology')
        .val(function() {
          return ontology;
        })
        .focus();
      });

      apFormFilledout();

      // Listen to the form and see if they are filled out.
      $('#ap-describe-main-form-container').find('h3')
      .click(function() {
        apFormFilledout();
      });

      $('#ap-describe-main-form-container').find('.ap-required')
      .blur(function() {
        apFormFilledout();
      });

      // Tell user what's next.
      $('#ap-main-form-fieldset').once(function() {
        $('<span class="text-next-step">&#x25B8; Next Step: Stage 4 - Save Data</span>')
          .insertAfter('#ap-next-stage-submit-field');
      });

      // Select field when clicked to ease entry of new value.
      $('.ap-crop-ontology').focusin(function() {
        $(this).select();
      });

      /**
       * Function: check if forms were filled out.
       */
       function apFormFilledout() {
         var formset = [];

         $('.ap-form-describe-trait').each(function(i) {
           formset[i] = 1;
           var reqFld = $(this).find('.ap-required');

           for (var j = 0; j < reqFld.length; j++) {
             var v = reqFld.eq(j).val();

             if (v === '' || v <= -1) {
               formset[i] = 0;
             }
           }
         });

         var filledCtr = formset.reduce((a, b) => a + b, 0);
         $('#ap-form-filled-count').text(filledCtr);
       }
      //
} }; }(jQuery));
