/**
 * @file
 * Manage magnitude of phenotypes behavior
 */
(function ($) {
Drupal.behaviors.MagnitudeOfPhenotypes = {
  attach: function(context, settings) {
  
  $('#magnitude-of-phenotypes-btn-reveal div').click(function() {
    var h, classRem, classAdd;
    var phenotypes = $('#field-magnitude-of-phenotypes-wrapper-phenotypes');
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
}}}(jQuery));