/**
 * @file
 * Manage NCIT Data field behavior
 */
(function ($) {
Drupal.behaviors.NCITData = {
  attach: function(context, settings) {
  // Auto select field or content of autocomplete field.
  $('#ap-search-autocomplete-textfield').focus();
  $('#ap-search-autocomplete-textfield').click(function() {
    $(this).select();
  });

  var wrapper = {};
  wrapper.main = '#ap-ncitdata-table-summary-wrapper';
  wrapper.btn  = '#ap-ncitdata-btn-reveal';

  // Attach listener to reveal/show less button.
  $(wrapper.btn + ' div').click(function() {
    var h, classRem, classAdd;
    var main = $(wrapper.main);

    // Default height of the table container.
    var minHeight = 300;

    if (main.height() == minHeight) {
      h = main[0].scrollHeight;
      classRem = 'on';
      classAdd = 'off';
    }
    else {
      h = minHeight + 'px';
      classRem = 'off';
      classAdd = 'on';
    }

    main.css('max-height', h);

    $(this)
      .removeClass('ap-ncitdata-table-row-reveal-' + classRem)
      .addClass('ap-ncitdata-table-row-reveal-' + classAdd);
  });


  // Summary information tool tip.
  $('#ap-summary-overview-list ul li').mouseover(function() {
    var pos = $(this).position();

    $(this).find('.ap-tooltip').show().css({
      'top': pos.top + 40,
      'left' : pos.left
    });
  })
  .mouseleave(function() {
    $('.ap-tooltip').hide();
  });
}}}(jQuery));
