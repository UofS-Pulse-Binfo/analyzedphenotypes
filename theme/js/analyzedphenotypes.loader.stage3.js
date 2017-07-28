/**
 * @file
 * Contains scripts to manage behaviours used in stage 3.
 */

(function($) {
  Drupal.behaviors.stage03 = {
    attach: function (context, settings) {
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

      // Start JQuery Accordion.
      $('#ap-describe-main-form-container').find('#accordion')
      .accordion({
        active: 0,
        autoHeight: false,
        collapsible: true,
      });

      // Reference ul.
      var listItem = $('.ap-item-list a');

      listItem.click(function(e) {
        e.preventDefault();
        var ontology = $(this).text();

        $(this).closest('div').prev('div')
        .find('.ap-autocomplete-search')
        .val(function() {
          return ontology;
        })
        .focus();
      });


      // Tell user what's next.
      $('#ap-main-form-fieldset').once(function() {
        $('<span class="text-next-step">&#x25B8; Next Step: Stage 4 - Save Data</span>')
          .insertAfter('#ap-next-stage-submit-field');
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

             if (v === '') {
               formset[i] = 0;
             }
           }
         });

         var filledCtr = formset.reduce((a, b) => a + b, 0);
         $('#ap-form-filled-count').text(filledCtr);
       }
      //
} }; }(jQuery));
