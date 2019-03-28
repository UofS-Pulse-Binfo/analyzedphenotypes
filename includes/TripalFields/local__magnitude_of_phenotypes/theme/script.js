/**
 * @file
 * Manage magnitude of phenotypes behavior
 */
(function ($) {
Drupal.behaviors.MagnitudeOfPhenotypes = {
  attach: function(context, settings) {
  
  // Wrapper.
  var wrapper = {};
  wrapper.id  = '#field-magnitude-of-phenotypes-wrapper-phenotypes';
  wrapper.btn   = '#magnitude-of-phenotypes-btn-reveal';
  wrapper.listId  = '#list-magnitude-of-phenotypes-phenotypes';
  wrapper.minHeight = 220;
  
  var listHeight = 0;

  // Phenotypes pane launched.
  $('a').click(function() {
    if ($(wrapper.id) && $(wrapper.id).is(':visible')) {
      listHeight = $(wrapper.listId).height();
      btnReveal();
    }
  });

  //
  if ($(wrapper.id)) {
    // List of phenotypes enabled.
    $(window).resize(function() { 
      listHeight = $(wrapper.listId).height();
      btnReveal();
    });


    // Attach listener to reveal/show less button.
    $(wrapper.btn + ' div').click(function() {
      var h, classRem, classAdd;
      var phenotypes = $(wrapper.id);
      // Default height of the container.
      var minHeight = 250;
 
      if (phenotypes.height() == minHeight) {
        h = phenotypes[0].scrollHeight;
        classRem = 'on';
        classAdd = 'off';
      }
      else {
        h = minHeight + 'px';
        classRem = 'off';
        classAdd = 'on';
      }

      phenotypes.css('max-height', h);

      $(this)
        .removeClass('magnitude-of-phenotypes-phenotypes-reveal-' + classRem)
        .addClass('magnitude-of-phenotypes-phenotypes-reveal-' + classAdd);
    });
    
    
    /**
     * Function toggle reveal button.
     */
    function btnReveal() {
      var btn = $(wrapper.btn);
      
      if (listHeight > wrapper.minHeight) {
        // Overflow detected.
        btn.show();
      } else {
        // All shown.
        btn.hide();
      }
    }
  }
}}}(jQuery));