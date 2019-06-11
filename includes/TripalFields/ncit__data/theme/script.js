/**
 * @file
 * Manage NCIT Data field behavior
 */
(function ($) {
Drupal.behaviors.NCITData = {
  attach: function(context, settings) {
  // Select value for quick edit.
  $('#ap-germplasm-search-autocomplete-textfield')
    .click(function() {
      $(this).select();
    })
    .focusout(function() {
      if (!$(this).val()) {
        $('#ap-germplasm-search-ajax-wrapper')
          .slideUp().empty();
      }
    });

  // Attach listener to reveal/show less button.
  var wrapper = {};
  wrapper.main = '#ap-germplasm-search-summary-table';
  wrapper.btn  = '#ap-germplasm-search-button-reveal';

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
      .removeClass('ap-germplasm-search-table-row-reveal-' + classRem)
      .addClass('ap-germplasm-search-table-row-reveal-' + classAdd);
  });


  // Summary information tool tip.
  $('#ap-germplasm-search-summary-overview-list ul li').mouseover(function() {
    var pos = $(this).position();

    $(this).find('.ap-germplasm-search-tooltip').show().css({
      'top': pos.top + 40,
      'left' : pos.left
    });
  })
  .mouseleave(function() {
    $('.ap-germplasm-search-tooltip').hide();
  });


  // Clear Search.
  var clear
  if (clear = $('#ap-germplasm-search-summary-overview-list a:last-child')) {
     clear.click(function(e) {
       e.preventDefault();
       $('#ap-germplasm-search-ajax-wrapper')
          .slideUp().empty();
       $('#ap-germplasm-search-autocomplete-textfield').val('').focus();
     });
  }
}}}(jQuery));
